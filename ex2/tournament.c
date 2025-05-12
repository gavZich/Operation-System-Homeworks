#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define NUM_GLADIATORS 4

int main() {
    // Step 1: Define gladiator names and matching files
    char* gladiator_names[NUM_GLADIATORS] = {"Maximus", "Lucius", "Commodus", "Spartacus"};
    char* gladiator_ids[NUM_GLADIATORS] = {"1", "2", "3", "4"};

    pid_t pids[NUM_GLADIATORS];

    // Step 2: Fork a process for each gladiator
    for (int i = 0; i < NUM_GLADIATORS; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
            exit(1);
        }

        if (pid == 0) {
            // Child process: execute the gladiator program with its ID
            execl("./gladiator", "gladiator", gladiator_ids[i], NULL);

            // If exec fails
            perror("exec failed");
            exit(1);
        }

        // Parent: save child PID
        pids[i] = pid;
    }

    // Step 3: Wait for all gladiators to finish and track the last to exit
    pid_t last_finished = 0;
    for (int i = 0; i < NUM_GLADIATORS; i++) {
        int status;
        pid_t finished = wait(&status);
        if (finished > 0) {
            last_finished = finished;
        }
    }

    // Step 4: Determine winner based on the last process to exit
    for (int i = 0; i < NUM_GLADIATORS; i++) {
        if (pids[i] == last_finished) {
            printf("The gods have spoken, the winner of the tournament is %s!\n", gladiator_names[i]);
            break;
        }
    }

    return 0;
}