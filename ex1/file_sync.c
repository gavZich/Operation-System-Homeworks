#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <strings.h>       

#define MAX_PATH_LEN 1024
#define MAX_FILES     100

// Function to create destination directory
static void create_destination_directory(const char *dest_dir) {
    // create child procces to use execl
    pid_t pid = fork();
    if (pid < 0) {
         perror("fork failed"); exit(1);
    }

    if (pid == 0) {
        execl("/bin/mkdir", "mkdir", "-p", dest_dir, NULL);
        perror("exec mkdir failed");
        _exit(1);
    }

    int st;
    waitpid(pid, &st, 0);
    if (!WIFEXITED(st) || WEXITSTATUS(st) != 0) {
        fprintf(stderr, "mkdir failed: %s\n", strerror(errno));
        exit(1);
    }
    printf("Created destination directory '%s'.\n", dest_dir);
}

// Function to getout the absolute path of the directory
static void get_absolute_path(const char *path, char *abs_path) {
    if (path[0] == '/') {
        strncpy(abs_path, path, MAX_PATH_LEN - 1);
        abs_path[MAX_PATH_LEN - 1] = '\0';
        return;
    }

    char cwd[MAX_PATH_LEN];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd failed"); exit(1);
    }
    snprintf(abs_path, MAX_PATH_LEN, "%s/%s", cwd, path);
}

// Function to check if a file is exist in the dir
static int file_exist(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}


// Function to compare between the files in src and dest by exec->diff
static int compare_files(const char *src, const char *dst) {
    // create child procces to use execl
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed"); return 2;
    }

    if (pid == 0) {
        int null_fd = open("/dev/null", O_WRONLY);
        dup2(null_fd, STDOUT_FILENO);
        close(null_fd);
        execl("/usr/bin/diff", "diff", "-q", src, dst, NULL);
        _exit(2);
    }
    int st;
    waitpid(pid, &st, 0);
    return WIFEXITED(st) ? WEXITSTATUS(st) : 2;
}


// copy file from src to dest by execl->cp
static int copy_file(const char *src, const char *dst)
{
    pid_t pid = fork();
    if (pid < 0) { perror("fork failed"); return 1; }

    if (pid == 0) {
        int null_fd = open("/dev/null", O_WRONLY);
        dup2(null_fd, STDOUT_FILENO);
        close(null_fd);
        execl("/bin/cp", "cp", src, dst, NULL);
        _exit(1);
    }
    int st;
    waitpid(pid, &st, 0);
    return WIFEXITED(st) ? WEXITSTATUS(st) : 1;
}


// Make a sort by alphabet and strcmp
static int name_cmp(const struct dirent **a, const struct dirent **b) {
    return strcasecmp((*a)->d_name, (*b)->d_name);
}


// The main loop of the synchronic is in this logic function
static void sync_files(const char *src_dir, const char *dst_dir) {

    struct dirent **list;
    int n = scandir(src_dir, &list, NULL, name_cmp);
    if (n < 0) {
        perror("scandir failed");
        exit(1);
    }
    
    int processed = 0;
    for (int i = 0 ; i < n && processed < MAX_FILES; ++i) {
        // create struct to every file
        struct dirent *e = list[i];
        if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;

        char src_path[MAX_PATH_LEN];
        char dst_path[MAX_PATH_LEN];

        snprintf(src_path, sizeof(src_path), "%s/%s", src_dir, e->d_name);
        snprintf(dst_path, sizeof(dst_path), "%s/%s", dst_dir, e->d_name);

        struct stat src_stat;
        if (stat(src_path, &src_stat) == -1 || !S_ISREG(src_stat.st_mode))
            continue;

        // if their is a missing
        if (!file_exist(dst_path)) {                    
            printf("New file found: %s\n", e->d_name);
            if (copy_file(src_path, dst_path) == 0)
                printf("Copied: %s -> %s\n", src_path, dst_path);
            else
                fprintf(stderr, "Failed to copy %s\n", e->d_name);
            // increase the processed in 1 to continue the loop
            ++processed;
            continue;
        }

        // compare the iles
        int diff = compare_files(src_path, dst_path);
        if (diff == 0) {
            printf("File %s is identical. Skipping...\n", e->d_name);
        } else if (diff == 1) {
            struct stat dst_stat;
            if (stat(dst_path, &dst_stat) == -1) {
                perror("stat dest"); continue;
            }
            if (src_stat.st_mtime > dst_stat.st_mtime) { 
                if (copy_file(src_path, dst_path) == 0) {
                    printf("File %s is newer in source. Updating...\n", e->d_name);
                    printf("Copied: %s -> %s\n", src_path, dst_path);
                } else
                    fprintf(stderr, "Failed to copy %s\n", e->d_name);
            } else {
                printf("File %s is newer in destination. Skipping...\n", e->d_name);
            }
        } else {
            fprintf(stderr, "Error comparing %s\n", e->d_name);
        }
        // increase the processed in 1 to continue the loop
        ++processed;
    }
    // free the structurs that create to every file and to the list
    for (int i = 0; i < n; ++i) {
        free(list[i]);
    } 
    free(list);
}


int main(int argc, char *argv[])
{
    char src_abs[MAX_PATH_LEN], dst_abs[MAX_PATH_LEN], cwd[MAX_PATH_LEN];

    if (getcwd(cwd, sizeof(cwd)) == NULL) { perror("getcwd"); return 1; }
    printf("Current working directory: %s\n", cwd);

    if (argc != 3) {
        printf("Usage: file_sync <source_directory> <destination_directory>\n");
        return 1;
    }

    get_absolute_path(argv[1], src_abs);
    get_absolute_path(argv[2], dst_abs);

    DIR *src_d = opendir(src_abs);
    if (!src_d) {
        printf("Error: Source directory '%s' does not exist.\n", argv[1]);
        return 1;
    }
    closedir(src_d);

    if (opendir(dst_abs) == NULL) {
        if (errno == ENOENT)
            create_destination_directory(argv[2]); 
        else { perror("opendir dest failed"); return 1; }
    }

    printf("Synchronizing from %s to %s\n", src_abs, dst_abs);

    char saved[MAX_PATH_LEN];
    getcwd(saved, sizeof(saved));
    chdir(src_abs);                  

    sync_files(src_abs, dst_abs);

    chdir(saved);                    
    printf("Synchronization complete.\n");
    return 0;
}