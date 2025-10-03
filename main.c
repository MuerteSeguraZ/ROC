#include "roc.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    printf("[+] Building mega ROC network...\n");
    RNetwork* net = create_network();

    // === Create nodes ===
    RNode* cpu  = create_node("CPU",  "Processor", 100);
    RNode* gpu  = create_node("GPU",  "Graphics",  80);
    RNode* ram  = create_node("RAM",  "Memory",    200);
    RNode* disk = create_node("Disk", "Storage",   150);
    RNode* nic  = create_node("NIC",  "Network",    80);
    RNode* fpga = create_node("FPGA", "Compute",    50);

    add_node(net, cpu);
    add_node(net, gpu);
    add_node(net, ram);
    add_node(net, disk);
    add_node(net, nic);
    add_node(net, fpga);

    // === Create links ===
    RLink* l1 = create_link(net, cpu, gpu, 50, 10);
    RLink* l2 = create_link(net, cpu, ram, 20, 5);
    RLink* l3 = create_link(net, gpu, disk, 10, 15);
    RLink* l4 = create_link(net, ram, disk, 5, 20);
    RLink* l5 = create_link(net, nic, ram, 25, 2);
    RLink* l6 = create_link(net, fpga, gpu, 30, 8);

    // === Modify links ===
    disable_link(l3); // GPU->Disk down
    set_link_bandwidth(l2, 100);
    set_link_latency(l2, 1);
    link_perm(l1, get_link_permissions(l1) & ~(1 << POLICY_SHORTEST)); // forbid CPU->GPU shortest

    // === List nodes & links ===
    list_nodes(net);
    list_links(net);

    // === Create controller ===
    RController* ctrl = create_controller(net, POLICY_SHORTEST);

    // === Discovery, aggregation, slicing ===
    int comp_count;
    RNode** compute_nodes = discover(net, "Processor", &comp_count);
    RNode* pool = aggregate(compute_nodes, comp_count, "ComputePool", "compute");
    RNode* cpu_slice = slice_node(cpu, "CPU_Slice", 40);
    free(compute_nodes);

    // === Send packets (shortest/widest) ===
    printf("\n=== Send 50 CPU->Disk (Shortest) ===\n");
    send_packet(ctrl, cpu, disk, 50); // should fail partially due to GPU->Disk disabled & CPU->GPU forbidden

    printf("\n=== Send 60 RAM->GPU (Widest) ===\n");
    set_policy(ctrl, POLICY_WIDEST);
    send_packet(ctrl, ram, gpu, 60);

    // === Timed sends and migrations ===
    printf("\n=== Timed 30 CPU->RAM (2s) ===\n");
    send_packet_timed(ctrl, cpu, ram, 30, 2000);

    printf("\n=== Timed migration 50 RAM->GPU (3s) ===\n");
    migrate_timed(ram, gpu, 50, 3000);

    // === Dynamic topology ===
    printf("\n=== Disconnect RAM<->Disk & remove GPU ===\n");
    disconnect_nodes(net, "RAM", "Disk");
    remove_node(net, gpu);
    list_nodes(net);
    list_links(net);

    // === Final node statuses ===
    RNode* nodes[] = { cpu, gpu, ram, disk, nic, fpga, pool, cpu_slice };
    printf("\n=== Node statuses ===\n");
    for(int i = 0; i < 8; i++) {
        if(nodes[i])
            printf("%s: %d/%d\n", nodes[i]->name, nodes[i]->available, nodes[i]->capacity);
    }

    destroy_controller(ctrl);
    destroy_network(net);
    printf("\n[+] Chaos complete!\n");
    return 0;
}
