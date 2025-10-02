#include "roc.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    printf("[+] Creating network and nodes...\n");
    RNetwork* net = create_network();

    RNode* cpu  = create_node("CPU",  "compute", 100);
    RNode* ram  = create_node("RAM",  "memory",  200);
    RNode* disk = create_node("Disk", "storage", 300);
    RNode* gpu  = create_node("GPU",  "compute", 150);

    add_node(net, cpu);
    add_node(net, ram);
    add_node(net, disk);
    add_node(net, gpu);

    printf("[+] Creating links...\n");
    RLink* l1 = create_link(net, cpu, ram, 20, 10);
    RLink* l2 = create_link(net, cpu, gpu, 15, 20);
    RLink* l3 = create_link(net, ram, disk, 10, 5);
    RLink* l4 = create_link(net, gpu, disk, 5, 50);

    printf("[+] Testing link permissions...\n");
    printf("Original permissions CPU->GPU: 0x%X\n", get_link_permissions(l2));
    link_perm(l2, l2->permissions & ~(1 << POLICY_SHORTEST));
    printf("Modified permissions CPU->GPU: 0x%X\n", get_link_permissions(l2));

    printf("[+] Testing bandwidth/latency getters/setters...\n");
    printf("CPU->RAM bandwidth: %d, latency: %d\n", get_link_bandwidth(l1), get_link_latency(l1));
    set_link_bandwidth(l1, 50);
    set_link_latency(l1, 2);
    printf("Updated CPU->RAM bandwidth: %d, latency: %d\n", get_link_bandwidth(l1), get_link_latency(l1));

    printf("[+] Testing enable/disable link...\n");
    printf("GPU->Disk enabled? %d\n", is_link_enabled(l4));
    disable_link(l4);
    printf("GPU->Disk enabled after disable? %d\n", is_link_enabled(l4));
    enable_link(l4);
    printf("GPU->Disk enabled after re-enable? %d\n", is_link_enabled(l4));

    printf("[+] Destroying links...\n");
    destroy_link(l1);
    destroy_link(l2);
    destroy_link(l3);
    destroy_link(l4);

    destroy_network(net);
    return 0;
}
