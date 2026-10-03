#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "ls.h"

void process_path(const char *path, const LsOptions *options) {
    struct stat st;
    if (lstat(path, &st) == -1) {
        perror(path);
        return;
    }
    
    if (S_ISDIR(st.st_mode) && !options->opt_d) {
        list_directory(path, options);
    } else {
        FileInfo *fi = malloc(sizeof(FileInfo));
        fi->name = strdup(path);
        fi->path = strdup(path);
        fi->st = st;
        print_file_info(fi, options);
        free(fi->name);
        free(fi->path);
        free(fi);
    }
}

void list_directory(const char *dir_path, const LsOptions *options) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror(dir_path);
        return;
    }
    
    struct dirent *entry;
    FileInfo **files = NULL;
    int count = 0;
    int capacity = 10;
    files = malloc(capacity * sizeof(FileInfo*));
    
    /* Calculate total blocks for directory */
    blkcnt_t total_blocks = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') {
            if (!options->opt_a && !options->opt_A) {
                continue; /* Skip hidden files */
            }
            if (options->opt_A && !options->opt_a) {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                    continue; /* Skip . and .. */
                }
            }
        }
        
        if (count >= capacity) {
            capacity *= 2;
            files = realloc(files, capacity * sizeof(FileInfo*));
        }
        
        FileInfo *fi = malloc(sizeof(FileInfo));
        fi->name = strdup(entry->d_name);
        fi->path = make_full_path(dir_path, entry->d_name);
        
        if (lstat(fi->path, &fi->st) == -1) {
            perror(fi->path);
            free(fi->name);
            free(fi->path);
            free(fi);
            continue;
        }
        
        /* 512-byte blocks. st_blocks is usually in 512-byte units on most POSIX */
        total_blocks += fi->st.st_blocks; 
        
        files[count++] = fi;
    }
    closedir(dir);
    
    sort_files(files, count, options);
    
    if (options->opt_l || options->opt_s) {
        /* Print total blocks if listing directory contents with -l or -s */
        long long display_blocks = total_blocks;
        if (options->opt_k) {
            display_blocks = (display_blocks * 512 + 1023) / 1024;
        } else if (options->opt_h) {
            /* -h overrides -k on block total? NetBSD says:
               "The total number of blocks in units of 512 bytes or BLOCKSIZE..."
               Let's keep it simple. */
        }
        /* In this simplified version, print basic total if -l or -s */
        printf("total %lld\n", display_blocks);
    }
    
    for (int i = 0; i < count; i++) {
        print_file_info(files[i], options);
    }
    
    /* Handle recursion */
    if (options->opt_R) {
        for (int i = 0; i < count; i++) {
            if (S_ISDIR(files[i]->st.st_mode) && 
                strcmp(files[i]->name, ".") != 0 && 
                strcmp(files[i]->name, "..") != 0) {
                printf("\n%s:\n", files[i]->path);
                list_directory(files[i]->path, options);
            }
        }
    }
    
    for (int i = 0; i < count; i++) {
        free(files[i]->name);
        free(files[i]->path);
        free(files[i]);
    }
    free(files);
}
