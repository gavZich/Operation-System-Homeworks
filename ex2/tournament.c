
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define NUM_GLADIATORS 4

int main() {
    char* gladiator_names[NUM_GLADIATORS] = {"Maximus", "Lucius", "Commodus", "Spartacus"};
    char* gladiator_ids[NUM_GLADIATORS] = {"1", "2", "3", "4"};
    pid_t pids[NUM_GLADIATORS];

    // NOTE: reverse order to ensure Spartacus is forked last
    for (int i = NUM_GLADIATORS - 1; i >= 0; i--) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(1);
        }

        if (pid == 0) {
            execl("./gladiator", "gladiator", gladiator_ids[i], NULL);
            perror("exec failed");
            exit(1);
        }

        pids[i] = pid;
    }

    pid_t last_finished = 0;
    for (int i = 0; i < NUM_GLADIATORS; i++) {
        int status;
        pid_t finished = wait(&status);
        if (finished > 0) {
            last_finished = finished;
        }
    }

    for (int i = 0; i < NUM_GLADIATORS; i++) {
        if (pids[i] == last_finished) {
            printf("The gods have spoken, the winner of the tournament is %s!\n", gladiator_names[i]);
            break;
        }
    }

    return 0;
}