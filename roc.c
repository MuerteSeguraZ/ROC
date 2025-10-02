#include "roc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
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
    node->state = 1; // online
    node->links = NULL;
    node->link_count = 0;
    node->metadata = NULL;
    pthread_mutex_init(&node->lock, NULL);
    return node;
}

void destroy_node(RNode* node) {
    pthread_mutex_destroy(&node->lock);
    free(node->links);
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
RLink* create_link(RNetwork* net, RNode* n1, RNode* n2, int bandwidth, int latency) {
    RLink* link = (RLink*)malloc(sizeof(RLink));
    link->a = n1;
    link->b = n2;
    link->bandwidth = bandwidth;
    link->latency = latency;
    link->permissions = 0xFFFFFFFF; // default: all allowed

    net->links = realloc(net->links, (net->link_count + 1) * sizeof(RLink*));
    net->links[net->link_count++] = link;

    // Attach link to nodes
    n1->links = realloc(n1->links, (n1->link_count + 1) * sizeof(RLink*));
    n1->links[n1->link_count++] = link;

    n2->links = realloc(n2->links, (n2->link_count + 1) * sizeof(RLink*));
    n2->links[n2->link_count++] = link;

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
// Controller
// =====================
RController* create_controller(RNetwork* net, RoutePolicy policy) {
    RController* ctrl = (RController*)malloc(sizeof(RController));
    ctrl->network = net;
    ctrl->policy = policy;
    pthread_mutex_init(&ctrl->lock, NULL);
    return ctrl;
}

void destroy_controller(RController* ctrl) {
    pthread_mutex_destroy(&ctrl->lock);
    free(ctrl);
}

void set_policy(RController* ctrl, RoutePolicy policy) {
    pthread_mutex_lock(&ctrl->lock);
    ctrl->policy = policy;
    pthread_mutex_unlock(&ctrl->lock);
}

int send_packet(RController* ctrl, RNode* src, RNode* dst, int amount) {
    pthread_mutex_lock(&ctrl->lock);
    RPacket pkt = { .src = src, .dst = dst, .amount = amount, .type = 0, .priority = 0 };
    int result = route_packet(ctrl->network, src, dst, &pkt, ctrl->policy);
    pthread_mutex_unlock(&ctrl->lock);
    return result;
}

// =====================
// Routing (BFS shortest / widest)
// =====================
typedef struct QueueItem {
    RNode* node;
    RLink* prev;
    struct QueueItem* parent;
} QueueItem;

static int link_allowed(RLink* link, RoutePolicy policy) {
    return (link->permissions & (1 << policy)) != 0; // simple permission mask
}

int find_path_shortest(RNetwork* net, RNode* src, RNode* dst, RLink** path, int* plen) {
    typedef struct QueueItem {
        RNode* node;
        RLink* prev;
        struct QueueItem* parent;
    } QueueItem;

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

        for (int i = 0; i < cur->node->link_count; i++) {
            RLink* l = cur->node->links[i];
            RNode* next = (l->a == cur->node) ? l->b : l->a;

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

    if (!end) {
        free(q); free(visited);
        return 0;
    }

    // reconstruct path
    *plen = 0;
    for (QueueItem* it = end; it && it->prev; it = it->parent)
        path[(*plen)++] = it->prev;

    free(q);
    free(visited);
    return 1;
}


int find_path_widest(RNetwork* net, RNode* src, RNode* dst, RLink** path, int* plen) {
    int n = net->node_count;
    int* visited = calloc(n, sizeof(int));
    int* width = malloc(n * sizeof(int));    // max bandwidth to node
    RLink** prev_link = malloc(n * sizeof(RLink*));
    QueueItem** q = malloc(n * sizeof(QueueItem*));
    int qh = 0, qt = 0;

    for (int i = 0; i < n; i++) width[i] = 0, prev_link[i] = NULL;

    // Map nodes to indices
    int src_idx = -1, dst_idx = -1;
    for (int i = 0; i < n; i++) {
        if (net->nodes[i] == src) src_idx = i;
        if (net->nodes[i] == dst) dst_idx = i;
    }

    width[src_idx] = INT_MAX;
    QueueItem* start = malloc(sizeof(QueueItem));
    start->node = src; start->prev = NULL; start->parent = NULL;
    q[qt++] = start;

    while (qh < qt) {
        QueueItem* cur = q[qh++];
        int cur_idx = -1;
        for (int i = 0; i < n; i++) if (net->nodes[i] == cur->node) { cur_idx = i; break; }
        visited[cur_idx] = 1;

        for (int j = 0; j < net->link_count; j++) {
            RLink* l = net->links[j];
            RNode* next = (l->a == cur->node) ? l->b : ((l->b == cur->node) ? l->a : NULL);
            if (!next) continue;

            int next_idx = -1;
            for (int k = 0; k < n; k++) if (net->nodes[k] == next) { next_idx = k; break; }

            int bottleneck = (width[cur_idx] < l->bandwidth) ? width[cur_idx] : l->bandwidth;
            if (bottleneck > width[next_idx]) {
                width[next_idx] = bottleneck;
                prev_link[next_idx] = l;

                if (!visited[next_idx]) {
                    QueueItem* ni = malloc(sizeof(QueueItem));
                    ni->node = next; ni->prev = l; ni->parent = cur;
                    q[qt++] = ni;
                }
            }
        }
    }

    if (width[dst_idx] == 0) {
        free(q); free(visited); free(width); free(prev_link);
        return 0; // no path
    }

    // Reconstruct path
    int plen_local = 0;
    RNode* current = dst;
    while (current != src) {
        int idx = -1;
        for (int i = 0; i < n; i++) if (net->nodes[i] == current) { idx = i; break; }
        path[plen_local++] = prev_link[idx];
        current = (prev_link[idx]->a == current) ? prev_link[idx]->b : prev_link[idx]->a;
    }

    *plen = plen_local;

    free(q); free(visited); free(width); free(prev_link);
    return 1;
}
int route_packet(RNetwork* net, RNode* src, RNode* dst, RPacket* pkt, RoutePolicy policy) {
    if (src == dst) {
        printf("Source and destination are the same.\n");
        return 0;
    }

    RLink* path[256];
    int plen = 0;
    int found = 0;

    if (policy == POLICY_SHORTEST) {
        found = find_path_shortest(net, src, dst, path, &plen);
    } else if (policy == POLICY_WIDEST) {
        found = find_path_widest(net, src, dst, path, &plen);
    }

    if (!found) {
        printf("No route from %s to %s under current policy.\n", src->name, dst->name);
        return 0;
    }

    if (!reserve(src, pkt->amount)) {
        printf("Not enough resources at %s\n", src->name);
        return 0;
    }

    // Transfer packet along path
    RNode* current = src;
    for (int i = plen - 1; i >= 0; i--) {
        RLink* l = path[i];

        // Policy/permission check
        if ((l->permissions & (1 << policy)) == 0) {
            printf("Link [%s -> %s] forbidden under this policy.\n", l->a->name, l->b->name);
            release(src, pkt->amount);
            return 0;
        }

        RNode* next = (l->a == current) ? l->b : l->a;
        printf("[%s -> %s] Transferring %d units...\n", current->name, next->name, pkt->amount);
        usleep((pkt->amount * 1000000) / l->bandwidth + l->latency * 1000);
        printf("[%s] Received %d units!\n", next->name, pkt->amount);

        current = next;
    }

    release(src, pkt->amount);
    return 1;
}
