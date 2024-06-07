#include <stdio.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/ipc.h> // for IPC_RMID, IPC_CREAT
#include <sys/wait.h>

// IPC_CREAT: This flag is used when creating a new IPC object (e.g., message queue, semaphore set, or shared memory segment). If the object with the specified key does not exist, it is created. If it already exists, this flag has no effect.

// IPC_EXCL: This flag is typically used with IPC_CREAT. It ensures that the object is newly created. If the object already exists, the shmget() or msgget() call will fail with EEXIST.

// IPC_NOWAIT: This flag is used with some IPC operations to specify that the operation should not block if it cannot be immediately completed. For example, if a semaphore operation is performed with IPC_NOWAIT, and the operation would cause the process to wait, it will instead return immediately with an error.

// IPC_PRIVATE: This flag is used to generate a unique key for IPC objects. Objects created with IPC_PRIVATE are typically intended for communication between related processes (e.g., parent and child processes).

#define SHM_SIZE 1024

#define BUFFER_SIZE 256

int SharedMemory() {
    int shmid;
    key_t key = ftok("shmfile", 65); // Generate unique key

    // Create a shared memory segment
    if ((shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    // Attach shared memory
    char *shm_ptr;
    if ((shm_ptr = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    // Write to shared memory
    sprintf(shm_ptr, "Hello, shared memory!");

    // Read from shared memory
    printf("Message from shared memory: %s\n", shm_ptr);

    // Fork a child process
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) { // Child process
        // Execute another program using exec
        execlp("./another_program", "another_program", NULL);
        perror("execlp");
        exit(1);
    } else { // Parent process
        // Wait for child to finish
        wait(NULL);
        printf("Child process finished.\n");
    }

    // Detach shared memory
    if (shmdt(shm_ptr) == -1) {
        perror("shmdt");
        exit(1);
    }

    // Remove shared memory segment
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(1);
    }

    return 0;
}


int ExecVariants() {
    // 1. execl(): This function is used to execute a file. It takes the file path and a list of arguments as separate parameters.
    execl("/bin/ls", "ls", "-l", NULL);

    // 2. execle(): Similar to `execl()`, but it also allows you to specify the environment of the executed program explicitly.
    char *envp[] = { "HOME=/", "PATH=/bin:/usr/bin", NULL };
    execle("/bin/ls", "ls", "-l", NULL, envp);

    // 3. execlp(): This function is used to execute a file by searching for it in the directories listed in the `PATH` environment variable.
    execlp("ls", "ls", "-l", NULL);

    // 4. execv(): This function is similar to `execl()`, but it takes an array of arguments instead of separate parameters.
    char *args[] = { "ls", "-l", NULL };
    execv("/bin/ls", args);

    // 5. execvp(): Similar to `execv()`, but it searches for the file to execute in the directories listed in the `PATH` environment variable.
    char *args2[] = { "ls", "-l", NULL };
    execvp("ls", args2);

    // 6. execvpe(): Similar to `execvp()`, but it also allows you to specify the environment of the executed program explicitly.
    char *args3[] = { "ls", "-l", NULL };
    char *envp2[] = { "HOME=/", "PATH=/bin:/usr/bin", NULL };
    execvpe("ls", args3, envp2);

    // 7. execve(): This is the most flexible function, as it allows you to specify both the file path and the environment explicitly.
    char *args4[] = { "ls", "-l", NULL };
    char *envp3[] = { "HOME=/", "PATH=/bin:/usr/bin", NULL };
    execve("/bin/ls", args4, envp3);

    return 0;
}

int UnNamedPipes() {
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

        // Read data from the pipe
        int nbytes = read(pipefd[0], buffer, BUFFER_SIZE);
        if (nbytes == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        printf("Child received message: %s\n", buffer);

        // Close the read end of the pipe
        close(pipefd[0]);
    } else { // Parent process
        close(pipefd[0]); // Close unused read end of the pipe

        // Write data to the pipe
        const char *message = "Hello, child!";
        if (write(pipefd[1], message, sizeof(message)) == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }

        // Close the write end of the pipe
        close(pipefd[1]);
    }

    return 0;
}