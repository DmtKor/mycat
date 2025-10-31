/*
Kind of copy of original cat.
Concatenate files and print on the standard output.


SYNOPSIS
mycat [OPTION]... [FILE]...:
With no FILE, or when FILE is -, read standard input.
mycat help:
Show this message and exit.
mycat version:
Show version.

OPTIONS
-n  Number all lines.
-b  Number nonempty lines (-nb works like -b).
-e  Display $ at the end of each line.
-h  Hide repeated empty lines.

RETURN CODES
0   Success
1   Error: invalid flag
2   Error: no access to file (no permission or file does not exist)
*/
#include <stdlib.h>   // malloc
#include <string.h>   // strlen
#include <stdio.h>    // puts, fgets, fputs, printf, fopen
#include <inttypes.h> // PRIu64

#define FLAG_N (char)((0b0001))
#define FLAG_B (char)((0b0010))
#define FLAG_E (char)((0b0100))
#define FLAG_H (char)((0b1000))

#define BUF_MAX_SIZE (1024)

void fail(const char *msg, int status)
{
    puts(msg);
    _Exit(status);
}

/*
Search for one of the flags and write them into cur_flags if found;
When encountering unknown flag, prints error message and exits.
*/
void parse_flags(const char *str, char *cur_flags)
{
    for (size_t i = 1; i < strlen(str); i++)
    {
        if (str[i] == 'n')
            *cur_flags |= FLAG_N;
        else if (str[i] == 'b')
            *cur_flags |= FLAG_B;
        else if (str[i] == 'e')
            *cur_flags |= FLAG_E;
        else if (str[i] == 'h')
            *cur_flags |= FLAG_H;
        else
        {
            char msg[24] = "Error - invalid flag: $";
            msg[22] = str[i];
            fail(msg, 1);
        }
    }
}

/*
Replace first occcurence of char c1 with c2
*/
void replace_char_once(char *str, char c1, char c2)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == c1)
        {
            str[i] = c2;
            return;
        }
    }
}

/*
Print all text line by line from fd to stdout, formatting specified by flags;
This function uses
*/
void print_file(char *buf, FILE *fd, const char *flags)
{
    short empty_l = 0;
    char *res = buf;
    char empty = 0;
    long counter = 0;
    while (1)
    {
        /* Reading */
        res = fgets(buf, BUF_MAX_SIZE, fd);
        if (res == NULL) // EOF or error
        {
            break;
        }
        replace_char_once(buf, '\n', '\0');
        /* Empty line handling  */
        if (buf[0] == '\0')
        {
            empty = 1;
            if (empty_l && (*flags & FLAG_H))
                continue;
            empty_l = 1;
        }
        else
        {
            empty = 0;
            empty_l = 0;
        }
        /* Counter */
        if (*flags & FLAG_N)
        {
            counter++;
        }
        else if ((*flags & FLAG_B) && !empty)
        {
            counter++;
        }
        /* FLAG_N/B (numeration) handling */
        if ((*flags & FLAG_N) || (*flags & FLAG_B))
            putc('\t', stdout);
        if ((*flags & FLAG_N) || (!empty && (*flags & FLAG_B)))
            printf("%ld\t", counter);
        else if ((*flags & FLAG_N) || (*flags & FLAG_B))
            putc('\t', stdout);
        /* Printing ($ on end with FLAG_E) */
        if (*flags & FLAG_E)
        {
            fputs(buf, stdout);
            puts("$");
        }
        else
            puts(buf);
    }
    if (fd == stdin) 
        clearerr(fd);
    else 
        fclose(fd);
}

int main(int argc, char **argv)
{
    /* Data */
    char flags = 0;
    int n_files = 0;
    FILE **fd = malloc(sizeof(FILE *) * argc);
    char **filenames = malloc(sizeof(char *) * argc);
    char *buf = malloc(sizeof(char) * BUF_MAX_SIZE + 2);

    /* Parsing arguments */
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            parse_flags(argv[i], &flags);
            if (argv[i][1] == '\0') 
            {
                filenames[n_files] = argv[i];
                n_files++;
            }
            continue;
        }
        filenames[n_files] = argv[i];
        n_files++;
    }

    /* Opening all file descriptors */
    for (int i = 0; i < n_files; i++)
    {
        if (strcmp(filenames[i], "-") == 0)
        {
            fd[i] = stdin;
            continue;
        }
        fd[i] = fopen(filenames[i], "r");
        if (fd[i] == NULL)
        {
            fail("Error - couldn't open the file", 2);
        }
    }
    if (n_files == 0)
    {
        fd[0] = stdin;
        n_files += 1;
    }

    /* Printing all files */
    for (int i = 0; i < n_files; i++)
    {
        print_file(buf, fd[i], &flags);
    }

    free(fd);
    free(filenames);
    free(buf);

    return 0;
}