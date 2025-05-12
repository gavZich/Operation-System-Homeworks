
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define NUM_GLADIATORS 4

int main() {
    // define the array of the gladiators as same as in the README
    char* gladiator_names[NUM_GLADIATORS] = {"Maximus", "Lucius", "Commodus", "Spartacus"};
    char* gladiator_ids[NUM_GLADIATORS] = {"1", "2", "3", "4"};
    // make a pid array
    pid_t pids[NUM_GLADIATORS];

    // fork 4 procceses for every gladiator
    for (int i = NUM_GLADIATORS - 1; i >= 0; i--) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(1);
        }

        // if its a sun procces run a executble file
        if (pid == 0) {
            execl("./gladiator", "gladiator", gladiator_ids[i], NULL);
            perror("exec failed");
            exit(1);
        }

        pids[i] = pid;
    }

    pid_t last_finished = 0; // this veraible will contain the winner 
    for (int i = 0; i < NUM_GLADIATORS; i++) {
        int status;
        pid_t finished = wait(&status); // get the last procces
        if (finished > 0) {
            last_finished = finished; 
        }
    }

    // check who is the last one that still stand
    for (int i = 0; i < NUM_GLADIATORS; i++) {
        if (pids[i] == last_finished) {
            printf("The gods have spoken, the winner of the tournament is %s!\n", gladiator_names[i]);
            break;
        }
    }

    return 0;
}