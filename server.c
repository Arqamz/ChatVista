#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>

#include "structures.h"

#define SHM_KEY_CLIENTS 0x1234
#define SHM_KEY_GROUPS 0x1235
#define SHM_KEY_MESSAGES 0x1236

typedef struct {
    Message messages[MAX_MESSAGES];
    int message_count;
} SharedMessage;

void handle_client(int client_id);
void handle_signal(int sig);

int shmid_clients, shmid_groups, shmid_messages;
ClientInfo *clients;
Group *groups;
SharedMessage *shared_messages;

int main() {
    int num_clients;
    printf("Enter the number of clients: ");
    scanf("%d", &num_clients);

    if (num_clients > MAX_CLIENTS) {
        printf("Number of clients exceeds the maximum limit of %d.\n", MAX_CLIENTS);
        exit(1);
    }

    // Shared memory for clients
    shmid_clients = shmget(SHM_KEY_CLIENTS, sizeof(ClientInfo) * MAX_CLIENTS, IPC_CREAT | 0666);
    if (shmid_clients < 0) {
        perror("shmget");
        exit(1);
    }
    clients = (ClientInfo *)shmat(shmid_clients, NULL, 0);
    if (clients == (ClientInfo *)-1) {
        perror("shmat");
        exit(1);
    }

    // Shared memory for groups
    shmid_groups = shmget(SHM_KEY_GROUPS, sizeof(Group) * MAX_GROUPS, IPC_CREAT | 0666);
    if (shmid_groups < 0) {
        perror("shmget");
        exit(1);
    }
    groups = (Group *)shmat(shmid_groups, NULL, 0);
    if (groups == (Group *)-1) {
        perror("shmat");
        exit(1);
    }

    // Shared memory for messages
    shmid_messages = shmget(SHM_KEY_MESSAGES, sizeof(SharedMessage), IPC_CREAT | 0666);
    if (shmid_messages < 0) {
        perror("shmget");
        exit(1);
    }
    shared_messages = (SharedMessage *)shmat(shmid_messages, NULL, 0);
    if (shared_messages == (SharedMessage *)-1) {
        perror("shmat");
        exit(1);
    }
    shared_messages->message_count = 0;

    // Initialize clients and groups
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].client_id = i;
        clients[i].active = 0;
    }

    for (int i = 0; i < MAX_GROUPS; i++) {
        groups[i].group_id = i;
        groups[i].message_count = 0;
        memset(groups[i].members, -1, sizeof(groups[i].members));
    }

    // Forking clients
    for (int i = 0; i < num_clients; i++) {
        if (fork() == 0) {
            handle_client(i);
            exit(0);
        }
    }

    signal(SIGINT, handle_signal);

    // Main server loop
    while (1) {
        // Handle incoming messages and group management
        for (int i = 0; i < shared_messages->message_count; i++) {
            Message *msg = &shared_messages->messages[i];
            if (msg->recipient_id == -1) {
                // Group message
                int group_id;
                for (group_id = 0; group_id < MAX_GROUPS; group_id++) {
                    if (msg->recipient_id == groups[group_id].group_id) {
                        break;
                    }
                }
                if (group_id < MAX_GROUPS) {
                    Group *group = &groups[group_id];
                    group->messages[group->message_count++] = *msg;
                    for (int j = 0; j < MAX_GROUP_MEMBERS; j++) {
                        if (group->members[j] != -1) {
                            printf("Message from client %d to group %d: %s\n",
                                   msg->sender_id, group_id, msg->content);
                        }
                    }
                }
            } else {
                // Private message
                if (clients[msg->recipient_id].active) {
                    printf("Message from client %d to client %d: %s\n",
                           msg->sender_id, msg->recipient_id, msg->content);
                }
            }
        }
        shared_messages->message_count = 0; // Reset message count after processing
        sleep(1); // Avoid busy-waiting
    }

    return 0;
}

void handle_client(int client_id) {
    char client_program[50];
    sprintf(client_program, "./client %d", client_id);
    execlp("gnome-terminal", "gnome-terminal", "--", "./client", client_program, NULL);
}

void handle_signal(int sig) {
    if (sig == SIGINT) {
        // Detach and remove shared memory
        shmdt(clients);
        shmdt(groups);
        shmdt(shared_messages);
        shmctl(shmid_clients, IPC_RMID, NULL);
        shmctl(shmid_groups, IPC_RMID, NULL);
        shmctl(shmid_messages, IPC_RMID, NULL);
        exit(0);
    }
}