#include "roc.h"
#include <stdio.h>

int main() {
    RNetwork* net = create_network();

    // Nodes
    RNode* cpu  = create_node("CPU",  "compute", 100);
    RNode* ram  = create_node("RAM",  "memory",  200);
    RNode* disk = create_node("Disk", "storage", 300);
    RNode* gpu  = create_node("GPU",  "compute", 150);

    add_node(net, cpu);
    add_node(net, ram);
    add_node(net, disk);
    add_node(net, gpu);

    // Links
    create_link(net, cpu, ram, 20);
    create_link(net, cpu, gpu, 15);
    create_link(net, ram, disk, 10);
    create_link(net, gpu, disk, 5);

    // Create controller
    RController* ctrl = create_controller(net, POLICY_SHORTEST);

    printf("\n=== Routing CPU -> Disk ===\n");
    send_packet(ctrl, cpu, disk, 50);

    printf("\n=== Routing RAM -> GPU ===\n");
    send_packet(ctrl, ram, gpu, 80);

    destroy_controller(ctrl);
    destroy_network(net);

    return 0;
}
