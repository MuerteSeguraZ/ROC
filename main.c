#include "roc.h"
#include <stdio.h>

int main() {
    RNetwork* net = create_network();

    // Create nodes
    RNode* cpu  = create_node("CPU-1", "CPU", 100);
    RNode* gpu  = create_node("GPU-1", "GPU", 50);
    RNode* ram  = create_node("RAM-1", "Memory", 200);
    RNode* disk = create_node("DISK-1", "Storage", 300);

    add_node(net, cpu);
    add_node(net, gpu);
    add_node(net, ram);
    add_node(net, disk);

    // Create links (topology)
    add_link(net, create_link(cpu, gpu, 50));
    add_link(net, create_link(cpu, ram, 40));
    add_link(net, create_link(ram, disk, 30));
    add_link(net, create_link(gpu, disk, 20));

    // Test transfers
    RPacket p1 = {25, cpu, disk}; // should go CPU->RAM->DISK or CPU->GPU->DISK
    RPacket p2 = {15, ram, gpu};  // RAM->CPU->GPU

    printf("\n=== Transfer 1 ===\n");
    transfer_packet(net, &p1);

    printf("\n=== Transfer 2 ===\n");
    transfer_packet(net, &p2);

    destroy_network(net);
    return 0;
}
