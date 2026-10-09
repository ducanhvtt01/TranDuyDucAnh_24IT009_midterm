# UNIX ls(1) Implementation in C

**Student Name:** Tran Duy Duc Anh  
**Student ID:** 24IT009  
**Course:** System Programming (Lập Trình Hệ Thống) (5)  
**Supervisor** Dr. Nguyen Nhat An  
**Project:** Midterm Project – Implement ls(1)  

## Description

This project is a simplified version of the standard UNIX `ls(1)` command, written in C from scratch. It utilizes standard POSIX system calls and standard C libraries to retrieve file metadata, read directory contents, and format the output according to various flags described in the provided `ls(1)` manual page.

## Implemented Features

The program successfully implements the following options as specified in the manual page snippet:
- **-1**: List one entry per line (by default when output to pipe or explicitly passed)
- **-A**: List all entries except for `.` and `..`
- **-a**: Include directory entries whose names begin with a dot (`.`)
- **-c**: Use time when file status was last changed for sorting or printing
- **-d**: Directories are listed as plain files
- **-F**: Display file type indicators (`/`, `*`, `@`, `=`, `|`)
- **-f**: Output is not sorted
- **-h**: Human readable sizes
- **-i**: Print the file serial number (inode number)
- **-k**: Sizes in kilobytes
- **-l**: List in long format (overrides -1)
- **Multi-column display**: Automatically adjusts to terminal width when standard output is a terminal (just like standard UNIX `ls`).
- **-n**: Numeric UID/GID for long format
- **-q**: Force printing of non-printable characters as `?`
- **-R**: Recursively list subdirectories
- **-r**: Reverse the sort order
- **-S**: Sort by size, largest first
- **-s**: Display the number of file system blocks
- **-t**: Sort by time modified
- **-u**: Use time of last access for sorting or printing
- **-w**: Force raw printing of non-printable characters
- **-G (New)**: Enable colorized output based on file types (directories in blue, executables in green, symlinks in cyan, etc.).
- **-T (New)**: Display the directory hierarchy in a tree-like format, similar to the standard `tree` command.
- **--octal (New)**: Display file permissions in octal format (e.g., `[0755]`) alongside the string format in long listings.

## Project Structure

- `main.c`: The entry point. Handles parsing command-line arguments using `getopt()` and coordinates the program logic.
- `ls_core.c`: Contains the core logic for iterating over directory entries and processing individual paths (including recursive logic).
- `utils.c`: Provides utility functions for parsing file modes, sorting the files (using `qsort()`), formatting human-readable output, and printing.
- `ls.h`: Header file containing structure definitions and function prototypes.
- `Makefile`: Script to automate the build process.
- `README.md`: This report file.

## Code Implementation Details (Function Explanations)

### 1. `main.c`

```c
void parse_options(int argc, char *argv[], LsOptions *options) {
    int opt;
    if (isatty(STDOUT_FILENO)) options->opt_q = true; 
    else options->opt_w = true;

    while ((opt = getopt(argc, argv, "AacdFfhiklnqRrSstuw")) != -1) {
        switch (opt) {
            case 'a': options->opt_a = true; break;
            case 'l': options->opt_l = true; options->opt_n = false; break;
            case 'R': options->opt_R = true; options->opt_d = false; break;
            /* ... (other flags omitted for brevity) ... */
        }
    }
}
```
**`parse_options(...)`**: Uses the POSIX `getopt` function to parse command-line arguments. It sets the corresponding boolean flags in the `LsOptions` structure based on the flags provided by the user.

- **`main(int argc, char *argv[])`**: The main entry point. It calls `parse_options()`, separates directory operands from file operands, sorts them, and then sequentially calls `print_file_info()` (for plain files) and `list_directory()` (for directories).

### 2. `ls_core.c`

```c
void process_path(const char *path, const LsOptions *options) {
    struct stat st;
    if (lstat(path, &st) == -1) {
        perror(path); exit_status = 1; return;
    }
    
    if (S_ISDIR(st.st_mode) && !options->opt_d) {
        list_directory(path, options);
    } else {
        FileInfo *fi = malloc(sizeof(FileInfo));
        /* ... (Store data and print file) ... */
        FileInfo *single_file[1] = { fi };
        print_files(single_file, 1, options);
    }
}
```
**`process_path(...)`**: Takes a path and uses `lstat()` to check if it's a directory or a file. If it's a directory (and `-d` is not set), it delegates to `list_directory()`. Otherwise, it prints the file information directly.

```c
void list_directory(const char *dir_path, const LsOptions *options) {
    DIR *dir = opendir(dir_path);
    struct dirent *entry;
    
    while ((entry = readdir(dir)) != NULL) {
        /* Filter hidden files based on -a and -A options */
        if (entry->d_name[0] == '.') {
            if (!options->opt_a && !options->opt_A) continue;
        }
        /* Read file metadata and add to array */
    }
    closedir(dir);
    
    sort_files(files, count, options);
    print_files(files, count, options);
    
    /* Recursively list subdirectories if -R is set */
    if (options->opt_R) { /* ... */ }
}
```
**`list_directory(...)`**: Uses `opendir()` and `readdir()` to iterate through all files inside a directory. It stores the file structures dynamically, calculates total allocated blocks (formatted as human-readable, kilobytes, or standard blocksize), sorts the entries, prints them via `print_files()`, and handles recursive directory traversal if the `-R` flag is enabled.

### 3. `utils.c`
- **`make_full_path(...)`**: Utility to concatenate a directory path and a file name into a full path string.
- **`compare_files(const void *a, const void *b)`**: The comparator function passed to `qsort()`. It contains the logic for sorting files lexicographically (default), by size (`-S`), or by time (`-t`, `-c`, `-u`). It also handles reverse sorting if `-r` is set.
- **`sort_files(...)`**: Wrapper around `qsort()` to sort an array of `FileInfo` structures based on user options.
- **`get_file_type_char(...)`**: Analyzes the file mode and returns the type indicator character (e.g., `d` for directory, `-` for regular file, `l` for symlink, `w` for whiteout).
- **`format_mode(...)`**: Translates the `st_mode` integer into the familiar 10-character permission string (e.g., `drwxr-xr-x`, `rws`, `rwt`).
- **`format_human_size(...)` / `print_human_readable_size(...)`**: Converts exact byte counts into human-readable formats (B, K, M, G, T) with standard rounding and unit suffix when the `-h` flag is provided.
- **`get_blocksize(...)`**: Reads the `BLOCKSIZE` environment variable or defaults to 512 bytes for block calculation.
- **`print_file_name(...)`**: Safely prints the file name. It handles appending type indicators for the `-F` flag and escaping non-printable characters for the `-q` flag.
- **`print_files(...)` / `print_files_columnar(...)`**: The core output coordination function. It dynamically detects terminal width via `ioctl()` to print clean, multi-column listings when connected to a terminal, or line-by-line format when redirected or when `-1` or `-l` is active.
- **`print_file_info(...)`**: Prints long listing entries (`-l`), including device major/minor numbers, links, permissions, owner, group, sizes, timestamps (`%b %e %H:%M`), and symlink targets.

## How to Compile and Run (on NetBSD VM)

**Step 1: Download the source code from GitHub**
Open the NetBSD terminal and run the following command to download the source code zip file:
```bash
ftp -o code.zip https://github.com/ducanhvtt01/TranDuyDucAnh_24IT009_midterm/archive/refs/heads/main.zip
```

**Step 2: Unzip and navigate to the project directory**
Unzip the downloaded file (overwriting any existing files if necessary) and change into the directory:
```bash
unzip -o code.zip
cd TranDuyDucAnh_24IT009_midterm-main
```

**Step 3: Compile the project**
Compile the project using the provided Makefile:
```bash
make
```

**Step 4: Run the program**
Run the program with any combination of supported flags. For example:
```bash
./ls -l -a
./ls -R -h
./ls -l -S -r /path/to/directory
```

**Testing the Bonus Features:**
```bash
# Test color output (shows files with beautiful ANSI colors)
./ls -G -l

# Test tree view (draws a tree hierarchy of directories)
./ls -T

# Test octal permissions (shows [0755] before the permission string)
./ls -l --octal
```

**After running the command successfully:**

![After running the command successfully](images/test.png)
   
**Step 5: Clean up the compiled binaries (Optional)**
```bash
make clean
```

## GitHub Repository

**Repository Link:** [https://github.com/ducanhvtt01/TranDuyDucAnh_24IT009_midterm]

