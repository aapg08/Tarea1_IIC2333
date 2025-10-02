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
        // Primero va a revisar si hay un evento activo
        find_active_event(scheduler);

        // Actualizar estado de procesos que terminaron su proceso de I/O (WAITING -> READY)
        update_io_processes(scheduler);
        // Actualizar estado de procesos en cola que cumplieron su deadline y no terminaron sus bursts (-> DEAD)
        remove_dead_processes(scheduler, scheduler->high_queue);
        remove_dead_processes(scheduler, scheduler->low_queue);
        // Revsion de proceso en CPU
        if (scheduler->running_process != NULL) {
            scheduler->running_process->remaining_burst--;
            scheduler->running_process->remaining_quantum--;
            // Si alcanzó su deadline (salida forsoza -> DEAD)
            if (scheduler->running_process->deadline <= scheduler->current_tick) {
                scheduler->running_process->state = DEAD;
                terminate_running_process(scheduler);
            } else if (scheduler->running_process->bursts_completed >= scheduler->running_process->total_bursts) { // Caso en que lo volvieran a llamar y su deadline no haya llegado
                scheduler->running_process->state = FINISHED;
                scheduler->running_process->finished_burst = 1; // Marco que terminó su ráfaga
                terminate_running_process(scheduler);
            } else if (scheduler->running_process->remaining_burst <= 0) { // Termino su CPU burst (-> WAITING o FINISHED)
                scheduler->running_process->bursts_completed++; // Revisar si aumenta el n° de interrupciones y rafagas, si no, editar
                // scheduler->running_process->interruptions++; // En ambos casos sale de CPU asi que asumo cuentan como interrupciones
                // Si terminó todas sus rafagas pasará a FINISHED
                if (scheduler->running_process->bursts_completed >= scheduler->running_process->total_bursts) {
                    scheduler->running_process->state = FINISHED;
                    scheduler->running_process->finished_burst = 1; // Marco que terminó su ráfaga
                    terminate_running_process(scheduler);
                } else { // Si le quedan, va a pasar a WAITING
                    scheduler->running_process->state = WAITING;
                    scheduler->running_process->remaining_io = scheduler->running_process->io_wait; // Reseteo el tiempo de I/O
                    scheduler->running_process->interruptions++; // Como no se si cuando termina su ultima rafaga suma una interrupcion, lo sumo solo en este caso
                    scheduler->running_process->finished_burst = 1; // Marco que terminó su ráfaga
                    take_out_running_process(scheduler);
                }
            } else if (scheduler->running_process->remaining_quantum <= 0) { // Se acabó su quantum (-> READY)
                scheduler->running_process->state = READY;
                scheduler->running_process->queue = 1; // Como sale por quantum voy a forzar que entre a LOW
                scheduler->running_process->finished_quantum = 1; // Marco que terminó su quantum
                take_out_running_process(scheduler);
            } else { // Otro proceso debe entrar por evento (-> READY)
                // Recorrido de Eventos
                if (scheduler->active_event != NULL) {
                    if (scheduler->active_event->pid != scheduler->running_process->pid) {
                        scheduler->running_process->state = READY;
                        scheduler->running_process->interruptions++; // Este si cuenta como interrupción
                        scheduler->running_process->max_priority = 1; // Le doy máxima prioridad para que entre primero a HIGH
                        scheduler->running_process->queue = 0; // Como sale por evento, vuelve a HIGH
                        take_out_running_process(scheduler);
                    }
                }
            }
        }
        // Ingreso proceso saliente a las colas
        if (scheduler->out_process != NULL) { // Si salió de la CPU (READY o WAITING)
            if (scheduler->out_process->queue == 0) { // Para cuando salio por evento o estaba previamente en HIGH
                scheduler->out_process->quantum = scheduler->high_queue->quantum; // Reseteo su quantum
                in_queue(scheduler->high_queue, scheduler->out_process);
            } else { // Si salio por quantum o estaba previamente en LOW
                scheduler->out_process->quantum = scheduler->low_queue->quantum; // Reseteo su quantum
                in_queue(scheduler->low_queue, scheduler->out_process);
            }
            scheduler->out_process = NULL;
        }
        // Termino tiempo de inicio (ingresa automaticamente a HIGH)
        start_processes(scheduler);
        // Subir proceso a HIGH si se cumple el requisito
        upqueue_processes(scheduler);
        // Recalcular el valor de prioridad de todos los procesos en colas
        update_queue_priorities(scheduler->high_queue, scheduler->current_tick);
        update_queue_priorities(scheduler->low_queue, scheduler->current_tick);
        // Ingresar proceso a la CPU
        if (scheduler->running_process == NULL) {
            // Cuando se activa un evento
            if (scheduler->active_event != NULL) {
                start_event_process(scheduler);
            } else if (!is_empty(scheduler->high_queue) || !is_empty(scheduler->low_queue)) {
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