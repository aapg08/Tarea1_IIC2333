#ifndef IO_UTILS_H
#define IO_UTILS_H

#include "../structs/scheduler.h"

void read_input_file(const char* filename, Scheduler** scheduler);
void write_output_file(const char* filename, Scheduler* scheduler);
void free_scheduler(Scheduler* scheduler);

#endif