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
    node->links = NULL;
    node->link_count = 0;
    pthread_mutex_init(&node->lock, NULL);
    return node;
}

void destroy_node(RNode* node) {
    free(node->links);
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
    link->from = n1;
    link->to = n2;
    link->bandwidth = bandwidth;

    // Add link to both nodes
    n1->links = (RLink**)realloc(n1->links, (n1->link_count + 1) * sizeof(RLink*));
    n1->links[n1->link_count++] = link;

    n2->links = (RLink**)realloc(n2->links, (n2->link_count + 1) * sizeof(RLink*));
    n2->links[n2->link_count++] = link;

    return link;
}

void destroy_link(RLink* link) {
    free(link);
}

// ===== Routing (simple BFS) =====
typedef struct {
    RNode* node;
    RNode* prev;
} NodeRecord;

int find_path(RNetwork* net, RNode* src, RNode* dst, RNode** path, int* path_len) {
    NodeRecord records[128];
    int visited[128] = {0};
    int qhead = 0, qtail = 0;

    records[qtail++] = (NodeRecord){src, NULL};
    visited[(size_t)src % 128] = 1;

    while (qhead < qtail) {
        NodeRecord rec = records[qhead++];
        if (rec.node == dst) {
            // reconstruct path
            int len = 0;
            RNode* cur = dst;
            while (cur) {
                path[len++] = cur;
                // find prev
                RNode* prev = NULL;
                for (int i = 0; i < qtail; i++) {
                    if (records[i].node == cur) {
                        prev = records[i].prev;
                        break;
                    }
                }
                cur = prev;
            }
            // reverse
            for (int i = 0; i < len/2; i++) {
                RNode* tmp = path[i];
                path[i] = path[len-1-i];
                path[len-1-i] = tmp;
            }
            *path_len = len;
            return 1;
        }
        // expand neighbors
        for (int i = 0; i < rec.node->link_count; i++) {
            RLink* link = rec.node->links[i];
            RNode* next = (link->from == rec.node) ? link->to : link->from;
            if (!visited[(size_t)next % 128]) {
                visited[(size_t)next % 128] = 1;
                records[qtail++] = (NodeRecord){next, rec.node};
            }
        }
    }
    return 0;
}

// ===== Packet transfer =====
void transfer_packet(RNetwork* net, RPacket* pkt) {
    RNode* path[64];
    int path_len = 0;

    if (!find_path(net, pkt->src, pkt->dst, path, &path_len)) {
        printf("No route from %s to %s\n", pkt->src->name, pkt->dst->name);
        return;
    }

    // reserve from source
    if (!reserve(pkt->src, pkt->amount)) {
        printf("Not enough resources at %s\n", pkt->src->name);
        return;
    }

    for (int i = 0; i < path_len - 1; i++) {
        RNode* cur = path[i];
        RNode* next = path[i+1];
        printf("[%s -> %s] Transferring %d units...\n", cur->name, next->name, pkt->amount);
        usleep((pkt->amount * 1000000) / 50); // fake bandwidth
        printf("[%s] Received %d units!\n", next->name, pkt->amount);
    }

    release(pkt->src, pkt->amount);
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
    for (int i = 0; i < net->node_count; i++) destroy_node(net->nodes[i]);
    for (int i = 0; i < net->link_count; i++) destroy_link(net->links[i]);
    free(net->nodes);
    free(net->links);
    free(net);
}
