cmake_minimum_required(VERSION 3.16)

set(PICO_SDK_FETCH_FROM_GIT ON)
set(FREERTOS_KERNEL_PATH "../FreeRTOS-Kernel")
set(PICO_BOARD none)
#set(PICO_DEOPTIMIZED_DEBUG 1)
#set(LOGGER rtt)
#add_compile_definitions(PICO_STDIO_STACK_BUFFER_SIZE=2048)
add_compile_definitions(ENABLE_DEBUG=1)
add_compile_definitions(PC_HOSTED=0)
add_compile_definitions(PICO_FLASH_SIZE_BYTES=16777216)
add_compile_definitions(PICO_RP2040_B0_SUPPORTED=0)
include(pico_sdk_import.cmake)
include(FreeRTOS_Kernel_import.cmake)

project(MiyakoLink)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_subdirectory(src)