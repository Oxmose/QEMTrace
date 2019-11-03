OS_BINARY=os_binary.bin
OS_BINARY_ELF=os_binary.elf

SRCS = $(wildcard *.c *.S)
OBJS = $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(SRCS)))

UNAME := $(shell uname)

AS = arm-none-eabi-as
LD = arm-none-eabi-ld
GCC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
QEMU = ../../../Qemu/build/arm-softmmu/qemu-system-arm

CFLAGS = -std=c11 -nostdinc -fno-builtin -nostdlib -fno-stack-protector \
         -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c \
		 -MD -ffreestanding -O0 -g
ASFLAGS = -march=armv7-a -mcpu=cortex-a15
LDFLAGS = -e loader -T linker.ld
QEMUOPTS = -rtc base=localtime -m 64M -gdb tcp::1234 -serial stdio\
		   -M vexpress-a15 -cpu cortex-a15 -kernel 


# Build system image
all: clean $(OS_BINARY)

# Run QEMU on our system
run:
	$(QEMU) $(QEMUOPTS) $(OS_BINARY_ELF)

debug:
	$(QEMU) $(QEMUOPTS) $(OS_BINARY_ELF) -S


$(OS_BINARY) : $(OS_BINARY_ELF)
	$(OBJCOPY) -O binary  $< $@

$(OS_BINARY_ELF) : $(OBJS) linker.ld
	$(LD) -T linker.ld $(OBJS) -o $@


%.o : %.S
	$(AS) $(ASFLAGS) $< -o $@

%.o: %.c
	$(GCC) $(CFLAGS) $< -o $@

clean :
	rm -f -v *.o *.elf *.bin
