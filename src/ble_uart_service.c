#include "ble_uart_service.h"
#include <stdio.h>
#include "pico/stdlib.h"

void ble_uart_service_init(void) {
    printf("BLE UART service initialized\n");
}

void ble_uart_service_run(void) {
    printf("BLE UART service running on core 1\n");
    while (1) {
        // Simple BLE service loop
        sleep_ms(1000);
        printf("BLE service heartbeat\n");
    }
}

void ble_uart_service_send(const uint8_t *data, size_t length) {
    printf("BLE send %zu bytes: ", length);
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
