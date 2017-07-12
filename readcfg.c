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


/******************** config *********************/

/* INIT_XX ( variable, defaultValue ) */
#define CFG_INIT \
  INIT_CH(  label,       "This is a label"   ) \
  INIT_DB(  width,       1280                ) \
  INIT_DB(  height,      720                 ) \
  INIT_DB(  contrast,    1.0                 )

/* SCAN_XX ( variable )
 * SCAN_END */
#define CFG_SCAN \
  SCAN_CH(  label     ) \
  SCAN_DB(  width     ) \
  SCAN_DB(  height    ) \
  SCAN_DB(  contrast  ) \
  SCAN_END

/* DFLT_CH ( variable )
 * DFLT_DB ( variable, condition )
 * DFLT_DB_OR ( variable, condition, condition ) */
#define CFG_DEFAULT \
  DFLT_CH   (  label                        ) \
  DFLT_DB   (  width,       <10             ) \
  DFLT_DB   (  height,      <10             ) \
  DFLT_DB_OR(  contrast,    <0.0, >=2.0     )

/*************************************************/


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
#define INIT_CH(var, def) \
  static const char *var = NULL; \
  static const char *var##Def = def; \
  static const char *var##Left = STRINGIFY(var) DELIM;

/* initialize [double] var and its default value [const double] def */
#define INIT_DB(var, def) \
  static double var = 0; \
  static const double var##Def = def; \
  static int var##Set = 0; \
  static const char *var##Left = STRINGIFY(var) DELIM;

/* set [const char] var if the config line declares it */
#define SCAN_CH(var) \
  if (strncmp(line, var##Left, len=strlen(var##Left)) == 0) { \
    var = strdup(line + len); \
  } else

/* set [double] var if the config line declares it */
#define SCAN_DB(var) \
  if (strncmp(line, var##Left, len=strlen(var##Left)) == 0) { \
    var = atof(line + len); \
    var##Set = 1; \
  } else

/* append this to close a trailing "else" with brackets */
#define SCAN_END  { }

/* set [const char] var to default if var is empty */
#define DFLT_CH(var) \
  if (var == NULL || strlen(var) == 0) { \
    var = var##Def; \
  }

/* set [double] var to default if (var check) returns true */
#define DFLT_DB(var, check) \
  if (var##Set == 0 || (var check)) { \
    var = var##Def; \
  }

/* set [double] var to default if (var checkA) returns true
 * or (var checkB) returns true */
#define DFLT_DB_OR(var, checkA, checkB) \
  if (var##Set == 0 || ((var checkA) || (var checkB))) { \
    var = var##Def; \
  }

/* error codes */
enum {
  READCFG_OK = 0,    /* nothing went wrong */
  READCFG_OPEN_FILE, /* cannot open file for reading */
  READCFG_SEEK_EOF,  /* cannot seek to end of file */
  READCFG_MAX_SIZE   /* file size exceeds MAX_SIZE */
};

/* initialize global values */
CFG_INIT

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
        CFG_SCAN
      }
    }

    /* close file */
    fclose(fp);
  }

  /* set default values if needed */
  CFG_DEFAULT

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

