#if defined(__WIN32__)
    #include <Windows.h>
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
    #include <sys/syslimits.h>
#elif defined(__BSD__)
    #include <limits.h>
    #include <sys/param.h>
    #include <sys/sysctl.h>
    #include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>

// tested for Linux (glibc), FreeBSD and Windows (MinGW compiler + Wine)


char *get_exe_path()
{
    char *rp = NULL;

#if defined(__linux__) || defined(__CYGWIN__)

    rp = realpath("/proc/self/exe", NULL);

#elif defined(__sun)

    const char *p = getexecname();

    if (p) {
        rp = strdup(p);
    }

#elif defined(__WIN32__)

    size_t n = 0;
    char buf[32*1024];
    wchar_t wbuf[32*1024];

    if (GetModuleFileNameW(NULL, wbuf, (32*1024)-1) > 0 &&
        wcstombs_s(&n, (char *)&buf, 32*1024, (const wchar_t *)&wbuf, (32*1024)-1) == 0)
    {
        rp = strdup(buf);
    }

#elif defined(__APPLE__)

    char buf[MAXPATHLEN+1];
    char result[MAXPATHLEN+1];
    uint32_t buflen = MAXPATHLEN;

    if (_NSGetExecutablePath(buf, &buflen) == 0 && realpath(buf, result)) {
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
