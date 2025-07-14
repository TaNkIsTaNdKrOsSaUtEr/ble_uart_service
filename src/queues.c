#include "queues.h"
#include "hardware/sync.h"

#define UART_RX_QUEUE_SIZE 256
#define DATA_QUEUE_SIZE 512

// UART RX Queue
static uint8_t uart_rx_buffer[UART_RX_QUEUE_SIZE];
static volatile size_t uart_rx_head = 0;
static volatile size_t uart_rx_tail = 0;
static spin_lock_t *uart_rx_lock;

// Data Queue
static uint8_t data_buffer[DATA_QUEUE_SIZE];
static volatile size_t data_head = 0;
static volatile size_t data_tail = 0;
static spin_lock_t *data_lock;

// UART RX Queue functions
void uart_rx_queue_init(void) {
    uart_rx_lock = spin_lock_init(spin_lock_claim_unused(true));
    uart_rx_head = 0;
    uart_rx_tail = 0;
}

bool uart_rx_queue_put(uint8_t data) {
    uint32_t save = spin_lock_blocking(uart_rx_lock);
    size_t next_head = (uart_rx_head + 1) % UART_RX_QUEUE_SIZE;
    if (next_head == uart_rx_tail) {
        spin_unlock(uart_rx_lock, save);
        return false; // Queue full
    }
    uart_rx_buffer[uart_rx_head] = data;
    uart_rx_head = next_head;
    spin_unlock(uart_rx_lock, save);
    return true;
}

bool uart_rx_queue_get(uint8_t *data) {
    uint32_t save = spin_lock_blocking(uart_rx_lock);
    if (uart_rx_head == uart_rx_tail) {
        spin_unlock(uart_rx_lock, save);
        return false; // Queue empty
    }
    *data = uart_rx_buffer[uart_rx_tail];
    uart_rx_tail = (uart_rx_tail + 1) % UART_RX_QUEUE_SIZE;
    spin_unlock(uart_rx_lock, save);
    return true;
}

bool uart_rx_queue_empty(void) {
    uint32_t save = spin_lock_blocking(uart_rx_lock);
    bool empty = (uart_rx_head == uart_rx_tail);
    spin_unlock(uart_rx_lock, save);
    return empty;
}

bool uart_rx_queue_full(void) {
    uint32_t save = spin_lock_blocking(uart_rx_lock);
    bool full = ((uart_rx_head + 1) % UART_RX_QUEUE_SIZE) == uart_rx_tail;
    spin_unlock(uart_rx_lock, save);
    return full;
}

size_t uart_rx_queue_count(void) {
    uint32_t save = spin_lock_blocking(uart_rx_lock);
    size_t count = (uart_rx_head - uart_rx_tail + UART_RX_QUEUE_SIZE) % UART_RX_QUEUE_SIZE;
    spin_unlock(uart_rx_lock, save);
    return count;
}

// Data Queue functions
void data_queue_init(void) {
    data_lock = spin_lock_init(spin_lock_claim_unused(true));
    data_head = 0;
    data_tail = 0;
}

bool data_queue_put(uint8_t data) {
    uint32_t save = spin_lock_blocking(data_lock);
    size_t next_head = (data_head + 1) % DATA_QUEUE_SIZE;
    if (next_head == data_tail) {
        spin_unlock(data_lock, save);
        return false; // Queue full
    }
    data_buffer[data_head] = data;
    data_head = next_head;
    spin_unlock(data_lock, save);
    return true;
}

bool data_queue_get(uint8_t *data) {
    uint32_t save = spin_lock_blocking(data_lock);
    if (data_head == data_tail) {
        spin_unlock(data_lock, save);
        return false; // Queue empty
    }
    *data = data_buffer[data_tail];
    data_tail = (data_tail + 1) % DATA_QUEUE_SIZE;
    spin_unlock(data_lock, save);
    return true;
}

bool data_queue_empty(void) {
    uint32_t save = spin_lock_blocking(data_lock);
    bool empty = (data_head == data_tail);
    spin_unlock(data_lock, save);
    return empty;
}

bool data_queue_full(void) {
    uint32_t save = spin_lock_blocking(data_lock);
    bool full = ((data_head + 1) % DATA_QUEUE_SIZE) == data_tail;
    spin_unlock(data_lock, save);
    return full;
}

size_t data_queue_count(void) {
    uint32_t save = spin_lock_blocking(data_lock);
    size_t count = (data_head - data_tail + DATA_QUEUE_SIZE) % DATA_QUEUE_SIZE;
    spin_unlock(data_lock, save);
    return count;
}
