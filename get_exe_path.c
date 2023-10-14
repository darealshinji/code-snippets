#if defined(_WIN32)

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>

#elif defined(__APPLE__)

#include <mach-o/dyld.h>
#include <sys/syslimits.h>

#elif defined(__BSD__)

#include <limits.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <unistd.h>

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/* return allocated full path to executable or NULL on error */
char *get_exe_path()
{
    char *rp = NULL;

#if defined(__linux__) || defined(__CYGWIN__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__))

    rp = realpath("/proc/self/exe", NULL);

#elif defined(__sun)

    const char *p = getexecname();

    if (p) {
        rp = strdup(p);
    }

#elif defined(_WIN32)

    size_t len;
    wchar_t wbuf[32*1024];
    const wchar_t *wptr = wbuf;

    if (GetModuleFileNameW(NULL, wbuf, sizeof(wbuf)-1) > 0 &&
        (len = wcstombs(NULL, wptr, 0)) != (size_t) -1)
    {
        rp = (char *)calloc(len + 1, 1);

        if (rp && wcstombs(rp, wptr, len) == (size_t) -1) {
            free(rp);
            return NULL;
        }
    }

#elif defined(__APPLE__)

    char buf[MAXPATHLEN+1];
    char result[MAXPATHLEN+1];
    uint32_t buflen = MAXPATHLEN;

    if (_NSGetExecutablePath(buf, &buflen) == 0 && realpath(buf, result) != NULL) {
        rp = strdup(result);
    }

#elif defined(__BSD__) && defined(KERN_PROC_PATHNAME)

#if defined(__NetBSD__)
    int name[4] = {CTL_KERN, KERN_PROC_ARGS, getpid(), KERN_PROC_PATHNAME};
#else
    int name[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
#endif

    char buf[MAXPATHLEN+1];
    size_t buflen = MAXPATHLEN;

    if (sysctl(name, 4, buf, &buflen, NULL, 0) == 0) {
        rp = realpath(buf, NULL);
    }

#endif

    return rp;
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
