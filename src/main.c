#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "pt_cornell_rp2040_v1_3.h"
#include "queues.h"
#include "data_processor.h"
#include "ble_uart_service.h"
#include "pico/multicore.h"

// UART configuration
#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

// Status LED
#define LED_PIN 25

// Thread control structures
static struct pt uart_pt, processor_pt, status_pt;

// Global status variables
static volatile bool ble_connected = false;
static volatile bool ble_authenticated = false;

// BLE status functions
bool is_client_connected(void) {
    return ble_connected;
}

bool is_client_authenticated(void) {
    return ble_authenticated;
}

void set_ble_status(bool connected, bool authenticated) {
    ble_connected = connected;
    ble_authenticated = authenticated;
}

// UART interrupt handler
void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);
        if (!uart_rx_queue_full()) {
            uart_rx_queue_put(ch);
        }
    }
}

// UART handler thread
PT_THREAD(uart_handler_thread(struct pt *pt)) {
    PT_BEGIN(pt);
    
    while (1) {
        // Wait for data in UART RX queue
        PT_WAIT_UNTIL(pt, !uart_rx_queue_empty());
        
        // Process received data
        uint8_t data;
        if (uart_rx_queue_get(&data)) {
            // Add to processing queue
            if (!data_queue_full()) {
                data_queue_put(data);
            }
        }
        
        PT_YIELD(pt);
    }
    
    PT_END(pt);
}

// Data processor thread
PT_THREAD(data_processor_thread(struct pt *pt)) {
    static uint8_t buffer[256];
    static size_t buffer_len = 0;
    
    PT_BEGIN(pt);
    
    while (1) {
        // Wait for data to process
        PT_WAIT_UNTIL(pt, !data_queue_empty());
        
        // Get data from queue
        uint8_t data;
        if (data_queue_get(&data)) {
            buffer[buffer_len++] = data;
            
            // Process when buffer has data or on newline
            if (buffer_len >= sizeof(buffer) - 1 || data == '\n' || data == '\r') {
                buffer[buffer_len] = '\0';
                
                // Process the data
                process_data(buffer, buffer_len);
                
                // Send via BLE if connected
                if (is_client_connected()) {
                    ble_uart_service_send(buffer, buffer_len);
                }
                
                buffer_len = 0;
            }
        }
        
        PT_YIELD(pt);
    }
    
    PT_END(pt);
}

// Status monitor thread
PT_THREAD(status_monitor_thread(struct pt *pt)) {
    static uint32_t last_blink = 0;
    static bool led_state = false;
    
    PT_BEGIN(pt);
    
    while (1) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        
        // Blink LED based on connection status
        if (is_client_connected()) {
            if (is_client_authenticated()) {
                // Fast blink when authenticated
                if (now - last_blink > 200) {
                    led_state = !led_state;
                    gpio_put(LED_PIN, led_state);
                    last_blink = now;
                }
            } else {
                // Slow blink when connected but not authenticated
                if (now - last_blink > 1000) {
                    led_state = !led_state;
                    gpio_put(LED_PIN, led_state);
                    last_blink = now;
                }
            }
        } else {
            // LED off when disconnected
            gpio_put(LED_PIN, false);
            led_state = false;
        }
        
        // Print status every 5 seconds
        static uint32_t last_status = 0;
        if (now - last_status > 5000) {
            printf("Status: BLE %s, Auth %s, RX Queue: %d, Data Queue: %d\n",
                   is_client_connected() ? "Connected" : "Disconnected",
                   is_client_authenticated() ? "Yes" : "No",
                   uart_rx_queue_count(),
                   data_queue_count());
            last_status = now;
        }
        
        PT_YIELD(pt);
    }
    
    PT_END(pt);
}

// Core 1 entry point (BLE stack)
void core1_entry() {
    printf("Core 1: Starting BLE service\n");
    ble_uart_service_run(); // This function never returns
}

int main() {
    // Initialize stdio
    stdio_init_all();
    
    // Wait a bit for USB to initialize
    sleep_ms(2000);
    printf("UART-BLE Bridge Starting...\n");
    
    // Initialize LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, false);
    
    // Initialize UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    
    // Enable UART interrupt
    uart_set_irq_enables(UART_ID, true, false);
    irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
    irq_set_enabled(UART0_IRQ, true);
    
    // Initialize queues
    uart_rx_queue_init();
    data_queue_init();
    
    // Initialize data processor
    data_processor_init();
    
    // Initialize BLE service
    ble_uart_service_init();
    
    // Initialize protothreads
    PT_INIT(&uart_pt);
    PT_INIT(&processor_pt);
    PT_INIT(&status_pt);
    
    printf("Starting BLE service on core 1...\n");
    
    // Start BLE service on core 1
    multicore_launch_core1(core1_entry);
    
    printf("Core 0: Starting main loop\n");
    
    // Main loop on core 0
    while (1) {
        // Run all threads
        uart_handler_thread(&uart_pt);
        data_processor_thread(&processor_pt);
        status_monitor_thread(&status_pt);
        
        // Small delay to prevent busy waiting
        sleep_ms(1);
    }
    
    return 0;
}
