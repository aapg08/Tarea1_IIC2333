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
    Process** finished_processes; // Arreglo de procesos terminados
    int finished_count; // Cantidad de procesos terminados
    Process* out_process; // Proceso que acaba de salir de la CPU para manejo mas simple
    Event* active_event;
    int event_count;
    int triggered_events;
    int added_to_queue;
} Scheduler;

Scheduler* create_scheduler(int q_parameter, int n_events);
void add_process(Scheduler* scheduler, Process* process);
void add_event(Scheduler* scheduler, int pid, int tick);
void remove_dead_processes(Scheduler* scheduler, Queue* queue);
void terminate_running_process(Scheduler* scheduler);
void take_out_running_process(Scheduler* scheduler);
void find_active_event(Scheduler* scheduler);
void update_io_processes(Scheduler* scheduler);
void start_processes(Scheduler* scheduler);
void upqueue_processes(Scheduler* scheduler);
void start_event_process(Scheduler* scheduler);
void start_process_by_queue_priority(Scheduler* scheduler);

#endif