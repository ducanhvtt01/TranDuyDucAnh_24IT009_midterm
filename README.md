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

