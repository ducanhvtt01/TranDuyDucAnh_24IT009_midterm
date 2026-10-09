#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "ls.h"

LsOptions current_sort_options;
int exit_status = 0;

void parse_options(int argc, char *argv[], LsOptions *options) {
    int opt;
    /* Default behavior for terminal vs pipe */
    if (isatty(STDOUT_FILENO)) {
        options->opt_q = true; /* default when output to terminal */
    } else {
        options->opt_w = true;
    }

    /* -A: Always set for the super-user */
    if (geteuid() == 0) {
        options->opt_A = true;
    }

    while ((opt = getopt(argc, argv, "1AacdFfhiklnqRrSstuw")) != -1) {
        switch (opt) {
            case '1':
                options->opt_1 = true;
                options->opt_l = false;
                options->opt_n = false;
                break;
            case 'A': options->opt_A = true; break;
            case 'a': options->opt_a = true; break;
            case 'c': options->opt_c = true; options->opt_u = false; break;
            case 'd': options->opt_d = true; options->opt_R = false; break;
            case 'F': options->opt_F = true; break;
            case 'f': options->opt_f = true; options->opt_a = true; break;
            case 'h': options->opt_h = true; options->opt_k = false; break;
            case 'i': options->opt_i = true; break;
            case 'k': options->opt_k = true; options->opt_h = false; break;
            case 'l': options->opt_l = true; options->opt_n = false; options->opt_1 = false; break;
            case 'n': options->opt_n = true; options->opt_l = true; options->opt_1 = false; break;
            case 'q': options->opt_q = true; options->opt_w = false; break;
            case 'R': options->opt_R = true; options->opt_d = false; break;
            case 'r': options->opt_r = true; break;
            case 'S': options->opt_S = true; break;
            case 's': options->opt_s = true; break;
            case 't': options->opt_t = true; break;
            case 'u': options->opt_u = true; options->opt_c = false; break;
            case 'w': options->opt_w = true; options->opt_q = false; break;
            default:
                fprintf(stderr, "Usage: %s [-1AacdFfhiklnqRrSstuw] [file ...]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    /* Set up global state for qsort */
    current_sort_options = *options;
}

int main(int argc, char *argv[]) {
    LsOptions options = {0};
    parse_options(argc, argv, &options);

    int num_operands = argc - optind;
    if (num_operands == 0) {
        process_path(".", &options);
    } else {
        /* Distinguish between directories and non-directories */
        FileInfo **files = malloc(num_operands * sizeof(FileInfo*));
        FileInfo **dirs = malloc(num_operands * sizeof(FileInfo*));
        int file_count = 0, dir_count = 0;
        
        for (int i = optind; i < argc; i++) {
            struct stat st;
            if (lstat(argv[i], &st) == -1) {
                perror(argv[i]);
                exit_status = 1;
                continue;
            }
            FileInfo *fi = malloc(sizeof(FileInfo));
            fi->name = strdup(argv[i]);
            fi->path = strdup(argv[i]);
            fi->st = st;
            
            if (S_ISDIR(st.st_mode) && !options.opt_d) {
                dirs[dir_count++] = fi;
            } else {
                files[file_count++] = fi;
            }
        }
        
        sort_files(files, file_count, &options);
        sort_files(dirs, dir_count, &options);
        
        /* Display non-directories first */
        if (file_count > 0) {
            print_files(files, file_count, &options);
            for (int i = 0; i < file_count; i++) {
                free(files[i]->name);
                free(files[i]->path);
                free(files[i]);
            }
        }
        
        if (file_count > 0 && dir_count > 0) {
            printf("\n");
        }
        
        /* Display directories */
        for (int i = 0; i < dir_count; i++) {
            if (num_operands > 1) {
                printf("%s:\n", dirs[i]->path);
            }
            list_directory(dirs[i]->path, &options);
            
            if (i < dir_count - 1) {
                printf("\n");
            }
            free(dirs[i]->name);
            free(dirs[i]->path);
            free(dirs[i]);
        }
        
        free(files);
        free(dirs);
    }
    
    return exit_status;
}
