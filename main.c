#include "roc.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

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
    // Create nodes
    RNode* cpu = create_node("CPU-1", "CPU", 100);
    RNode* gpu = create_node("GPU-1", "GPU", 50);

    // Create link
    RLink link = {cpu, gpu, 20};

    // Launch multiple packets concurrently
    pthread_t threads[5];
    for (int i = 0; i < 5; i++) {
        PacketTask* task = (PacketTask*)malloc(sizeof(PacketTask));
        task->link = &link;
        task->from = cpu;
        task->packet.amount = 20 + i * 5; // 20, 25, 30, 35, 40
        pthread_create(&threads[i], NULL, packet_thread, task);
    }

    // Wait for all threads
    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }

    // Monitor
    printf("CPU available: %d\n", monitor(cpu));
    printf("GPU available: %d\n", monitor(gpu));

    destroy_node(cpu);
    destroy_node(gpu);

    return 0;
}
