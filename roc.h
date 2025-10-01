#ifndef ROC_H
#define ROC_H

#include <pthread.h>

// Resource Node
typedef struct RNode {
    char name[50];
    char type[20];
    int capacity;
    int available;
    pthread_mutex_t lock;
} RNode;

// Resource Packet
typedef struct RPacket {
    int amount;
} RPacket;

// Resource Link
typedef struct RLink {
    RNode* node1;
    RNode* node2;
    int bandwidth; // units per second
} RLink;

// Functions
RNode* create_node(const char* name, const char* type, int capacity);
void destroy_node(RNode* node);

int reserve(RNode* node, int amount);
void release(RNode* node, int amount);
int monitor(RNode* node);

void transfer(RLink* link, RPacket* packet);

#endif
