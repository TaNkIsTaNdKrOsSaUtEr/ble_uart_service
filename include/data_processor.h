#ifndef DATA_PROCESSOR_H
#define DATA_PROCESSOR_H

#include <stdint.h>
#include <stddef.h>

void data_processor_init(void);
void process_data(const uint8_t *data, size_t length);

#endif
