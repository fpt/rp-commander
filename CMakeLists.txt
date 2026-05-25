cmake_minimum_required(VERSION 3.13)

# waveshare_rp2040_zero if your SDK has it; otherwise 'pico' (generic RP2040) also works
set(PICO_BOARD pico CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(rp_commander C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(commander
    src/main.c
    src/lcd.c
    src/font.c
    src/encoder.c
    src/buttons.c
    src/usb_hid.c
    src/usb_descriptors.c
    src/profiles.c
    src/ui.c
    src/icons.c
    src/config_store.c
    src/cdc_config.c
)

# USB HID only — no USB CDC stdio (serial debugging via UART0 on GP0/GP1)
pico_enable_stdio_usb(commander 0)
pico_enable_stdio_uart(commander 1)

target_include_directories(commander PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
)

target_link_libraries(commander
    pico_stdlib
    hardware_clocks
    hardware_dma
    hardware_flash
    hardware_gpio
    hardware_pwm
    hardware_spi
    hardware_watchdog
    tinyusb_device
    tinyusb_board
    pico_unique_id
)

pico_add_extra_outputs(commander)
