# include <stdio.h>
#include <stdlib.h>
#include <strings.h>       
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_SIZE 256
#define NUM_OF_VARIABLES 5
/*
* The order of the stats:
* Health[0], Attack[1], Opponent1[2], Opponent2[3], Opponent3[4]
*/
// Function that read value of specific field 
int getValueFromFileByIndex(const char* ownFile, int releventOffset) {
      FILE* f = fopen(ownFile, "r");
    if (!f) {
        perror("Failed to open file for reading health");
        return -1;  // error value
    }

    char line[MAX_SIZE];
    if (!fgets(line, MAX_SIZE, f)) {
        perror("Failed to read line");
        fclose(f);
        return -1;
    }

    fclose(f);

    // Split the line into tokens
    char* token = strtok(line, ",");
    int i = 0;
    while (token) {
        while (*token == ' ') token++;  // Trim leading spaces
        if (i == releventOffset) {
            return atoi(token);  // Convert the token to int and return
        }
        token = strtok(NULL, ",");
        i++;
    }

    // If the offset is out of range
    fprintf(stderr, "Invalid offset: %d\n", releventOffset);
    return -1;
}

// Builds the filename of an opponent given their number
void getOpponentFilename(int opponentNumber, char* buffer, size_t bufferSize) {
    snprintf(buffer, bufferSize, "g_%d.txt", opponentNumber);
}


// This function opens the opponent's file and reduces their health according to the attacker's power
int attackAnOpponent(int attackerPower, const char* opponent) {
    int current_health = getHealthFromFile(opponent);  // Extract the current health from the file
    int new_health = current_health - attackerPower;   // Reduce the health by the attacker's power

    // Open the opponent's file in read mode
    FILE* opponentFile = fopen(opponent, "r");
    // Check if opening the file succeeded
    if (!opponentFile){
        perror("Failed to open the opponent file");
        return 0;
    }

    char line[MAX_SIZE]; // Define a buffer that will contain the stats
    fgets(line, MAX_SIZE, opponentFile); // Extract the stats line from the file
    fclose(opponentFile); // Close the file

    // Divide the stats line into independent variables (tokens)
    char* tokens[NUM_OF_VARIABLES];
    int i = 0;
    char* token = strtok(line, ",");
    while (token && i < NUM_OF_VARIABLES) {
        tokens[i++] = token;
        token = strtok(NULL, ",");
    }

    // Replace the first token (health) with the new value
    static char buffer[20];
    snprintf(buffer, sizeof(buffer), "%d", new_health);
    tokens[0] = buffer;

    // Open a new temporary file in write mode where the changes will be stored
    FILE* dirFile = fopen("tmp.txt", "w");
    // Check if opening the new file succeeded
    if (!dirFile){
        perror("Failed to open the new file");
        return 0;
    }
      
    // Loop to insert the stats after parsing, including the updated health
    for (int j = 0; j < i; j++) {
        fprintf(dirFile, "%s", tokens[j]);
        if (j < i - 1)
            fprintf(dirFile, ", ");
    }

    fprintf(dirFile, "\n");
    fclose(dirFile); // Close the temporary file

    // Replace the original file with the updated temporary file
    if (remove(opponent) != 0) {
        perror("Failed to remove the original file");
        return 0;
    }

    if (rename("tmp.txt", opponent) != 0) {
        perror("Failed to rename the temporary file");
        return 0;
    }
    
    // Return 1 if succeeded
    return 1;
}

// Function that open a log file for every gladiator
void createLogFile(int playerNumber) {
    char filename[30];
    snprintf(filename, sizeof(filename), "G%d_log.txt", playerNumber);

    FILE* f = fopen(filename, "w");  // "w" = create new or overwrite existing
    if (!f) {
        perror("Failed to create log file");
        return;
    }

    // Optionally write an initial line
    fprintf(f, "Gladiator process started. 1234:\n");

    fclose(f);  // Close the file to save changes
}


// Function that write the log actions in a file log
void writeValueToFile(const char* gladiatorFile, const char* filename, int opponentNumber) {
    // get the file name of the current opponent
    char opponentFilename[30];
    getOpponentFilename(opponentNumber, opponentFilename, sizeof(opponentFilename));
    
    // get the atttack power of the opponent
    int opponentPower = getValueFromFileByIndex(opponentFilename, 1); // index 1 is power
    
    // reduce gladiator's health
    attackAnOpponent(opponentPower, gladiatorFile);

    // Get the updated health
    int updatedHealth = getValueFromFileByIndex(gladiatorFile, 0); // index 0 is health

    FILE* f = fopen(filename, "a");  // Open in append to add the lines under the existing content 
    if (!f) {
        perror("Failed to open file for writing");
        return;
    }

    // write the action line
    fprintf(f, "Facing opponent %d... Taking %d damage", opponentNumber, opponentPower);
    // write the response line
    if (updatedHealth > 0) {
        fprintf(f, "Are you not entertained? Remaining health: %d\n", updatedHealth);
    } else {
        fprintf(f, "The gladiator has fallen... Final health: %d\n", updatedHealth);
    }

    fclose(f);  // Close the file (saves changes)
}


int main(int argc, char* argv[]) {

    char Ownfilename[20];
    getOpponentFilename(, Ownfilename, sizeof(Ownfilename));
    
    // initializ an array of opponents
    int opponentArray[3];
    for (int i = 0; i < 3; i++) {
        int relevantIndex = 2 + i;
        opponentArray[i] = getValueFromFileByIndex(Ownfilename, relevantIndex);
    }

    // extract the power attack
    int attackPower = getValueFromFileByIndex(Ownfilename, 1);

    for (int i = 0; i < 3; i++) {
        char opponentfilename[20];
        getOpponentFilename(opponentArray[i], opponentfilename, sizeof(opponentfilename));
        attackAnOpponent(attackPower, opponentfilename);
    }
    
    return 0;
}