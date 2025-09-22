#ifndef QUEUE_H
#define QUEUE_H

#include "process.h"

typedef struct Queue {
    Process* head;
    Process* cola;
    int quantum;
    int size;
} Queue;

Queue* create_queue(int quantum);
void in_queue(Queue* queue, Process* process);
Process* out_queue(Queue* queue);
Process* review_queue(Queue* queue);
int is_empty(Queue* queue); // revisar si es necesaria 
void remove_from_queue(Queue* queue, Process* process);
Process* dequeue_highest_priority(Queue* queue);
void free_queue(Queue* queue);

#endif