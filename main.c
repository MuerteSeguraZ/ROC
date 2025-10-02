#include "roc.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct PacketTask {
    RLink* link;
    RPacket packet;
    RNode* from;
} PacketTask;

void* packet_thread(void* arg) {
    PacketTask* task = (PacketTask*)arg;
    if (reserve(task->from, task->packet.amount)) {
        printf("Reserved %d units from %s\n", task->packet.amount, task->from->name);
        transfer(task->link, &task->packet);
        release(task->from, task->packet.amount);
        printf("Released %d units from %s\n", task->packet.amount, task->from->name);
    } else {
        printf("Not enough resources to reserve from %s\n", task->from->name);
    }
    free(task);
    return NULL;
}

int main() {
    srand(time(NULL));

    // Create network
    RNetwork* net = create_network();

    // Add nodes
    RNode* cpu = create_node("CPU-1", "CPU", 100);
    RNode* gpu = create_node("GPU-1", "GPU", 50);
    RNode* ram = create_node("RAM-1", "Memory", 200);
    RNode* disk = create_node("DISK-1", "Storage", 300);

    add_node(net, cpu);
    add_node(net, gpu);
    add_node(net, ram);
    add_node(net, disk);

    // Add links
    add_link(net, create_link(cpu, gpu, 30));
    add_link(net, create_link(cpu, ram, 40));
    add_link(net, create_link(ram, disk, 25));
    add_link(net, create_link(gpu, disk, 20));

    // Spawn traffic
    pthread_t threads[10];
    for (int i = 0; i < 10; i++) {
        RLink* link = net->links[rand() % net->link_count];
        PacketTask* task = (PacketTask*)malloc(sizeof(PacketTask));
        task->link = link;
        task->from = link->node1;
        task->packet.amount = (rand() % 40) + 10; // 10-50 units
        pthread_create(&threads[i], NULL, packet_thread, task);
    }

    // Wait for all traffic
    for (int i = 0; i < 10; i++) {
        pthread_join(threads[i], NULL);
    }

    // Show final state
    printf("\n--- Final Resource States ---\n");
    for (int i = 0; i < net->node_count; i++) {
        printf("%s (%s): %d/%d available\n",
               net->nodes[i]->name,
               net->nodes[i]->type,
               monitor(net->nodes[i]),
               net->nodes[i]->capacity);
    }

    destroy_network(net);
    return 0;
}
