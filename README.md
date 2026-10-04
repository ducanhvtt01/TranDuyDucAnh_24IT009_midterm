# UNIX ls(1) Implementation in C

**Student Name:** Tran Duy Duc Anh 
**Student ID:** 24IT009 
**Course:** System Programming (Lập Trình Hệ Thống)
**Project:** Midterm Project – Implement ls(1)

## Description

This project is a simplified version of the standard UNIX `ls(1)` command, written in C from scratch. It utilizes standard POSIX system calls and standard C libraries to retrieve file metadata, read directory contents, and format the output according to various flags described in the provided `ls(1)` manual page.

## Implemented Features

The program successfully implements the following options as specified in the manual page snippet:
- **-A**: List all entries except for `.` and `..`
- **-a**: Include directory entries whose names begin with a dot (`.`)
- **-c**: Use time when file status was last changed for sorting or printing
- **-d**: Directories are listed as plain files
- **-F**: Display file type indicators (`/`, `*`, `@`, `=`, `|`)
- **-f**: Output is not sorted
- **-h**: Human readable sizes
- **-i**: Print the file serial number (inode number)
- **-k**: Sizes in kilobytes
- **-l**: List in long format
- **-n**: Numeric UID/GID for long format
- **-q**: Force printing of non-printable characters as `?`
- **-R**: Recursively list subdirectories
- **-r**: Reverse the sort order
- **-S**: Sort by size, largest first
- **-s**: Display the number of file system blocks
- **-t**: Sort by time modified
- **-u**: Use time of last access for sorting or printing
- **-w**: Force raw printing of non-printable characters

## Project Structure

- `main.c`: The entry point. Handles parsing command-line arguments using `getopt()` and coordinates the program logic.
- `ls_core.c`: Contains the core logic for iterating over directory entries and processing individual paths (including recursive logic).
- `utils.c`: Provides utility functions for parsing file modes, sorting the files (using `qsort()`), formatting human-readable output, and printing.
- `ls.h`: Header file containing structure definitions and function prototypes.
- `Makefile`: Script to automate the build process.
- `README.md`: This report file.

## Code Implementation Details (Function Explanations)

### 1. `main.c`
- **`parse_options(int argc, char *argv[], LsOptions *options)`**: Uses the POSIX `getopt` function to parse command-line arguments. It sets the corresponding boolean flags in the `LsOptions` structure based on the flags provided by the user.
- **`main(int argc, char *argv[])`**: The main entry point. It calls `parse_options()`, separates directory operands from file operands, sorts them, and then sequentially calls `print_file_info()` (for plain files) and `list_directory()` (for directories).

### 2. `ls_core.c`
- **`process_path(const char *path, const LsOptions *options)`**: Takes a path and uses `lstat()` to check if it's a directory or a file. If it's a directory (and `-d` is not set), it delegates to `list_directory()`. Otherwise, it prints the file information directly.
- **`list_directory(const char *dir_path, const LsOptions *options)`**: Uses `opendir()` and `readdir()` to iterate through all files inside a directory. It stores the file structures dynamically, calculates total allocated blocks, sorts the entries, prints them, and handles recursive directory traversal if the `-R` flag is enabled.

### 3. `utils.c`
- **`make_full_path(...)`**: Utility to concatenate a directory path and a file name into a full path string.
- **`compare_files(const void *a, const void *b)`**: The comparator function passed to `qsort()`. It contains the logic for sorting files lexicographically (default), by size (`-S`), or by time (`-t`, `-c`, `-u`). It also handles reverse sorting if `-r` is set.
- **`sort_files(...)`**: Wrapper around `qsort()` to sort an array of `FileInfo` structures based on user options.
- **`get_file_type_char(...)`**: Analyzes the file mode and returns the type indicator character (e.g., `d` for directory, `-` for regular file, `l` for symlink).
- **`format_mode(...)`**: Translates the `st_mode` integer into the familiar 10-character permission string (e.g., `drwxr-xr-x`).
- **`print_human_readable_size(...)`**: Converts exact byte counts into human-readable formats (K, M, G, T) when the `-h` flag is provided.
- **`print_file_name(...)`**: Safely prints the file name. It handles appending type indicators for the `-F` flag and escaping non-printable characters for the `-q` flag.
- **`print_file_info(...)`**: The main formatting and output function. It checks the active flags (`-l`, `-i`, `-s`) and prints the required metadata for a single file, such as inode, blocks, permissions, owner, group, size, and modification time.

## How to Compile and Run

1. Open a terminal in the project directory.
2. Compile the project using the provided Makefile:
   ```bash
   make
   ```
3. Run the program with any combination of supported flags. For example:
   ```bash
   ./ls_program -l -a
   ./ls_program -R -h
   ./ls_program -l -S -r /path/to/directory
   ```
4. Clean up the compiled binaries:
   ```bash
   make clean
   ```

## GitHub Repository

**Repository Link:** [https://github.com/ducanhvtt01/TranDuyDucAnh_24IT009_midterm]

