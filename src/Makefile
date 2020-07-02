CFLAGS_OPT  ?= -O0
CC_NOCOLOR = 1
WERROR = 0

APPDIR = $(CURDIR)
BOARDSDIR = $(CURDIR)/platforms
BINDIR = $(CURDIR)/../lib/RIOT
RIOTBASE ?= $(CURDIR)/../RIOT

#QUIET ?= 1

USB_VID ?= 1D50
USB_PID ?= 6018

CFLAGS += -DCONFIG_USB_VID=0x$(USB_VID) -DCONFIG_USB_PID=0x$(USB_PID)

#BIN_USEMODULE += $(APPLICATION_MODULE)
BIN_DIRS += target
PROBE_HOST ?= $(BOARD)
PLATFORM_DIR = $(BOARDSDIR)/$(PROBE_HOST)
#VPATH += $(PLATFORM_DIR) platforms/common target
ENABLE_DEBUG ?= 0
USEMODULE += xtimer blackmagic_target
USEMODULE += isrpipe usbus usbus_cdc_acm isrpipe_read_timeout
#USEMODULE += auto_init_usbus stdio_cdc_acm
#USEMODULE += core_thread_flags 

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

ifndef TARGET
TARGET=blackmagic
endif

ifdef NO_OWN_LL
#SRC += jtagtap_generic.c swdptap_generic.c
endif

ifndef OWN_HL
SRC += jtag_scan.c jtagtap.c swdptap.c
SRC += remote.c
else
#CFLAGS += -DOWN_HL
endif

include $(RIOTBASE)/Makefile.include
