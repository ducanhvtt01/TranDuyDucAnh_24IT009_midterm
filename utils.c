#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include "ls.h"

char* make_full_path(const char *dir, const char *file) {
    size_t len = strlen(dir) + strlen(file) + 2;
    char *path = malloc(len);
    snprintf(path, len, "%s/%s", dir, file);
    return path;
}

int compare_files(const void *a, const void *b) {
    if (current_sort_options.opt_f) return 0; /* No sort */
    
    const FileInfo *fa = *(const FileInfo **)a;
    const FileInfo *fb = *(const FileInfo **)b;
    
    int result = 0;
    
    if (current_sort_options.opt_S) {
        if (fa->st.st_size < fb->st.st_size) result = 1;
        else if (fa->st.st_size > fb->st.st_size) result = -1;
        else result = strcmp(fa->name, fb->name);
    } else if (current_sort_options.opt_t) {
        time_t time_a = fa->st.st_mtime;
        time_t time_b = fb->st.st_mtime;
        
        if (current_sort_options.opt_c) {
            time_a = fa->st.st_ctime;
            time_b = fb->st.st_ctime;
        } else if (current_sort_options.opt_u) {
            time_a = fa->st.st_atime;
            time_b = fb->st.st_atime;
        }
        
        if (time_a < time_b) result = 1;
        else if (time_a > time_b) result = -1;
        else result = strcmp(fa->name, fb->name);
    } else {
        result = strcmp(fa->name, fb->name);
    }
    
    if (current_sort_options.opt_r) {
        result = -result;
    }
    
    return result;
}

void sort_files(FileInfo **files, int count, const LsOptions *options) {
    if (options->opt_f) return;
    qsort(files, count, sizeof(FileInfo *), compare_files);
}

char get_file_type_char(mode_t mode) {
    if (S_ISREG(mode)) return '-';
    if (S_ISDIR(mode)) return 'd';
    if (S_ISCHR(mode)) return 'c';
    if (S_ISBLK(mode)) return 'b';
    if (S_ISFIFO(mode)) return 'p';
#ifdef S_ISLNK
    if (S_ISLNK(mode)) return 'l';
#endif
#ifdef S_ISSOCK
    if (S_ISSOCK(mode)) return 's';
#endif
    return '?';
}

void format_mode(mode_t mode, char *str) {
    str[0] = get_file_type_char(mode);
    str[1] = (mode & S_IRUSR) ? 'r' : '-';
    str[2] = (mode & S_IWUSR) ? 'w' : '-';
    str[3] = (mode & S_IXUSR) ? 'x' : '-';
    if (mode & S_ISUID) str[3] = (mode & S_IXUSR) ? 's' : 'S';
    
    str[4] = (mode & S_IRGRP) ? 'r' : '-';
    str[5] = (mode & S_IWGRP) ? 'w' : '-';
    str[6] = (mode & S_IXGRP) ? 'x' : '-';
    if (mode & S_ISGID) str[6] = (mode & S_IXGRP) ? 's' : 'S';
    
    str[7] = (mode & S_IROTH) ? 'r' : '-';
    str[8] = (mode & S_IWOTH) ? 'w' : '-';
    str[9] = (mode & S_IXOTH) ? 'x' : '-';
#ifdef S_ISVTX
    if (mode & S_ISVTX) str[9] = (mode & S_IXOTH) ? 't' : 'T';
#endif
    str[10] = '\0';
}

void print_human_readable_size(off_t size) {
    const char *units[] = {"B", "K", "M", "G", "T", "P"};
    int i = 0;
    double dsize = size;
    while (dsize >= 1024 && i < 5) {
        dsize /= 1024;
        i++;
    }
    if (i == 0) printf("%4.0f", dsize);
    else printf("%4.1f%s", dsize, units[i]);
}

void print_file_name(const FileInfo *file, const LsOptions *options) {
    char name[1024];
    strncpy(name, file->name, sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';
    
    if (options->opt_q) {
        for (int i = 0; name[i]; i++) {
            if (name[i] >= 0 && name[i] < 32) {
                name[i] = '?';
            }
        }
    }
    
    printf("%s", name);
    
    if (options->opt_F) {
        mode_t m = file->st.st_mode;
        if (S_ISDIR(m)) printf("/");
#ifdef S_ISLNK
        else if (S_ISLNK(m)) printf("@");
#endif
#ifdef S_ISSOCK
        else if (S_ISSOCK(m)) printf("=");
#endif
        else if (S_ISFIFO(m)) printf("|");
        else if (m & (S_IXUSR | S_IXGRP | S_IXOTH)) printf("*");
    }
}

void print_file_info(const FileInfo *file, const LsOptions *options) {
    if (options->opt_i) {
        printf("%llu ", (unsigned long long)file->st.st_ino);
    }
    
    if (options->opt_s) {
        long long blocks = file->st.st_blocks;
        if (options->opt_k) blocks = (blocks * 512 + 1023) / 1024;
        printf("%4lld ", blocks);
    }
    
    if (options->opt_l) {
        char mode_str[11];
        format_mode(file->st.st_mode, mode_str);
        printf("%s %3u ", mode_str, (unsigned)file->st.st_nlink);
        
        if (options->opt_n) {
            printf("%-8u %-8u ", (unsigned)file->st.st_uid, (unsigned)file->st.st_gid);
        } else {
            struct passwd *pw = getpwuid(file->st.st_uid);
            struct group *gr = getgrgid(file->st.st_gid);
            
            if (pw) printf("%-8s ", pw->pw_name);
            else printf("%-8u ", (unsigned)file->st.st_uid);
            
            if (gr) printf("%-8s ", gr->gr_name);
            else printf("%-8u ", (unsigned)file->st.st_gid);
        }
        
        if (options->opt_h) {
            print_human_readable_size(file->st.st_size);
            printf(" ");
        } else if (options->opt_k) {
            printf("%8lld ", (long long)(file->st.st_size + 1023) / 1024);
        } else {
            printf("%8lld ", (long long)file->st.st_size);
        }
        
        char time_buf[64];
        time_t time_val = file->st.st_mtime;
        if (options->opt_c) time_val = file->st.st_ctime;
        if (options->opt_u) time_val = file->st.st_atime;
        
        struct tm *tm_info = localtime(&time_val);
        strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", tm_info);
        printf("%s ", time_buf);
    }
    
    print_file_name(file, options);
    
#ifdef S_ISLNK
    if (options->opt_l && S_ISLNK(file->st.st_mode)) {
        char link_target[1024];
        ssize_t len = readlink(file->path, link_target, sizeof(link_target) - 1);
        if (len != -1) {
            link_target[len] = '\0';
            printf(" -> %s", link_target);
        }
    }
#endif
    
    printf("\n");
}
