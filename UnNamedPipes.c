#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#define BUFFER_SIZE 256

int main() {
    int pipefd[2]; // File descriptors for the pipe
    char buffer[BUFFER_SIZE];
    pid_t pid;

    // Create the pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Fork a child process
    pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Child process
        close(pipefd[1]); // Close unused write end of the pipe

        while (1) {
            // Read data from the pipe
            int nbytes = read(pipefd[0], buffer, BUFFER_SIZE);
            if (nbytes == -1) {
                perror("read");
                exit(EXIT_FAILURE);
            } else if (nbytes == 0) {
                printf("Parent closed the pipe. Exiting.\n");
                break;
            }

            printf("Child received message: %s\n", buffer);

            // Reply to the parent
            const char *reply = "Message received!";
            write(STDOUT_FILENO, reply, strlen(reply) + 1);
        }

        // Close the read end of the pipe
        close(pipefd[0]);
    } else { // Parent process
        close(pipefd[0]); // Close unused read end of the pipe

        while (1) {
            printf("Enter message for child: ");
            fgets(buffer, BUFFER_SIZE, stdin);

            // Write data to the pipe
            if (write(pipefd[1], buffer, strlen(buffer) + 1) == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }

            // Read child's reply
            int nbytes = read(STDIN_FILENO, buffer, BUFFER_SIZE);
            if (nbytes == -1) {
                perror("read");
                exit(EXIT_FAILURE);
            }

            printf("Parent received reply: %s\n", buffer);
        }

        // Close the write end of the pipe
        close(pipefd[1]);
    }

    return 0;
}
