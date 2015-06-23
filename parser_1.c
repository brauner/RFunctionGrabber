#include <errno.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MATCHES 4
#define NAME 3

int main(int argc, char *argv[])
{
    int count;
    int duplicate;
    int nameless_funcs;
    int printed;
    int reti;
    char error[1000];
    char line[4096];
    char filename[1000];
    char *name = argv[1];
    char *dir = argv[2];
    FILE *file_r;
    FILE *file_w;
    regex_t regex;
    regmatch_t groups[MATCHES];
    count = 0;
    duplicate = 0;
    nameless_funcs = 0;

    if (argc > 3) {
        fprintf(stderr, "Too many arguments\n");
        exit(EXIT_FAILURE);
    }
    reti = regcomp(&regex, "((\"|\')*([a-zA-Z0-9.:_-|]+)[[:blank:]]*(\"|\')*(<-|=))*[[:space:]]*function[[:blank:]]*\\([[:blank:]]*", REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Regex compilation failed\n");
        regerror(reti, &regex, error, sizeof(error));
        perror(error);
        exit(EXIT_FAILURE);
    }

    errno = 0;
    mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (errno) {
        perror(strerror(errno));
    }

    errno = 0;
    file_r = fopen(name, "r");
    if (!file_r) {
        fprintf(stderr, "Failed to open file\n");
        perror(name);
        exit(EXIT_FAILURE);
    }

    sprintf(filename, "%s/Start.R", dir);
    errno = 0;
    file_w = fopen(filename, "wx");
    if (!file_w) {
        fprintf(stderr, "Failed to create file\n");
        perror(filename);
        exit(EXIT_FAILURE);
    }

    while (fgets(line, sizeof(line), file_r) != NULL) {
        if (regexec(&regex, line, MATCHES, groups, 0) == 0 && count == 0) {
            char newname[1000];
            sprintf(newname, "%s/%.*s-%d.R",
                    dir,
                    groups[NAME].rm_eo - groups[NAME].rm_so,
                    line + groups[NAME].rm_so, duplicate);
            if(rename(filename, newname) != 0) {
                fprintf(stderr, "Error renaming %s.\n", filename);
            }
            count++;
        }
        else
            if (regexec(&regex, line, MATCHES, groups, 0) == 0) {
                fclose(file_w);
                printed = sprintf(filename, "%s/%.*s.R",
                        dir,
                        groups[NAME].rm_eo - groups[NAME].rm_so,
                        line + groups[NAME].rm_so);
                if (printed == 3 + 2 + 1) {
                    nameless_funcs++;
                    sprintf(filename, "%s/func%d.R", dir, nameless_funcs);
                    file_w = fopen(filename, "wx");
                    if (!file_w) {
                        fprintf(stderr, "Failed to create file\n");
                        exit(EXIT_FAILURE);
                    }
                }
                else if (access(filename, F_OK) == 0) {
                    duplicate++;
                    sprintf(filename, "%s/%.*s-%d.R",
                            dir,
                            groups[NAME].rm_eo - groups[NAME].rm_so,
                            line + groups[NAME].rm_so, duplicate);
                    file_w = fopen(filename, "wx");
                    if (!file_w) {
                        fprintf(stderr, "Failed to create file\n");
                        exit(EXIT_FAILURE);
                    }
                }
                else {
                    file_w = fopen(filename, "wx");
                    if (!file_w) {
                        fprintf(stderr, "Failed to create file\n");
                        exit(EXIT_FAILURE);
                    }
                }
                count++;
            }
        fprintf(file_w, "%s", line);
    }

    printf("Total number of functions: %d\n", count);
    printf("Total number of named functions: %d\n", count - nameless_funcs);
    printf("Total number of duplicate functions: %d\n", duplicate);
    printf("Total number of nameless functions: %d\n", nameless_funcs);
    fclose(file_r);
    fclose(file_w);
    regfree(&regex);

    exit(EXIT_SUCCESS);
}

