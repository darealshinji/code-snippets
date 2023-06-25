/* find the highest version GLIBC_* symbol in a file or directory */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <assert.h>
#include <ctype.h>
#include <elf.h>
#include <fcntl.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#ifdef __LP64__
#define ELFCLASS_CURRENT ELFCLASS64
typedef Elf64_Ehdr Elf_Ehdr;
typedef Elf64_Shdr Elf_Shdr;
typedef Elf64_Sym  Elf_Sym;
#else
#define ELFCLASS_CURRENT ELFCLASS32
typedef Elf32_Ehdr Elf_Ehdr;
typedef Elf32_Shdr Elf_Shdr;
typedef Elf32_Sym  Elf_Sym;
#endif

char *sym_found = NULL;
char *path_found = NULL;


inline void copy_string(char **dest, const char *src)
{
    *dest = realloc(*dest, strlen(src) + 1);
    strcpy(*dest, src);
}

char *lookup_symbol(void *addr, const size_t fsize)
{
    char *sym = NULL;

    /* check magic bytes */
    if (memcmp(addr, ELFMAG, SELFMAG) != 0) {
        return NULL;
    }

    Elf_Ehdr *ehdr = addr;

    /* check size, class and version */
    if (ehdr->e_shoff > fsize ||
        ehdr->e_ident[EI_CLASS] != ELFCLASS_CURRENT ||
        ehdr->e_ident[EI_VERSION] != EV_CURRENT)
    {
        return NULL;
    }

    /* section header string table */
    Elf_Shdr *shdr = addr + ehdr->e_shoff;
    Elf_Shdr *sh_strtab = &shdr[ehdr->e_shstrndx];
    const char *strtab = addr + sh_strtab->sh_offset;
    Elf_Shdr *sh_dynstr = NULL;

    /* iterate through sections to find .dynstr */
    for (size_t i = 0; i < ehdr->e_shnum; i++) {
        if (memcmp(strtab + shdr[i].sh_name, ".dynstr\0", 8) == 0) {
            sh_dynstr = &shdr[i];
            break;
        }
    }

    if (!sh_dynstr) return NULL;

    const char *ptr = addr + sh_dynstr->sh_offset;
    const char *start = ptr;
    const char * const end = ptr + sh_dynstr->sh_size;

    if (*end != '\0') return NULL;

    /* parse .dynstr section data for GLIBC_* symbols */
    for (; ptr < end; ptr++) {
        if (*ptr != '\0') continue;

        if (*start == 'G' && strncmp(start+1, "LIBC_", 5) == 0 &&
            isdigit(start[6]) &&
            strchr(start, '.') &&
            (!sym || strverscmp(sym, start) < 0))
        {
            copy_string(&sym, start);
        }

        start = ptr + 1;
    }

    return sym;
}

int ftw_callback(const char *fpath, const struct stat *sb, int typeflag, struct FTW *)
{
    int fd;

    /* check only regular files */
    if (typeflag != FTW_F) return 0;

    if (sb->st_size < sizeof(Elf_Ehdr)) {
        return 0;
    }

    /* open() file */
    if ((fd = open(fpath, O_RDONLY)) < 0) {
        return 0;
    }

    /* mmap() library */
    void *addr = mmap(NULL, sb->st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap()");
        close(fd);
        return 1;
    }

    /* find symbol */
    char *ptr = lookup_symbol(addr, sb->st_size);

    /* free resources */
    munmap(addr, sb->st_size);
    close(fd);

    if (!ptr) return 0;

    /* take over pointer */
    if (!sym_found) {
        sym_found = ptr;
        copy_string(&path_found, fpath);
        return 0;
    }

    /* compare version strings */
    if (strverscmp(sym_found, ptr) < 0) {
        copy_string(&sym_found, ptr);
        copy_string(&path_found, fpath);
    }

    free(ptr);

    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s <FILE|DIRECTORY>\n", argv[0]);
        return 1;
    }

    if (nftw(argv[1], ftw_callback, 20, FTW_MOUNT | FTW_PHYS) != 0) {
        perror("nftw()");
        exit(1);
    }

    if (sym_found) {
        puts(sym_found);
        free(sym_found);
    }

    if (path_found) {
        puts(path_found);
        free(path_found);
    }

    return 0;
}
