#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../structs/scheduler.h"
#include "../io_utils/io_utils.h"


int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Uso: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    Scheduler* scheduler = NULL;

    // Leer archivo de input
    read_input_file(argv[1], &scheduler);

    // Aquí iría la lógica para ejecutar la simulación del scheduler
    // ...

    // Escribir resultados al archivo de output
    write_output_file(argv[2], scheduler);

    // Liberar memoria
    free_scheduler(scheduler);

    return 0;
}