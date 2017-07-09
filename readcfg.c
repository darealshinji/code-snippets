/***
A simple function to read values
from a text file like this:

//commentA
#commentB
label:My Label
width:1920
height:1080
contrast:1.3
***/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* delimiter used in the config file */
#define DELIM  ":"

/* maximum size in bytes to accept a config file */
#define MAX_SIZE  1024*1024

#define STRINGIFY(x)   STRINGIFY_(x)
#define STRINGIFY_(x)  #x

/* initialize [const char] var and its default value [const char] def */
#define INIT_CHAR(var, def) \
  static const char *var = NULL; \
  static const char *var##Def = def; \
  static const char *var##Left = STRINGIFY(var) DELIM;

/* initialize [double] var and its default value [const double] def */
#define INIT_DOUBLE(var, def) \
  static double var = 0; \
  static const double var##Def = def; \
  static int var##Set = 0; \
  static const char *var##Left = STRINGIFY(var) DELIM;

/* set [const char] var if the config line declares it */
#define LINE_HAS_CHAR(var) \
  if (strncmp(line, var##Left, len=strlen(var##Left)) == 0) { \
    var = strdup(line + len); \
  }

/* set [double] var if the config line declares it */
#define LINE_HAS_DOUBLE(var) \
  if (strncmp(line, var##Left, len=strlen(var##Left)) == 0) { \
    var = atof(line + len); \
    var##Set = 1; \
  }

/* set [const char] var to default if var is empty */
#define CHECK_CHAR_DEFAULT(var) \
  if (var == NULL || strcmp(var, "") == 0) { \
    var = var##Def; \
  }

/* set [double] var to default if (var check) returns true */
#define CHECK_DOUBLE_DEFAULT(var, check) \
  if (var##Set == 0 || (var##Set == 1 && var check)) { \
    var = var##Def; \
  }

/* set [double] var to default if (var checkA) returns true
 * or (var checkB) returns true */
#define CHECK_DOUBLE_DEFAULT_OR(var, checkA, checkB) \
  if (var##Set == 0 || (var##Set == 1 && (var checkA || var checkB))) { \
    var = var##Def; \
  }

/* how to set the required macros INIT_VALUES, SCAN_LINES and CHECK_DEFAULT_VALUES */
// #define INIT_VALUES  <init_macro> <init_macro> ...
// #define SCAN_LINES  <line_macro> else <line_macro> else ... <line_macro>
// #define CHECK_DEFAULT_VALUES  <check_macro> <check_macro> ...



/******************** config *********************/
#define INIT_VALUES                               \
  INIT_CHAR   (label,    "This is a label")       \
  INIT_DOUBLE (width,    1280)                    \
  INIT_DOUBLE (height,   720)                     \
  INIT_DOUBLE (contrast, 1.0)

#define SCAN_LINES \
  LINE_HAS_CHAR        (label)                    \
  else LINE_HAS_DOUBLE (width)                    \
  else LINE_HAS_DOUBLE (height)                   \
  else LINE_HAS_DOUBLE (contrast)

#define CHECK_DEFAULT_VALUES                      \
  CHECK_CHAR_DEFAULT      (label)                 \
  CHECK_DOUBLE_DEFAULT    (width, <10)            \
  CHECK_DOUBLE_DEFAULT    (height, <10)           \
  CHECK_DOUBLE_DEFAULT_OR (contrast, <0.0, >=2.0)
/*************************************************/



/* error codes */
enum {
  READCFG_OK = 0,    /* nothing went wrong */
  READCFG_OPEN_FILE, /* cannot open file for reading */
  READCFG_SEEK_EOF,  /* cannot seek to end of file */
  READCFG_MAX_SIZE   /* file size exceeds MAX_SIZE */
};

/* initialize global values */
INIT_VALUES;

int readcfg(const char *file)
{
  FILE *fp;
  int rv = READCFG_OK;
  char line[255];

  /* open file for reading */
  if ((fp = fopen(file, "r")) == NULL)
  {
    rv = READCFG_OPEN_FILE;
  }

  /* seek to the end so we can find out the file size */
  if (rv == READCFG_OK && fseek(fp, 0L, SEEK_END) == -1)
  {
    fclose(fp);
    rv = READCFG_SEEK_EOF;
  }

  /* reject the file if it's too big */
  if (rv == READCFG_OK && ftell(fp) > MAX_SIZE)
  {
    fclose(fp);
    rv = READCFG_MAX_SIZE;
  }

  if (rv == READCFG_OK)
  {
    /* go back to the beginning of the file */
    rewind(fp);

    /* check each line until the EOF is reached */
    while (fgets(line, sizeof(line), fp) != NULL)
    {
      size_t len = strlen(line);

      /* minimum size to set a value is 4: x=1\n
       * ignore lines beginning with // or # */
      if (len >= 4 && (line[0] != '#' && (line[0] != '/' && line[1] != '/')))
      {
        char last1 = line[len - 1];
        char last2 = line[len - 2];

        /* remove newline characters:
         * \r\n = Windows; \n = OSX/Unix; \r = Classic Mac */
        if (last2 == '\r' && last1 == '\n')
        {
          line[len - 2] = '\0';
          line[len - 1] = '\0';
        }
        else if (last1 == '\n' || last1 == '\r')
        {
          line[len - 1] = '\0';
        }
        len = strlen(line);

        /* split at first delimiter found and get value */
        SCAN_LINES;
      }
    }

    /* close file */
    fclose(fp);
  }

  /* set default values if needed */
  CHECK_DEFAULT_VALUES;

  return rv;
}

int main(void)
{
  int rv = readcfg("test.cfg");

  /* do something with the aquired values, i.e. print on screen */
  printf("label" DELIM "%s\n", label);
  printf("width" DELIM "%d\n", (int)width); //cast to int
  printf("height" DELIM "%d\n", (int)height); //cast to int
  printf("contrast" DELIM "%f\n", contrast);

  return rv;
}

