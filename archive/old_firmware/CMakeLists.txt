cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
project(first_keeb C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_CXX_ENABLE_RTTI 1)
pico_sdk_init()

add_executable(main
main.cpp
  ssd1306/ssd1306.c
  usb_descriptors.c
)

target_include_directories(main PRIVATE  ${CMAKE_CURRENT_LIST_DIR})
pico_generate_pio_header(main ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

target_link_libraries(main 
  pico_multicore
  pico_stdlib 
  hardware_adc 
  hardware_i2c 
  hardware_pio
  tinyusb_board
  tinyusb_device
  hardware_flash
  hardware_sync
)

pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 1)
pico_add_extra_outputs(main)
