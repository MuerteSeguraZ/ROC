#include "roc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// ===== Node functions =====
RNode* create_node(const char* name, const char* type, int capacity) {
    RNode* node = (RNode*)malloc(sizeof(RNode));
    strcpy(node->name, name);
    strcpy(node->type, type);
    node->capacity = capacity;
    node->available = capacity;
    pthread_mutex_init(&node->lock, NULL);
    return node;
}

void destroy_node(RNode* node) {
    pthread_mutex_destroy(&node->lock);
    free(node);
}

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

void release(RNode* node, int amount) {
    pthread_mutex_lock(&node->lock);
    node->available += amount;
    if (node->available > node->capacity) node->available = node->capacity;
    pthread_mutex_unlock(&node->lock);
}

int monitor(RNode* node) {
    pthread_mutex_lock(&node->lock);
    int avail = node->available;
    pthread_mutex_unlock(&node->lock);
    return avail;
}

// ===== Link functions =====
RLink* create_link(RNode* n1, RNode* n2, int bandwidth) {
    RLink* link = (RLink*)malloc(sizeof(RLink));
    link->node1 = n1;
    link->node2 = n2;
    link->bandwidth = bandwidth;
    return link;
}

void destroy_link(RLink* link) {
    free(link);
}

void transfer(RLink* link, RPacket* packet) {
    printf("[%s -> %s] Transferring %d units...\n",
           link->node1->name, link->node2->name, packet->amount);
    usleep((packet->amount * 1000000) / link->bandwidth); // simulate
    printf("[%s] Received %d units!\n", link->node2->name, packet->amount);
}

// ===== Network functions =====
RNetwork* create_network() {
    RNetwork* net = (RNetwork*)malloc(sizeof(RNetwork));
    net->nodes = NULL;
    net->node_count = 0;
    net->links = NULL;
    net->link_count = 0;
    return net;
}

void add_node(RNetwork* net, RNode* node) {
    net->nodes = (RNode**)realloc(net->nodes, (net->node_count + 1) * sizeof(RNode*));
    net->nodes[net->node_count++] = node;
}

void add_link(RNetwork* net, RLink* link) {
    net->links = (RLink**)realloc(net->links, (net->link_count + 1) * sizeof(RLink*));
    net->links[net->link_count++] = link;
}

void destroy_network(RNetwork* net) {
    for (int i = 0; i < net->node_count; i++) {
        destroy_node(net->nodes[i]);
    }
    for (int i = 0; i < net->link_count; i++) {
        destroy_link(net->links[i]);
    }
    free(net->nodes);
    free(net->links);
    free(net);
}
