MODULE = board

PICOSDK_PATH := $(CURDIR)/../../../pico-sdk
PICOSDK_RP2 := $(PICOSDK_PATH)/src/rp2_common
PICOSDK_COMMON := $(PICOSDK_PATH)/src/common
PICOSDK_BOARDS := $(PICOSDK_PATH)/src/boards
PICOSDK_RP2040 := $(PICOSDK_PATH)/src/rp2040

INCLUDES += -I$(PICOSDK_RP2040)/hardware_regs/include
INCLUDES += -I$(PICOSDK_STDIO)/include -I$(PICOSDK_PLATFORM)/include
INCLUDES += -I$(PICOSDK_BOARDS)/include -I$(PICOSDK_PICO_BASE)/include

TINYUSB_DIR := $(CURDIR)/../../../pico-sdk/lib/tinyusb
TINYUSB_LIB := $(TINYUSB_DIR)/lib
BOARD_PATH := $(PLATFORM_DIR)
FAMILY := rp2040

INCLUDES += -I$(TINYUSB_DIR)/hw -I$(TINYUSB_DIR)/src -I$(TINYUSB_DIR)/hw/bsp/rp2040
INCLUDES += -I$(TINYUSB_LIB)/SEGGER_RTT/RTT

SRC += \
	$(TINYUSB_LIB)/SEGGER_RTT/RTT/SEGGER_RTT.c \
	$(TINYUSB_LIB)/SEGGER_RTT/RTT/SEGGER_RTT_printf.c \
	$(TINYUSB_LIB)/SEGGER_RTT/Syscalls/SEGGER_RTT_Syscalls_GCC.c \
	$(TINYUSB_DIR)/hw/bsp/board.c \
	$(TINYUSB_DIR)/hw/bsp/rp2040/family.c \
	$(TINYUSB_DIR)/src/tusb.c \
	$(TINYUSB_DIR)/src/common/tusb_fifo.c \
	$(TINYUSB_DIR)/src/device/usbd.c \
	$(TINYUSB_DIR)/src/device/usbd_control.c \
	$(TINYUSB_DIR)/src/class/audio/audio_device.c \
	$(TINYUSB_DIR)/src/class/cdc/cdc_device.c \
	$(TINYUSB_DIR)/src/class/dfu/dfu_device.c \
	$(TINYUSB_DIR)/src/class/dfu/dfu_rt_device.c \
	$(TINYUSB_DIR)/src/class/hid/hid_device.c \
	$(TINYUSB_DIR)/src/class/midi/midi_device.c \
	$(TINYUSB_DIR)/src/class/msc/msc_device.c \
	$(TINYUSB_DIR)/src/class/net/net_device.c \
	$(TINYUSB_DIR)/src/class/usbtmc/usbtmc_device.c \
	$(TINYUSB_DIR)/src/class/vendor/vendor_device.c

include $(RIOTBASE)/Makefile.base