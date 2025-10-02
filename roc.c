#include "roc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// =====================
// Node functions
// =====================
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

// =====================
// Link functions
// =====================
RLink* create_link(RNetwork* net, RNode* n1, RNode* n2, int bandwidth) {
    RLink* link = (RLink*)malloc(sizeof(RLink));
    link->a = n1;
    link->b = n2;
    link->bandwidth = bandwidth;

    net->links = realloc(net->links, (net->link_count + 1) * sizeof(RLink*));
    net->links[net->link_count++] = link;

    return link;
}

void destroy_link(RLink* link) {
    free(link);
}

// =====================
// Network functions
// =====================
RNetwork* create_network() {
    RNetwork* net = (RNetwork*)malloc(sizeof(RNetwork));
    net->nodes = NULL;
    net->node_count = 0;
    net->links = NULL;
    net->link_count = 0;
    return net;
}

void add_node(RNetwork* net, RNode* node) {
    net->nodes = realloc(net->nodes, (net->node_count + 1) * sizeof(RNode*));
    net->nodes[net->node_count++] = node;
}

void destroy_network(RNetwork* net) {
    for (int i = 0; i < net->node_count; i++)
        destroy_node(net->nodes[i]);
    for (int i = 0; i < net->link_count; i++)
        destroy_link(net->links[i]);
    free(net->nodes);
    free(net->links);
    free(net);
}

// =====================
// Routing (BFS shortest / widest)
// =====================
typedef struct QueueItem {
    RNode* node;
    RLink* prev;
    struct QueueItem* parent;
} QueueItem;

static int link_connects(RLink* l, RNode* n1, RNode* n2) {
    return (l->a == n1 && l->b == n2) || (l->a == n2 && l->b == n1);
}

void route_packet(RNetwork* net, RNode* src, RNode* dst, RPacket* pkt, RoutePolicy policy) {
    if (src == dst) {
        printf("Source and destination are the same.\n");
        return;
    }

    // BFS queue
    QueueItem** q = malloc(net->node_count * sizeof(QueueItem*));
    int qh = 0, qt = 0;

    int* visited = calloc(net->node_count, sizeof(int));
    QueueItem* start = malloc(sizeof(QueueItem));
    start->node = src;
    start->prev = NULL;
    start->parent = NULL;
    q[qt++] = start;

    QueueItem* end = NULL;

    while (qh < qt) {
        QueueItem* cur = q[qh++];
        if (cur->node == dst) {
            end = cur;
            break;
        }
        for (int i = 0; i < net->link_count; i++) {
            RLink* l = net->links[i];
            RNode* next = NULL;
            if (l->a == cur->node) next = l->b;
            else if (l->b == cur->node) next = l->a;
            if (next) {
                int idx = -1;
                for (int j = 0; j < net->node_count; j++)
                    if (net->nodes[j] == next) { idx = j; break; }
                if (idx >= 0 && !visited[idx]) {
                    visited[idx] = 1;
                    QueueItem* ni = malloc(sizeof(QueueItem));
                    ni->node = next;
                    ni->prev = l;
                    ni->parent = cur;
                    q[qt++] = ni;
                }
            }
        }
    }

    if (!end) {
        printf("No route from %s to %s\n", src->name, dst->name);
        free(q);
        free(visited);
        return;
    }

    // Reconstruct path
    RLink* path[256];
    int plen = 0;
    for (QueueItem* it = end; it && it->prev; it = it->parent)
        path[plen++] = it->prev;

    if (!reserve(src, pkt->amount)) {
        printf("Not enough resources at %s\n", src->name);
        return;
    }

    // Send along path (from src to dst)
    RNode* current = src;
    for (int i = plen - 1; i >= 0; i--) {
        RLink* l = path[i];
        RNode* next = (l->a == current) ? l->b : l->a;

        printf("[%s -> %s] Transferring %d units...\n", current->name, next->name, pkt->amount);
        usleep((pkt->amount * 1000000) / l->bandwidth);
        printf("[%s] Received %d units!\n", next->name, pkt->amount);

        current = next;
    }

    release(src, pkt->amount);

    free(q);
    free(visited);
}
