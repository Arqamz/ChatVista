#ifndef CHAT_H
#define CHAT_H

#include <time.h>

// Constants
#define MAX_CLIENTS 10
#define MAX_GROUPS 5
#define MAX_MESSAGES 100
#define MAX_MESSAGE_LENGTH 256
#define MAX_GROUP_MEMBERS 10

// Message Structure
typedef struct {
    int sender_id;
    int recipient_id; // -1 for group messages
    time_t timestamp;
    char content[MAX_MESSAGE_LENGTH];
} Message;

// Group Structure
typedef struct {
    int group_id;
    int members[MAX_GROUP_MEMBERS]; // Array of client IDs
    Message messages[MAX_MESSAGES]; // Chat history
    int message_count; // Number of messages in the group
} Group;

// ClientInfo Structure
typedef struct {
    int client_id;
    int active; // 1 if active, 0 if not
} ClientInfo;

#endif // CHAT_H
