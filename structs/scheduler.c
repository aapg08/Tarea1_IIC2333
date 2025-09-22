#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"

Scheduler* create_scheduler(int q_parameter) {
    Scheduler* new_scheduler = (Scheduler*)malloc(sizeof(Scheduler));
    new_scheduler->high_queue = create_queue(2*q_parameter); // Cola de alta prioridad
    new_scheduler->low_queue = create_queue(q_parameter); // Cola de baja prioridad
    new_scheduler->running_process = NULL;
    new_scheduler->events = NULL;
    new_scheduler->current_tick = 0;
    new_scheduler->q_parameter = q_parameter;
    new_scheduler->all_processes = NULL;
    new_scheduler->process_count = 0;
    return new_scheduler;
}

void add_process(Scheduler* scheduler, Process* process) {
    if (!scheduler || !process) return;
    scheduler->process_count++;
    scheduler->all_processes = realloc(scheduler->all_processes, 
        scheduler->process_count * sizeof(Process*));
    scheduler->all_processes[scheduler->process_count-1] = process;
    if (process->start_time <= scheduler->current_tick) {
        in_queue(scheduler->high_queue, process);
        process->current_queue = 0; // Alta prioridad
    }
}

void add_event(Scheduler* scheduler, int pid, int tick) {
    Event* new_event = (Event*)malloc(sizeof(Event));
    new_event->pid = pid;
    new_event->tick = tick;
    new_event->next = NULL;

    if (scheduler->events == NULL || scheduler->events->tick > tick) {
        new_event->next = scheduler->events;
        scheduler->events = new_event;
    } else {
        Event* current = scheduler->events;
        while (current->next != NULL && current->next->tick <= tick) {
            current = current->next;
        }
        new_event->next = current->next;
        current->next = new_event;
    }
}
