#ifndef PROCESS_H
#define PROCESS_H

typedef enum {
    RUNNING,
    READY,
    WAITING,
    FINISHED,
    DEAD
} ProcessState;

typedef struct Process{
    // Atributos del proceso
    char* name;
    int pid;
    ProcessState state; // Estado actual
    int start_time; // Para manejar el tiempo de llegada del proceso
    int burst_time; // Tiempo de ejecución por ráfaga
    int total_bursts; // Número de ráfagas de ejecución
    int bursts_completed; // Número de ráfagas completadas
    int io_wait; // Tiempo de espera para I/O
    int deadline;

    // Para el output
    int interruptions;
    int turnaround_time;
    int response_time;
    int waiting_time;

    // Para ordenar los procesos en las colas
    double priority; // Prioridad 

    struct Process* next;
} Process;

Process* create_process(char* name, int pid, int start_time, int burst_time,
    int total_bursts, int io_wait, int deadline);
void free_process(Process* process);
void update_process_priority(Process* process, int current_tick);


#endif