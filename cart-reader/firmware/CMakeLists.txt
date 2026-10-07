cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(cart_reader C CXX ASM)
set(CMAKE_C_STANDARD 11)

pico_sdk_init()

add_executable(cart_reader
    src/main.c
    src/frame.c
    src/sr595.c
    src/bus.c
    src/flash_prog.c
    usb_descriptors.c
)

target_include_directories(cart_reader PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/../hardware
)

target_link_libraries(cart_reader PRIVATE
    pico_stdlib
    pico_multicore
    hardware_gpio
    tinyusb_device
    tinyusb_board
)

# Bus edges are cycle-counted. Debug (-O0) stretches /RD and misses the speed target.
set_source_files_properties(
    src/bus.c
    src/sr595.c
    PROPERTIES COMPILE_OPTIONS "-O2"
)

pico_add_extra_outputs(cart_reader)

# Build: cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
# Requires Pico SDK 2.x (tud_init / bsp/board_api.h).
