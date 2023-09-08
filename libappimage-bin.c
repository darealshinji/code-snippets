/***
This is a simple low-level command line tool for the libappimage API

Copyright (c) 2023 AppImage Community

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

https://github.com/AppImageCommunity
***/

/*
PFX="$PWD"
gcc -O3 -Wall -Wextra -pedantic -I"$PFX/include" -o libappimage-bin libappimage-bin.c \
    -L"$PFX/lib" -lappimage -lappimage_shared -Wl,-rpath,"$PFX/lib" -s
*/
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <appimage/appimage.h>

/* should this be made optional? */
#define VERBOSE false

#ifdef LIBAPPIMAGE_DESKTOP_INTEGRATION_ENABLED
#define DES(x) x
#else
#define DES(x) /**/
#endif

const char *argv0 = NULL;

void usage()
{
    printf("usage:\n"
        "  %s COMMAND FILE [OPTIONS[...]]\n"
        "  %s --help\n"
        "  %s --version\n"
        "\n"
        "COMMAND is based on the C API function name without the `appimage_` prefix and\n"
        "with dashes instead of underscores, i.e. `appimage_get_type()` becomes `get-type`.\n"
        "Some commands may require additional arguments.\n"
        "\n"
        "commands:\n"
#ifdef LIBAPPIMAGE_THUMBNAILER_ENABLED
        "  create-thumbnail\n"
#endif
        "  extract-file-following-symlinks <file_path> <target_file_path>\n"
        "  get-elf-section-offset-and-length <section_name>\n"
        "  get-md5\n"
        "  get-payload-offset\n"
        "  get-type\n"
    DES("  is-registered-in-system\n")
        "  is-terminal-app\n"
        "  list-files\n"
        "  print-binary <offset> <length>\n"
        "  print-hex <offset> <length>\n"
    DES("  register-in-system\n")
        "  registered-desktop-file-path\n"
    DES("  shall-not-be-integrated\n")
        "  type2-digest-md5\n"
    DES("  unregister-in-system\n")
        "\n"
        "The following commands do nothing:\n"
        "  hexlify\n"
        "  read-file-into-buffer-following-symlinks\n"
        "  string-list-free\n",
        argv0, argv0, argv0
    );
}

int main(int argc, char **argv)
{
    argv0 = argv[0];

    if (argc < 2) {
        usage();
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            usage();
            return 0;
        } else if (strcmp(argv[i], "--version") == 0) {
            puts("libappimage version " LIBAPPIMAGE_VERSION);
            return 0;
        }
    }

    char *p = NULL;
    const char *cmd = argv[1];
    const char *path = argv[2];

#define CHECK_ARGLEN(x) \
    if (argc < x) { \
        fprintf(stderr, "Missing arguments. See `%s --help`\n", argv0); \
        return 1; \
    }

    /* parse "COMMAND" argument */
    if (strcmp(cmd, "get-md5") == 0) {
        p = appimage_get_md5(path);
        if (!p) return 1;
        printf("%s\n", p);
        free(p);
        return 0;
    }
    else if (strcmp(cmd, "get-payload-offset") == 0) {
        printf("%ld\n", appimage_get_payload_offset(path));
        return 0;
    }
    else if (strcmp(cmd, "get-type") == 0) {
        printf("%d\n", appimage_get_type(path, VERBOSE));
        return 0;
    }
    else if (strcmp(cmd, "registered-desktop-file-path") == 0) {
        p = appimage_registered_desktop_file_path(path, NULL, VERBOSE);
        if (!p) return 1;
        printf("%s\n", p);
        free(p);
        return 0;
    }
    else if (strcmp(cmd, "extract-file-following-symlinks") == 0) {
        CHECK_ARGLEN(5);
        appimage_extract_file_following_symlinks(path, argv[3], argv[4]);
        return 0;
    }
    else if (strcmp(cmd, "list-files") == 0) {
        char **list = appimage_list_files(path);
        if (!list) return 1;

        for (int i=0; list[i] != NULL; i++) {
            printf("%s\n", list[i]);
        }
        appimage_string_list_free(list);
        return 0;
    }
    else if (strcmp(cmd, "is-terminal-app") == 0) {
        int rv = appimage_is_terminal_app(path);

        if (rv > 0) {
            puts("true");
            return 0;
        } else if (rv == 0) {
            puts("false");
            return 0;
        } else if (rv < 0) {
            puts("error");
        }
        return 1;
    }
#ifdef LIBAPPIMAGE_DESKTOP_INTEGRATION_ENABLED
    else if (strcmp(cmd, "shall-not-be-integrated") == 0) {
        int rv = appimage_shall_not_be_integrated(path);

        if (rv > 0) {
            puts("true");
            return 0;
        } else if (rv == 0) {
            puts("false");
            return 0;
        } else if (rv < 0) {
            puts("error");
        }
        return 1;
    }
    else if (strcmp(cmd, "is-registered-in-system") == 0) {
        if (appimage_is_registered_in_system(path) == true) {
            puts("true");
        } else {
            puts("false");
        }
        return 0;
    }
    else if (strcmp(cmd, "register-in-system") == 0) {
        return appimage_register_in_system(path, VERBOSE);
    }
    else if (strcmp(cmd, "unregister-in-system") == 0) {
        return appimage_unregister_in_system(path, VERBOSE);
    }
#ifdef LIBAPPIMAGE_THUMBNAILER_ENABLED
    else if (strcmp(cmd, "create-thumbnail") == 0) {
        return (appimage_create_thumbnail(path, VERBOSE) == true) ? 0 : 1;
    }
#endif
#endif
    else if (strcmp(cmd, "type2-digest-md5") == 0) {
        char buf[16];

        if (appimage_type2_digest_md5(path, (char *)&buf) == false ||
            (p = appimage_hexlify((const char *)&buf, sizeof(buf))) == NULL)
        {
            return 1;
        }
        printf("%s\n", p);
        free(p);
        return 0;
    }
    else if (strcmp(cmd, "get-elf-section-offset-and-length") == 0) {
        unsigned long offset = 0, length = 0;
        CHECK_ARGLEN(4);

        if (appimage_get_elf_section_offset_and_length(path, argv[3], &offset, &length) == false ||
            (offset == 0 && length == 0))
        {
            return 1;
        }
        printf("offset %ld\nlength %ld\n", offset, length);
        return 0;
    }
    else if (strcmp(cmd, "print-hex") == 0) {
        CHECK_ARGLEN(5);
        return appimage_print_hex(path, atol(argv[3]), atol(argv[4]));
    }
    else if (strcmp(cmd, "print-binary") == 0) {
        CHECK_ARGLEN(5);
        return appimage_print_binary(path, atol(argv[3]), atol(argv[4]));
    }
    else if (
        strcmp(cmd, "hexlify") == 0 ||
        strcmp(cmd, "read-file-into-buffer-following-symlinks") == 0 ||
        strcmp(cmd, "string-list-free") == 0)
    {
        fprintf(stderr, "command not implemented\n");
        return 0;
    } else {
        fprintf(stderr, "unknown command: %s\nTry `%s --help`\n", cmd, argv0);
    }

    return 1;
}
