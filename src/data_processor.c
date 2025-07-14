#include "data_processor.h"
#include <stdio.h>
#include <string.h>

void data_processor_init(void) {
    printf("Data processor initialized\n");
}

void process_data(const uint8_t *data, size_t length) {
    // Simple data processing - just print for now
    printf("Processing %zu bytes: ", length);
    for (size_t i = 0; i < length && i < 32; i++) {
        if (data[i] >= 32 && data[i] < 127) {
            printf("%c", data[i]);
        } else {
            printf("\\x%02X", data[i]);
        }
    }
    if (length > 32) {
        printf("...");
    }
    printf("\n");
}
