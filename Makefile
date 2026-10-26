TARGET_COMPILE ?= aarch64-none-elf-
CC := $(TARGET_COMPILE)gcc
LD := $(TARGET_COMPILE)ld
OBJCOPY := $(TARGET_COMPILE)objcopy

MODULE_NAME := krypt

INCLUDE_DIRS := \
	-Ikernel/include \
	-kernel/patch/include \
	-kernel/linux/include \
	-kernel/linux/arch/arm64/include

CFLAGS := -Wall -Werror -O2 -fno-strict-aliasing -fno-common -fshort-wchar -fno-PIE -fno-stack-protector -mgeneral-regs-only -mcmodel=large
LDFLAGS := -T hello.lds -e kpm_init -z norelro

OBJS := krypt.o

all: $(MODULE_NAME).kpm

$(MODULE_NAME).kpm: $(OBJS)
	$(LD) $(LDFLAGS) -o $(MODULE_NAME).elf $(OBJS)
	$(OBJCOPY) -O binary $(MODULE_NAME).elf $(MODULE_NAME).kpm

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@

clean:
	rm -f *.o *.elf *.kpm
