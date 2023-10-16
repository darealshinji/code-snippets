#include <stdio.h>
#include <stdlib.h>

/* https://github.com/gpakosz/whereami */
#ifndef HAS_PROC_SELF_EXE
#include "whereami.c"
#endif


/* return allocated full path to executable or NULL on error */
char *get_exe_path()
{
#ifdef HAS_PROC_SELF_EXE
    /* quick solution on Linux */
    return realpath("/proc/self/exe", NULL);
#else
    char *path = NULL;
    int length, dirname_length;

    length = wai_getExecutablePath(NULL, 0, &dirname_length);

    if (length < 1) {
        return NULL;
    }

    path = (char *)malloc(length + 1);
    if (!path) abort();

    wai_getExecutablePath(path, length, &dirname_length);
    path[length] = '\0';

    return path;
#endif
}


int main()
{
    char *path = get_exe_path();

    if (path) {
        printf("%s\n", path);
        free(path);
    }

    return 0;
}
