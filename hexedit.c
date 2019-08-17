#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

void print_help(char *argv0)
{
    fprintf(stderr, "Replace a specific byte in a file.\n"
        "\n"
        "usage: %s --mode=<get|set> --offset=... --char=... --file=... [--print-raw] [--no-newline]\n"
        "  %s --help\n"
        "\n"
        "  Offset must be a positive decimal number or in hexadecimal format prefixed with '0x' or '\\x'\n"
        "  Char must be a character, a hexadecimal number between 0x00 and 0xFF prefixed with '0x' or '\\x',\n"
        "       a decimal number between 0 and 255 prefixed with '\\' or an escape character such as '\\n'\n"
        "\n"
        "Hint: put quotes around '\\' prefixed arguments to prevent your shell from processing the escape sequence.\n"
        "", argv0, argv0);
}

/* assuming string to begin with \x or 0x */
unsigned char hextouchar(const char *in)
{
    char *p = NULL;
    char s[] = { '0','x','0',0,0 };
    size_t len = strlen(in);

    if (len > 2) {
        s[2] = in[2];
    }
    if (len > 3) {
        s[3] = in[3];
    }

    return (unsigned char)strtoul(s, &p, 16);
}

unsigned long getoffset(const char *offset_str)
{
    unsigned long offset;
    int base = 10;
    char *p = NULL;
    char *buf = strdup(offset_str);

    if (strncasecmp(buf, "0x", 2) == 0 || strncasecmp(buf, "\\x", 2) == 0) {
        buf[0] = '0';
        buf[1] = 'x';
        base = 16;
    }

    offset = strtoul(buf, &p, base);
    free(buf);

    return offset;
}

int getbyte(FILE *fp, const char *format, char newline)
{
    int c = fgetc(fp);

    fclose(fp);

    if (c == EOF) {
        fprintf(stderr, "error: cannot read byte\n");
        return 1;
    }

    printf(format, c, newline);
    return 0;
}

int setbyte(FILE *fp, const char *ins)
{
    unsigned char c;
    unsigned long l;
    char *p = NULL;

    if (!ins || strlen(ins) == 0) {
        fprintf(stderr, "error: no byte given to insert\n");
        fclose(fp);
        return 1;
    }

    if (strncasecmp(ins, "0x", 2) == 0) {
        /* 0xFF */
        c = hextouchar(ins);
    } else if (ins[0] == '\\') {
        if (ins[1] == '\0') {
            c = '\\';
        } else {
            /* \... */
            if (ins[1] == 'x' || ins[1] == 'X') {
                /* \xFF */
                c = hextouchar(ins);
            } else if (ins[1] >= '0' && ins[1] <= '9') {
                /* \255 */
                if ((l = strtoul(ins + 1, &p, 10)) > 255) {
                    fprintf(stderr, "error: decimal value exceeds 255\n");
                    fclose(fp);
                    return 1;
                }
                c = (unsigned char)l;
            } else {
                /* escape sequences */
                switch (ins[1]) {
                    /* common ones */
                    case 'n': c = '\n'; break;
                    case 't': c = '\t'; break;
                    case 'r': c = '\r'; break;

                    /* special characters in shell; not needed, but
                     * the user may have escaped them twice */
                    case '\\':
                    case '\'':
                    case '"':
                    case '`':
                    case '$':
                    case '?':
                    case '*':
                    case '#':
                        c = ins[1];
                        break;

                    /* not so commonly used */
                    case 'a': c = '\a'; break;
                    case 'b': c = '\b'; break;
                    case 'e': c = 0x1B; break;
                    case 'f': c = '\f'; break;
                    case 'v': c = '\v'; break;

                    default:
                        fprintf(stderr, "error: unknown escape sequence\n");
                        fclose(fp);
                        return 1;
                }
            }
        }
    } else {
        c = ins[0];
    }

    if (fputc(c, fp) == EOF) {
        fprintf(stderr, "error: cannot write byte to file\n");
        fclose(fp);
        return 1;
    }

    fclose(fp);

    return 0;
}

int main(int argc, char *argv[])
{
    int i;
    char mode = 0;
    unsigned long offset;
    const char *arg;
    const char *arg_char = NULL;
    const char *arg_file = NULL;
    const char *arg_offset = NULL;
    const char *format = "%02X%c";
    char newline = '\n';
    FILE *fp = NULL;

    if (argc < 2) {
        print_help(argv[0]);
        return 1;
    }

    for (i = 1; i < argc; ++i) {
        arg = argv[i];

        if (strncmp(arg, "--mode=", 7) == 0) {
            arg += 7;
            if (strcmp(arg, "get") == 0) {
                mode = 'g';
            } else if (strcmp(arg, "set") == 0) {
                mode = 's';
            } else {
                fprintf(stderr, "error: `--mode' must be `get' or `set'\n");
                return 1;
            }
        } else if (strncmp(arg, "--offset=", 9) == 0) {
            arg_offset = arg + 9;
        } else if (strncmp(arg, "--char=", 7) == 0) {
            arg_char = arg + 7;
        } else if (strncmp(arg, "--file=", 7) == 0) {
            arg_file = arg + 7;
        } else if (strcmp(arg, "--print-raw") == 0) {
            format = "%c%c";
        } else if (strcmp(arg, "--no-newline") == 0) {
            newline = '\0';
        } else if (strcmp(arg, "--help") == 0) {
            print_help(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "error: unknown argument or missing parameter: `%s'\n", arg);
            return 1;
        }
    }

    if (mode == 0) {
        fprintf(stderr, "error: `--mode' not set\n");
        return 1;
    }

    if (!arg_file || strlen(arg_file) == 0) {
        fprintf(stderr, "error: `--file' not set\n");
        return 1;
    }

    if (!arg_offset || strlen(arg_offset) == 0) {
        fprintf(stderr, "error: `--offset' not set\n");
        return 1;
    }

    if (mode == 's' && (!arg_char || strlen(arg_char) == 0)) {
        fprintf(stderr, "error: `--char' not set\n");
        return 1;
    }

    if ((offset = getoffset(arg_offset)) == ULONG_MAX) {
        fprintf(stderr, "error: offset overflow\n");
        return 1;
    }

    if ((fp = fopen(arg_file, (mode == 'g') ? "rb" : "r+b")) == NULL) {
        fprintf(stderr, "error: cannot open file `%s'\n", arg_file);
        return 1;
    }

    if (fseek(fp, offset, SEEK_SET) != 0) {
        fprintf(stderr, "error: cannot seek to offset 0x%lx (%lu)\n", offset, offset);
        fclose(fp);
        return 1;
    }

    return (mode == 's') ? setbyte(fp, arg_char) : getbyte(fp, format, newline);
}

