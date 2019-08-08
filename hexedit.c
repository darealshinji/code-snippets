#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

typedef unsigned long ulong;
typedef unsigned char uchar;


int print_help(char *argv0)
{
    const char *format = "Replace a specific byte in a file.\n"
        "\n"
        "usage: %s OFFSET CHAR FILE\n"
        "\n"
        "  OFFSET must be a positive decimal number or in hexadecimal format prefixed with '0x' or '\\x'\n"
        "  CHAR must be a character or a hexadecimal number between 0x00 and 0xFF prefixed with '0x' or '\\x',\n"
        "       or a decimal number between 0 and 255 prefixed with '\\'\n"
        "\n"
        "Hint: put quotes around '\\' prefixed arguments to prevent your shell from precessing the escape sequence.\n";

    fprintf(stderr, format, argv0);
    return 1;
}

/* assuming hex_str to begin with \x or 0x */
uchar hextouchar(const char *hex_str)
{
    char *p;
    char s[5];
    ulong l;

    memcpy(s, hex_str, 4);
    s[4] = '\0';
    l = strtoul(s, &p, 16);

    return l > 255 ? '\0' : (uchar)l;
}

int main(int argc, char *argv[])
{
    ulong offset;
    uchar c;
    char arg_offset[5];
    char *p = NULL;
    int base = 10;
    const char *arg_insert, *file;
    FILE *fp;

    if (argc != 4) {
        return print_help(argv[0]);
    }

    memcpy(arg_offset, argv[1], 4);
    arg_offset[4] = '\0';
    arg_insert = argv[2];
    file = argv[3];

    if (strncasecmp(arg_offset, "0x", 2) == 0) {
        base = 16;
    } else if (strncasecmp(arg_offset, "\\x", 2) == 0) {
        arg_offset[0] = '0';
        base = 16;
    }

    offset = strtoul(arg_offset, &p, base);
    fp = fopen(file, "r+b");

    if (fp == NULL) {
        fprintf(stderr, "error: cannot open file `%s'\n", file);
        return 1;
    }

    if (fseek(fp, offset, SEEK_SET) != 0) {
        fprintf(stderr, "error: cannot seek to offset %lu\n", offset);
        fclose(fp);
        return 1;
    }

    if (strncasecmp(arg_insert, "0x", 2) == 0) {
        /* 0xFF */
        c = hextouchar(arg_insert);
    } else if (arg_insert[0] == '\\' && arg_insert[1] != '\0') {
        /* \... */
        if (arg_insert[1] == 'x' || arg_insert[1] == 'X') {
            /* \xFF */
            c = hextouchar(arg_insert);
        } else if (arg_insert[1] >= '0' && arg_insert[1] <= '9') {
            /* \255 */
            ulong l = strtoul(arg_insert + 1, &p, 10);
            c = l > 255 ? '\\' : (uchar)l;
        } else {
            /* escape sequence or backslash */
            switch (arg_insert[1]) {
                case 'n': c = '\n'; break;
                case 'r': c = '\r'; break;
                case 't': c = '\t'; break;
                case 'a': c = '\a'; break;
                case 'b': c = '\b'; break;
                case 'e': c = 0x1B; break;
                case 'f': c = '\f'; break;
                case 'v': c = '\v'; break;
                default:  c = '\\'; break;
            }
        }
    } else {
        c = arg_insert[0];
    }

    if (fputc(c, fp) == EOF) {
        fprintf(stderr, "error: cannot write byte to file\n");
        fclose(fp);
        return 1;
    }

    fclose(fp);

    return 0;
}
