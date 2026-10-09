#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#if defined(__linux__)
#include <sys/sysmacros.h>
#endif
#include "ls.h"

#ifndef major
#define major(dev) ((int)(((dev) >> 8) & 0xff))
#define minor(dev) ((int)((dev) & 0xff))
#endif

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
#ifdef S_ISWHT
    if (S_ISWHT(mode)) return 'w';
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

void format_human_size(off_t size, char *buf, size_t buflen) {
    const char *units[] = {"B", "K", "M", "G", "T", "P"};
    if (size < 1024) {
        snprintf(buf, buflen, "%lldB", (long long)size);
        return;
    }
    double dsize = (double)size;
    int i = 0;
    while (dsize >= 1024.0 && i < 5) {
        dsize /= 1024.0;
        i++;
    }
    if (dsize >= 9.95) {
        snprintf(buf, buflen, "%.0f%s", dsize, units[i]);
    } else {
        snprintf(buf, buflen, "%.1f%s", dsize, units[i]);
    }
}

void print_human_readable_size(off_t size) {
    char buf[32];
    format_human_size(size, buf, sizeof(buf));
    printf("%5s", buf);
}

long get_blocksize(void) {
    char *bs_env = getenv("BLOCKSIZE");
    if (bs_env) {
        long val = atol(bs_env);
        if (val > 0) return val;
    }
    return 512;
}

void print_file_name(const FileInfo *file, const LsOptions *options) {
    char name[1024];
    strncpy(name, file->name, sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';
    
    if (options->opt_q) {
        for (int i = 0; name[i]; i++) {
            if ((unsigned char)name[i] < 32 || (unsigned char)name[i] == 127) {
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
#ifdef S_ISWHT
        else if (S_ISWHT(m)) printf("%%");
#endif
        else if (m & (S_IXUSR | S_IXGRP | S_IXOTH)) printf("*");
    }
}

void print_file_info(const FileInfo *file, const LsOptions *options) {
    if (options->opt_i) {
        printf("%llu ", (unsigned long long)file->st.st_ino);
    }
    
    if (options->opt_s) {
        if (options->opt_h) {
            char blk_buf[32];
            format_human_size((off_t)file->st.st_blocks * 512, blk_buf, sizeof(blk_buf));
            printf("%5s ", blk_buf);
        } else if (options->opt_k) {
            printf("%4lld ", (long long)(file->st.st_blocks * 512 + 1023) / 1024);
        } else {
            long bs = get_blocksize();
            printf("%4lld ", (long long)(file->st.st_blocks * 512 + bs - 1) / bs);
        }
    }
    
    if (options->opt_l) {
        char mode_str[11];
        format_mode(file->st.st_mode, mode_str);
        printf("%s  %u ", mode_str, (unsigned)file->st.st_nlink);
        
        if (options->opt_n) {
            printf("%-8u  %-8u  ", (unsigned)file->st.st_uid, (unsigned)file->st.st_gid);
        } else {
            struct passwd *pw = getpwuid(file->st.st_uid);
            struct group *gr = getgrgid(file->st.st_gid);
            
            if (pw) printf("%s  ", pw->pw_name);
            else printf("%-8u  ", (unsigned)file->st.st_uid);
            
            if (gr) printf("%s ", gr->gr_name);
            else printf("%-8u ", (unsigned)file->st.st_gid);
        }
        
        if (S_ISCHR(file->st.st_mode) || S_ISBLK(file->st.st_mode)) {
            printf("%3u, %3u ", major(file->st.st_rdev), minor(file->st.st_rdev));
        } else if (options->opt_h) {
            char size_buf[32];
            format_human_size(file->st.st_size, size_buf, sizeof(size_buf));
            printf("%5s ", size_buf);
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
        strftime(time_buf, sizeof(time_buf), "%b %e %H:%M", tm_info);
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

static int get_terminal_width(void) {
    int term_width = 80;
#ifdef TIOCGWINSZ
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) {
        return ws.ws_col;
    }
#endif
    char *cols = getenv("COLUMNS");
    if (cols) {
        int c = atoi(cols);
        if (c > 0) return c;
    }
    return term_width;
}

static void format_entry_short(const FileInfo *file, const LsOptions *options, char *buf, size_t size) {
    char name_buf[1024];
    strncpy(name_buf, file->name, sizeof(name_buf) - 1);
    name_buf[sizeof(name_buf) - 1] = '\0';
    
    if (options->opt_q) {
        for (int i = 0; name_buf[i]; i++) {
            if ((unsigned char)name_buf[i] < 32 || (unsigned char)name_buf[i] == 127) {
                name_buf[i] = '?';
            }
        }
    }
    
    char suffix[2] = "";
    if (options->opt_F) {
        mode_t m = file->st.st_mode;
        if (S_ISDIR(m)) suffix[0] = '/';
#ifdef S_ISLNK
        else if (S_ISLNK(m)) suffix[0] = '@';
#endif
#ifdef S_ISSOCK
        else if (S_ISSOCK(m)) suffix[0] = '=';
#endif
        else if (S_ISFIFO(m)) suffix[0] = '|';
#ifdef S_ISWHT
        else if (S_ISWHT(m)) suffix[0] = '%';
#endif
        else if (m & (S_IXUSR | S_IXGRP | S_IXOTH)) suffix[0] = '*';
        suffix[1] = '\0';
    }
    
    int offset = 0;
    if (options->opt_i) {
        offset += snprintf(buf + offset, size > (size_t)offset ? size - offset : 0, 
                           "%llu ", (unsigned long long)file->st.st_ino);
    }
    if (options->opt_s) {
        if (options->opt_h) {
            char blk_buf[32];
            format_human_size((off_t)file->st.st_blocks * 512, blk_buf, sizeof(blk_buf));
            offset += snprintf(buf + offset, size > (size_t)offset ? size - offset : 0, 
                               "%5s ", blk_buf);
        } else if (options->opt_k) {
            offset += snprintf(buf + offset, size > (size_t)offset ? size - offset : 0, 
                               "%4lld ", (long long)(file->st.st_blocks * 512 + 1023) / 1024);
        } else {
            long bs = get_blocksize();
            offset += snprintf(buf + offset, size > (size_t)offset ? size - offset : 0, 
                               "%4lld ", (long long)(file->st.st_blocks * 512 + bs - 1) / bs);
        }
    }
    snprintf(buf + offset, size > (size_t)offset ? size - offset : 0, 
             "%s%s", name_buf, suffix);
}

static void print_files_columnar(FileInfo **files, int count, const LsOptions *options) {
    if (count <= 0) return;

    char **entries = malloc(count * sizeof(char *));
    int *lens = malloc(count * sizeof(int));
    if (!entries || !lens) {
        for (int i = 0; i < count; i++) {
            print_file_info(files[i], options);
        }
        if (entries) free(entries);
        if (lens) free(lens);
        return;
    }

    for (int i = 0; i < count; i++) {
        char buf[2048];
        format_entry_short(files[i], options, buf, sizeof(buf));
        entries[i] = strdup(buf);
        lens[i] = (int)strlen(buf);
    }

    int term_width = get_terminal_width();

    int best_rows = count;
    int best_cols = 1;
    int *best_col_widths = malloc(count * sizeof(int));

    for (int r = 1; r <= count; r++) {
        int num_rows = r;
        int num_cols = (count + num_rows - 1) / num_rows;
        int total_width = 0;
        int possible = 1;

        int *col_widths = malloc(num_cols * sizeof(int));
        for (int c = 0; c < num_cols; c++) {
            col_widths[c] = 0;
            for (int row = 0; row < num_rows; row++) {
                int idx = c * num_rows + row;
                if (idx < count) {
                    if (lens[idx] > col_widths[c]) {
                        col_widths[c] = lens[idx];
                    }
                }
            }
            total_width += col_widths[c];
        }

        total_width += (num_cols - 1) * 2; /* 2 spaces between columns */

        if (total_width > term_width && num_cols > 1) {
            possible = 0;
        }

        if (possible) {
            best_rows = num_rows;
            best_cols = num_cols;
            for (int c = 0; c < num_cols; c++) {
                best_col_widths[c] = col_widths[c];
            }
            free(col_widths);
            break;
        }
        free(col_widths);
    }

    for (int r = 0; r < best_rows; r++) {
        for (int c = 0; c < best_cols; c++) {
            int idx = c * best_rows + r;
            if (idx >= count) continue;

            int has_next = 0;
            for (int next_c = c + 1; next_c < best_cols; next_c++) {
                if (next_c * best_rows + r < count) {
                    has_next = 1;
                    break;
                }
            }

            if (has_next) {
                printf("%-*s  ", best_col_widths[c], entries[idx]);
            } else {
                printf("%s", entries[idx]);
            }
        }
        printf("\n");
    }

    free(best_col_widths);
    for (int i = 0; i < count; i++) {
        free(entries[i]);
    }
    free(entries);
    free(lens);
}

void print_files(FileInfo **files, int count, const LsOptions *options) {
    if (count <= 0) return;

    if (options->opt_l) {
        for (int i = 0; i < count; i++) {
            print_file_info(files[i], options);
        }
        return;
    }

    if (options->opt_1 || !isatty(STDOUT_FILENO)) {
        for (int i = 0; i < count; i++) {
            char buf[2048];
            format_entry_short(files[i], options, buf, sizeof(buf));
            printf("%s\n", buf);
        }
        return;
    }

    print_files_columnar(files, count, options);
}

