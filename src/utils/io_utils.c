#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "io_utils.h"
#include "../structs/scheduler.h"
#include "../structs/process.h"

#define MAX_LINE_LENGTH 256

void read_input_file(const char* filename, Scheduler** scheduler) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];

    // 1. Leer parámetro q
    int q_parameter;
    if (fgets(line, sizeof(line), file)) {
        q_parameter = atoi(line);
        printf("q_parameter: %d\n", q_parameter);
    } else {
        fprintf(stderr, "Error reading q parameter\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // 2. Leer cantidad de procesos K
    int process_count;
    if (fgets(line, sizeof(line), file)) {
        process_count = atoi(line);
        printf("process_count: %d\n", process_count);
    } else {
        fprintf(stderr, "Error reading process count\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // 3. Leer cantidad de eventos N
    int event_count;
    if (fgets(line, sizeof(line), file)) {
        event_count = atoi(line);
        printf("event_count: %d\n", event_count);
        *scheduler = create_scheduler(q_parameter, event_count);
    } else {
        fprintf(stderr, "Error reading event count\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // 4. Leer los K procesos
    for (int i=0; i < process_count; i++) {
        if (!fgets(line, sizeof(line), file)) {
            fprintf(stderr, "Error reading process data\n");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        char name[100];
        // int pid, start_time, total_cpu_burst, n_bursts, io_wait, t_deadline;
        int pid, start_time, burst_time, total_bursts, io_wait, deadline; // Con los mismos nombres de los atributos del struct Process
        int parsed = sscanf(line, "%s %d %d %d %d %d %d", name, &pid, &start_time, &burst_time, &total_bursts, &io_wait, &deadline);
        if (parsed != 7) {
            fprintf(stderr, "Error parsing process data\n");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        printf("Process: %s %d %d %d %d %d %d\n", name, pid, start_time, burst_time, total_bursts, io_wait, deadline);

        // Crear y agregar el proceso al scheduler
        Process* new_process = create_process(name, pid, start_time, burst_time, total_bursts, io_wait, deadline);
        add_process(*scheduler, new_process);
    }

    // 5. Leer los N eventos
    for (int i=0; i < event_count; i++) {
        if (!fgets(line, sizeof(line), file)) {
            fprintf(stderr, "Error reading event data\n");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        int pid, event_time;

        int parsed = sscanf(line, "%d %d", &pid, &event_time);
        if (parsed != 2) {
            fprintf(stderr, "Error parsing event data\n");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        printf("Event: pid=%d, event_time=%d\n", pid, event_time);

        // event_time corresponde al tick en que el proceso llega
        add_event(*scheduler, pid, event_time);
    }
    fclose(file);
}

// Función auxiliar para ordenar procesos por PID
int compare_process_pid(const void* a, const void* b) {
    Process* pa = *(Process**)a;
    Process* pb = *(Process**)b;
    return pa->pid - pb->pid;
}

void write_output_file(const char* filename, Scheduler* scheduler) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Error opening output file");
        return;
    }
    // Ordenamos por PID los procesos finalizados -> como volvi a la linea original, creo que ya no es necesario sumar po id
    // qsort(scheduler->finished_processes, scheduler->finished_count, sizeof(Process*), compare_process_pid);

    // Escribir los procesos en el archivo de salida
    for (int i=0; i < scheduler->process_count; i++) {
        Process* process = scheduler->all_processes[i];
        const char* state_str;
        switch (process->state) {
            case FINISHED: state_str = "FINISHED"; break;
            case DEAD: state_str = "DEAD"; break;
            case RUNNING: state_str = "RUNNING"; break;
            case READY: state_str = "READY"; break;
            case WAITING: state_str = "WAITING"; break;
            default: state_str = "UNKNOWN";
        }
        if (process->response_time == -1) {
            process->response_time = 0; // Si nunca se ejecutó, response time es 0
        }

        fprintf(file, "%s,%d,%s,%d,%d,%d,%d",
            process->name,
            process->pid,
            state_str,
            process->interruptions,
            process->turnaround_time,
            process->response_time,
            process->waiting_time);

        if (i < scheduler->process_count - 1) {
            fprintf(file, "\n");
        }
    }

    fclose(file);
}


void free_scheduler(Scheduler* scheduler) {
    if (!scheduler) return;
    if (scheduler->high_queue) free_queue(scheduler->high_queue);
    if (scheduler->low_queue) free_queue(scheduler->low_queue);

    // Liberar eventos
    Event* current_event = scheduler->events;
    while (current_event) {
        Event* next = current_event->next;
        free(current_event);
        current_event = next;
    }

    // Liberar procesos
    for (int i=0; i < scheduler->process_count; i++) {
        if (scheduler->all_processes[i]) {
            free_process(scheduler->all_processes[i]);
        }
    }
    free(scheduler->finished_processes);
    free(scheduler->all_processes);

    free(scheduler);
}