#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>

#include "structures.h"

#define SHM_KEY_CLIENTS 0x1234
#define SHM_KEY_GROUPS 0x1235

void display_menu();
void send_message(ClientInfo *clients, Group *groups, int client_id);
void read_messages(ClientInfo *clients, Group *groups, int client_id);
void create_group(ClientInfo *clients, Group *groups, int client_id);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <client_id>\n", argv[0]);
        exit(1);
    }

    int client_id = atoi(argv[1]);

    // Shared memory for clients
    int shmid_clients = shmget(SHM_KEY_CLIENTS, sizeof(ClientInfo) * MAX_CLIENTS, 0666);
    if (shmid_clients < 0) {
        perror("shmget");
        exit(1);
    }
    ClientInfo *clients = (ClientInfo *)shmat(shmid_clients, NULL, 0);
    if (clients == (ClientInfo *)-1) {
        perror("shmat");
        exit(1);
    }

    // Shared memory for groups
    int shmid_groups = shmget(SHM_KEY_GROUPS, sizeof(Group) * MAX_GROUPS, 0666);
    if (shmid_groups < 0) {
        perror("shmget");
        exit(1);
    }
    Group *groups = (Group *)shmat(shmid_groups, NULL, 0);
    if (groups == (Group *)-1) {
        perror("shmat");
        exit(1);
    }

    // Mark this client as active
    clients[client_id].active = 1;

    while (1) {
        display_menu();
        int choice;
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                send_message(clients, groups, client_id);
                break;
            case 2:
                read_messages(clients, groups, client_id);
                break;
            case 3:
                create_group(clients, groups, client_id);
                break;
            case 4:
                // Exit and mark this client as inactive
                clients[client_id].active = 0;
                shmdt(clients);
                shmdt(groups);
                exit(0);
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }

    return 0;
}

void display_menu() {
    printf("1. Send a message\n");
    printf("2. Read messages\n");
    printf("3. Create a group\n");
    printf("4. Exit\n");
    printf("Enter your choice: ");
}

void send_message(ClientInfo *clients, Group *groups, int client_id) {
    int recipient_id;
    char message[MAX_MESSAGE_LENGTH];

    printf("Enter recipient client ID or -1 for group message: ");
    scanf("%d", &recipient_id);
    getchar(); // Consume newline character left by scanf
    printf("Enter your message: ");
    fgets(message, MAX_MESSAGE_LENGTH, stdin);

    // Find an empty spot in the messages array for this client or group
    if (recipient_id == -1) {
        int group_id;
        printf("Enter group ID: ");
        scanf("%d", &group_id);

        if (group_id >= 0 && group_id < MAX_GROUPS && groups[group_id].members[0] != -1) {
            int msg_index = groups[group_id].message_count;
            groups[group_id].messages[msg_index].sender_id = client_id;
            groups[group_id].messages[msg_index].recipient_id = recipient_id;
            groups[group_id].messages[msg_index].timestamp = time(NULL);
            strncpy(groups[group_id].messages[msg_index].content, message, MAX_MESSAGE_LENGTH);
            groups[group_id].message_count++;
        } else {
            printf("Invalid group ID or group does not exist.\n");
        }
    } else {
        if (recipient_id >= 0 && recipient_id < MAX_CLIENTS && clients[recipient_id].active) {
            // Send a private message to another client (for simplicity, assume a global message array)
            // For now, we just print to the screen, but in a real implementation we would add this to the shared memory
            printf("Message sent to client %d: %s", recipient_id, message);
        } else {
            printf("Invalid client ID or client is not active.\n");
        }
    }
}

void read_messages(ClientInfo *clients, Group *groups, int client_id) {
    // Read and display messages intended for this client or groups they are part of
    printf("Reading messages for client %d...\n", client_id);

    for (int i = 0; i < MAX_GROUPS; i++) {
        for (int j = 0; j < groups[i].message_count; j++) {
            if (groups[i].messages[j].recipient_id == -1 || groups[i].messages[j].recipient_id == client_id) {
                printf("Message from client %d at %s: %s", 
                       groups[i].messages[j].sender_id, 
                       ctime(&groups[i].messages[j].timestamp), 
                       groups[i].messages[j].content);
            }
        }
    }
}

void create_group(ClientInfo *clients, Group *groups, int client_id) {
    int group_id;
    printf("Enter new group ID: ");
    scanf("%d", &group_id);

    if (group_id >= 0 && group_id < MAX_GROUPS && groups[group_id].members[0] == -1) {
        int num_members;
        printf("Enter number of members: ");
        scanf("%d", &num_members);

        if (num_members > MAX_GROUP_MEMBERS) {
            printf("Number of members exceeds the maximum limit of %d.\n", MAX_GROUP_MEMBERS);
            return;
        }

        printf("Enter client IDs of members: ");
        for (int i = 0; i < num_members; i++) {
            int member_id;
            scanf("%d", &member_id);
            if (member_id >= 0 && member_id < MAX_CLIENTS && clients[member_id].active) {
                groups[group_id].members[i] = member_id;
            } else {
                printf("Invalid client ID or client is not active.\n");
                groups[group_id].members[i] = -1;
            }
        }
        printf("Group %d created successfully.\n", group_id);
    } else {
        printf("Invalid group ID or group already exists.\n");
    }
}
