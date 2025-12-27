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
CFLAGS += -I. -I$(TOP) -I$(BUILD) -I$(TOP)/../Include -I$(TOP)/../RTOS/FreeRTOS/Source/include -I$(TOP)/../RTOS/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -I$(TOP)/../main/inc
CFLAGS += -I$(TOP)/../GD32H7X_lib/GD32H7xx_standard_peripheral/Include
CFLAGS += -I$(TOP)/../GD32H7X_lib/CMSIS/GD/GD32H7xx/Include
CFLAGS += -I$(TOP)/../GD32H7X_lib/CMSIS/Core/Include
CFLAGS += -I$(TOP)/../micropython/
CFLAGS += -I$(TOP)/../micropython/ports/gd32h757
CFLAGS += -I$(TOP)/../app
CFLAGS += -I$(TOP)/../app/communication
CFLAGS += -I$(TOP)/../app/file_system
CFLAGS += -I$(TOP)/../app/ini
CFLAGS += -I$(TOP)/../app/protocol
CFLAGS += -I$(TOP)/../app/sensors
CFLAGS += -I$(TOP)/../main/inc
CFLAGS += -I$(TOP)/../Include
CFLAGS += -I$(TOP)/../
CFLAGS += -I$(TOP)/../RTOS/FreeRTOS/Source/include
CFLAGS += -I$(TOP)/../RTOS/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1
CFLAGS += -I$(TOP)/../driver
CFLAGS += -I$(TOP)/../driver/beep
CFLAGS += -I$(TOP)/../driver/ble
CFLAGS += -I$(TOP)/../driver/ble_dfu
CFLAGS += -I$(TOP)/../driver/buffer
CFLAGS += -I$(TOP)/../driver/CW2015
CFLAGS += -I$(TOP)/../driver/debug
CFLAGS += -I$(TOP)/../driver/fatfs
CFLAGS += -I$(TOP)/../driver/gui
CFLAGS += -I$(TOP)/../driver/i2c
CFLAGS += -I$(TOP)/../driver/key
CFLAGS += -I$(TOP)/../driver/lcd
CFLAGS += -I$(TOP)/../driver/lcdBacklight
CFLAGS += -I$(TOP)/../driver/led
CFLAGS += -I$(TOP)/../driver/power
CFLAGS += -I$(TOP)/../driver/random
CFLAGS += -I$(TOP)/../driver/sdio
CFLAGS += -I$(TOP)/../driver/sdram
CFLAGS += -I$(TOP)/../driver/slip
CFLAGS += -I$(TOP)/../driver/usb
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/core
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/display
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/draw
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/font
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/indev
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/layouts
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/layouts/flex
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/layouts/grid
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/misc
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/misc/cache
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/osal
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/stdlib
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/stdlib/builtin
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/stdlib/clib
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/stdlib/micropython
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/stdlib/rtthread
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/themes
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/themes/default
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/themes/mono
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/themes/simple
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/tick
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/animimage
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/arc
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/bar
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/button
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/buttonmatrix
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/calendar
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/canvas
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/chart
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/checkbox
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/dropdown
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/image
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/imagebutton
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/keyboard
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/label
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/led
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/line
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/list
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/lottie
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/menu
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/msgbox
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/objx_templ
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/property
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/roller
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/scale
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/slider
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/span
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/spinbox
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/spinner
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/switch
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/table
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/tabview
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/textarea
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/tileview
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/widgets/win
CFLAGS += -I$(TOP)/../GUI/lvgl
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/libs/bin_decoder
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/draw/sw
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/draw/sw/blend
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/others/sysmon
CFLAGS += -I$(TOP)/../GUI/lvgl/lvgl/src/others/observer
CFLAGS += -I$(TOP)/../GUI/my_gui
CFLAGS += -I$(TOP)/../GUI/mycode/driver
CFLAGS += -I$(TOP)/../GUI/mycode/gui
CFLAGS += -I$(TOP)/../GUI/mycode/img
CFLAGS += -I$(TOP)/../GUI/mycode/sensor
CFLAGS += -I$(TOP)/../GD32H7xx_usbhs_library/driver/Include
CFLAGS += -I$(TOP)/../GD32H7xx_usbhs_library/device/core/Include
CFLAGS += -I$(TOP)/../GD32H7xx_usbhs_library/device/class/cdc/Include
CFLAGS += -I$(TOP)/../GD32H7xx_usbhs_library/ustd/common
CFLAGS += -I$(TOP)/../GD32H7xx_usbhs_library/ustd/class/cdc
CFLAGS += -Wall -Wpointer-arith -Werror -Wdouble-promotion -Wfloat-conversion -std=gnu99 # -nostdlib
CFLAGS +=  -mtune=cortex-m7 -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb
CSUPEROPT = -Os # save some code space for performance-critical code
# $(info CFLAGS = $(CFLAGS))
# Select debugging or optimisation build.
ifeq ($(DEBUG), 1)
CFLAGS += -Og
else
CFLAGS += -Os -DNDEBUG -DGD32H7XX -DUSE_STDPERIPH_DRIVER -DCODE_IN_SDRAM -DUSE_USB_HS
CFLAGS += -Os -DUSE_USBHS0
CFLAGS += -fdata-sections -ffunction-sections
endif

# Set linker flags.
LDFLAGS += -nostdlib -T stm32f405.ld --gc-sections

# Define the required source files.
SRC_C += lib.c main.c gccollect.c mphalport.c mpthreadport.c sys_sensors.c sys_timer.c sys_ui.c

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
