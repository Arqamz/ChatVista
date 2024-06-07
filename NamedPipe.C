// CREATE NAMED PIPE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define PIPE_NAME "/tmp/my_named_pipe"

int main() {
    // Create the named pipe
    if (mkfifo(PIPE_NAME, 0666) == -1) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }

    printf("Named pipe created: %s\n", PIPE_NAME);
    return 0;
}

// PROCESS 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define PIPE_NAME "/tmp/my_named_pipe"
#define BUFFER_SIZE 256

int main() {
    int fd;
    char buffer[BUFFER_SIZE];

    // Open the named pipe for writing
    if ((fd = open(PIPE_NAME, O_WRONLY)) == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("Enter message for process 2: ");
        fgets(buffer, BUFFER_SIZE, stdin);

        // Write the message to the pipe
        if (write(fd, buffer, strlen(buffer) + 1) == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }

        // Read the reply from process 2
        if ((fd = open(PIPE_NAME, O_RDONLY)) == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        if (read(fd, buffer, BUFFER_SIZE) == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        printf("Process 1 received reply: %s\n", buffer);
        close(fd);
    }

    close(fd);
    return 0;
}

// PROCESS 2

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define PIPE_NAME "/tmp/my_named_pipe"
#define BUFFER_SIZE 256

int main() {
    int fd;
    char buffer[BUFFER_SIZE];

    while (1) {
        // Open the named pipe for reading
        if ((fd = open(PIPE_NAME, O_RDONLY)) == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        // Read the message from the pipe
        if (read(fd, buffer, BUFFER_SIZE) == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        printf("Process 2 received message: %s\n", buffer);
        close(fd);

        // Open the named pipe for writing
        if ((fd = open(PIPE_NAME, O_WRONLY)) == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        // Write a reply to the pipe
        const char *reply = "Message received!";
        if (write(fd, reply, strlen(reply) + 1) == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }

        close(fd);
    }

    return 0;
}
