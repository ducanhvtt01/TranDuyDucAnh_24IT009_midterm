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
        exit_status = 1;
        return;
    }
    
    if (S_ISDIR(st.st_mode) && !options->opt_d) {
        list_directory(path, options);
    } else {
        FileInfo *fi = malloc(sizeof(FileInfo));
        fi->name = strdup(path);
        fi->path = strdup(path);
        fi->st = st;
        FileInfo *single_file[1] = { fi };
        print_files(single_file, 1, options);
        free(fi->name);
        free(fi->path);
        free(fi);
    }
}

void list_directory(const char *dir_path, const LsOptions *options) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror(dir_path);
        exit_status = 1;
        return;
    }
    
    struct dirent *entry;
    FileInfo **files = NULL;
    int count = 0;
    int capacity = 10;
    files = malloc(capacity * sizeof(FileInfo*));
    
    /* Calculate total blocks and bytes for directory */
    blkcnt_t total_blocks = 0;
    off_t total_bytes = 0;
    
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
            exit_status = 1;
            free(fi->name);
            free(fi->path);
            free(fi);
            continue;
        }
        
        /* 512-byte blocks. st_blocks is usually in 512-byte units on most POSIX */
        total_blocks += fi->st.st_blocks; 
        total_bytes += fi->st.st_size;
        
        files[count++] = fi;
    }
    closedir(dir);
    
    sort_files(files, count, options);
    
    if (options->opt_l || options->opt_s) {
        /* Print total blocks if listing directory contents with -l or -s */
        if (options->opt_h) {
            char total_buf[32];
            format_human_size(total_bytes, total_buf, sizeof(total_buf));
            printf("total %s\n", total_buf);
        } else if (options->opt_k) {
            long long display_blocks = (total_blocks * 512 + 1023) / 1024;
            printf("total %lld\n", display_blocks);
        } else {
            long bs = get_blocksize();
            long long display_blocks = (total_blocks * 512 + bs - 1) / bs;
            printf("total %lld\n", display_blocks);
        }
    }
    
    print_files(files, count, options);
    
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
