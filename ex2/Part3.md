# Backup Tool

## Overview
This project is a **Backup Tool** that recursively copies a directory and its contents while preserving **symlinks** and **file permissions**. Instead of copying the actual content of regular files, the tool creates **hard links** to maintain storage efficiency.

## Features
- Recursively copies all files and directories.
- **Preserves symbolic links** (does not copy the actual file but recreates the symlink).
- **Creates hard links** instead of duplicating file content (for regular files).
- **Maintains file permissions** and attributes.

## How It Works
- The tool scans the given directory.
- If an item is:
  - A **regular file** → Creates a **hard link** in the backup location.
  - A **symbolic link** → Replicates the symlink (not the actual file).
  - A **directory** → Creates the same directory structure in the backup.
- The backup maintains the same structure as the original directory.

## Usage
### Compilation
```sh
gcc -o backup backup.c
```

### Running the Backup Tool
```sh
./backup <source_directory> <backup_directory>
```
Example:
```sh
./backup /home/omer/Documents /home/omer/Backup
```
> **Note**:  
> **You can get the full path like '/home/omer/Documents' or 'Documents' if we run it in '/home/omer/'.**
> **This should not affect your solution, but just clarifying :)**

## Example
```sh
omer@Omer:~/uni/os/targilim/2025B$ tree src_dir/
```
```
src_dir/
├── file1.txt
├── file2.txt
├── subdir1
│   ├── file3.txt
│   └── file4.txt
└── subdir2
    ├── file5.txt
    └── link_to_file1 -> ../file1.txt

2 directories, 6 files
```
After running:
```sh
omer@Omer:~/uni/os/targilim/2025B$ ./backup src_dir/ dest_dir
```
The `dest_dir` directory will contain:
```
dest_dir/
├── file1.txt
├── file2.txt
├── subdir1
│   ├── file3.txt
│   └── file4.txt
└── subdir2
    ├── file5.txt
    └── link_to_file1 -> ../file1.txt

2 directories, 6 files
```

## Why Hard Links Instead of Copying?
- Saves **disk space** (multiple hard links to the same file use one inode).
- **Faster** than copying file content.
- Changes to the original file reflect in all hard links.
- Copies permissions and more **immediately**

## Notes
- The source directory **must exist**, otherwise `perror("src dir")`.
- The backup directory **must not already exist**, otherwise `perror("backup dir")`.
- The copied symbolic links **are not pointing to the original file** but to the copied file (see the example).

## Conclusion
This tool is a **fast and efficient** way to back up directories while preserving **hard links and symlinks**, making it useful for versioning, backups, and system management!

## To Make Your Life Easier
I would recommend u to use this functions:
```c
void create_hard_link(const char *src, const char *dst);
void copy_symlink(const char *src, const char *dst);
void copy_directory(const char *src, const char *dst);
```
# 🐧 🐧 🐧 