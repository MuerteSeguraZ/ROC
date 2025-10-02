#include "roc.h"
#include <stdio.h>

int main() {
    RNetwork* net = create_network();

    // Create nodes
    RNode* cpu  = create_node("CPU",  "compute", 100);
    RNode* ram  = create_node("RAM",  "memory",  200);
    RNode* disk = create_node("Disk", "storage", 300);
    RNode* gpu  = create_node("GPU",  "compute", 150);

    // Add to network
    add_node(net, cpu);
    add_node(net, ram);
    add_node(net, disk);
    add_node(net, gpu);

    // Create links (bidirectional is implicit in routing logic)
    create_link(net, cpu, ram, 20);
    create_link(net, cpu, gpu, 15);
    create_link(net, ram, disk, 10);
    create_link(net, gpu, disk, 5);

    // Packets
    RPacket p1 = { .amount = 50 };
    RPacket p2 = { .amount = 80 };

    // Route packets
    printf("\n=== Routing CPU -> Disk ===\n");
    route_packet(net, cpu, disk, &p1, POLICY_SHORTEST);

    printf("\n=== Routing RAM -> GPU ===\n");
    route_packet(net, ram, gpu, &p2, POLICY_SHORTEST);

    // Cleanup
    destroy_network(net);
    return 0;
}
