#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define MAX_LINE 1024
#define READ_RESULT_FILE "read_results.txt"

void process_read(int dataFileDescriptor, int resultFileDescriptor, int start, int end) {
    // check if the range is valid
    if (start < 0 || end < start) {
        return;
    }

    // define the length of the data and allocate buffer
    int len = end - start + 1;
    char* buffer = (char*)malloc(len);
    if (!buffer) return;

    off_t file_size = lseek(dataFileDescriptor, 0, SEEK_END);
    if (start >= file_size) {
        free(buffer);
        return;
    }

    if (end >= file_size) {
        end = file_size - 1;
    }

    len = end - start + 1;

    lseek(dataFileDescriptor, start, SEEK_SET); // seek to start
    read(dataFileDescriptor, buffer, len); // read bytesinto the buffer
    write(resultFileDescriptor, buffer, len); // write the content
    write(resultFileDescriptor, "\n", 1);
    free(buffer);
}

void process_write(int dataFileDescriptor, int offset, char* text) {
    // check if the range is valid
    if (offset < 0 || !text) {
        return;
    }
    // Gets the file size & ensures the offset is within the file.
    int text_len = strlen(text);

    off_t file_size = lseek(dataFileDescriptor, 0, SEEK_END);
    if (offset > file_size) {
        return;
    }

    // define & allocate memory for the data that will be pushed
    int tail_len = file_size - offset;
    char* tail = (char*)malloc(tail_len);
    if (!tail) return;

    lseek(dataFileDescriptor, offset, SEEK_SET); // 
    read(dataFileDescriptor, tail, tail_len); // read the tail 

    lseek(dataFileDescriptor, offset, SEEK_SET);
    write(dataFileDescriptor, text, text_len);
    write(dataFileDescriptor, tail, tail_len);

    free(tail);
}

int main(int argc, char* argv[]) {
    // ensure that the procces provided two files
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <data_file> <requests_file>\n", argv[0]);
        exit(1);
    }

    // Open the data file, extract the file name from the args
    int dataFileDescriptor = open(argv[1], O_RDWR);
    if (dataFileDescriptor < 0) {
        perror(argv[1]);
        exit(1);
    }

    // Open the request file, extract the file name from the args
    int requestFileDescriptor = open(argv[2], O_RDONLY);
    if (requestFileDescriptor < 0) {
        perror(argv[2]);
        close(dataFileDescriptor);
        exit(1);
    }

    // open a result file wuth the permissions: write, creat if not excist, clean the content
    int resultFileDescriptor = open(READ_RESULT_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (resultFileDescriptor < 0) {
        perror(READ_RESULT_FILE);
        close(dataFileDescriptor);
        close(requestFileDescriptor);
        exit(1);
    }

    FILE* req_file = fdopen(requestFileDescriptor, "r"); // open and convert into file*
    if (!req_file) {
        perror("fdopen");
        close(dataFileDescriptor);
        close(resultFileDescriptor);
        exit(1);
    }

    // define a line to contain the data
    char line[MAX_LINE];
    // as long as we can extract data the loop will run
    while (fgets(line, sizeof(line), req_file)) {
        // Q = quit: quit from the loop
        if (line[0] == 'Q') {
            break;
        }
        // R = read: read data for the line
        else if (line[0] == 'R') {
            // creat a varaibles to offset
            int start, end;
            // read from string and insert by patterns
            if (sscanf(line, "R %d %d", &start, &end) == 2) { 
                process_read(dataFileDescriptor, resultFileDescriptor, start, end);
            }
        // W = write: write data into the file
        } else if (line[0] == 'W') {
            int offset;
            char text[MAX_LINE];
            if (sscanf(line, "W %d %s", &offset, text) == 2) {
                process_write(dataFileDescriptor, offset, text);
            }
        }
    }

    close(dataFileDescriptor);
    close(resultFileDescriptor);
    fclose(req_file);
    return 0;
}
