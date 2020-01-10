OS_BINARY=kernel.bin
OS_BINARY_ELF=kernel.elf

GCC=powerpc-linux-gnu-gcc
OBJCP=powerpc-linux-gnu-objcopy
LD=powerpc-linux-gnu-ld
GCCOPT=-mcpu=powerpc

BUILD_DIR ?= build
SRC_DIRS ?= src

SRCS := $(shell find $(SRC_DIRS) -name "*.c" -or -name "*.S")
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

INC_DIRS := ./include
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

$(OS_BINARY) : $(OS_BINARY_ELF)
	$(OBJCP) -O binary  $< $@

$(OS_BINARY_ELF) : $(OBJS)
	$(LD) -T linker.ld $(OBJS) -o $@ -Map=output.map 

# assembly
$(BUILD_DIR)/%.S.o: %.S
	$(MKDIR_P) $(dir $@)
	$(GCC) $(INC_FLAGS) -nostdinc -fno-builtin -nostdlib -fno-stack-protector \
           -nostartfiles -nodefaultlibs -Wall -Wextra  -c -fno-pie \
		   -no-pie -ffreestanding -mcpu=8540 \
		   -mbig-endian -g -c -gstabs+ -Wa,-L -o $@ $<

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(GCC) $(INC_FLAGS) -nostdinc -fno-builtin -nostdlib -fno-stack-protector \
           -nostartfiles -nodefaultlibs -Wall -Wextra  -c -fno-pie \
		   -no-pie -ffreestanding -mbig-endian -mcpu=8540 -O0 -g $< -o $@

	
.PHONY: clean

clean:
	echo $(SRCS)
	$(RM) -r $(BUILD_DIR)
	rm -f -v *.o *.elf *.bin *.lst *.map

run:
	../../Qemu/build/ppc-softmmu/qemu-system-ppc -cpu e500 -d guest_errors,unimp -M ppce500 -serial stdio -s -bios $(OS_BINARY_ELF)

MKDIR_P ?= mkdir -p