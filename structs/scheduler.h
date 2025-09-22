#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"
#include "queue.h"

typedef struct Event {
    int pid;
    int tick;
    struct Event* next;
} Event;

typedef struct Scheduler {
    Queue* high_queue;
    Queue* low_queue;
    Process* running_process; // Proceso actual en ejecución
    Event* events; // Lista de eventos por procesar
    int current_tick; // Tick actual de la simulación
    int q_parameter; // Parametro q del input para calcular quantum
    Process** all_processes; // Arreglo de todos los procesos
    int process_count; // Cantidad de procesos totales
} Scheduler;

Scheduler* create_scheduler(int q_parameter);
void add_process(Scheduler* scheduler, Process* process);
void add_event(Scheduler* scheduler, int pid, int tick);

#endif