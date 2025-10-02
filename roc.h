#ifndef ROC_H
#define ROC_H

#include <pthread.h>

// Forward declarations
struct RNode;
struct RLink;

// Resource Node
typedef struct RNode {
    char name[50];
    char type[20];
    int capacity;
    int available;

    struct RLink** links;
    int link_count;

    pthread_mutex_t lock;
} RNode;

// Resource Packet
typedef struct RPacket {
    int amount;
    RNode* src;
    RNode* dst;
} RPacket;

// Resource Link
typedef struct RLink {
    RNode* from;
    RNode* to;
    int bandwidth; // units per second
} RLink;

// Resource Network
typedef struct RNetwork {
    RNode** nodes;
    int node_count;
    RLink** links;
    int link_count;
} RNetwork;

// Node management
RNode* create_node(const char* name, const char* type, int capacity);
void destroy_node(RNode* node);

// Resource operations
int reserve(RNode* node, int amount);
void release(RNode* node, int amount);
int monitor(RNode* node);

// Link operations
RLink* create_link(RNode* n1, RNode* n2, int bandwidth);
void destroy_link(RLink* link);

// Transfer with routing
void transfer_packet(RNetwork* net, RPacket* packet);

// Network operations
RNetwork* create_network();
void add_node(RNetwork* net, RNode* node);
void add_link(RNetwork* net, RLink* link);
void destroy_network(RNetwork* net);

#endif
