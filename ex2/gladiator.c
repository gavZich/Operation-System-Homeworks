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
* Health, Attack, Opponent1, Opponent2, Opponent3
*/
 // define a function that read how is the next opponent

// Function that read health value from opponent's file
int getHealthFromFile(const char* filename) {
    FILE* f = fopen(filename, "r");
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

    // extract the first token (health value)
    char* token = strtok(line, ",");
    if (!token) {
        fprintf(stderr, "No health value found\n");
        return -1;
    }

    while (*token == ' ') token++;  // remove leading spaces
    return atoi(token);  // convert to int and return
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

int main(int argc, char* argv[]) {

    return 0;
}