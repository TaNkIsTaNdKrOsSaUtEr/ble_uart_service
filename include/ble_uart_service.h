#ifndef BLE_UART_SERVICE_H
#define BLE_UART_SERVICE_H

#include <stdint.h>
#include <stddef.h>

void ble_uart_service_init(void);
void ble_uart_service_run(void);
void ble_uart_service_send(const uint8_t *data, size_t length);

#endif
