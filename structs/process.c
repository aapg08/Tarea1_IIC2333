#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"

Process* create_process(char* name, int pid, int start_time, int burst_time,
    int total_bursts, int io_wait, int deadline) {
    Process* new_process = (Process*)malloc(sizeof(Process));
    new_process->name = strdup(name);
    new_process->pid = pid;
    new_process->start_time = start_time;
    new_process->burst_time = burst_time;
    new_process->total_bursts = total_bursts;
    new_process->io_wait = io_wait;
    new_process->deadline = deadline;

    new_process->state = READY;

    new_process->interruptions = 0;
    new_process->turnaround_time = 0;
    new_process->response_time = -1; // No ha entrado a CPU
    new_process->waiting_time = 0;

    new_process->bursts_completed = 0; // Inicialmente no ha completado ninguna ráfaga
    new_process->priority = 0.0; // Prioridad inicial temporal

    new_process->next = NULL;

    update_process_priority(new_process, start_time); // Inicializa la prioridad

    return new_process;
}

void free_process(Process* process) {
    if (process) {
        free(process->name);
        free(process);
    }
}

void update_process_priority(Process* process, int current_tick) {
    // Se espera que al llamar a esta función, se entregue el proceso y el tick en que se está haciendo
    if (process->state == FINISHED || process->state == DEAD) {
        process->priority = -1.0; // Prioridad mínima
        return;
    }

    int time_until_deadline = process->deadline - current_tick;
    int burst_remaining = process->total_bursts - process->bursts_completed;

    if (time_until_deadline <= 0) {
        // Para evitar la divición por cero
        process->priority = 0.0; 
    } else {
        process->priority = (1.0 / time_until_deadline) + bursts_remaining;
    }
}