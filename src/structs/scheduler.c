#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"
#include "queue.h"

Scheduler* create_scheduler(int q_parameter, int n_events) {
    Scheduler* new_scheduler = (Scheduler*)malloc(sizeof(Scheduler));
    new_scheduler->high_queue = create_queue(2*q_parameter); // Cola de alta prioridad
    new_scheduler->low_queue = create_queue(q_parameter); // Cola de baja prioridad
    new_scheduler->running_process = NULL;
    new_scheduler->events = NULL;
    new_scheduler->current_tick = 0;
    new_scheduler->q_parameter = q_parameter;
    new_scheduler->all_processes = NULL;
    new_scheduler->process_count = 0;
    new_scheduler->finished_processes = NULL;
    new_scheduler->finished_count = 0;
    new_scheduler->out_process = NULL;
    new_scheduler->active_event = NULL;
    new_scheduler->event_count = n_events;
    new_scheduler->triggered_events = 0;
    new_scheduler->added_to_queue = 0;
    return new_scheduler;
}

void add_process(Scheduler* scheduler, Process* process) {
    if (!scheduler || !process) return;
    scheduler->process_count++;
    scheduler->all_processes = realloc(scheduler->all_processes, 
        scheduler->process_count * sizeof(Process*));
    scheduler->all_processes[scheduler->process_count-1] = process;
    /* if (process->start_time <= scheduler->current_tick) {
        in_queue(scheduler->high_queue, process);
        // process->current_queue = 0; // Alta prioridad
    }*/
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

void remove_dead_processes(Scheduler* scheduler, Queue* queue) {
    Process* current_node = queue->head;
    while (current_node != NULL) {
        Process* next_node = current_node->next;
        if (current_node->deadline <= scheduler->current_tick) {
            current_node->state = DEAD;
            current_node->turnaround_time = scheduler->current_tick - current_node->start_time;
            if (scheduler->running_process->response_time == -1) { // en caso de que no haya alcanzado a ejecutar, igual debo setearlo en 0
                scheduler->running_process->response_time = 0;
            }
            scheduler->finished_count++;
            remove_from_queue(queue, current_node);
        }
        current_node = next_node;
    }
    return;
}

void terminate_running_process(Scheduler* scheduler) {
    if (scheduler->running_process->already_finished == 0) {
        scheduler->running_process->already_finished = 1;
    }
    if (scheduler->running_process->response_time <= -1) { // en caso de que no haya alcanzado a ejecutar, igual debo setearlo en 0
        scheduler->running_process->response_time = 0;
    }
    scheduler->running_process->turnaround_time = scheduler->current_tick - scheduler->running_process->start_time;
    printf("Turnaround: %d\n", scheduler->running_process->turnaround_time);
    scheduler->finished_count++;
    scheduler->running_process = NULL;
    return;
}

void take_out_running_process(Scheduler* scheduler) {
    scheduler->running_process->last_CPU_out = scheduler->current_tick;
    scheduler->out_process = scheduler->running_process;
    scheduler->running_process = NULL;
    return;
}

void find_active_event(Scheduler* scheduler) {
    Event* current_event = scheduler->events;
    while (current_event != NULL) {
        if (current_event->tick == scheduler->current_tick) {
            scheduler->active_event = current_event;
            scheduler->triggered_events++;
            return;
        }
        current_event = current_event->next;
    }
    return;
}

void update_io_processes(Scheduler* scheduler) {
    Process* current_process = NULL;
    printf("Procesos en estado WAITING o READY:\n");
    for (int i = 0; i < scheduler->process_count; i++) {
        current_process = scheduler->all_processes[i];
        if (current_process->state == WAITING || current_process->state == READY) {// Toca solo los procesos en estado waiting
            printf("Proceso con pid %d\n", current_process->pid);
            if (current_process->state == WAITING){
                current_process->remaining_io--;
                printf("i/o restante: %d\n", current_process->remaining_io);
                if (current_process->remaining_io <= 0) {
                    printf("proceso pasa a estado READY\n");
                    current_process->state = READY;
                /*} else {
                    current_process->remaining_io--;
                    printf("i/o restante: %d\n", current_process->remaining_io);*/
                }
            }
            if (scheduler->current_tick > current_process->start_time) {
                printf("proceso %d espera 1 tick\n", current_process->pid);
                current_process->waiting_time++; // Suma tiempo solo si el proceso ya se inicializó
                printf("Proceso con waiting time actual: %d\n", current_process->waiting_time);
            }
        }
    }
    return;
}

void start_processes(Scheduler* scheduler) {
    Process* current_process = NULL;
    for (int i = 0; i < scheduler->process_count; i++) {
        current_process = scheduler->all_processes[i];
        if (current_process->start_time == scheduler->current_tick) {
            printf("Proceso %d termina su tiempo de inicio y es agregado a la cola HIGH\n", current_process->pid);
            scheduler->added_to_queue++;
            in_queue(scheduler->high_queue, current_process); // Cuando el proceso entre a la cola por primera vez lo hará a HIGH
            current_process->quantum = scheduler->high_queue->quantum; // Se le asigna el quantum de la cola HIGH
            current_process->queue = 0;
            current_process->remaining_burst = current_process->burst_time; // Seteo el burst time
            current_process->remaining_quantum = current_process->quantum;
        }
    }
    return;
}

void upqueue_processes(Scheduler* scheduler) {
    int process_counter = scheduler->low_queue->size;
    Process* current_node = scheduler->low_queue->head;
    while (process_counter > 0) {
        Process* next_node = current_node->next; // Guardar el siguiente nodo antes de cualquier posible modificación
        if ((current_node->deadline * 2) < (scheduler->current_tick - current_node->last_CPU_out)) { 
            remove_from_queue(scheduler->low_queue, current_node);
            in_queue(scheduler->high_queue, current_node);
            printf("Proceso %d ya cumplio el requisito y por lo tanto sube a cola HIGH\n", current_node->pid);
            current_node->quantum = scheduler->high_queue->quantum; // Se le asigna el quantum de la cola HIGH
            current_node->queue = 0;
        }
        current_node = next_node;
        process_counter--;
    }
    return;
}

void start_event_process(Scheduler* scheduler) {
    Process* current_process = NULL;
    for (int i = 0; i < scheduler->process_count; i++){
        current_process = scheduler->all_processes[i];
        if (current_process->pid == scheduler->active_event->pid) {
            break;
        }
    }
    // Busca el proceso en colas
    start_process_from_queue(scheduler->high_queue, current_process);
    start_process_from_queue(scheduler->low_queue, current_process);
    // Si no está en ninguna cola es porque termino o murió; si terminó va a salir immediatamente de la CPU por remaining time <= 0, y si murió tambien sale immediatamente por deadline
    if (current_process->state == FINISHED || current_process->state == DEAD) {
        printf("Proceso %d previamente finalizado es enviado a la CPU\n", current_process->pid);
        current_process->state = RUNNING;
        scheduler->finished_count--; // Como lo voy a "revivir", momentaneamente lo saco de la cuenta de procesos terminados
    }
    scheduler->running_process = current_process;
    scheduler->active_event = NULL;
    return;
}

void start_process_by_queue_priority(Scheduler* scheduler) {
    Process* current_process = NULL;
    if (!is_empty(scheduler->high_queue)) {
        current_process = start_process_by_priority(scheduler->high_queue, scheduler->current_tick);
    } else if (!is_empty(scheduler->low_queue) && current_process == NULL) {
        current_process = start_process_by_priority(scheduler->low_queue, scheduler->current_tick);
    }
    scheduler->running_process = current_process;
    return;
}
