#include <errno.h>
#include <ftw.h>
#include <stdio.h>
#include <unistd.h>


int main()
{
    const char *path = "/tmp/delete-me-now";

    auto lambda = [] (const char *fpath, const struct stat *, int, struct FTW *) -> int {
        return remove(fpath);
    };

    if (nftw(path, lambda, 20, FTW_DEPTH|FTW_MOUNT|FTW_PHYS) != 0 && errno != ENOENT) {
        perror("nftw");
        return 1;
    }

    return 0;
}
