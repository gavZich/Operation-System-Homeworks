# File Synchronization Tool in C
==============================

Overview
--------

In this part of the assignment, you will implement a file synchronization tool in C called `file_sync.c`. The program will compare files between two directories and copy newer or missing files from a source directory to a destination directory. This practical utility demonstrates the use of key system calls such as `chdir()`, `getcwd()`, `fork()`, `exec()`, and `wait()`.

Objectives
----------

* Gain hands-on experience with file system navigation using `chdir()` and `getcwd()`
* Practice process creation and management with `fork()`, `exec()`, and `wait()`
* Learn to work with directory content and file attributes

Program Requirements
-------------------

### Command-Line Arguments

The program must accept exactly two arguments:
- **Source directory**: The path to the directory containing files to be synchronized.
- **Destination directory**: The path to the directory where files will be synchronized to.

### Expected Behavior

1. **Argument Validation**: If the number of arguments is incorrect, print: `"Usage: ./file_sync <source_directory> <destination_directory>"` and exit with status 1.
2. **Directory Checks**:
   - If the source directory does not exist, print: `"Error: Source directory '<source_dir>' does not exist."` and exit with status 1.
   - If the destination directory does not exist, create it and print: `"Created destination directory '<dest_dir>'."`.
3. **Directory Navigation**:
   - Use `getcwd()` to obtain and display the current working directory at the start of the program: `"Current working directory: <current_dir>"`
   - Use `chdir()` to navigate between directories during synchronization.
4. **File Synchronization**:
   - For each file in the source directory:
     - If the file doesn't exist in the destination directory, copy it and print: `"New file found: <filename>"`
     - If the file exists in both directories:
       - Use `fork()` and `exec()` to run the `diff` command to compare files.
       - Use `wait()` to get the result.
       - If files are identical, print: `"File <filename> is identical. Skipping..."`
       - If files differ and the source file is newer, copy it and print: `"File <filename> is newer in source. Updating..."`
       - If files differ but the destination file is newer, print: `"File <filename> is newer in destination. Skipping..."`
5. **Process Management**:
   - Use `fork()` to create child processes for running external commands.
   - Use an appropriate `exec()` function to execute command line utilities like `diff` and `cp`.
   - Use `wait()` to manage the parent-child relationship correctly.
6. **Completion Message**: After processing all files, print: `"Synchronization complete."`

### Technical Requirements

1. **System Calls**: Your implementation must use the following system calls:
   - `getcwd()` - To get the current working directory
   - `chdir()` - To change directories
   - `fork()` - To create child processes
   - `exec()` (any variant such as `execl()` or `execvp()`) - To execute external commands
   - `wait()` or `waitpid()` - To wait for child processes

2. **Child Process Operations**:
   - Use `fork()` and `exec()` to run the `diff` command for file comparison
   - Use `fork()` and `exec()` to run the `cp` command to copy files
   - Use `fork()` and `exec()` to create directories if needed

3. **Error Handling**:
   - Proper error checking for all system calls
   - Use `perror()` to report system call failures, e.g., `perror("fork failed")`
   - Exit with appropriate status codes when errors occur

### Restrictions

- Do not use system() function
- Do not use popen() function
- Process only regular files, not subdirectories
- Handle up to 100 files in each directory
- Maximum path length is 1024 characters
- Maximum filename length is 256 characters
- When you are iterating over the files of the source directory, checking whether they need to be created/copied on the destination directory, make sure you do so alphabetically, so you will get the same print order as we did during the automatic tests.
- Your program will be compiled using the command: `gcc -o file_sync file_sync.c`. Make sure it works, warnings can be ignored.

Examples
-------

### Example 1: Basic Usage

```
$ ./file_sync source_dir dest_dir
Current working directory: /home/user/assignment
Synchronizing from /home/user/assignment/source_dir to /home/user/assignment/dest_dir
New file found: file1.txt
Copied: /home/user/assignment/source_dir/file1.txt -> /home/user/assignment/dest_dir/file1.txt
New file found: file2.txt
Copied: /home/user/assignment/source_dir/file2.txt -> /home/user/assignment/dest_dir/file2.txt
Synchronization complete.
```

### Example 2: Creating Destination Directory

```
$ ./file_sync source_dir new_dest_dir
Current working directory: /home/user/assignment
Created destination directory 'new_dest_dir'.
Synchronizing from /home/user/assignment/source_dir to /home/user/assignment/new_dest_dir
New file found: file1.txt
Copied: /home/user/assignment/source_dir/file1.txt -> /home/user/assignment/new_dest_dir/file1.txt
New file found: file2.txt
Copied: /home/user/assignment/source_dir/file2.txt -> /home/user/assignment/new_dest_dir/file2.txt
Synchronization complete.
```

### Example 3: Error Handling

```
$ ./file_sync nonexistent_dir dest_dir
Current working directory: /home/user/assignment
Error: Source directory 'nonexistent_dir' does not exist.
```

### Example 4: File Comparison

```
$ ./file_sync source_dir dest_dir
Current working directory: /home/user/assignment
Synchronizing from /home/user/assignment/source_dir to /home/user/assignment/dest_dir
File file1.txt is identical. Skipping...
File file2.txt is newer in source. Updating...
Copied: /home/user/assignment/source_dir/file2.txt -> /home/user/assignment/dest_dir/file2.txt
File file3.txt is newer in destination. Skipping...
Synchronization complete.
```

### Example 5: Incorrect Arguments

```
$ ./file_sync source_dir
Current working directory: /home/user/assignment
Usage: ./file_sync <source_directory> <destination_directory>
```

Testing
-------

Your program will be evaluated on both functionality and adherence to the requirements, including the use of specific system calls. To test your implementation, we will use a script that creates test directories with various files, runs your program, and verifies the output matches the expected behavior. Follow the output specifications **exactly** to ensure your program passes the automated tests.


Guidance
-------

Since using execl with arguments is new to you, we've wanted to include a small example to help you understand how to use it - the following snippet runs the `ls` command with the `-l` agument:
`execl("/bin/ls", "ls", "-l", NULL);`

In addition, because the `diff` command prints to stdout when it is being run, you will need to suprass its output and only use its return code to see whether the files are equal or different. To do so, add the following code before running `execl`:
```c 
int null_fd = open("/dev/null", O_WRONLY);
dup2(null_fd, STDOUT_FILENO);
close(null_fd);
execl("/usr/bin/diff", "diff", "-q", path1, path2, NULL);
```
