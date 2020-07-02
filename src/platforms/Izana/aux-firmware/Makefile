APPLICATION ?= MiyakoLink
DEBUG ?= 1
PREFIX := arm-none-eabi-
CC := $(PREFIX)gcc
LD := $(PREFIX)ld

.PHONY: aux_fw clean

SRC_DIR = src
SRC = $(SRC_DIR)/startup.c \
	  $(SRC_DIR)/system.c \
	  $(SRC_DIR)/main.c \
	  $(SRC_DIR)/device.c \
	  $(SRC_DIR)/target.c

BSP_DIR = bsp/lpc_chip_8xx
BSP_SRC_DIR = $(BSP_DIR)/src
BSP_SRC = $(BSP_SRC_DIR)/chip_8xx.c \
          $(BSP_SRC_DIR)/clock_8xx.c \
          $(BSP_SRC_DIR)/gpio_8xx.c \
          $(BSP_SRC_DIR)/irc_8xx.c \
          $(BSP_SRC_DIR)/pinint_8xx.c \
          $(BSP_SRC_DIR)/syscon_8xx.c

OBJECT_DIR = obj
BSP_OBJECT_DIR = $(OBJECT_DIR)/bsp

OBJECTS = $(addsuffix .o, $(addprefix $(OBJECT_DIR)/, $(basename $(notdir $(SRC)))))
OBJECTS += $(addsuffix .o, $(addprefix $(BSP_OBJECT_DIR)/, $(basename $(notdir $(BSP_SRC)))))

BIN_DIR = bin
TARGET_BIN = $(BIN_DIR)/$(APPLICATION)_aux_fw.elf

CFLAGS_CPU = -mcpu=cortex-m0plus -mthumb
ifeq ($(DEBUG), 1)
CFLAGS_OPT = -O0
else
CFLAGS_OPT = -Os	
endif
CFLAGS_DEFINE = -DCORE_M0PLUS
CFLAGS_INCLUDE = -Iinclude -I$(BSP_DIR)/inc
CFLAGS = $(CFLAGS_CPU) $(CFLAGS_OPT) $(CFLAGS_DEFINE) $(CFLAGS_INCLUDE)

LD_SCRIPT = aux_ld.ld
MAP_FILE = $(BIN_DIR)/$(APPLICATION)_aux_fw.map
LDFLAGS = -nostartfiles -Xlinker --gc-sections -Wl,-Map,$(MAP_FILE)

aux_fw: clean $(TARGET_BIN)
	@echo $(APPLICATION)

$(TARGET_BIN): $(OBJECTS) $(BSP_OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CC) -T $(LD_SCRIPT) -o $@ $^ $(LDFLAGS)
	
$(OBJECT_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BSP_OBJECT_DIR)/%.o: $(BSP_SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	@rm -rf $(OBJECT_DIR)
	@rm -rf $(BIN_DIR)