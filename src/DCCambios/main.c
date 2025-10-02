#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../structs/scheduler.h"
#include "../utils/io_utils.h"


int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Uso: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    Scheduler* scheduler = NULL;

    // Leer archivo de input
    read_input_file(argv[1], &scheduler);

    // Aquí iría la lógica para ejecutar la simulación del scheduler
    while (1) {
        if (scheduler->finished_count >= scheduler->process_count && scheduler->triggered_events >= scheduler->event_count) {
            break; // Todos los procesos han terminado
        }
        printf("Tick: %d\n", scheduler->current_tick);
        // Primero va a revisar si hay un evento activo
        find_active_event(scheduler);

        // Actualizar estado de procesos que terminaron su proceso de I/O (WAITING -> READY)
        printf("Revision de I/O\n");
        update_io_processes(scheduler);
        // Actualizar estado de procesos en cola que cumplieron su deadline y no terminaron sus bursts (-> DEAD)
        printf("Revision de procesos muertos\n");
        remove_dead_processes(scheduler, scheduler->high_queue);
        remove_dead_processes(scheduler, scheduler->low_queue);
        // Revsion de proceso en CPU
        if (scheduler->running_process != NULL) {
            printf("Revision de proceso actual %d en CPU\n", scheduler->running_process->pid);
            // Si alcanzó su deadline (salida forsoza -> DEAD)
            if (scheduler->running_process->deadline <= scheduler->current_tick) {
                scheduler->running_process->state = DEAD;
                printf("Proceso alcanzo su deadline, muere\n");
                terminate_running_process(scheduler);
            } else if (scheduler->running_process->remaining_burst <= 0) { // Si es que terminó su rafaga
                printf("Proceso acabó su burst\n");
                scheduler->running_process->bursts_completed++; // voy a sumar una rafaga completada
                scheduler->running_process->finished_burst = 1; // Indico que si terminó su burst
                // scheduler->running_process->interruptions++; // No estoy segura si cuando termina todas sus rafagas igual suma una interrupcion asi que lo dejo comentado
                if (scheduler->running_process->bursts_completed >= scheduler->running_process->total_bursts) { // Si es que ya terminó todas sus rafagas, termina
                    printf("Proceso acabó sus rafagas, finaliza\n");
                    scheduler->running_process->state = FINISHED;
                    terminate_running_process(scheduler);
                } else { // Si no, va a pasar a estado WAITING
                    printf("Proceso pasa a waiting en cola original\n");
                    scheduler->running_process->interruptions++; // Asumiendo que solo se suma cuando no ha terminado todas las rafagas
                    scheduler->running_process->state = WAITING;
                    scheduler->running_process->remaining_io = scheduler->running_process->io_wait; // Reseteo el tiempo de I/O para el proximo tick
                    take_out_running_process(scheduler);
                }
            } else if (scheduler->running_process->remaining_quantum <= 0) { // Si no terminó la rafaga, reviso si terminó su quantum
                printf("Proceso acabó su quantum, pasa a ready en cola LOW\n");
                scheduler->running_process->state = READY;
                scheduler->running_process->queue = 1; // Como acabó su quantum, lo voy a forzar a LOW
                scheduler->running_process->finished_quantum = 1; // Indico que si terminó su quantum
                take_out_running_process(scheduler);
            } else {
                if (scheduler->active_event != NULL) { // Paso a chequear si hay un evento activo
                    if (scheduler->active_event->pid != scheduler->running_process->pid) { // Si el proceso del evento es distinto al en ejecucion
                        printf("Proceso es removido porque se gatillo un evento con un proceso distinto\n");
                        scheduler->running_process->state = READY;
                        scheduler->running_process->interruptions++; // Este si cuenta como interrupción
                        scheduler->running_process->max_priority = 1; // Le doy máxima prioridad para que entre primero a HIGH
                        scheduler->running_process->queue = 0; // Como sale por evento, vuelve a HIGH
                        take_out_running_process(scheduler);
                    }
                }
            } // Si no pasó nada y el proceso no salio de la CPU, va a ejecutar el tick
            if (scheduler->running_process != NULL) {
                printf("Proceso ejecuta sus ticks\n");
                scheduler->running_process->remaining_burst--;
                scheduler->running_process->remaining_quantum--;
                if (scheduler->running_process->response_time == -1) { // Si es su primera ejecución en CPU
                    scheduler->running_process->response_time = scheduler->current_tick - scheduler->running_process->start_time;
                    printf("Proceso ejecuta por primera vez, response time: %d\n", scheduler->running_process->response_time);
                }
            }
        }
        // Ingreso proceso saliente a las colas
        if (scheduler->out_process != NULL) { // Si salió de la CPU (READY o WAITING)
            printf("Reingresa proceso saliente a la cola correspondiente\n");
            if (scheduler->out_process->queue == 0) { // Para cuando salio por evento o estaba previamente en HIGH
                scheduler->out_process->quantum = scheduler->high_queue->quantum; // Reseteo su valor de quantum
                in_queue(scheduler->high_queue, scheduler->out_process);
            } else { // Si salio por quantum o estaba previamente en LOW
                scheduler->out_process->quantum = scheduler->low_queue->quantum; // Reseteo su valor de quantum
                in_queue(scheduler->low_queue, scheduler->out_process);
            }
            scheduler->out_process = NULL;
        } else {
            // Termino tiempo de inicio (ingresa automaticamente a HIGH)
            start_processes(scheduler);
            // Subir proceso a HIGH si se cumple el requisito
            if (scheduler->added_to_queue == 0) { // Solo si no se cumplieron los pasos anteriores
                upqueue_processes(scheduler);
            }
            scheduler->added_to_queue = 0; // Reseteo el contador
        }
        // Recalcular el valor de prioridad de todos los procesos en colas
        printf("Actualizzación de prioridades en ambas colas\n");
        update_queue_priorities(scheduler->high_queue, scheduler->current_tick);
        update_queue_priorities(scheduler->low_queue, scheduler->current_tick);
        // Ingresar proceso a la CPU
        if (scheduler->running_process == NULL) {
            // Cuando se activa un evento
            if (scheduler->active_event != NULL) {
                start_event_process(scheduler);
            } 
            if (scheduler->running_process == NULL && (!is_empty(scheduler->high_queue) || !is_empty(scheduler->low_queue))) {
                printf("Envia proceso por cola a la CPU\n");
                start_process_by_queue_priority(scheduler);
            }
        }
        // aumenta tick
        scheduler->current_tick++;
    }
    // ...

    // Escribir resultados al archivo de output
    write_output_file(argv[2], scheduler);

    // Liberar memoria
    free_scheduler(scheduler);

    return 0;
}