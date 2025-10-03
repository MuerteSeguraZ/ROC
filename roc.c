#include "include/roc.h"
#include <pthread.h>
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

RNode** discover(RNetwork* net, const char* type, int* out_count) {
    int count = 0;
    RNode** results = malloc(net->node_count * sizeof(RNode*));

    for (int i = 0; i < net->node_count; i++) {
        RNode* node = net->nodes[i];
        pthread_mutex_lock(&node->lock);
        if (strcmp(node->type, type) == 0 && node->available > 0) {
            results[count++] = node;
        }
        pthread_mutex_unlock(&node->lock);
    }

    *out_count = count;
    return results;
}

RNode* aggregate(RNode** nodes, int count, const char* name, const char* type) {
    int total_capacity = 0;
    int total_available = 0;

    for (int i = 0; i < count; i++) {
        total_capacity += nodes[i]->capacity;
        pthread_mutex_lock(&nodes[i]->lock);
        total_available += nodes[i]->available;
        pthread_mutex_unlock(&nodes[i]->lock);
    }

    RNode* agg = (RNode*)malloc(sizeof(RNode));
    strcpy(agg->name, name);
    strcpy(agg->type, type);
    agg->capacity = total_capacity;
    agg->available = total_available;
    pthread_mutex_init(&agg->lock, NULL);

    // Optionally, store pointers to constituent nodes for migration
    agg->links = NULL; // not used
    agg->link_count = 0;

    return agg;
}

RNode* slice_node(RNode* node, const char* name, int capacity) {
    if (capacity > node->available) {
        printf("Cannot slice %d units from %s (only %d available)\n",
               capacity, node->name, node->available);
        return NULL;
    }

    // Reserve the capacity in the original node
    if (!reserve(node, capacity)) return NULL;

    // Create a new node representing the slice
    RNode* slice = (RNode*)malloc(sizeof(RNode));
    strcpy(slice->name, name);
    strcpy(slice->type, node->type);
    slice->capacity = capacity;
    slice->available = capacity;
    pthread_mutex_init(&slice->lock, NULL);

    printf("Created slice %s with capacity %d\n", slice->name, slice->capacity);
    return slice;
}

int migrate(RPacket* pkt, RNode* from, RNode* to) {
    if (!reserve(to, pkt->amount)) {
        printf("Migration failed: target node %s has insufficient capacity.\n", to->name);
        return 0;
    }

    printf("Migrating %d units from %s -> %s...\n", pkt->amount, from->name, to->name);
    usleep(pkt->amount * 50000); // simulate transfer delay

    release(from, pkt->amount);
    printf("Migration complete.\n");
    return 1;
}

int migrate_timed(RNode* from, RNode* to, int amount, int timeout_ms) {
    int reserved = reserve_timed(from, amount, timeout_ms);
    if (!reserved) {
        printf("Migration failed: not enough resources on %s\n", from->name);
        return 0;
    }

    printf("Migrating %d units from %s -> %s (timed %d ms)...\n",
           amount, from->name, to->name, timeout_ms);

    // Simulate transfer delay proportional to amount
    usleep(amount * 50000); // arbitrary transfer time for demo
    release(from, amount);
    reserve(to, amount); // immediately add to destination

    printf("Migration complete.\n");
    return 1;
}

NodeStatus status(RNode* node) {
    pthread_mutex_lock(&node->lock);
    int avail = node->available;
    int cap = node->capacity;
    pthread_mutex_unlock(&node->lock);

    if (avail == cap) return STATUS_OK;
    else if (avail > cap / 2) return STATUS_BUSY;
    else return STATUS_OVERLOAD;
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
    link->enabled = 1;

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

void link_perm(RLink* link, unsigned int permissions) {
    if (!link) return;
    link->permissions = permissions;
}

unsigned int get_link_permissions(RLink* link) {
    if (!link) return 0;
    return link->permissions;
}

void set_link_bandwidth(RLink* link, int bandwidth) {
    if (!link) return;
    link->bandwidth = bandwidth;
}

int get_link_bandwidth(RLink* link) {
    return link->bandwidth;
}

void set_link_latency(RLink* link, int latency) {
    link->latency = latency;
}

int get_link_latency(RLink* link) {
    return link->latency;
}

void disable_link(RLink* link) {
    link->enabled = 0;
}

void enable_link(RLink* link) {
    link->enabled = 1;
}

int is_link_enabled(RLink* link) {
    return link->enabled;
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

int remove_node(RNetwork* net, RNode* node) {
    int idx = -1;
    for (int i = 0; i < net->node_count; i++) {
        if (net->nodes[i] == node) {
            idx = i;
            break;
        }
    }
    if (idx == -1) return 0;

    // Remove links involving this node
    for (int i = 0; i < net->link_count;) {
        if (net->links[i]->a == node || net->links[i]->b == node) {
            destroy_link(net->links[i]);
            for (int j = i; j < net->link_count - 1; j++) {
                net->links[j] = net->links[j + 1];
            }
            net->link_count--;
        } else {
            i++;
        }
    }

    // Remove node from list
    for (int i = idx; i < net->node_count - 1; i++) {
        net->nodes[i] = net->nodes[i + 1];
    }
    net->node_count--;
    return 1;
}

RNode* find_node(RNetwork* net, const char* name) {
    for (int i = 0; i < net->node_count; i++) {
        if (strcmp(net->nodes[i]->name, name) == 0)
            return net->nodes[i];
    }
    return NULL;
}

int count_nodes(RNetwork* net) {
    return net->node_count;
}

void list_nodes(RNetwork* net) {
    printf("Nodes in network (%d):\n", net->node_count);
    for (int i = 0; i < net->node_count; i++) {
        printf(" - %s (%s, %d/%d)\n",
               net->nodes[i]->name,
               net->nodes[i]->type,
               net->nodes[i]->available,
               net->nodes[i]->capacity);
    }
}

int count_links(RNetwork* net) {
    return net->link_count;
}

void list_links(RNetwork* net) {
    printf("Links in network (%d):\n", net->link_count);
    for (int i = 0; i < net->link_count; i++) {
        RLink* l = net->links[i];
        printf(" - [%s <-> %s] bw=%d, lat=%d, enabled=%d\n",
               l->a->name, l->b->name, l->bandwidth, l->latency, l->enabled);
    }
}

int connect_nodes(RNetwork* net, const char* name1, const char* name2, int bandwidth, int latency) {
    RNode* n1 = find_node(net, name1);
    RNode* n2 = find_node(net, name2);
    if (!n1 || !n2) return 0;
    create_link(net, n1, n2, bandwidth, latency);
    return 1;
}

int disconnect_nodes(RNetwork* net, const char* name1, const char* name2) {
    for (int i = 0; i < net->link_count; i++) {
        RLink* l = net->links[i];
        if ((strcmp(l->a->name, name1) == 0 && strcmp(l->b->name, name2) == 0) ||
            (strcmp(l->a->name, name2) == 0 && strcmp(l->b->name, name1) == 0)) {
            destroy_link(l);
            for (int j = i; j < net->link_count - 1; j++) {
                net->links[j] = net->links[j + 1];
            }
            net->link_count--;
            return 1;
        }
    }
    return 0;
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

int send_packet_timed(RController* ctrl, RNode* src, RNode* dst, int amount, int timeout_ms) {
    pthread_mutex_lock(&ctrl->lock);

    RPacket pkt = { .amount = amount };
    int reserved = reserve_timed(src, amount, timeout_ms);
    if (!reserved) {
        printf("Failed to reserve %d units on %s\n", amount, src->name);
        pthread_mutex_unlock(&ctrl->lock);
        return 0;
    }

    // Route packet normally
    int result = route_packet(ctrl->network, src, dst, &pkt, ctrl->policy);

    pthread_mutex_unlock(&ctrl->lock);
    return result;
}

// =====================
// Routing (BFS shortest / widest)
// =====================
static int link_allowed(RLink* link, RoutePolicy policy) {
    return (link->permissions & (1 << policy)) != 0; // simple permission mask
}

int find_path_shortest(RNetwork* net, RNode* src, RNode* dst, RLink** path, int* plen) {

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

        // Check if link is enabled
        if (!l->enabled) {
            printf("Link [%s -> %s] is disabled.\n", l->a->name, l->b->name);
            release(src, pkt->amount);
            return 0;
        }

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

// =====================
// Timing stuff
// =====================

static void* timed_release_thread(void* arg) {
    TimedReserveArgs* args = (TimedReserveArgs*)arg;
    usleep(args->timeout_ms * 1000); // sleep in microseconds
    release(args->node, args->amount);
    free(args);
    return NULL;
}

int reserve_timed(RNode* node, int amount, int timeout_ms) {
    if (!reserve(node, amount)) return 0;

    // Start a detached thread to auto-release
    pthread_t tid;
    TimedReserveArgs* args = malloc(sizeof(TimedReserveArgs));
    args->node = node;
    args->amount = amount;
    args->timeout_ms = timeout_ms;

    pthread_create(&tid, NULL, timed_release_thread, args);
    pthread_detach(tid);

    return 1;
}
