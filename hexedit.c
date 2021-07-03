#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

void print_help(char *argv0)
{
    fprintf(stderr, "Replace a specific byte in a file.\n"
        "\n"
        "usage:\n"
        "  %s get --offset=... --char=... --file=... [--length=...] [--print-raw]\n"
        "    [--no-newline]\n"
        "  %s set --offset=... <--char=...|--hex=...> --file=...\n"
        "  %s --help\n"
        "\n"
        "  `--offset' and `--length' must be a positive decimal number or in hexadecimal\n"
        "  format prefixed with '0x' or '\\x'\n"
        "\n"
        "  `--char' must be a character, a hexadecimal number between 0x00 and 0xFF prefixed\n"
        "  with '0x' or '\\x', a decimal number between 0 and 255 prefixed with '\\' or an\n"
        "  escape character such as '\\n'\n"
        "\n"
        "  `--hex' must be a list of hexadecimal numbers between 0x00 and 0xFF, not prefixed,\n"
        "  separated by spaces and/or tabs\n"
        "\n"
        "Hint: put quotes around '\\' prefixed arguments to prevent your shell from\n"
        "processing the escape sequence.\n"
        "", argv0, argv0, argv0);
}

unsigned char hextouchar(const char *in)
{
    char *p = NULL;
    char s[] = { '0','x','0',0,0 };
    size_t len = strlen(in);

    if (len > 0) {
        s[2] = in[0];
    }
    if (len > 1) {
        s[3] = in[1];
    }

    return (unsigned char)strtoul(s, &p, 16);
}

unsigned long str_to_ulong(const char *str)
{
    unsigned long l;
    int base = 10;
    char *p = NULL;
    char *buf = strdup(str);

    if (strncasecmp(buf, "0x", 2) == 0 || strncasecmp(buf, "\\x", 2) == 0) {
        buf[0] = '0';
        buf[1] = 'x';
        base = 16;
    }

    l = strtoul(buf, &p, base);
    free(buf);

    return l;
}

int getbyte(const char *ins)
{
    unsigned char c;
    unsigned long l;
    char *p = NULL;

    if (!ins || strlen(ins) == 0) {
        fprintf(stderr, "error: no byte given to insert\n");
        return -1;
    }

    if (strncasecmp(ins, "0x", 2) == 0) {
        /* 0xFF */
        c = hextouchar(ins + 2);
    } else if (ins[0] == '\\') {
        if (ins[1] == '\0') {
            c = '\\';
        } else {
            /* \... */
            if (ins[1] == 'x' || ins[1] == 'X') {
                /* \xFF */
                c = hextouchar(ins + 2);
            } else if (ins[1] >= '0' && ins[1] <= '9') {
                /* \255 */
                if ((l = strtoul(ins + 1, &p, 10)) > 255) {
                    fprintf(stderr, "error: decimal value exceeds 255\n");
                    return -1;
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
                        return -1;
                }
            }
        }
    } else {
        c = ins[0];
    }

    return (int)c;
}

int main(int argc, char *argv[])
{
    int i = 0;
    char mode = 0, c = 0, raw = 0, hex = 1;
    unsigned long offset = 0, length = 1;
    const char *arg = NULL;
    const char *arg_char = NULL;
    const char *arg_hex = NULL;
    const char *arg_file = NULL;
    const char *arg_offset = NULL;
    const char *arg_length = NULL;
    const char *fopen_mode = "rb";
    const char *format = "%02X ";
    const char *delim = " \t";
    char *buf = NULL;
    char *token = NULL;
    size_t len = 0;
    char newline = 1;
    FILE *fp = NULL;

    if (argc < 2) {
        print_help(argv[0]);
        return 1;
    }

    arg = argv[1];

    if (strcmp(arg, "get") == 0) {
        mode = 'g';
    } else if (strcmp(arg, "set") == 0) {
        mode = 's';
        fopen_mode = "r+b";
    } else if (strcmp(arg, "--help") == 0) {
        print_help(argv[0]);
        return 0;
    } else {
        fprintf(stderr, "error: first argument must be `get', `set' or `--help'\n");
        return 1;
    }

    for (i = 2; i < argc; ++i) {
        arg = argv[i];

        if (strncmp(arg, "--offset=", 9) == 0) {
            arg_offset = arg + 9;
        } else if (strncmp(arg, "--char=", 7) == 0) {
            arg_char = arg + 7;
        } else if (strncmp(arg, "--hex=", 6) == 0) {
            arg_hex = arg + 6;
        } else if (strncmp(arg, "--file=", 7) == 0) {
            arg_file = arg + 7;
        } else if (strncmp(arg, "--length=", 9) == 0) {
            arg_length = arg + 9;
        } else if (strcmp(arg, "--print-raw") == 0) {
            raw = 1;
        } else if (strcmp(arg, "--no-newline") == 0) {
            newline = 0;
        } else {
            fprintf(stderr, "error: unknown argument or missing parameter: `%s'\n", arg);
            return 1;
        }
    }

    if (!arg_file || strlen(arg_file) == 0) {
        fprintf(stderr, "error: `--file' not set\n");
        return 1;
    }

    if (!arg_offset || strlen(arg_offset) == 0) {
        fprintf(stderr, "error: `--offset' not set\n");
        return 1;
    }

    if (mode == 's' && !arg_char && !arg_hex) {
        fprintf(stderr, "error: `--char' or `--hex' not set\n");
        return 1;
    }

    if (mode == 's' && arg_char && arg_hex) {
        fprintf(stderr, "error: cannot use `--char' and `--hex' together\n");
        return 1;
    }

    if ((offset = str_to_ulong(arg_offset)) == ULONG_MAX) {
        fprintf(stderr, "error: offset overflow\n");
        return 1;
    }

    if (arg_length && (length = str_to_ulong(arg_length)) == ULONG_MAX) {
        fprintf(stderr, "error: length overflow\n");
        return 1;
    }

    if (length == 0) {
        fprintf(stderr, "error: zero size length\n");
        return 1;
    }

    if ((fp = fopen(arg_file, fopen_mode)) == NULL) {
        fprintf(stderr, "error: cannot open file `%s'\n", arg_file);
        return 1;
    }

    if (fseek(fp, offset, SEEK_SET) != 0) {
        fprintf(stderr, "error: cannot seek to offset 0x%lx (%lu)\n", offset, offset);
        fclose(fp);
        return 1;
    }

    // set byte
    if (mode == 's') {
        if (arg_char) {
            if ((i = getbyte(arg_char)) == -1) {
                fclose(fp);
                return 1;
            }

            c = (unsigned char)i;

            if (fputc(c, fp) == EOF) {
                fprintf(stderr, "error: cannot write byte to file\n");
                fclose(fp);
                return 1;
            }
        } else {
            buf = strdup(arg_hex);
            token = strtok(buf, delim);

            while (token) {
                len = strlen(token);
                hex = 1;

                if (len > 0) {
                    if (len > 2) {
                        hex = 0;
                    }

                    if (hex && !(token[0] >= '0' && token[0] <= '9') &&
                        !(token[0] >= 'A' && token[0] <= 'F') &&
                        !(token[0] >= 'a' && token[0] <= 'f'))
                    {
                        hex = 0;
                    }

                    if (len > 1 && hex &&
                        !(token[1] >= '0' && token[1] <= '9') &&
                        !(token[1] >= 'A' && token[1] <= 'F') &&
                        !(token[1] >= 'a' && token[1] <= 'f'))
                    {
                        hex = 0;
                    }

                    if (!hex) {
                        fprintf(stderr, "error: not a hexadecimal input value: %s\n", token);
                        fclose(fp);
                        free(buf);
                        return 1;
                    }

                    c = hextouchar(token);

                    if (fputc(c, fp) == EOF) {
                        fprintf(stderr, "error: cannot write byte to file\n");
                        fclose(fp);
                        free(buf);
                        return 1;
                    }
                }

                token = strtok(NULL, delim);
            }

            free(buf);
        }

        printf("success\n");
        fclose(fp);
        return 0;
    }

    // get byte
    if ((c = fgetc(fp)) == EOF) {
        fprintf(stderr, "error: cannot read byte\n");
        fclose(fp);
        return 1;
    }

    if (raw) {
        putchar(c);
        format = "%c";
    } else {
        printf("%02X", (unsigned char)c);
        format = " %02X";
    }

    for (i = 1; i < length; ++i) {
        if ((c = fgetc(fp)) == EOF) {
            if (feof(fp) != 0) {
                // end of file
                break;
            }

            fprintf(stderr, "\nerror: cannot read byte\n");
            fclose(fp);
            return 1;
        }

        printf(format, (unsigned char)c);
    }

    if (newline) {
        putchar('\n');
    }

    fclose(fp);

    return 0;
}

