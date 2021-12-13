APPLICATION = MiyakoLink
export APPLICATION

CFLAGS_OPT  ?= -O0
CC_NOCOLOR = 1
WERROR = 0

APPDIR = $(CURDIR)
BOARDSDIR = $(CURDIR)/platforms
BINDIR = $(CURDIR)/../lib/RIOT
RIOTBASE ?= $(CURDIR)/../RIOT
OBJCOPY := arm-none-eabi-objcopy
QUIET = 0
V = 1

BOARD ?= RubyLink
PROBE_HOST ?= $(BOARD)
PLATFORM_DIR = $(BOARDSDIR)/$(PROBE_HOST)
USEMODULE += xtimer
USEMODULE += isrpipe isrpipe_read_timeout

ifneq ($(V), 1)
MAKEFLAGS += --no-print-dir
Q := @
endif

INCLUDES += -I$(CURDIR)/include -I$(CURDIR)/platforms/common -I$(PLATFORM_DIR) -I$(CURDIR)/target

CFLAGS += -DNO_LIBOPENCM3

ifeq ($(ENABLE_DEBUG), 1)
CFLAGS += -DENABLE_DEBUG
endif

SRC =			\
	adiv5.c		\
	adiv5_jtagdp.c	\
	adiv5_swdp.c	\
	command.c	\
	cortexa.c	\
	cortexm.c	\
	crc32.c		\
	efm32.c		\
	exception.c	\
	gdb_if.c	\
	gdb_main.c	\
	gdb_hostio.c	\
	gdb_packet.c	\
	hex_utils.c	\
	jtag_devs.c	\
	lmi.c		\
	lpc_common.c	\
	lpc11xx.c	\
	lpc17xx.c	\
	lpc15xx.c	\
	lpc43xx.c	\
	kinetis.c	\
	main.c		\
	morse.c		\
	msp432.c	\
	nrf51.c		\
	nxpke04.c	\
	platform.c	\
	sam3x.c		\
	sam4l.c		\
	samd.c		\
	samx5x.c	\
	stm32f1.c	\
	stm32f4.c	\
	stm32h7.c	\
	stm32l0.c	\
	stm32l4.c	\
	target.c	\
	jtag_scan.c \
	jtagtap.c   \
	swdptap.c   \
    remote.c    \

TINYUSB_DIR := $(CURDIR)/../pico-sdk/lib/tinyusb
BOARD_PATH := $(PLATFORM_DIR)
FAMILY := rp2040

INCLUDES += -I$(TINYUSB_DIR)/hw -I$(TINYUSB_DIR)/src -I$(TINYUSB_DIR)/hw/bsp/rp2040

SRC += \
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

ifndef TARGET
TARGET=blackmagic
endif

include $(RIOTBASE)/Makefile.include
