OS_IMAGE=os_image.img
OS_BINARY=os_binary.bin
OS_IMAGE_TIME=os_image_time.img
OS_BINARY_TIME=os_binary_time.bin

# Build system image
all: clean $(OS_IMAGE)

# Run QEMU on our system
run:
	../../../Qemu/build/i386-softmmu/qemu-system-i386 -gdb tcp::1234 -drive format=raw,file=$(OS_IMAGE)

debug:
	../../../Qemu/build/i386-softmmu/qemu-system-i386 -S -gdb tcp::1234 -drive format=raw,file=$(OS_IMAGE)

time: clean $(OS_IMAGE_TIME)
	../../../Qemu/build/i386-softmmu/qemu-system-i386 -gdb tcp::1234 -drive format=raw,file=$(OS_IMAGE_TIME)

debugtime: clean $(OS_IMAGE_TIME)
	../../../Qemu/build/i386-softmmu/qemu-system-i386 -S -gdb tcp::1234 -drive format=raw,file=$(OS_IMAGE_TIME)

# Build system image
$(OS_IMAGE): bootstage012.bin
	cat $^ > $(OS_BINARY)
	../../../Qemu/build/qemu-img create -f raw $(OS_IMAGE) 4M
	dd status=noxfer conv=notrunc if=$(OS_BINARY) of=$(OS_IMAGE)

$(OS_IMAGE_TIME): bootstage01time.bin
	cat $^ > $(OS_BINARY_TIME)
	../../../Qemu/build/qemu-img create -f raw $(OS_IMAGE_TIME) 4M
	dd status=noxfer conv=notrunc if=$(OS_BINARY_TIME) of=$(OS_IMAGE_TIME)

bootstage012.bin: boot_stage0.bin boot_stage1.bin boot_stage2.bin idt_zone.bin
	cat $^ > $@

bootstage01time.bin: boot_stage0.bin boot_stage1.bin boot_stage_time.bin idt_zone.bin
	cat $^ > $@
# Build boot sector
%.bin:%.asm
	nasm $< -f bin -o $@

# Clean
clean:
	rm -rf *.o *.bin $(OS_BINARY) $(OS_IMAGE)
