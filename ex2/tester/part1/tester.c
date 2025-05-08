#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define NUM_GLADIATORS 4
void runCommand(char command[], int fd) {
    int old_df = dup(1);
    dup2(fd, 1);
    system(command);
    dup2(old_df, 1);
}
int compareWinners(const char filename[]) {
    char* line1,* line2;
    ssize_t bytesRead1, bytesRead2, len1 = 0, len2 = 0;
    FILE* filePtr = fopen(filename, "r");
    bytesRead1 = getline(&line1, &len1, filePtr);
    bytesRead2 = getline(&line2, &len2, filePtr);
    printf(YELLOW "Your Program:" RESET "\n%s\n", line2);
    printf(YELLOW "Tester Program:" RESET" \n%s\n", line1);
    if (!strcmp(line1, line2)) {
        printf(GREEN "✅ The winner message is the same!\n" RESET);
    } else {
        printf(RED "❌ The messages are NOT the same!\n" RESET);
    }
    fclose(filePtr);
}
int main(int argc, char* argv[]) {
    const char winners_file[] = "winners.txt";
    int fd = open(winners_file, O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (fd == -1) {
        fprintf(stderr, RED "winners.txt already open\n");
        exit(EXIT_FAILURE);
    }
    runCommand("./tournament_tester", fd);
    runCommand("./tournament", fd);
    compareWinners(winners_file);
    unlink(winners_file);
    char command[256], filename[35], buf[10];
    int output_fd, bread, old_df;
    printf("\n");
    for (int i = 1; i <= NUM_GLADIATORS; i++) {
        snprintf(command, sizeof(command), "diff G%d_log.txt new_copy_G%d_log.txt | wc -l", i, i);
        printf("Cheking G%d_log.txt with tester file:\n", i);
        output_fd = open("output.txt", O_RDWR | O_CREAT, 0644);
        runCommand(command, output_fd);
        lseek(output_fd, SEEK_SET, 0);
        bread = read(output_fd, buf, 9);
        buf[bread] = '\0';
        if (atoi(buf) == 4) {
            printf(GREEN "✅ The output of G%d_log.txt is good!\n" RESET, i);
        } else {
            printf(RED "❌ The output of G%d_log.txt is wrong!\n" RESET, i);
        }
        printf("\n");
        sprintf(filename, "new_copy_G%d_log.txt", i);
        // if u want to see the tester output files just put this line in comment :)
        unlink(filename);
    }
    unlink("output.txt");
}