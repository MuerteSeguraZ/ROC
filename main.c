#include "roc.h"
#include <stdio.h>

int main() {
    RNetwork* net = create_network();

    // Create nodes
    RNode* cpu  = create_node("CPU",  "compute", 100);
    RNode* ram  = create_node("RAM",  "memory",  200);
    RNode* disk = create_node("Disk", "storage", 300);
    RNode* gpu  = create_node("GPU",  "compute", 150);

    add_node(net, cpu);
    add_node(net, ram);
    add_node(net, disk);
    add_node(net, gpu);

    // Create links (bandwidth, latency)
    create_link(net, cpu, ram, 20, 5);
    create_link(net, cpu, gpu, 15, 10);
    create_link(net, ram, disk, 10, 8);
    create_link(net, gpu, disk, 5, 15);

    // Create controller with POLICY_SHORTEST
    RController* ctrl = create_controller(net, POLICY_SHORTEST);

    printf("\n=== Routing CPU -> Disk (Shortest) ===\n");
    send_packet(ctrl, cpu, disk, 50);

    // Change policy to widest path
    set_policy(ctrl, POLICY_WIDEST);
    printf("\n=== Routing RAM -> GPU (Widest) ===\n");
    send_packet(ctrl, ram, gpu, 80);

    // Show node availability
    printf("\n--- Node availability ---\n");
    printf("CPU:  %d/%d\n", monitor(cpu), cpu->capacity);
    printf("RAM:  %d/%d\n", monitor(ram), ram->capacity);
    printf("GPU:  %d/%d\n", monitor(gpu), gpu->capacity);
    printf("Disk: %d/%d\n", monitor(disk), disk->capacity);

    destroy_controller(ctrl);
    destroy_network(net);
    return 0;
}
