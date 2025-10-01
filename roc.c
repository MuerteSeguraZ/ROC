#include "roc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Create a new resource node
RNode* create_node(const char* name, const char* type, int capacity) {
    RNode* node = (RNode*)malloc(sizeof(RNode));
    strcpy(node->name, name);
    strcpy(node->type, type);
    node->capacity = capacity;
    node->available = capacity;
    pthread_mutex_init(&node->lock, NULL);
    return node;
}

// Destroy node
void destroy_node(RNode* node) {
    pthread_mutex_destroy(&node->lock);
    free(node);
}

// Reserve resources from a node
int reserve(RNode* node, int amount) {
    pthread_mutex_lock(&node->lock);
    int success = 0;
    if (node->available >= amount) {
        node->available -= amount;
        success = 1;
    }
    pthread_mutex_unlock(&node->lock);
    return success;
}

// Release resources back to a node
void release(RNode* node, int amount) {
    pthread_mutex_lock(&node->lock);
    node->available += amount;
    if (node->available > node->capacity) node->available = node->capacity;
    pthread_mutex_unlock(&node->lock);
}

// Monitor available resources
int monitor(RNode* node) {
    pthread_mutex_lock(&node->lock);
    int avail = node->available;
    pthread_mutex_unlock(&node->lock);
    return avail;
}

// Transfer a packet across a link
void transfer(RLink* link, RPacket* packet) {
    printf("[%s -> %s] Transferring %d units...\n", link->node1->name, link->node2->name, packet->amount);
    usleep((packet->amount * 1000000) / link->bandwidth); // simulate transfer time
    printf("[%s] Received %d units!\n", link->node2->name, packet->amount);
}
