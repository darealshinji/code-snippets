/*
 * xsel -- manipulate the X selection (a very stripped down version)
 * Copyright (C) 2001 Conrad Parker <conrad@vergenet.net>
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <setjmp.h>
#include <signal.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

/* Maximum line length for error messages */
#define MAXLINE 4096

/* An instance of a MULTIPLE SelectionRequest being served */
typedef struct _MultTrack MultTrack;

struct _MultTrack {
  MultTrack * mparent;
  Display * display;
  Window requestor;
  Atom property;
  Atom selection;
  Time time;
  Atom * atoms;
  unsigned long length;
  unsigned long index;
  unsigned char * sel;
};

/* Selection serving states */
typedef enum {
  S_NULL=0,
  S_INCR_1,
  S_INCR_2
} IncrState;

/* An instance of a selection being served */
typedef struct _IncrTrack IncrTrack;

struct _IncrTrack {
  MultTrack * mparent;
  IncrTrack * prev, * next;
  IncrState state;
  Display * display;
  Window requestor;
  Atom property;
  Atom selection;
  Time time;
  Atom target;
  int format;
  unsigned char * data;
  int nelements; /* total */
  int offset, chunk, max_elements; /* all in terms of nelements */
};

/* Status of request handling */
typedef int HandleResult;
#define HANDLE_OK         0
#define HANDLE_ERR        (1<<0)
#define HANDLE_INCOMPLETE (1<<1)
#define DID_DELETE        (1<<2)

/* The name we were invoked as (argv[0]) */
static char * progname;

/* Our X Display and Window */
static Display * display;
static Window window;

/* Maxmimum request size supported by this X server */
static long max_req;

/* Our timestamp for all operations */
static Time timestamp;

static Atom timestamp_atom; /* The TIMESTAMP atom */
static Atom multiple_atom; /* The MULTIPLE atom */
static Atom targets_atom; /* The TARGETS atom */
static Atom delete_atom; /* The DELETE atom */
static Atom incr_atom; /* The INCR atom */
static Atom null_atom; /* The NULL atom */
static Atom text_atom; /* The TEXT atom */
static Atom utf8_atom; /* The UTF8 atom */

/* Number of selection targets served by this.
 * (MULTIPLE, INCR, TARGETS, TIMESTAMP, DELETE, TEXT, UTF8_STRING and STRING)
 * NB. We do not currently serve COMPOUND_TEXT; we can retrieve it but do not
 * perform charset conversion.
 */
#define MAX_NUM_TARGETS 9
static int NUM_TARGETS;
static Atom supported_targets[MAX_NUM_TARGETS];

/*
 * exit_err (fmt)
 *
 * Print a formatted error message and errno information to stderr,
 * then exit with return code 1.
 */
static void
exit_err (const char * fmt, ...)
{
  va_list ap;
  int errno_save;
  char buf[MAXLINE];
  int n;

  errno_save = errno;

  va_start (ap, fmt);

  snprintf (buf, MAXLINE, "%s: ", progname);
  n = strlen (buf);

  vsnprintf (buf+n, MAXLINE-n, fmt, ap);
  n = strlen (buf);

  snprintf (buf+n, MAXLINE-n, ": %s\n", strerror (errno_save));

  fflush (stdout); /* in case stdout and stderr are the same */
  fputs (buf, stderr);
  fflush (NULL);

  va_end (ap);
  exit (1);
}

/*
 * xs_malloc (size)
 *
 * Malloc wrapper. Always returns a successful allocation. Exits if the
 * allocation didn't succeed.
 */
static void *
xs_malloc (size_t size)
{
  void * ret;

  if (size == 0) size = 1;
  if ((ret = malloc (size)) == NULL) {
    exit_err ("malloc error");
  }

  return ret;
}

/*
 * xs_strlen (s)
 *
 * strlen wrapper for unsigned char *
 */
#define xs_strlen(s) (strlen ((const char *) s))

static sigset_t exit_sigs;

/*
 * become_daemon ()
 *
 * Perform the required procedure to become a daemon process, as
 * outlined in the Unix programming FAQ:
 * http://www.steve.org.uk/Reference/Unix/faq_2.html#SEC16
 * and open a logfile.
 */
static void
become_daemon (void)
{
  pid_t pid;
  int null_r_fd, null_w_fd;

  if ((pid = fork()) == -1) {
    exit_err ("error forking");
  } else if (pid > 0) {
    _exit (0);
  }

  if (setsid () == -1) {
    exit_err ("setsid error");
  }

  if ((pid = fork()) == -1) {
    exit_err ("error forking");
  } else if (pid > 0) {
    _exit (0);
  }

  umask (0);

  null_r_fd = open ("/dev/null", O_RDONLY);
  if (null_r_fd == -1) {
    exit_err ("error opening /dev/null for reading");
  }
  if (dup2 (null_r_fd, 0) == -1) {
    exit_err ("error duplicating /dev/null on stdin");
  }

  /* dup2 /dev/null on stdout */
  null_w_fd = open ("/dev/null", O_WRONLY|O_APPEND);
  if (null_w_fd == -1) {
    exit_err ("error opening /dev/null for writing");
  }
  if (dup2 (null_w_fd, 1) == -1) {
    exit_err ("error duplicating /dev/null on stdout");
  }
}

/*
 * SELECTION RETRIEVAL
 * ===================
 *
 * The following functions implement retrieval of an X selection,
 * optionally within a user-specified timeout.
 *
 *
 * Selection timeout handling.
 * ---------------------------
 *
 * The selection retrieval can time out if no response is received within
 * a user-specified time limit. In order to ensure we time the entire
 * selection retrieval, we use an interval timer and catch SIGALRM.
 * [Calling select() on the XConnectionNumber would only provide a timeout
 * to the first XEvent.]
 */

/* Forward declaration of refuse_all_incr () */
static void
refuse_all_incr (void);

/*
 * handle_x_errors ()
 *
 * XError handler.
 */
static int
handle_x_errors (Display * display, XErrorEvent * eev)
{
  char err_buf[MAXLINE];

  /* Make sure to send a refusal to all waiting INCR requests
   * and delete the corresponding properties. */
  if (eev->error_code == BadAlloc) refuse_all_incr ();

  XGetErrorText (display, eev->error_code, err_buf, MAXLINE);
  exit_err (err_buf);

  return 0;
}

/*
 * own_selection (selection)
 *
 * Requests ownership of the X selection. Returns True if ownership was
 * granted, and False otherwise.
 */
static Bool
own_selection (Atom selection)
{
  Window owner;

  XSetSelectionOwner (display, selection, window, timestamp);
  /* XGetSelectionOwner does a round trip to the X server, so there is
   * no need to call XSync here. */
  owner = XGetSelectionOwner (display, selection);
  if (owner != window) {
    return False;
  } else {
    XSetErrorHandler (handle_x_errors);
    return True;
  }
}


static IncrTrack * incrtrack_list = NULL;

/*
 * add_incrtrack (it)
 *
 * Add 'it' to the head of incrtrack_list.
 */
static void
add_incrtrack (IncrTrack * it)
{
  if (incrtrack_list) {
    incrtrack_list->prev = it;
  }
  it->prev = NULL;
  it->next = incrtrack_list;
  incrtrack_list = it;
}

/*
 * remove_incrtrack (it)
 *
 * Remove 'it' from incrtrack_list.
 */
static void
remove_incrtrack (IncrTrack * it)
{
  if (it->prev) {
    it->prev->next = it->next;
  }
  if (it->next) {
    it->next->prev = it->prev;
  }

  if (incrtrack_list == it) {
    incrtrack_list = it->next;
  }
}

/*
 * fresh_incrtrack ()
 *
 * Create a new incrtrack, and add it to incrtrack_list.
 */
static IncrTrack *
fresh_incrtrack (void)
{
  IncrTrack * it;

  it = xs_malloc (sizeof (IncrTrack));
  add_incrtrack (it);

  return it;
}

/*
 * trash_incrtrack (it)
 *
 * Remove 'it' from incrtrack_list, and free it.
 */
static void
trash_incrtrack (IncrTrack * it)
{
  remove_incrtrack (it);
  free (it);
}

/*
 * find_incrtrack (atom)
 *
 * Find the IncrTrack structure within incrtrack_list pertaining to 'atom',
 * if it exists.
 */
static IncrTrack *
find_incrtrack (Atom atom)
{
  IncrTrack * iti;

  for (iti = incrtrack_list; iti; iti = iti->next) {
    if (atom == iti->property) return iti;
  }

  return NULL;
}

/* Forward declaration of handle_multiple() */
static HandleResult
handle_multiple (Display * display, Window requestor, Atom property,
                 unsigned char * sel, Atom selection, Time time,
                 MultTrack * mparent);

/* Forward declaration of process_multiple() */
static HandleResult
process_multiple (MultTrack * mt, Bool do_parent);

/*
 * confirm_incr (it)
 *
 * Confirm the selection request of ITER tracked by 'it'.
 */
static void
notify_incr (IncrTrack * it, HandleResult hr)
{
  XSelectionEvent ev;

  /* Call XSync here to make sure any BadAlloc errors are caught before
   * confirming the conversion. */
  XSync (it->display, False);

  /* Prepare a SelectionNotify event to send, placing the selection in the
   * requested property. */
  ev.type = SelectionNotify;
  ev.display = it->display;
  ev.requestor = it->requestor;
  ev.selection = it->selection;
  ev.time = it->time;
  ev.target = it->target;

  if (hr & HANDLE_ERR) ev.property = None;
  else ev.property = it->property;

  XSendEvent (display, ev.requestor, False,
              (unsigned long)NULL, (XEvent *)&ev);
}

/*
 * refuse_all_incr ()
 *
 * Refuse all INCR transfers in progress. ASSUMES that this is called in
 * response to an error, and that the program is about to bail out;
 * ie. incr_track is not cleaned out.
 */
static void
refuse_all_incr (void)
{
  IncrTrack * it;

  for (it = incrtrack_list; it; it = it->next) {
    XDeleteProperty (it->display, it->requestor, it->property);
    notify_incr (it, HANDLE_ERR);
    /* Don't bother trashing and list-removing these; we are about to
     * bail out anyway. */
  }
}

/*
 * complete_incr (it)
 *
 * Finish off an INCR retrieval. If it was part of a multiple, continue
 * that; otherwise, send confirmation that this completed.
 */
static void
complete_incr (IncrTrack * it, HandleResult hr)
{
  MultTrack * mparent = it->mparent;

  if (mparent) {
    trash_incrtrack (it);
    process_multiple (mparent, True);
  } else {
    notify_incr (it, hr);
    trash_incrtrack (it);
  }
}

/*
 * notify_multiple (mt, hr)
 *
 * Confirm the selection request initiated with MULTIPLE tracked by 'mt'.
 */
static void
notify_multiple (MultTrack * mt, HandleResult hr)
{
  XSelectionEvent ev;

  /* Call XSync here to make sure any BadAlloc errors are caught before
   * confirming the conversion. */
  XSync (mt->display, False);

  /* Prepare a SelectionNotify event to send, placing the selection in the
   * requested property. */
  ev.type = SelectionNotify;
  ev.display = mt->display;
  ev.requestor = mt->requestor;
  ev.selection = mt->selection;
  ev.time = mt->time;
  ev.target = multiple_atom;

  if (hr & HANDLE_ERR) ev.property = None;
  else ev.property = mt->property;

  XSendEvent (display, ev.requestor, False,
              (unsigned long)NULL, (XEvent *)&ev);
}

/*
 * complete_multiple (mt, do_parent, hr)
 *
 * Complete a MULTIPLE transfer. Iterate to its parent MULTIPLE if
 * 'do_parent' is true. If there is not parent MULTIPLE, send notification
 * of its completion with status 'hr'.
 */
static void
complete_multiple (MultTrack * mt, Bool do_parent, HandleResult hr)
{
  MultTrack * mparent = mt->mparent;

  if (mparent) {
    free (mt);
    if (do_parent) process_multiple (mparent, True);
  } else {
    notify_multiple (mt, hr);
    free (mt);
  }
}

/*
 * change_property (display, requestor, property, target, format, mode,
 *                  data, nelements)
 *
 * Wrapper to XChangeProperty that performs INCR transfer if required and
 * returns status of entire transfer.
 */
static HandleResult
change_property (Display * display, Window requestor, Atom property,
                 Atom target, int format, int mode,
                 unsigned char * data, int nelements,
                 Atom selection, Time time, MultTrack * mparent)
{
  XSelectionEvent ev;
  long nr_bytes;
  IncrTrack * it;

  nr_bytes = nelements * format / 8;

  if (nr_bytes <= max_req) {
    XChangeProperty (display, requestor, property, target, format, mode,
                     data, nelements);

    return HANDLE_OK;
  }

  /* Send a SelectionNotify event */
  ev.type = SelectionNotify;
  ev.display = display;
  ev.requestor = requestor;
  ev.selection = selection;
  ev.time = time;
  ev.target = target;
  ev.property = property;

  XSelectInput (ev.display, ev.requestor, PropertyChangeMask);

  XChangeProperty (ev.display, ev.requestor, ev.property, incr_atom, 32,
                   PropModeReplace, (unsigned char *)&nr_bytes, 1);

  XSendEvent (display, requestor, False,
              (unsigned long)NULL, (XEvent *)&ev);

  /* Set up the IncrTrack to track this */
  it = fresh_incrtrack ();

  it->mparent = mparent;
  it->state = S_INCR_1;
  it->display = display;
  it->requestor = requestor;
  it->property = property;
  it->selection = selection;
  it->time = time;
  it->target = target;
  it->format = format;
  it->data = data;
  it->nelements = nelements;
  it->offset = 0;

  /* Maximum nr. of elements that can be transferred in one go */
  it->max_elements = max_req * 8 / format;

  /* Nr. of elements to transfer in this instance */
  it->chunk = MIN (it->max_elements, it->nelements - it->offset);

  return HANDLE_INCOMPLETE;
}

static HandleResult
incr_stage_1 (IncrTrack * it)
{
  /* First pass: PropModeReplace, from data, size chunk */
  XChangeProperty (it->display, it->requestor, it->property, it->target,
                   it->format, PropModeReplace, it->data, it->chunk);

  it->offset += it->chunk;

  it->state = S_INCR_2;

  return HANDLE_INCOMPLETE;
}

static HandleResult
incr_stage_2 (IncrTrack * it)
{
  it->chunk = MIN (it->max_elements, it->nelements - it->offset);

  if (it->chunk <= 0) {

    /* Now write zero-length data to the property */
    XChangeProperty (it->display, it->requestor, it->property, it->target,
                     it->format, PropModeAppend, NULL, 0);
    it->state = S_NULL;
    return HANDLE_OK;
  } else {
    XChangeProperty (it->display, it->requestor, it->property, it->target,
                     it->format, PropModeAppend, it->data+it->offset,
                     it->chunk);
    it->offset += it->chunk;
    return HANDLE_INCOMPLETE;
  }
}


/*
 * handle_timestamp (display, requestor, property)
 *
 * Handle a TIMESTAMP request.
 */
static HandleResult
handle_timestamp (Display * display, Window requestor, Atom property,
                  Atom selection, Time time, MultTrack * mparent)
{
  return
    change_property (display, requestor, property, XA_INTEGER, 32,
                     PropModeReplace, (unsigned char *)&timestamp, 1,
                     selection, time, mparent);
}

/*
 * handle_targets (display, requestor, property)
 *
 * Handle a TARGETS request.
 */
static HandleResult
handle_targets (Display * display, Window requestor, Atom property,
                Atom selection, Time time, MultTrack * mparent)
{
  Atom * targets_cpy;
  HandleResult r;

  targets_cpy = malloc (sizeof (supported_targets));
  memcpy (targets_cpy, supported_targets, sizeof (supported_targets));

  r = change_property (display, requestor, property, XA_ATOM, 32,
                     PropModeReplace, (unsigned char *)targets_cpy,
                     NUM_TARGETS, selection, time, mparent);
  free(targets_cpy);
  return r;
}

/*
 * handle_string (display, requestor, property, sel)
 *
 * Handle a STRING request; setting 'sel' as the data
 */
static HandleResult
handle_string (Display * display, Window requestor, Atom property,
               unsigned char * sel, Atom selection, Time time,
               MultTrack * mparent)
{
  return
    change_property (display, requestor, property, XA_STRING, 8,
                     PropModeReplace, sel, xs_strlen(sel),
                     selection, time, mparent);
}

/*
 * handle_utf8_string (display, requestor, property, sel)
 *
 * Handle a UTF8_STRING request; setting 'sel' as the data
 */
static HandleResult
handle_utf8_string (Display * display, Window requestor, Atom property,
                    unsigned char * sel, Atom selection, Time time,
                    MultTrack * mparent)
{
  return
    change_property (display, requestor, property, utf8_atom, 8,
                     PropModeReplace, sel, xs_strlen(sel),
                     selection, time, mparent);
}

/*
 * handle_delete (display, requestor, property)
 *
 * Handle a DELETE request.
 */
static HandleResult
handle_delete (Display * display, Window requestor, Atom property)
{
  XChangeProperty (display, requestor, property, null_atom, 0,
                   PropModeReplace, NULL, 0);

  return DID_DELETE;
}

/*
 * process_multiple (mt, do_parent)
 *
 * Iterate through a MultTrack until it completes, or until one of its
 * entries initiates an interated selection.
 *
 * If 'do_parent' is true, and the actions proscribed in 'mt' are
 * completed during the course of this call, then process_multiple
 * is iteratively called on mt->mparent.
 */
static HandleResult
process_multiple (MultTrack * mt, Bool do_parent)
{
  HandleResult retval = HANDLE_OK;
  unsigned long i;

  if (!mt) return retval;

  for (; mt->index < mt->length; mt->index += 2) {
    i = mt->index;
    if (mt->atoms[i] == timestamp_atom) {
      retval |= handle_timestamp (mt->display, mt->requestor, mt->atoms[i+1],
                                  mt->selection, mt->time, mt);
    } else if (mt->atoms[i] == targets_atom) {
      retval |= handle_targets (mt->display, mt->requestor, mt->atoms[i+1],
                                mt->selection, mt->time, mt);
    } else if (mt->atoms[i] == multiple_atom) {
      retval |= handle_multiple (mt->display, mt->requestor, mt->atoms[i+1],
                                 mt->sel, mt->selection, mt->time, mt);
    } else if (mt->atoms[i] == XA_STRING || mt->atoms[i] == text_atom) {
      retval |= handle_string (mt->display, mt->requestor, mt->atoms[i+1],
                               mt->sel, mt->selection, mt->time, mt);
    } else if (mt->atoms[i] == utf8_atom) {
      retval |= handle_utf8_string (mt->display, mt->requestor, mt->atoms[i+1],
                                    mt->sel, mt->selection, mt->time, mt);
    } else if (mt->atoms[i] == delete_atom) {
      retval |= handle_delete (mt->display, mt->requestor, mt->atoms[i+1]);
    } else if (mt->atoms[i] == None) {
      /* the only other thing we know to handle is None, for which we
       * do nothing. This block is, like, __so__ redundant. Welcome to
       * Over-engineering 101 :) This comment is just here to keep the
       * logic documented and separate from the 'else' block. */
    } else {
      /* for anything we don't know how to handle, we fail the conversion
       * by setting this: */
      mt->atoms[i] = None;
    }

    /* If any of the conversions failed, signify this by setting that
     * atom to None ...*/
    if (retval & HANDLE_ERR) {
      mt->atoms[i] = None;
    }
    /* ... but don't propogate HANDLE_ERR */
    retval &= (~HANDLE_ERR);

    if (retval & HANDLE_INCOMPLETE) break;
  }

  if ((retval & HANDLE_INCOMPLETE) == 0) {
    complete_multiple (mt, do_parent, retval);
  }

  return retval;
}

/*
 * continue_incr (it)
 *
 * Continue an incremental transfer of IncrTrack * it.
 *
 * NB. If the incremental transfer was part of a multiple request, this
 * function calls process_multiple with do_parent=True because it is
 * assumed we are continuing an interrupted ITER, thus we must continue
 * the multiple as its original handler did not complete.
 */
static HandleResult
continue_incr (IncrTrack * it)
{
  HandleResult retval = HANDLE_OK;

  if (it->state == S_INCR_1) {
    retval = incr_stage_1 (it);
  } else if (it->state == S_INCR_2) {
    retval = incr_stage_2 (it);
  }

  /* If that completed the INCR, deal with completion */
  if ((retval & HANDLE_INCOMPLETE) == 0) {
    complete_incr (it, retval);
  }

  return retval;
}

/*
 * handle_multiple (display, requestor, property, sel, selection, time)
 *
 * Handle a MULTIPLE request; possibly setting 'sel' if any STRING
 * requests are processed within it. Return value has DID_DELETE bit set
 * if any delete requests are processed.
 *
 * NB. This calls process_multiple with do_parent=False because it is
 * assumed we are "handling" the multiple request on behalf of a
 * multiple already in progress, or (more likely) directly off a
 * SelectionRequest event.
 */
static HandleResult
handle_multiple (Display * display, Window requestor, Atom property,
                 unsigned char * sel, Atom selection, Time time,
                 MultTrack * mparent)
{
  MultTrack * mt;
  int format;
  unsigned long bytesafter;
  HandleResult retval = HANDLE_OK;

  mt = xs_malloc (sizeof (MultTrack));

  XGetWindowProperty (display, requestor, property, 0L, 1000000,
                      False, (Atom)AnyPropertyType, &mt->property,
                      &format, &mt->length, &bytesafter,
                      (unsigned char **)&mt->atoms);

  /* Make sure we got the Atom list we want */
  if (format != 32) return HANDLE_OK;


  mt->mparent = mparent;
  mt->display = display;
  mt->requestor = requestor;
  mt->sel = sel;
  mt->selection = selection;
  mt->time = time;
  mt->index = 0;

  retval = process_multiple (mt, False);

  return retval;
}

/*
 * handle_selection_request (event, sel)
 *
 * Processes a SelectionRequest event 'event' and replies to its
 * sender appropriately, eg. with the contents of the string 'sel'.
 * Returns False if a DELETE request is processed, indicating to
 * the calling function to delete the corresponding selection.
 * Returns True otherwise.
 */
static Bool
handle_selection_request (XEvent event, unsigned char * sel)
{
  XSelectionRequestEvent * xsr = &event.xselectionrequest;
  XSelectionEvent ev;
  HandleResult hr = HANDLE_OK;
  Bool retval = True;

  /* Prepare a SelectionNotify event to send, either as confirmation of
   * placing the selection in the requested property, or as notification
   * that this could not be performed. */
  ev.type = SelectionNotify;
  ev.display = xsr->display;
  ev.requestor = xsr->requestor;
  ev.selection = xsr->selection;
  ev.time = xsr->time;
  ev.target = xsr->target;

  if (xsr->property == None && ev.target != multiple_atom) {
      /* Obsolete requestor */
      xsr->property = xsr->target;
  }

  if (ev.time != CurrentTime && ev.time < timestamp) {
    /* If the time is outside the period we have owned the selection,
     * which is any time later than timestamp, or if the requested target
     * is not a string, then refuse the SelectionRequest. NB. Some broken
     * clients don't set a valid timestamp, so we have to check against
     * CurrentTime here. */
    ev.property = None;
  } else if (ev.target == timestamp_atom) {
    /* Return timestamp used to acquire ownership if target is TIMESTAMP */
    ev.property = xsr->property;
    hr = handle_timestamp (ev.display, ev.requestor, ev.property,
                           ev.selection, ev.time, NULL);
  } else if (ev.target == targets_atom) {
    /* Return a list of supported targets (TARGETS)*/
    ev.property = xsr->property;
    hr = handle_targets (ev.display, ev.requestor, ev.property,
                         ev.selection, ev.time, NULL);
  } else if (ev.target == multiple_atom) {
    if (xsr->property == None) { /* Invalid MULTIPLE request */
      ev.property = None;
    } else {
      /* Handle MULTIPLE request */
      hr = handle_multiple (ev.display, ev.requestor, ev.property, sel,
                            ev.selection, ev.time, NULL);
    }
  } else if (ev.target == XA_STRING || ev.target == text_atom) {
    /* Received STRING or TEXT request */
    ev.property = xsr->property;
    hr = handle_string (ev.display, ev.requestor, ev.property, sel,
                        ev.selection, ev.time, NULL);
  } else if (ev.target == utf8_atom) {
    /* Received UTF8_STRING request */
    ev.property = xsr->property;
    hr = handle_utf8_string (ev.display, ev.requestor, ev.property, sel,
                             ev.selection, ev.time, NULL);
  } else if (ev.target == delete_atom) {
    /* Received DELETE request */
    ev.property = xsr->property;
    hr = handle_delete (ev.display, ev.requestor, ev.property);
    retval = False;
  } else {
    /* Cannot convert to requested target. This includes most non-string
     * datatypes, and INSERT_SELECTION, INSERT_PROPERTY */
    ev.property = None;
  }

  /* Return False if a DELETE was processed */
  retval = (hr & DID_DELETE) ? False : True;

  /* If there was an error in the transfer, it should be refused */
  if (hr & HANDLE_ERR) {
    ev.property = None;
  }

  if ((hr & HANDLE_INCOMPLETE) == 0) {
    XSendEvent (display, ev.requestor, False,
                (unsigned long)NULL, (XEvent *)&ev);

    /* If we return False here, we may quit immediately, so sync out the
     * X queue. */
    if (!retval) XSync (display, False);
  }

  return retval;
}

/*
 * set_selection (selection, sel)
 *
 * Takes ownership of the selection 'selection', then loops waiting for
 * its SelectionClear or SelectionRequest events.
 *
 * Handles SelectionRequest events, first checking for additional
 * input if the user has specified 'follow' mode. Returns when a
 * SelectionClear event is received for the specified selection.
 */
static void
set_selection (Atom selection, unsigned char * sel)
{
  XEvent event;
  IncrTrack * it;

  if (own_selection (selection) == False) return;

  for (;;) {
    /* Flush before unblocking signals so we send replies before exiting */
    XFlush (display);
    sigprocmask (SIG_UNBLOCK, &exit_sigs, NULL);
    XNextEvent (display, &event);
    sigprocmask (SIG_BLOCK, &exit_sigs, NULL);

    switch (event.type) {
    case SelectionClear:
      if (event.xselectionclear.selection == selection) return;
      break;
    case SelectionRequest:
      if (event.xselectionrequest.selection != selection) break;

      if (!handle_selection_request (event, sel)) return;

      break;
    case PropertyNotify:
      if (event.xproperty.state != PropertyDelete) break;

      it = find_incrtrack (event.xproperty.atom);

      if (it != NULL) {
        continue_incr (it);
      }

      break;
    default:
      break;
    }
  }
}

int
xsel_main(int argc, char *argv[])
{
  char * display_name = NULL;
  unsigned char * sel = NULL;

  progname = argv[0];

  if (argc != 2) {
    return 1;
  }

  sel = (unsigned char *)argv[1];

  display = XOpenDisplay (display_name);
  if (display==NULL) {
    exit_err ("Can't open display: %s\n", display_name ? display_name : "(null)");
  }

  /* Create an unmapped window for receiving events */
  int black = BlackPixel (display, DefaultScreen (display));
  window = XCreateSimpleWindow (display, XDefaultRootWindow (display), 0, 0, 1, 1, 0, black, black);

  /* Get the maximum incremental selection size in bytes */
  /*max_req = MAX_SELECTION_INCR (display);*/
  max_req = 4000;

  Atom selection = XInternAtom (display, "CLIPBOARD", False);

  if (sel == NULL || sel[0] == '\0') {
    /* clear selection */
    XSetSelectionOwner (display, selection, None, timestamp);
    /* Call XSync to ensure this operation completes before program
     * termination, especially if this is all we are doing. */
    XSync (display, False);
    return 1;
  }

  become_daemon ();

  set_selection (selection, sel);

  return 0;
}
