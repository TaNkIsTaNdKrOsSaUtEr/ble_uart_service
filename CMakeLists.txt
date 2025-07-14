cmake_minimum_required(VERSION 3.13)

# Include pico-sdk
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(pico_uart_ble_bridge C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

# Add executable
add_executable(pico_uart_ble_bridge
    src/main.c
    src/queues.c
    src/data_processor.c
    src/ble_uart_service.c
)

# Include directories
target_include_directories(pico_uart_ble_bridge PRIVATE
    include/
)

# Link libraries
target_link_libraries(pico_uart_ble_bridge
    pico_stdlib
    hardware_uart
    hardware_gpio
    hardware_timer
    hardware_sync
    pico_multicore
)

# Enable USB output, disable UART output for debugging
pico_enable_stdio_usb(pico_uart_ble_bridge 1)
pico_enable_stdio_uart(pico_uart_ble_bridge 0)

# Create basic outputs
pico_set_program_name(pico_uart_ble_bridge "UART BLE Bridge")
pico_set_program_version(pico_uart_ble_bridge "1.0")

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(pico_uart_ble_bridge)
