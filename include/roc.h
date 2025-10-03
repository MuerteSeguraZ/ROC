#ifndef ROC_H
#define ROC_H

#include <pthread.h>

typedef enum { NODE_CPU, NODE_GPU, NODE_MEMORY, NODE_STORAGE } NodeType;

// =====================
// Resource Node
// =====================
typedef struct RNode {
    char name[50];
    char type[20];        // CPU, GPU, Memory, Storage…
    int capacity;         // total units
    int available;        // free units
    int state;            // 0=offline, 1=online, 2=busy, etc.
    pthread_mutex_t lock; // for thread-safe access

    struct RLink** links; // connected links
    int link_count;

    void* metadata;       // optional user-defined data
} RNode;

// =====================
// Resource Packet
// =====================
typedef struct RPacket {
    RNode* src;
    RNode* dst;
    int amount;
    int type;       // optional: CPU request, memory allocation…
    int priority;   // optional scheduling priority
} RPacket;

// =====================
// Resource Link
// =====================
typedef struct RLink {
    RNode* a;
    RNode* b;
    int bandwidth;  // units/sec
    int latency;    // milliseconds
    unsigned int permissions;
    int enabled;
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

// =====================
// Queue
// =====================
typedef struct QueueItem {
    RNode* node;
    RLink* prev;
    struct QueueItem* parent;
} QueueItem;

// =====================
// Timing struct
// =====================
typedef struct TimedReserveArgs {
    RNode* node;
    int amount;
    int timeout_ms;
} TimedReserveArgs;

// =====================
// Routing policy
// =====================
typedef enum {
    POLICY_SHORTEST,
    POLICY_WIDEST
} RoutePolicy;

// Forward declarations for policy-based pathfinding
int find_path_shortest(RNetwork* net, RNode* src, RNode* dst, RLink** path, int* plen);
int find_path_widest(RNetwork* net, RNode* src, RNode* dst, RLink** path, int* plen);

// =====================
// Node management
// =====================
RNode* create_node(const char* name, const char* type, int capacity);
void destroy_node(RNode* node);

// =====================
// Resource operations
// =====================
typedef enum { STATUS_OK, STATUS_BUSY, STATUS_OVERLOAD } NodeStatus;

int reserve(RNode* node, int amount);
void release(RNode* node, int amount);
int monitor(RNode* node);
int migrate(RPacket* pkt, RNode* from, RNode* to);
int migrate_timed(RNode* from, RNode* to, int amount, int timeout_ms);
int reserve_timed(RNode* node, int amount, int timeout_ms);
RNode* aggregate(RNode** nodes, int count, const char* name, const char* type);
RNode* slice_node(RNode* node, const char* name, int capacity);
RNode** discover(RNetwork* net, const char* type, int* out_count);
NodeStatus status(RNode* node);

// =====================
// Link management
// =====================
RLink* create_link(RNetwork* net, RNode* n1, RNode* n2, int bandwidth, int latency);
void destroy_link(RLink* link);
void link_perm(RLink* link, unsigned int permissions);
unsigned int get_link_permissions(RLink* link);
void set_link_bandwidth(RLink* link, int bandwidth);
int get_link_bandwidth(RLink* link);
void set_link_latency(RLink* link, int latency);
int get_link_latency(RLink* link);
void disable_link(RLink* link);
void enable_link(RLink* link);
int is_link_enabled(RLink* link);

// =====================
// Network management
// =====================
RNetwork* create_network();
void add_node(RNetwork* net, RNode* node);
void destroy_network(RNetwork* net);
int remove_node(RNetwork* net, RNode* node);
RNode* find_node(RNetwork* net, const char* name);
int count_nodes(RNetwork* net);
void list_nodes(RNetwork* net);
int count_links(RNetwork* net);
void list_links(RNetwork* net);
int connect_nodes(RNetwork* net, const char* name1, const char* name2, int bandwidth, int latency);
int disconnect_nodes(RNetwork* net, const char* name1, const char* name2);

// =====================
// Routing / transfer
// =====================
int route_packet(RNetwork* net, RNode* src, RNode* dst, RPacket* pkt, RoutePolicy policy);

// =====================
// RController
// =====================
typedef struct RController {
    RNetwork* network;
    RoutePolicy policy;       // Default routing policy
    pthread_mutex_t lock;     // For thread-safe operations
} RController;

// Controller operations
RController* create_controller(RNetwork* net, RoutePolicy policy);
void destroy_controller(RController* ctrl);

// Sends a packet from src -> dst using controller's routing policy
int send_packet(RController* ctrl, RNode* src, RNode* dst, int amount);
int send_packet_timed(RController* ctrl, RNode* src, RNode* dst, int amount, int timeout_ms);

// Change the controller's default policy
void set_policy(RController* ctrl, RoutePolicy policy);

#endif
