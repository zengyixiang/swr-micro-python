QSTR_DEFS += qstrdefsport.h

# Include the core environment definitions; this will set $(TOP).
include ../../py/mkenv.mk

# Include py core make definitions.
include $(TOP)/py/py.mk
include $(TOP)/extmod/extmod.mk
# Set makefile-level MicroPython feature configurations.
MICROPY_ROM_TEXT_COMPRESSION ?= 0

# Define toolchain and other tools.
CROSS_COMPILE ?= arm-none-eabi-
DFU ?= $(TOP)/tools/dfu.py
PYDFU ?= $(TOP)/tools/pydfu.py

# Set CFLAGS.
CFLAGS += -I. -I$(TOP) -I$(BUILD) -I$(TOP)/../Include
CFLAGS += -Wall -Werror -std=c11 # -nostdlib
CFLAGS +=  -mtune=cortex-m7 -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb
CSUPEROPT = -Os # save some code space for performance-critical code
# $(info CFLAGS = $(CFLAGS))
# Select debugging or optimisation build.
ifeq ($(DEBUG), 1)
CFLAGS += -Og
else
CFLAGS += -Os -DNDEBUG
CFLAGS += -fdata-sections -ffunction-sections
endif

# Set linker flags.
LDFLAGS += -nostdlib -T stm32f405.ld --gc-sections

# Define the required source files.
SRC_C += lib.c main.c gccollect.c mphalport.c

SHARED_SRC_C += $(addprefix shared/,\
	netutils/dhcpserver.c \
	netutils/netutils.c \
	netutils/trace.c \
	readline/readline.c \
	runtime/gchelper_native.c \
	runtime/interrupt_char.c \
	runtime/mpirq.c \
	runtime/pyexec.c \
	runtime/stdout_helpers.c \
	runtime/sys_stdio_mphal.c \
	timeutils/timeutils.c \
	tinyusb/mp_usbd.c \
	tinyusb/mp_usbd_cdc.c \
	tinyusb/mp_usbd_descriptor.c \
	tinyusb/mp_usbd_runtime.c \
	)



SRC_O += \
	shared/runtime/gchelper_thumb2.o
# Define the required object files.
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_O))
OBJ += $(addprefix $(BUILD)/, $(SHARED_SRC_C:.c=.o))
OBJ += $(PY_CORE_O)
OBJ += $(PY_O)

# Define the top-level target, the main firmware.
all: $(BUILD)/firmware.dfu

$(BUILD)/firmware.elf: $(OBJ)
	$(ECHO) "LINK $@"
	$(Q)$(LD) $(LDFLAGS) -o $@ $^
	$(Q)$(SIZE) $@

$(BUILD)/firmware.bin: $(BUILD)/firmware.elf
	$(ECHO) "Create $@"
	$(Q)$(OBJCOPY) -O binary -j .isr_vector -j .text -j .data $^ $@

$(BUILD)/firmware.dfu: $(BUILD)/firmware.bin
	$(ECHO) "Create $@"
	$(Q)$(PYTHON) $(DFU) -b 0x08000000:$^ $@

SRC_QSTR += $(SRC_C) $(SRC_CXX) $(SHARED_SRC_C) $(GEN_PINS_SRC)

deploy: $(BUILD)/firmware.dfu
	$(Q)$(PYTHON) $(PYDFU) -u $^

# Include remaining core make rules.
include $(TOP)/py/mkrules.mk
