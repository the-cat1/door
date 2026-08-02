# Makefile


# Export root
export ROOT_PATH 		:= $(PWD)


BOCHS_SRC 			= tools/bochs/bochsrc.txt
SUBDIR				= bootloader kernel

# Output files
FLOPPY				= floppy.img

include Makefile.config

all:
	$(call make_subdir)

floppy:
	$(MAKE) all
	$(MFORMAT) -f 1440 -C -i $(FLOPPY) ::
	$(MMD) -i $(FLOPPY) ::/boot
	$(MCOPY) -i $(FLOPPY) bootloader/loader.bin ::/boot/
	$(MCOPY) -i $(FLOPPY) bootloader/loader.cfg ::/boot/
	$(MCOPY) -i $(FLOPPY) kernel/kernel ::/boot/
	$(DD) if=bootloader/bootsect.bin of=$(FLOPPY) bs=512 count=1 conv=notrunc

# Run the floppy image in QEMU or Bochs
qemu:
	$(MAKE) floppy
	$(QEMU) -fda $(FLOPPY)

qemu-kernel:
	$(MAKE) all
	$(QEMU) -s -kernel kernel/kernel

qemu-kernel-dbg:
	$(MAKE) all
	cd kernel; $(QEMU) -s -S -kernel kernel & \
	$(GDB) -ex "target remote localhost:1234" -ex "symbol-file kernel"

bochs:
	$(MAKE) floppy
	$(BOCHS) -q -f $(BOCHS_SRC)

bochs-dbg:
	$(MAKE) floppy
	$(BOCHS_DBG) -f $(BOCHS_SRC)

clean:
	$(call make_subdir, clean)
	$(RM) $(FLOPPY)

help:
	@echo "Makefile for Door bootloader"
	@echo "Usage:"
	@echo "  make all               - Build the floppy disk image"
	@echo "  make qemu              - Run the floppy image in QEMU"
	@echo "  make qemu-kernel       - Run the kernel in QEMU"
	@echo "  make qemu-kernel-dbg   - Run the kernel in QEMU with debugger"
	@echo "  make bochs             - Run the floppy image in Bochs"
	@echo "  make bochs-dbg         - Run the floppy image in Bochs with debugger"
	@echo "  make clean             - Clean up generated files"
	@echo "  make help              - Show this help message"

.PHONY : all qemu qemu-kernel qemu-kernel-dbg bochs bochs-dbg clean help floppy
