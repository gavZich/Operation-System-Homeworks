#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>

#define PATH_BUFFER 4096

 // Declare the functions that used
void create_hard_link(const char *src, const char *dst);
void copy_symlink(const char *src, const char *dst);
void copy_directory(const char *src, const char *dst);
void backup_item(const char *src_path, const char *dst_path);

// A regular file → Creates a hard link in the backup location
void create_hard_link(const char *src, const char *dst) {
    // make a hard link by 'link' function
    if (link(src, dst) == -1) {
        perror("Error creating hard link");
        exit(EXIT_FAILURE);
    }
}

// A symbolic link → Replicates the symlink (not the actual file).
void copy_symlink(const char *src, const char *dst) {
    // get a buffer to maintain the path of the original linkd file
    char link_holds[PATH_BUFFER];
    // get the path of the original linkd file
    ssize_t len = readlink(src, link_holds, sizeof(link_holds) - 1);
    
    if (len == -1) {
        perror("Error reading symbolic link");
        exit(EXIT_FAILURE);
    }
    
    link_holds[len] = '\0';
    
    // make a soft limk to the original linkd file
    if (symlink(link_holds, dst) == -1) {
        perror("Error creating symbolic link");
        exit(EXIT_FAILURE);
    }
}

// A directory → Creates the same directory structure in the backup.
void copy_directory(const char *src, const char *dst) {
    struct stat st;
    // check that is a directory by structer stat
    if (stat(src, &st) == -1) {
        perror("Error getting directory stats");
        exit(EXIT_FAILURE);
    }
    
    // create a new directory
    if (mkdir(dst, st.st_mode) == -1) {
        perror("Error creating directory");
        exit(EXIT_FAILURE);
    }
}

// Recursively back up a file or directory
void backup_item(const char *src_path, const char *dst_path) {
    struct stat st;
    
    // Get information about the source file/directory
    if (lstat(src_path, &st) == -1) {
        perror("Error getting file stats");
        exit(EXIT_FAILURE);
    }
    
    // Handle based on file type
    if (S_ISREG(st.st_mode)) {
        // Regular file - create hard link
        create_hard_link(src_path, dst_path);
    } else if (S_ISLNK(st.st_mode)) {
        // Symbolic link - replicate the link
        copy_symlink(src_path, dst_path);
    } else if (S_ISDIR(st.st_mode)) {
        // Directory - create the directory and process contents
        copy_directory(src_path, dst_path);
        
        // Open the directory
        DIR *dir = opendir(src_path);
        if (!dir) {
            perror("Error opening directory");
            exit(EXIT_FAILURE);
        }
        
        // Read directory contents
        struct dirent *entry;
        // that loop run over all the directories or files that availble in the current directory
        while ((entry = readdir(dir)) != NULL) {
            // Skip "." and ".." entries that cous to an infinite loop
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            
            // Create paths for the source and destination items
            char src_item_path[PATH_BUFFER];
            char dst_item_path[PATH_BUFFER];
            
            // connect to a new path
            snprintf(src_item_path, sizeof(src_item_path), "%s/%s", src_path, entry->d_name);
            snprintf(dst_item_path, sizeof(dst_item_path), "%s/%s", dst_path, entry->d_name);
            
            // Recursively backup this item
            backup_item(src_item_path, dst_item_path);
        }
        
        closedir(dir);
    } else {
        // Other file types (devices, sockets, etc.) - not handled
        fprintf(stderr, "Unsupported file type: %s\n", src_path);
    }
}

int main(int argc, char *argv[]) {
    // Check arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_directory> <backup_directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    const char *src_dir = argv[1];
    const char *dst_dir = argv[2];
    
    // Check if source directory exists
    struct stat st;
    if (stat(src_dir, &st) == -1) {
        perror("src dir");
        exit(EXIT_FAILURE);
    }
    
    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Source path is not a directory\n");
        exit(EXIT_FAILURE);
    }
    
    // Check if destination directory already exists
    if (stat(dst_dir, &st) == 0) {
        errno = EEXIST;  // Set errno to "File exists"
        perror("backup dir");
        exit(EXIT_FAILURE);
    } else if (errno != ENOENT) { // If the error is something other than "No such file or directory"
        perror("backup dir");
        exit(EXIT_FAILURE);
    }
    
    // Create the destination directory and backup all contents
    backup_item(src_dir, dst_dir);
    
    return EXIT_SUCCESS;
}