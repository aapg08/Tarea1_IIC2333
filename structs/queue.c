#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

Queue* create_queue(int quantum) {
    Queue* new_queue = (Queue*)malloc(sizeof(Queue));
    new_queue->head = NULL;
    new_queue->cola = NULL;
    new_queue->quantum = quantum;
    new_queue->size = 0;
    return new_queue;
}

void in_queue(Queue* queue, Process* process) {
    if (!queue || !process) return;

    process->next = NULL; // Asegurarse de que el siguiente sea NULL

    if (is_empty(queue)) {
        queue->head = process;
        queue->cola = process;
    } else {
        queue->cola->next = process;
        queue->cola = process;
    }
    queue->size++;
}

Process* out_queue(Queue* queue) {
    if (is_empty(queue)) return NULL;

    Process* remove_process = queue->head;
    queue->head = queue->head->next;
    if (queue->head == NULL) {
        queue->cola = NULL; // La cola también debe ser NULL si la cabeza es NULL
    }
    remove_process->next = NULL; // Desconectar el proceso de la cola
    queue->size--;
    return remove_process;
}

Process* review_queue(Queue* queue) {
    if (is_empty(queue)) return NULL;
    return queue->head;
}

int is_empty(Queue* queue) {
    return (queue == NULL || queue->head == NULL);
}

void remove_from_queue(Queue* queue, Process* process) {
    if (is_empty(queue) || process == NULL) return;
    // Si el proceso a eliminar es el primero
    if (queue->head == process) {
        out_queue(queue);
        return;
    }
    Process* current = queue->head;
    Process* previous = NULL;
    while (current != NULL && current != process) {
        previous = current;
        current = current->next;
    }
    if (current == process) {
        if (previous != NULL) {
            previous->next = current->next;
        }
        if (current == queue->cola) {
            queue->cola = previous; // Actualizar la cola si es necesario
        }
        current->next = NULL; // Desconectar el proceso de la cola
        queue->size--;
    }
}

Process* dequeue_highest_priority(Queue* queue) {
    if (is_empty(queue)) return NULL;

    if (queue->head->next == NULL) {
        // Solo hay un proceso en la cola
        return out_queue(queue);
    }

    Process* current = queue->head;
    Process* highest_priority_process = current;
    Process* prev_highest = NULL;
    Process* prev_current = NULL;

    while (current != NULL) {
        if (current->priority > highest_priority_process->priority || 
            (current->priority == highest_priority_process->priority && current->pid < highest_priority_process->pid)) {
            highest_priority_process = current;
            prev_highest = prev_current;
        }
        prev_current = current;
        current = current->next;
    }

    if (highest_priority_process == queue->head) {
        // El proceso con mayor prioridad es el primero
        return out_queue(queue);
    } else {
        // El proceso con mayor prioridad no es el primero
        if (prev_highest != NULL) {
            prev_highest->next = highest_priority_process->next;
        }
        if (highest_priority_process == queue->cola) {
            queue->cola = prev_highest; // Actualizar la cola si es necesario
        }
        highest_priority_process->next = NULL; // Desconectar el proceso de la cola
        queue->size--;
        return highest_priority_process;
    }
}

void free_queue(Queue* queue) {
    if (!queue) return;
    free(queue);
}