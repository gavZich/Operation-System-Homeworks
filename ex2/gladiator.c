#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINE 256
#define NUM_OPPONENTS 3

// function that get the power of the opponent to use it by the battle
int getOpponentattackPower(int opponentID) {
    char opponentFile[20]; // buffer of the name
    snprintf(opponentFile, sizeof(opponentFile), "G%d.txt", opponentID);

    // open opponent's file
    FILE* f = fopen(opponentFile, "r");
    if (!f) return 0;

    // get the stats line from the file
    char line[MAX_LINE];
    if (!fgets(line, sizeof(line), f)) {
        fclose(f);
        return 0;
    }
    fclose(f);

    int health, attackPowerPower;
    sscanf(line, "%d, %d", &health, &attackPowerPower);
    return attackPowerPower;
}


int main(int argc, char* argv[]) {
    // if their is no enough arguments
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <gladiator_id>\n", argv[0]);
        return 1;
    }

    // convert from char to int
    int gladiatorID = atoi(argv[1]);


    char statFile[20];
    snprintf(statFile, sizeof(statFile), "G%d.txt", gladiatorID);

    // open file with a permission of read
    FILE* f = fopen(statFile, "r");
    if (!f) {
        perror("Failed to open stat file");
        return 1;
    }


    // get the line of the stat 
    char line[MAX_LINE];
    if (!fgets(line, sizeof(line), f)) {
        perror("Failed to read line");
        fclose(f);
        return 1;
    }
    fclose(f);

    int health, attackPower;
    // array of opponents
    int opponents[NUM_OPPONENTS];
    // insert the stats to veraibles.
    sscanf(line, "%d, %d, %d, %d, %d", &health, &attackPower, &opponents[0], &opponents[1], &opponents[2]);

    // log file part
    char logFile[30];
    snprintf(logFile, sizeof(logFile), "G%d_log.txt", gladiatorID);
    FILE* log = fopen(logFile, "w");
    if (!log) {
        perror("Failed to create log file");
        return 1;
    }

    // write the opening line into the log file
    fprintf(log, "Gladiator process started. %d:\n", getpid());

    // loop from README
    while (health > 0) {
        for (int i = 0; i < NUM_OPPONENTS; i++) {
            int opponent_attackPowerPowerPower = getOpponentattackPower(opponents[i]);
            fprintf(log, "Facing opponent %d... Taking %d damage\n", opponents[i], opponent_attackPowerPowerPower);
            health -= opponent_attackPowerPowerPower;
            if (health > 0) {
                fprintf(log, "Are you not entertained? Remaining health: %d\n", health);
            } else {
                fprintf(log, "The gladiator has fallen... Final health: %d\n", health);
                break;
            }
        }
    }

    fclose(log);
    return 0;
}