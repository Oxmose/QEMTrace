OS_BINARY=os_binary.bin
OS_BINARY_TIME=os_binary_time.bin
OS_BINARY_ELF=os_binary.elf
OS_BINARY_TIME_ELF=os_binary_time.elf

SRCS = $(wildcard *.c *.S)
OBJS = $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(SRCS)))

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
export GCC=powerpc-linux-gnu-gcc
export OBJCP=powerpc-linux-gnu-objcopy
export LD=powerpc-linux-gnu-ld
export GCCOPT=-mcpu=powerpc
else
export GCC=ppc-gcc
export OBJCP=ppc-objcopy
export LD=ppc-ld
endif

# Build system image
all: clean $(OS_BINARY)

# Run QEMU on our system
run:
	../../../Qemu/build/ppc-softmmu/qemu-system-ppc -cpu e500 -d guest_errors,unimp -M ppce500 -serial stdio -s -bios $(OS_BINARY_ELF)

debug:
	../../../Qemu/build/ppc-softmmu/qemu-system-ppc -cpu e500 -d guest_errors,unimp -M ppce500 -serial stdio -s -S -bios $(OS_BINARY_ELF)

time: clean $(OS_BINARY_TIME)
	../../../Qemu/build/ppc-softmmu/qemu-system-ppc -cpu e500 -d guest_errors,unimp -M ppce500 -serial stdio -s -bios $(OS_BINARY_TIME_ELF)

debugtime: clean $(OS_BINARY_TIME)
	../../../Qemu/build/ppc-softmmu/qemu-system-ppc -cpu e500 -d guest_errors,unimp -M ppce500 -serial stdio -s -S -bios $(OS_BINARY_TIME_ELF)

$(OS_BINARY) : $(OS_BINARY_ELF)
	$(OBJCP) -O binary  $< $@

$(OS_BINARY_ELF) : $(OBJS) linker.ld
	$(LD) -T linker.ld $(OBJS) -o $@

$(OS_BINARY_TIME) : $(OS_BINARY_TIME_ELF)
	$(OBJCP) -O binary  $< $@

$(OS_BINARY_TIME_ELF) : pre_time $(OBJS) linker.ld
	$(LD) -T linker.ld $(OBJS) -o $@

pre_time:
	rm -f .tests.S
	echo "#include \"ppc-asm.h\"" > ./tests.S

	echo ".file	\"tests.S\"" >> ./tests.S
	echo ".text" >> ./tests.S

	echo "  /* Define kernel entry point */" >> ./tests.S
	echo ".globl	test_entry" >> ./tests.S
	echo ".type	test_entry,@function" >> ./tests.S

	echo "test_entry:" >> ./tests.S
	echo "/* PR */" >> ./tests.S
	echo "mflr r0" >> ./tests.S
	echo "stw r0, 8(sp)" >> ./tests.S
	echo "stwu sp, -16(sp)" >> ./tests.S
	echo "/* Enable tracing: 0xFFFFFFF0 */" >> ./tests.S
	echo ".long 0xFFFFFFF0" >> ./tests.S
	echo "/* Tests will be cat here */" >> ./tests.S

	cat "time.S_INC" >> ./tests.S

	echo "/* EP */" >> ./tests.S
	echo "/* Disable tracing: 0xFFFFFFF1 */" >> ./tests.S
	echo ".long 0xFFFFFFF1" >> ./tests.S
	echo "addi sp, sp, 16" >> ./tests.S
	echo "lwz r0, 8(sp)" >> ./tests.S
	echo "mtlr r0" >> ./tests.S
	echo "blr" >> ./tests.S

boot.o : boot.S
	$(GCC) -mbig-endian -g -c -gstabs+ -Wa,-alh=boot.lst,-L -o $@ $<

%.o : %.S
	$(GCC) -mbig-endian -g -c -gstabs+ -Wa,-L -o $@ $<

%.o: %.c
	$(GCC) -Iextra_freescale -c -mcpu=8540 -g $< -o $@

clean :
	rm -f -v *.o *.elf *.bin *.lst
