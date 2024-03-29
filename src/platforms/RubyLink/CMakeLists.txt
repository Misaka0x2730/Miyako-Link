add_executable(RubyLink)

add_compile_definitions(NO_LIBOPENCM3)

set(MiyakoLink_src_path ${CMAKE_CURRENT_SOURCE_DIR}/../..)

target_include_directories(RubyLink PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/Ethernet
        ${CMAKE_CURRENT_SOURCE_DIR}/Ethernet/W5500
        ${CMAKE_CURRENT_SOURCE_DIR}/Internet/DHCP
        )

target_include_directories(RubyLink PUBLIC
        ${MiyakoLink_src_path}/include
        ${MiyakoLink_src_path}/platforms/common
        ${MiyakoLink_src_path}/target
        )

target_sources(RubyLink PUBLIC
        ${MiyakoLink_src_path}/main.c
        ${MiyakoLink_src_path}/target/adiv5.c
        ${MiyakoLink_src_path}/target/adiv5_jtagdp.c
        ${MiyakoLink_src_path}/target/adiv5_swdp.c
        ${MiyakoLink_src_path}/command.c
        ${MiyakoLink_src_path}/target/cortexa.c
        ${MiyakoLink_src_path}/target/cortexm.c
        ${MiyakoLink_src_path}/crc32.c
        ${MiyakoLink_src_path}/target/efm32.c
        ${MiyakoLink_src_path}/exception.c
        ${MiyakoLink_src_path}/gdb_main.c
        ${MiyakoLink_src_path}/gdb_hostio.c
        ${MiyakoLink_src_path}/gdb_packet.c
        ${MiyakoLink_src_path}/general.c
        ${MiyakoLink_src_path}/hex_utils.c
        ${MiyakoLink_src_path}/target/jtag_devs.c
        ${MiyakoLink_src_path}/target/lmi.c
        ${MiyakoLink_src_path}/target/lpc_common.c
        ${MiyakoLink_src_path}/target/lpc11xx.c
        ${MiyakoLink_src_path}/target/lpc17xx.c
        ${MiyakoLink_src_path}/target/lpc15xx.c
        ${MiyakoLink_src_path}/target/lpc43xx.c
        ${MiyakoLink_src_path}/target/lpc546xx.c
        ${MiyakoLink_src_path}/target/kinetis.c
        ${MiyakoLink_src_path}/morse.c
        ${MiyakoLink_src_path}/target/msp432.c
        ${MiyakoLink_src_path}/target/nrf51.c
        ${MiyakoLink_src_path}/target/nxpke04.c
        ${MiyakoLink_src_path}/target/rp.c
        ${MiyakoLink_src_path}/target/sam3x.c
        ${MiyakoLink_src_path}/target/sam4l.c
        ${MiyakoLink_src_path}/target/samd.c
        ${MiyakoLink_src_path}/target/samx5x.c
        ${MiyakoLink_src_path}/target/stm32f1.c
        ${MiyakoLink_src_path}/target/stm32f4.c
        ${MiyakoLink_src_path}/target/stm32g0.c
        ${MiyakoLink_src_path}/target/stm32h7.c
        ${MiyakoLink_src_path}/target/stm32l0.c
        ${MiyakoLink_src_path}/target/stm32l4.c
        ${MiyakoLink_src_path}/target/target.c
        ${MiyakoLink_src_path}/target/jtag_scan.c
        ${MiyakoLink_src_path}/remote.c
        )

target_sources(RubyLink PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/board.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gdb_if.c
        ${CMAKE_CURRENT_SOURCE_DIR}/platform.c
        ${CMAKE_CURRENT_SOURCE_DIR}/platform_target_interface_iso.c
        ${CMAKE_CURRENT_SOURCE_DIR}/platform_target_interface_non_iso.c
        ${CMAKE_CURRENT_SOURCE_DIR}/jtagtap.c
        ${CMAKE_CURRENT_SOURCE_DIR}/swdptap.c
        ${CMAKE_CURRENT_SOURCE_DIR}/timing.c
        ${CMAKE_CURRENT_SOURCE_DIR}/usb_descriptors.c
        ${CMAKE_CURRENT_SOURCE_DIR}/system_func.c
        ${CMAKE_CURRENT_SOURCE_DIR}/system_pins.c
        ${CMAKE_CURRENT_SOURCE_DIR}/mux_task.c
        ${CMAKE_CURRENT_SOURCE_DIR}/mem_manager.c
        ${CMAKE_CURRENT_SOURCE_DIR}/eeprom.c
        ${CMAKE_CURRENT_SOURCE_DIR}/device_config.c
        ${CMAKE_CURRENT_SOURCE_DIR}/usb_task.c
        ${CMAKE_CURRENT_SOURCE_DIR}/usb_cdc_task.c
        ${CMAKE_CURRENT_SOURCE_DIR}/usb_uart_task.c
        ${CMAKE_CURRENT_SOURCE_DIR}/Ethernet/socket.c
        ${CMAKE_CURRENT_SOURCE_DIR}/Ethernet/wizchip_conf.c
        ${CMAKE_CURRENT_SOURCE_DIR}/Ethernet/W5500/w5500.c
        ${CMAKE_CURRENT_SOURCE_DIR}/Internet/DHCP/dhcp.c
        )

pico_generate_pio_header(RubyLink ${CMAKE_CURRENT_LIST_DIR}/pio/target_interface_non_iso_swd_low_write.pio)
pico_generate_pio_header(RubyLink ${CMAKE_CURRENT_LIST_DIR}/pio/target_interface_non_iso_swd_low_access_write.pio)
pico_generate_pio_header(RubyLink ${CMAKE_CURRENT_LIST_DIR}/pio/target_interface_non_iso_swd_low_access_read.pio)
pico_generate_pio_header(RubyLink ${CMAKE_CURRENT_LIST_DIR}/pio/target_interface_non_iso_swd_low_seq_out.pio)
pico_generate_pio_header(RubyLink ${CMAKE_CURRENT_LIST_DIR}/pio/target_interface_non_iso_swd_low_seq_in.pio)

pico_generate_pio_header(RubyLink ${CMAKE_CURRENT_LIST_DIR}/pio/target_interface_non_iso_jtag_tdi_tdo_seq.pio)
pico_generate_pio_header(RubyLink ${CMAKE_CURRENT_LIST_DIR}/pio/target_interface_non_iso_jtag_next.pio)

target_link_libraries(RubyLink PUBLIC hardware_spi hardware_i2c hardware_pio hardware_uart hardware_dma hardware_timer)
target_link_libraries(RubyLink PUBLIC pico_stdlib tinyusb_device tinyusb_board tinyusb_additions)
target_link_libraries(RubyLink PUBLIC FreeRTOS-Kernel FreeRTOS-Kernel-Heap1)

get_target_property(DirDefs tinyusb_common_base INTERFACE_COMPILE_DEFINITIONS)
list(FILTER DirDefs EXCLUDE REGEX [[^CFG_TUSB_OS=OPT_OS_PICO$]])
set_property(TARGET tinyusb_common_base PROPERTY INTERFACE_COMPILE_DEFINITIONS ${DirDefs})
target_compile_definitions(tinyusb_common_base INTERFACE CFG_TUSB_OS=OPT_OS_FREERTOS)

IF(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

pico_add_extra_outputs(RubyLink)