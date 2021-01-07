#if !defined(_GNU_SOURCE) && defined(__GNUC__)
# define _GNU_SOURCE
#endif

/* strdup() */
#if defined(_XOPEN_SOURCE) && _XOPEN_SOURCE < 500
# undef _XOPEN_SOURCE
#endif
#ifndef _XOPEN_SOURCE
# define _XOPEN_SOURCE 500
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __cplusplus
# define CAST(t,x)    reinterpret_cast<t>(x)
#else
# ifdef __GNUC__
#  define CAST(t,x)   x
# else
#  define CAST(t,x)  (t)x
# endif
#endif


/* resolve '.' and '..' but don't follow symlinks
 * based on https://www.geeksforgeeks.org/simplify-directory-path-unix-like/
 *
 * returns allocated string on success and NULL on error
 */
char *simplify_directory_path(const char *in)
{
  char *buf, *p, *ptr;
  char **stack;
  size_t len, len2, nElem, nSt, i;

  if (!in || (len = strlen(in)) == 0) {
    return NULL;
  }

  /* quick handling of '/' and '.' */
  if (len == 1) {
    if (in[0] == '/') {
      return strdup("/");
    } else if (in[0] == '.') {
#ifdef _GNU_SOURCE
      return get_current_dir_name();
#else
      char tmp[4096];
      ptr = getcwd(tmp, sizeof(tmp) - 1);
      return ptr ? strdup(ptr) : NULL;
#endif
    }
  }

  if (in[0] == '/') {
    /* absolute path */
    p = strdup(in);

    /* remove trailing slashes */
    while (len > 0 && p[len - 1] == '/') {
      p[len - 1] = 0;
      len--;
    }

    if (p[0] == 0) {
      /* only slashes */
      free(p);
      return strdup("/");
    }

    ptr = strrchr(p, '/');

    /* check if it ends on '/.' or '/..' or
     * if there are '/./', '/../', or '//' entries */
    if (strcmp(ptr, "/.") != 0 && strcmp(ptr, "/..") != 0 &&
        !strstr(p, "//") && !strstr(p, "/./") && !strstr(p, "/../"))
    {
      return CAST( char *, realloc(p, len) );
    }
  } else {
    /* relative path */

#ifdef _GNU_SOURCE
    ptr = get_current_dir_name();
#else
    char tmp[4096];
    ptr = getcwd(tmp, sizeof(tmp) - 1);
#endif

    if (!ptr) {
      return NULL;
    }

    /* prepend "<cdn>/" to relative path */
    len = strlen(ptr);
    p = CAST( char *, malloc(len + strlen(in) + 2) );
    strcpy(p, ptr);
    p[len] = '/';
    strcpy(p + len + 1, in);
    len = strlen(p);
#ifdef _GNU_SOURCE
    free(ptr);
#endif
  }

  /* count elements */
  for (i = nElem = 0; i < len; ++i) {
    char found = 0;

    while (p[i] == '/') {
      i++;
    }

    if (p[i] == '.') {
      if (p[i + 1] == '/') {
        /* skip '.' entry */
        i++;
        continue;
      } else if (p[i + 1] == 0) {
        /* last element is '.' */
        break;
      }
    }

    while (i < len && p[i] != '/') {
      i++;
      found = 1;
    }

    if (found == 1) {
      nElem++;
    }
  }

  /* path consisted only of slahes and maybe "."
   * elements, so we can already resolve it to "/" */
  if (nElem == 0) {
    free(p);
    return strdup("/");
  }

  buf = CAST( char *, malloc(len + 1) );
  stack = CAST( char **, malloc(nElem * sizeof(char *)) );

  /* initialze all stack entries with NULL */
  for (i = 0; i < nElem; ++i) {
    stack[i] = NULL;
  }

  for (i=len2=nSt = 0; i < len; ++i) {
    ptr = buf;

    /* clear buffer */
    memset(buf, 0, len);

    /* skip all '/' */
    while (p[i] == '/') {
      i++;
    }

    /* copy element into buffer */
    while (i < len && p[i] != '/') {
      *ptr = p[i];
      ptr++;
      i++;
    }

    if (strcmp(buf, "..") == 0) {
      /* if element was ".." and stack is not empty,
       * pop the last element, otherwise ignore */
      if (stack[0] != NULL) {
        free(stack[nSt]);
        stack[nSt] = NULL;

        if (nSt > 0) {
          nSt--;
        }
      }
    }
    else if (strcmp(buf, ".") == 0) {
      /* ignore "." entries */
      continue;
    }
    else if (strlen(buf) != 0) {
      /* push back */
      stack[nSt] = strdup(buf);
      len2 += strlen(buf) + 1;

      if (++nSt >= nElem) {
        break;
      }
    }
  }

  free(p);
  free(buf);

  /* path was resolved from i.e. "/usr/../" to "/" */
  if (nSt == 0) {
    return strdup("/");
  }

  /* copy elements to buffer and free() the stack */
  buf = CAST( char *, malloc(len2 + 1) );
  buf[0] = 0;
  ptr = buf;

  for (i = 0; i < nSt; ++i) {
    *ptr = '/';
    strcpy(ptr+1, stack[i]);
    free(stack[i]);

    while (*ptr) {
      ptr++;
    }
  }

  free(stack);

  return buf;
}

int main()
{
  const char *path = "//./usr/local/../bin//./";
  char *path2 = simplify_directory_path(path);

  if (path2) {
    printf("%s  ==>  %s\n", path, path2);
    free(path2);
  } else {
    printf("error\n");
  }

  return 0;
}

