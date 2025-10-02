#ifndef ROC_H
#define ROC_H

#include <pthread.h>

// =====================
// Resource Node
// =====================
typedef struct RNode {
    char name[50];
    char type[20];
    int capacity;
    int available;
    pthread_mutex_t lock;
} RNode;

// =====================
// Resource Packet
// =====================
typedef struct RPacket {
    int amount;
} RPacket;

// =====================
// Resource Link
// =====================
typedef struct RLink {
    RNode* a;
    RNode* b;
    int bandwidth;
} RLink;

// =====================
// Resource Network
// =====================
typedef struct RNetwork {
    RNode** nodes;
    int node_count;
    RLink** links;
    int link_count;
} RNetwork;

// Routing policy
typedef enum {
    POLICY_SHORTEST,
    POLICY_WIDEST
} RoutePolicy;

// Node management
RNode* create_node(const char* name, const char* type, int capacity);
void destroy_node(RNode* node);

// Resource operations
int reserve(RNode* node, int amount);
void release(RNode* node, int amount);
int monitor(RNode* node);

// Link management
RLink* create_link(RNetwork* net, RNode* n1, RNode* n2, int bandwidth);
void destroy_link(RLink* link);

// Network management
RNetwork* create_network();
void add_node(RNetwork* net, RNode* node);
void destroy_network(RNetwork* net);

// Routing
void route_packet(RNetwork* net, RNode* src, RNode* dst, RPacket* pkt, RoutePolicy policy);

#endif
