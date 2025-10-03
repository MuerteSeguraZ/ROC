
# ROC - Resource Orchestrator & Scheduler

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
10. [Jobs & Job Queue (`roc_job.h` / `roc_job.c`, `roc_job_queue.h` / `roc_job_queue.c`)](#jobs--job-queue)
11. [Workflows & Workflow Queue (`roc_workflow.h` / `roc_workflow.c` / `roc_workflow_queue.h` / `roc_workflow_queue.c`)](#workflows--workflow-queue)
12. [Pipes & Pipe Queue (`roc_pipe.h` / `roc_pipe.c` / `roc_pipe_queue.h` / `roc_pipe_queue.c`)](#pipes--pipe-queue)
13. [Stages & Stage Queue (`roc_stage.h` / `roc_stage.c` / `roc_stage_queue.h` / `roc_stage_queue.c`)](#stages--stage-queue)
14. [Phases & Phase Queue (`roc_phase.h` / `roc_phase.c` / `roc_phase_queue.h` / `roc_phase_queue.c`)](#phases--phase-queue)
15. [Bundles & Bundle Queue (`roc_bundle.h` / `roc_bundle.c` `roc_bundle_queue.h` / `roc_bundle_queue.c`)](#bundles--bundle-queue)
16. [Campaigns & Campaign Queue (`roc_campaign.h` / `roc_campaign.c` / `roc_campaign_queue.h` / `roc_campaign_queue.c`)](#campaigns--campaign-queue)
17. [Example Usage](#example-usage)

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

## Jobs & Job Queue

Jobs represent a **collection of tasks** that can be scheduled together. Each job has a priority and can track dependencies between its tasks. The job queue allows **priority-based scheduling** of multiple jobs.

**Key Types:**

```c
typedef struct {
    char name[50];
    RTask* tasks[MAX_TASKS_PER_JOB];
    int task_count;
    int priority;           // job-level priority
    JobStatus status;       // JOB_PENDING, JOB_RUNNING, JOB_COMPLETED, JOB_FAILED
    pthread_mutex_t lock;
    int dep_matrix[MAX_TASKS_PER_JOB][MAX_TASKS_PER_JOB]; 
    // dep_matrix[i][j] = 1 means task i depends on task j
} RJob;
```

**Job Functions:**

```c
RJob* create_job(const char* name, int priority);
void destroy_job(RJob* job);

int job_add_task(RJob* job, RTask* task);
int job_run(RJob* job, RTaskScheduler* sched);  // Enqueue all tasks
JobStatus job_status(RJob* job);                // Check current job status
```

**Job Queue Functions:**

```c
typedef struct {
    RJob* jobs[MAX_JOBS];
    int job_count;
    pthread_mutex_t lock;
} RJobQueue;

RJobQueue* create_job_queue();
void destroy_job_queue(RJobQueue* queue);

int enqueue_job(RJobQueue* queue, RJob* job);
RJob* dequeue_job(RJobQueue* queue);          // Dequeues highest-priority job
int process_job_queue(RJobQueue* queue, RTaskScheduler* sched); // Process all jobs
```

**Behavior:**

* Jobs are executed **task by task**, respecting task dependencies.
* Jobs can have a **priority**, so higher-priority jobs run before lower-priority ones.
* Job queue allows multiple jobs to be enqueued and processed sequentially or based on priority.
* Task completion automatically updates job status.

---

### Example Usage: Jobs & Job Queue

```c
#include "roc.h"
#include "roc_task.h"
#include "roc_scheduler.h"
#include "roc_job.h"
#include "roc_job_queue.h"
#include <stdio.h>
#include <unistd.h>

int main() {
    printf("=== ROC Job Queue Demo ===\n");

    // --- Create nodes ---
    RNode* cpu = create_node("CPU", "CPU", 4);

    // --- Create scheduler ---
    RTaskScheduler* sched = create_scheduler();
    scheduler_start(sched);

    // --- Create jobs ---
    RJob* job1 = create_job("Job-Alpha", 10);
    RTask* t1 = create_task("Task-A1", 5);
    add_resource_req(t1, cpu, 2);
    job_add_task(job1, t1);

    RJob* job2 = create_job("Job-Beta", 20);
    RTask* t2 = create_task("Task-B1", 5);
    add_resource_req(t2, cpu, 2);
    job_add_task(job2, t2);

    // --- Create job queue ---
    RJobQueue* queue = create_job_queue();
    enqueue_job(queue, job1);
    enqueue_job(queue, job2);

    // --- Process job queue ---
    process_job_queue(queue, sched);

    // Wait for jobs to complete
    while (job_status(job1) != JOB_COMPLETED || job_status(job2) != JOB_COMPLETED) {
        usleep(50000);
    }

    printf("[Main] All jobs processed.\n");

    // Cleanup
    destroy_job(job1);
    destroy_job(job2);
    destroy_job_queue(queue);
    scheduler_stop(sched);
    destroy_scheduler(sched);
    destroy_node(cpu);

    return 0;
}
```

**Output Example:**

```
[Main] Starting job 'Job-Beta'
Running task 'Task-B1' using 2 resource units...
Task 'Task-B1' completed.
[Main] Job 'Job-Beta' completed.
[Main] Starting job 'Job-Alpha'
Running task 'Task-A1' using 2 resource units...
Task 'Task-A1' completed.
[Main] Job 'Job-Alpha' completed.
[Main] All jobs processed.
```

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

## Workflows & Workflow Queue

Workflows represent a sequence of tasks that can be executed together as a logical unit. Unlike jobs, workflows can contain **dependent tasks** and **parallel task execution**. Workflow-level priorities allow you to control which workflows run first when multiple workflows compete for resources.

**Key Features:**
* Workflow-level priority
* Parallel and sequential task execution
* Thread-safe operations
* Workflow queues for ordering multiple workflows

**Key Functions:**

```c
RWorkflow* create_workflow(const char* name, int priority);
void destroy_workflow(RWorkflow* wf);

int workflow_add_task(RWorkflow* wf, RTask* task);
int workflow_run(RWorkflow* wf, RTaskScheduler* sched);
WorkflowStatus workflow_status(RWorkflow* wf);
```

**Example Usage:**
```c
RWorkflow* wf1 = create_workflow("Workflow-Alpha", 10);
RTask* t1 = create_task("Task-1", 5);
add_resource_req(t1, cpu, 2);
workflow_add_task(wf1, t1);

RWorkflow* wf2 = create_workflow("Workflow-Beta", 5);
RTask* t2 = create_task("Task-2", 3);
add_resource_req(t2, gpu, 1);
workflow_add_task(wf2, t2);

// Run workflows in parallel
workflow_run(wf1, sched);
workflow_run(wf2, sched);
```

---

## Pipes & Pipe Queue
Pipes allow you to define **sequential or dependent task streams** where the output of one task can trigger the next. Like workflows, pipes support priorities and can be enqueued in a **pipe queue**.

**Key Features:**

* Pipe-level priority
* Sequential task execution
* Can run multiple pipes in parallel or in a queue
* Thread-safe operations

**Key Functions:**

```c
RPipe* create_pipe(const char* name, int priority);
void destroy_pipe(RPipe* pipe);

int pipe_add_task(RPipe* pipe, RTask* task);
int pipe_run(RPipe* pipe, RTaskScheduler* sched);
PipeStatus pipe_status(RPipe* pipe);

// Pipe queues
RPipeQueue* create_pipe_queue(const char* name);
void destroy_pipe_queue(RPipeQueue* queue);
int pipe_queue_add(RPipeQueue* queue, RPipe* pipe);
int pipe_queue_run(RPipeQueue* queue, RTaskScheduler* sched);
```

**Example Usage:**
```c
RPipe* p1 = create_pipe("Pipe-Alpha", 5);
RPipe* p2 = create_pipe("Pipe-Beta", 10);

RTask* t1 = create_task("Task-1", 2);
RTask* t2 = create_task("Task-2", 1);
pipe_add_task(p1, t1);
pipe_add_task(p1, t2);

RPipeQueue* queue = create_pipe_queue("MainQueue");
pipe_queue_add(queue, p1);
pipe_queue_add(queue, p2);

// Run queue
pipe_queue_run(queue, sched);
```

**Notes:**
* Pipes queues respect pipe priorities.
* Tasks within a pipe execute sequentially, but multiple pipes can run in parallel depending on available resources.
* Useful for modeling pipelines of dependent tasks with resource constraints.

---

## Stages & Stage Queue

Stages allow you to group multiple jobs and/or workflows together, so they can be executed as a single unit. Stages can have a priority level and support both sequential and parallel execution of their contained items.

**Key Concepts:**

* **RStage** – Represents a collection of jobs and workflows executed together.
* **StageItem** – Can be either a job or a workflow inside a stage.
* **RStageQueue** – A queue of stages executed in order, respecting their priorities.

**Key Functions:**

```c
// Stage operations
RStage* create_stage(const char* name, int priority);
void destroy_stage(RStage* stage);
int stage_add_item(RStage* stage, void* item, StageItemType type);
int stage_run(RStage* stage, RTaskScheduler* sched);
StageStatus stage_status(RStage* stage);

// Stage queue operations
RStageQueue* create_stage_queue(const char* name);
void destroy_stage_queue(RStageQueue* queue);
int stage_queue_add(RStageQueue* queue, RStage* stage);
int stage_queue_run(RStageQueue* queue, RTaskScheduler* sched);
StageQueueStatus stage_queue_status(RStageQueue* queue);
```

**Usage Example:**
```c
RStage* stage = create_stage("Stage-Alpha", 10);
stage_add_item(stage, job1, STAGE_ITEM_JOB);
stage_add_item(stage, wf1, STAGE_ITEM_WORKFLOW);
stage_run(stage, scheduler);

RStageQueue* queue = create_stage_queue("MainQueue");
stage_queue_add(queue, stage1);
stage_queue_add(queue, stage2);
stage_queue_run(queue, scheduler);
```

**Notes:**
* Stages are thread-safe.
* Supports mixed contents: jobs and workflows in the same stage.
* Stage queues ensure sequential execution of multiple stages.
* Stage and stage queue statuses allow monitoring of completion or failure.

---

## Phases & Phase Queue

Phases are high-level orchestration units that group multiple **stages**. Each phase can contain several stages, and you can assign **priority** to phases for ordered execution. Phases can also include tasks directly if needed.

Phase Queues allow you to run multiple phases sequentially or based on priority, similar to stage queues or workflow queues.

---

## Phase Concepts

* **RPhase** – Represents a phase containing multiple stages or tasks.
* **PhaseStatus** – Status of a phase:
  * `PHASE_PENDING`
  * `PHASE_RUNNING`
  * `PHASE_COMPLETED`
  * `PHASE_FAILED`

---

**Key Functions**

```c
// Phase creation and destruction
RPhase* create_phase(const char* name, int priority);
void destroy_phase(RPhase* phase);

// Add tasks or stages
int phase_add_task(RPhase* phase, RTask* task);
int phase_add_stage(RPhase* phase, RStage* stage);

// Run phase (requires scheduler)
int phase_run(RPhase* phase, RTaskScheduler* sched);

// Check status
PhaseStatus phase_status(RPhase* phase);
```

## Phase Queue

A Phase Queue executes multiple phases in order, respecting their priority. You can run phases sequentially or concurrently depending on the scheduler.

```c
typedef struct {
    char name[50];
    RPhase* phases[32];
    int phase_count;
} RPhaseQueue;

// Queue operations
RPhaseQueue* create_phase_queue(const char* name);
void destroy_phase_queue(RPhaseQueue* queue);
int phase_queue_add(RPhaseQueue* queue, RPhase* phase);
int phase_queue_run(RPhaseQueue* queue, RTaskScheduler* sched);  // Runs phases respecting priority
```

**Example Usage:**
```c
#include "roc_phase.h"
#include "roc_phase_queue.h"
#include "roc_scheduler.h"
#include "roc_task.h"
#include <stdio.h>

int main() {
    RTaskScheduler* sched = create_scheduler();
    scheduler_start(sched);

    // --- Create phases ---
    RPhase* phase1 = create_phase("Phase-Prime", 10);
    RPhase* phase2 = create_phase("Phase-Secondary", 5);

    // --- Add tasks to phases ---
    RTask* t1 = create_task("Task-1", 5);
    RTask* t2 = create_task("Task-2", 3);
    phase_add_task(phase1, t1);
    phase_add_task(phase2, t2);

    // --- Create and run phase queue ---
    RPhaseQueue* pq = create_phase_queue("MainPhaseQueue");
    phase_queue_add(pq, phase1);
    phase_queue_add(pq, phase2);
    phase_queue_run(pq, sched);

    // Cleanup
    destroy_task(t1);
    destroy_task(t2);
    destroy_phase(phase1);
    destroy_phase(phase2);
    destroy_phase_queue(pq);
    scheduler_stop(sched);
    destroy_scheduler(sched);

    printf("[Main] All phases completed\n");
    return 0;
}
```

---

Here’s the section for **Phases & Phase Queue** fully formatted and consistent with your README style. I polished it to include concepts, functions, statuses, and examples while matching the style of the rest of your document:

---

## Phases & Phase Queue

Phases are high-level orchestration units that group multiple **stages** or tasks together. Each phase can have a **priority**, and multiple phases can be executed sequentially or based on priority.

Phase Queues allow you to enqueue multiple phases and execute them in order while respecting priorities.

---

### Phase Concepts

* **RPhase** – Represents a phase containing multiple stages and/or tasks.
* **PhaseStatus** – Status of a phase:

  * `PHASE_PENDING`
  * `PHASE_RUNNING`
  * `PHASE_COMPLETED`
  * `PHASE_FAILED`

*Phases can contain:*

* Individual tasks (`RTask`)
* Entire stages (`RStage`)

---

### Key Functions

```c
// Phase operations
RPhase* create_phase(const char* name, int priority);
void destroy_phase(RPhase* phase);

int phase_add_task(RPhase* phase, RTask* task);
int phase_add_stage(RPhase* phase, RStage* stage);

int phase_run(RPhase* phase, RTaskScheduler* sched);
PhaseStatus phase_status(RPhase* phase);

// Phase queue operations
RPhaseQueue* create_phase_queue(const char* name);
void destroy_phase_queue(RPhaseQueue* queue);

int phase_queue_add(RPhaseQueue* queue, RPhase* phase);
int phase_queue_run(RPhaseQueue* queue, RTaskScheduler* sched);
PhaseQueueStatus phase_queue_status(RPhaseQueue* queue);
```

---

### Example Usage

```c
#include "roc_phase.h"
#include "roc_phase_queue.h"
#include "roc_scheduler.h"
#include "roc_task.h"
#include <stdio.h>

int main() {
    // Create scheduler
    RTaskScheduler* sched = create_scheduler();
    scheduler_start(sched);

    // --- Create phases ---
    RPhase* phase1 = create_phase("Phase-Prime", 10);
    RPhase* phase2 = create_phase("Phase-Secondary", 5);

    // --- Add tasks to phases ---
    RTask* t1 = create_task("Task-1", 5);
    RTask* t2 = create_task("Task-2", 3);
    phase_add_task(phase1, t1);
    phase_add_task(phase2, t2);

    // --- Create and run phase queue ---
    RPhaseQueue* pq = create_phase_queue("MainPhaseQueue");
    phase_queue_add(pq, phase1);
    phase_queue_add(pq, phase2);
    phase_queue_run(pq, sched);

    // Cleanup
    destroy_task(t1);
    destroy_task(t2);
    destroy_phase(phase1);
    destroy_phase(phase2);
    destroy_phase_queue(pq);
    scheduler_stop(sched);
    destroy_scheduler(sched);

    printf("[Main] All phases completed\n");
    return 0;
}
```

---

## Bundles & Bundle Queue

Bundles are **collections of tasks, jobs, or workflows** that can be executed together as a single unit. Bundles provide a higher-level abstraction for managing related work and can be executed **sequentially or in parallel** depending on the scheduler.

Bundle Queues allow you to enqueue multiple bundles and process them in **priority order** or sequentially.

---

### Bundle Concepts

* **RBundle** – Represents a collection of tasks, jobs, or workflows.
* **BundleStatus** – Status of a bundle:

  * `BUNDLE_PENDING`
  * `BUNDLE_RUNNING`
  * `BUNDLE_COMPLETED`
  * `BUNDLE_FAILED`

*Bundles can contain:*

* Tasks (`RTask`)
* Jobs (`RJob`)
* Workflows (`RWorkflow`)

---

### Key Functions

```c
// Bundle operations
RBundle* create_bundle(const char* name, int priority);
void destroy_bundle(RBundle* bundle);

int bundle_add_task(RBundle* bundle, RTask* task);
int bundle_add_job(RBundle* bundle, RJob* job);
int bundle_add_workflow(RBundle* bundle, RWorkflow* wf);

int bundle_run(RBundle* bundle, RTaskScheduler* sched);
BundleStatus bundle_status(RBundle* bundle);

// Bundle queue operations
RBundleQueue* create_bundle_queue(const char* name);
void destroy_bundle_queue(RBundleQueue* queue);

int bundle_queue_add(RBundleQueue* queue, RBundle* bundle);
int bundle_queue_run(RBundleQueue* queue, RTaskScheduler* sched);
BundleQueueStatus bundle_queue_status(RBundleQueue* queue);
```

---

### Example Usage

```c
#include "roc_bundle.h"
#include "roc_bundle_queue.h"
#include "roc_scheduler.h"
#include "roc_task.h"
#include "roc_job.h"
#include <stdio.h>
#include <unistd.h>

int main() {
    RTaskScheduler* sched = create_scheduler();
    scheduler_start(sched);

    // --- Create nodes ---
    RNode* cpu = create_node("CPU", "CPU", 4);

    // --- Create tasks ---
    RTask* t1 = create_task("Task-CPU1", 2);
    RTask* t2 = create_task("Task-CPU2", 1);
    add_resource_req(t1, cpu, 2);
    add_resource_req(t2, cpu, 1);

    // --- Create jobs ---
    RJob* job1 = create_job("Job-Alpha", 10);
    job_add_task(job1, t1);

    RJob* job2 = create_job("Job-Beta", 5);
    job_add_task(job2, t2);

    // --- Create bundles ---
    RBundle* bundle1 = create_bundle("Bundle-Alpha", 10);
    bundle_add_job(bundle1, job1);

    RBundle* bundle2 = create_bundle("Bundle-Beta", 5);
    bundle_add_job(bundle2, job2);

    // --- Create and run bundle queue ---
    RBundleQueue* bq = create_bundle_queue("MainBundleQueue");
    bundle_queue_add(bq, bundle1);
    bundle_queue_add(bq, bundle2);
    bundle_queue_run(bq, sched);

    // Wait for bundles to complete
    while (bundle_status(bundle1) != BUNDLE_COMPLETED ||
           bundle_status(bundle2) != BUNDLE_COMPLETED) {
        usleep(50000);
    }

    printf("[Main] All bundles processed.\n");

    // Cleanup
    destroy_task(t1);
    destroy_task(t2);
    destroy_job(job1);
    destroy_job(job2);
    destroy_bundle(bundle1);
    destroy_bundle(bundle2);
    destroy_bundle_queue(bq);
    scheduler_stop(sched);
    destroy_scheduler(sched);
    destroy_node(cpu);

    return 0;
}
```

---

## Campaigns & Campaign Queue

Campaigns are **high-level orchestration units** that group multiple bundles together. They provide an extra layer for managing large workflows or distributed tasks, allowing you to execute bundles **sequentially or in parallel** with campaign-level priorities.

Campaign Queues allow you to enqueue multiple campaigns and execute them based on **priority or order**.

---

### Campaign Concepts

* **RCampaign** – Represents a collection of bundles executed as a single logical unit.
* **CampaignStatus** – Status of a campaign:

  * `CAMPAIGN_PENDING`
  * `CAMPAIGN_RUNNING`
  * `CAMPAIGN_COMPLETED`
  * `CAMPAIGN_FAILED`

*Campaigns contain:*

* Bundles (`RBundle`)

---

### Key Functions

```c
// Campaign operations
RCampaign* create_campaign(const char* name, int priority);
void destroy_campaign(RCampaign* campaign);

int campaign_add_bundle(RCampaign* campaign, RBundle* bundle);
int campaign_run(RCampaign* campaign, RTaskScheduler* sched);
CampaignStatus campaign_status(RCampaign* campaign);

// Campaign queue operations
RCampaignQueue* create_campaign_queue(const char* name);
void destroy_campaign_queue(RCampaignQueue* queue);

int campaign_queue_add(RCampaignQueue* queue, RCampaign* campaign);
int campaign_queue_run(RCampaignQueue* queue, RTaskScheduler* sched);
CampaignQueueStatus campaign_queue_status(RCampaignQueue* queue);
```

---

### Example Usage

```c
#include "roc_campaign.h"
#include "roc_campaign_queue.h"
#include "roc_bundle.h"
#include "roc_bundle_queue.h"
#include "roc_scheduler.h"
#include "roc_task.h"
#include "roc_job.h"
#include <stdio.h>
#include <unistd.h>

int main() {
    RTaskScheduler* sched = create_scheduler();
    scheduler_start(sched);

    // --- Create nodes ---
    RNode* cpu = create_node("CPU", "CPU", 4);

    // --- Create tasks and jobs ---
    RTask* t1 = create_task("Task-CPU1", 2);
    RTask* t2 = create_task("Task-CPU2", 1);
    add_resource_req(t1, cpu, 2);
    add_resource_req(t2, cpu, 1);

    RJob* job1 = create_job("Job-Alpha", 10);
    job_add_task(job1, t1);
    RJob* job2 = create_job("Job-Beta", 5);
    job_add_task(job2, t2);

    // --- Create bundles ---
    RBundle* bundle1 = create_bundle("Bundle-Alpha", 10);
    bundle_add_job(bundle1, job1);
    RBundle* bundle2 = create_bundle("Bundle-Beta", 5);
    bundle_add_job(bundle2, job2);

    // --- Create campaigns ---
    RCampaign* campaign1 = create_campaign("Campaign-One", 10);
    campaign_add_bundle(campaign1, bundle1);

    RCampaign* campaign2 = create_campaign("Campaign-Two", 5);
    campaign_add_bundle(campaign2, bundle2);

    // --- Create and run campaign queue ---
    RCampaignQueue* cq = create_campaign_queue("MainCampaignQueue");
    campaign_queue_add(cq, campaign1);
    campaign_queue_add(cq, campaign2);
    campaign_queue_run(cq, sched);

    // Wait for campaigns to complete
    while (campaign_status(campaign1) != CAMPAIGN_COMPLETED ||
           campaign_status(campaign2) != CAMPAIGN_COMPLETED) {
        usleep(50000);
    }

    printf("[Main] All campaigns completed.\n");

    // Cleanup
    destroy_task(t1);
    destroy_task(t2);
    destroy_job(job1);
    destroy_job(job2);
    destroy_bundle(bundle1);
    destroy_bundle(bundle2);
    destroy_campaign(campaign1);
    destroy_campaign(campaign2);
    destroy_campaign_queue(cq);
    scheduler_stop(sched);
    destroy_scheduler(sched);
    destroy_node(cpu);

    return 0;
}
```

---

### Notes

* Campaigns contain **multiple bundles**, which themselves can contain tasks, jobs, or workflows.
* Campaign queues respect **priority** and **sequential execution**.
* Task and bundle dependencies are automatically respected.
* Campaign statuses allow monitoring of **completion or failure**.
* Useful for **large-scale orchestration** of complex simulations or pipelines.

---

### Notes

* Bundles can contain **mixed items**: tasks, jobs, workflows.
* Bundle queues allow **priority-based execution**.
* Tasks inside a bundle respect **dependencies and resource allocations**.
* Bundle statuses track **completion and failure**.
* Multiple bundles can execute **concurrently** if resources allow.

---

### Notes

* Phases can mix **stages and tasks** in the same unit.
* Phase queues execute phases **sequentially** respecting priority.
* Supports **thread-safe execution** through the scheduler.
* Phase and phase queue statuses allow monitoring of **completion or failure**.

---

## Notes

* All operations on nodes, tasks, and controllers are **thread-safe**.
* Tasks simulate "real" work proportional to resource units.
* Network routing supports **shortest-path** and **widest-path** policies.
* Nodes can be **aggregated** or **sliced** to model virtualized resources.
* `reserve_timed` allows automatic resource release after a timeout.

---
