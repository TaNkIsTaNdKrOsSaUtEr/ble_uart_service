#ifndef QUEUES_H
#define QUEUES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// UART RX Queue
void uart_rx_queue_init(void);
bool uart_rx_queue_put(uint8_t data);
bool uart_rx_queue_get(uint8_t *data);
bool uart_rx_queue_empty(void);
bool uart_rx_queue_full(void);
size_t uart_rx_queue_count(void);

// Data processing queue
void data_queue_init(void);
bool data_queue_put(uint8_t data);
bool data_queue_get(uint8_t *data);
bool data_queue_empty(void);
bool data_queue_full(void);
size_t data_queue_count(void);

#endif
