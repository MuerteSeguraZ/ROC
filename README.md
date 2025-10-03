---

# ROC – Resource Orchestrator & Scheduler

**ROC** is a lightweight C-based framework for simulating and managing computational resources, tasks, and resource-aware scheduling. It allows you to create a network of nodes (CPU, GPU, memory, storage), connect them via links, send "packets" of resources, and schedule tasks that consume these resources in a thread-safe manner.

---

## Table of Contents

1. [Project Structure](#project-structure)
2. [Core Concepts](#core-concepts)
3. [Nodes (`roc.h` / `roc.c`)](#nodes)
4. [Links (`roc.h` / `roc.c`)](#links)
5. [Network (`roc.h` / `roc.c`)](#network)
6. [Controller (`roc.h` / `roc.c`)](#controller)
7. [Packets & Routing](#packets--routing)
8. [Tasks (`roc_task.h` / `roc_task.c`)](#tasks)
9. [Scheduler (`roc_scheduler.h` / `roc_scheduler.c`)](#scheduler)
10. [Example Usage](#example-usage)

---

## Project Structure

```
ROC/
├── md/
├── include/
│   ├── roc.h            # Core types and function declarations
│   ├── roc_task.h       # Task type and functions
│   └── roc_scheduler.h  # Scheduler types and functions
├── roc.c                # Node, link, network, controller, and routing implementations
├── roc_task.c           # Task creation, execution, and resource allocation
├── roc_scheduler.c      # Scheduler for running tasks asynchronously
└── main.c               # Demo usage of nodes, tasks, and scheduler
```

---

## Core Concepts

* **RNode** – Represents a resource node: CPU, GPU, memory, or storage. Each node has capacity, availability, state, and connected links.
* **RLink** – Represents a network link between two nodes with bandwidth, latency, and permissions.
* **RNetwork** – A collection of nodes and links forming a resource network.
* **RController** – Manages packet transfers between nodes using routing policies (shortest or widest).
* **RPacket** – Represents a "resource transfer" between nodes.
* **RTask** – Represents a computational task that requires resources.
* **RTaskScheduler** – Manages tasks and executes them asynchronously while respecting resource availability.

---

## Nodes

Nodes are the building blocks of the network.

**Key Functions:**

```c
RNode* create_node(const char* name, const char* type, int capacity);
void destroy_node(RNode* node);
int reserve(RNode* node, int amount);   // Reserve resource units
void release(RNode* node, int amount);  // Release resource units
int monitor(RNode* node);               // Get available units
RNode* aggregate(RNode** nodes, int count, const char* name, const char* type);
RNode* slice_node(RNode* node, const char* name, int capacity);
NodeStatus status(RNode* node);        // STATUS_OK, STATUS_BUSY, STATUS_OVERLOAD
```

---

## Links

Links connect nodes and simulate network constraints.

**Key Functions:**

```c
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
```

---

## Network

A network manages nodes and links together.

**Key Functions:**

```c
RNetwork* create_network();
void add_node(RNetwork* net, RNode* node);
int remove_node(RNetwork* net, RNode* node);
RNode* find_node(RNetwork* net, const char* name);
int count_nodes(RNetwork* net);
void list_nodes(RNetwork* net);
int count_links(RNetwork* net);
void list_links(RNetwork* net);
int connect_nodes(RNetwork* net, const char* name1, const char* name2, int bandwidth, int latency);
int disconnect_nodes(RNetwork* net, const char* name1, const char* name2);
void destroy_network(RNetwork* net);
```

---

## Controller

Controllers manage packet transfers in the network with routing policies.

**Key Functions:**

```c
RController* create_controller(RNetwork* net, RoutePolicy policy);
void destroy_controller(RController* ctrl);
void set_policy(RController* ctrl, RoutePolicy policy);
int send_packet(RController* ctrl, RNode* src, RNode* dst, int amount);
int send_packet_timed(RController* ctrl, RNode* src, RNode* dst, int amount, int timeout_ms);
```

**Routing Policies:**

* `POLICY_SHORTEST` – finds the shortest path
* `POLICY_WIDEST` – finds the path with the maximum bandwidth

---

## Packets & Routing

Packets (`RPacket`) represent resource transfers. Routing functions simulate sending units across the network respecting link bandwidth, latency, and permissions:

```c
int route_packet(RNetwork* net, RNode* src, RNode* dst, RPacket* pkt, RoutePolicy policy);
int find_path_shortest(RNetwork* net, RNode* src, RNode* dst, RLink** path, int* plen);
int find_path_widest(RNetwork* net, RNode* src, RNode* dst, RLink** path, int* plen);
int migrate(RPacket* pkt, RNode* from, RNode* to);
int migrate_timed(RNode* from, RNode* to, int amount, int timeout_ms);
int reserve_timed(RNode* node, int amount, int timeout_ms);
```

---

## Tasks

Tasks (`RTask`) represent jobs that consume resources. Each task can require multiple nodes/resources.

**Key Functions:**

```c
RTask* create_task(const char* name, int priority);
void destroy_task(RTask* task);
int add_resource_req(RTask* task, RNode* node, int amount);
int remove_resource_req(RTask* task, int index);
int allocate_task(RTask* task);      // Reserve all resources
void release_task(RTask* task);      // Release all resources
int run_task(RTask* task);           // Execute task asynchronously
int run_task_async(RTask* task);     // Run task in a detached thread
TaskStatus task_status(RTask* task); // TASK_PENDING, TASK_RUNNING, TASK_COMPLETED, TASK_FAILED
```

---

## Scheduler

The scheduler manages multiple tasks, running them asynchronously and allocating resources dynamically.

* Tasks are executed in parallel threads using `pthread`.
* Scheduler ensures tasks only run when required resources are available.
* Task completion automatically releases reserved resources.

---

## Example Usage

```c
#include "include/roc.h"
#include "include/roc_task.h"
#include "include/roc_scheduler.h"
#include <stdio.h>
#include <unistd.h>

int main() {
    printf("=== ROC Task Scheduler Demo ===\n");

    // --- Create nodes properly ---
    RNode* cpu1 = create_node("CPU1", "CPU", 8);
    RNode* cpu2 = create_node("CPU2", "CPU", 4);
    RNode* gpu1 = create_node("GPU1", "GPU", 2);

    printf("Nodes in network (3):\n");
    printf(" - %s (CPU, %d/%d)\n", cpu1->name, cpu1->available, cpu1->capacity);
    printf(" - %s (CPU, %d/%d)\n", cpu2->name, cpu2->available, cpu2->capacity);
    printf(" - %s (GPU, %d/%d)\n", gpu1->name, gpu1->available, gpu1->capacity);

    // --- Create scheduler ---
    RTaskScheduler* sched = create_scheduler();
    scheduler_start(sched);

    // --- Create tasks ---
    RTask* t1 = create_task("Task-Heavy", 10);
    add_resource_req(t1, cpu1, 6);

    RTask* t2 = create_task("Task-Medium", 5);
    add_resource_req(t2, cpu2, 3);
    add_resource_req(t2, gpu1, 1);

    RTask* t3 = create_task("Task-Light", 1);
    add_resource_req(t3, cpu1, 2);

    // --- Add tasks to scheduler ---
    scheduler_add_task(sched, t1);
    scheduler_add_task(sched, t2);
    scheduler_add_task(sched, t3);

    printf("[Main] Tasks added to scheduler.\n");

    // Wait for tasks to complete
    int done = 0;
    while (!done) {
        done = 1;
        if (task_status(t1) != TASK_COMPLETED) done = 0;
        if (task_status(t2) != TASK_COMPLETED) done = 0;
        if (task_status(t3) != TASK_COMPLETED) done = 0;
        usleep(50000);
    }

    // Cleanup
    destroy_task(t1);
    destroy_task(t2);
    destroy_task(t3);
    scheduler_stop(sched);
    destroy_scheduler(sched);

    // Destroy nodes
    destroy_node(cpu1);
    destroy_node(cpu2);
    destroy_node(gpu1);

    printf("[Main] All tasks completed.\n");
    return 0;
}
```

---

## Notes

* All operations on nodes, tasks, and controllers are **thread-safe**.
* Tasks simulate "real" work proportional to resource units.
* Network routing supports **shortest-path** and **widest-path** policies.
* Nodes can be **aggregated** or **sliced** to model virtualized resources.
* `reserve_timed` allows automatic resource release after a timeout.

---
