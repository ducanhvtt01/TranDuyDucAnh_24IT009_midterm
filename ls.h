#ifndef LS_H
#define LS_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

/* Options parsing structure */
typedef struct {
    bool opt_1;
    bool opt_A;
    bool opt_a;
    bool opt_c;
    bool opt_d;
    bool opt_F;
    bool opt_f;
    bool opt_h;
    bool opt_i;
    bool opt_k;
    bool opt_l;
    bool opt_n;
    bool opt_q;
    bool opt_R;
    bool opt_r;
    bool opt_S;
    bool opt_s;
    bool opt_t;
    bool opt_u;
    bool opt_w;
    bool opt_G;
    bool opt_T;
    bool opt_octal;
} LsOptions;

/* Structure to store file information */
typedef struct {
    char *name;
    char *path;
    struct stat st;
} FileInfo;

/* ls_core.c */
void process_path(const char *path, const LsOptions *options);
void list_directory(const char *dir_path, const LsOptions *options);
void print_tree_directory(const char *dir_path, const char *prefix, const LsOptions *options);

/* utils.c */
void print_file_info(const FileInfo *file, const LsOptions *options);
void print_file_name(const FileInfo *file, const LsOptions *options);
void print_files(FileInfo **files, int count, const LsOptions *options);
int compare_files(const void *a, const void *b);
void sort_files(FileInfo **files, int count, const LsOptions *options);
void format_mode(mode_t mode, char *str);
void print_human_readable_size(off_t size);
void format_human_size(off_t size, char *buf, size_t buflen);
long get_blocksize(void);
char get_file_type_char(mode_t mode);
char* make_full_path(const char *dir, const char *file);

/* Global options instance for passing to sort comparator */
extern LsOptions current_sort_options;
extern int exit_status;

#endif /* LS_H */
