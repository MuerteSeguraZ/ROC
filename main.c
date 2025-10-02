#include "roc.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    // =====================
    // Create network and nodes
    // =====================
    RNetwork* net = create_network();
    RNode* cpu  = create_node("CPU",  "compute", 100);
    RNode* ram  = create_node("RAM",  "memory",  200);
    RNode* gpu  = create_node("GPU",  "compute", 150);
    RNode* disk = create_node("Disk", "storage", 300);

    add_node(net, cpu);
    add_node(net, ram);
    add_node(net, gpu);
    add_node(net, disk);

    // Links
    create_link(net, cpu, ram, 20, 5);
    create_link(net, cpu, gpu, 15, 10);
    create_link(net, ram, disk, 10, 15);
    create_link(net, gpu, disk, 5, 20);

    RController* ctrl = create_controller(net, POLICY_SHORTEST);

    // =====================
    // Test discover
    // =====================
    int count;
    RNode** compute_nodes = discover(net, "compute", &count);
    printf("Discovered %d compute nodes:\n", count);
    for (int i = 0; i < count; i++)
        printf(" - %s (%d/%d available)\n", compute_nodes[i]->name, compute_nodes[i]->available, compute_nodes[i]->capacity);
    free(compute_nodes);

    // =====================
    // Test aggregate
    // =====================
    RNode* agg = aggregate((RNode*[]){cpu, gpu}, 2, "ComputePool", "compute");
    printf("Aggregated node %s with capacity %d and available %d\n", agg->name, agg->capacity, agg->available);

    // =====================
    // Test slice
    // =====================
    RNode* cpu_slice = slice(cpu, 40, "CPU_Slice");
    if (cpu_slice)
        printf("Created slice %s with capacity %d\n", cpu_slice->name, cpu_slice->capacity);

    // =====================
    // Test send_packet
    // =====================
    printf("\n--- Routing CPU -> Disk ---\n");
    send_packet(ctrl, cpu, disk, 50);

    // =====================
    // Test migrate
    // =====================
    RPacket pkt = { .amount = 30 };
    migrate(&pkt, ram, gpu);

    // =====================
    // Test status
    // =====================
    NodeStatus s_cpu  = status(cpu);
    NodeStatus s_ram  = status(ram);
    NodeStatus s_gpu  = status(gpu);
    NodeStatus s_disk = status(disk);

    printf("\n--- Node statuses ---\n");
    printf("CPU:  %d\nRAM:  %d\nGPU:  %d\nDisk: %d\n", s_cpu, s_ram, s_gpu, s_disk);

    // =====================
    // Cleanup
    // =====================
    destroy_node(cpu_slice);
    destroy_node(agg);
    destroy_controller(ctrl);
    destroy_network(net);

    return 0;
}
