CODE=src/
EMU=qemu-system-riscv64
EFLAGS=-machine virt -m 256m -nographic -device virtio-blk-device,scsi=off,drive=foo -bios none -global virtio-mmio.force-legacy=false -device virtio-gpu-device -s #-S

all: mount

run:
	$(EMU) $(EFLAGS) -kernel build/boot/kernel -drive if=none,format=raw,file=build/drive.iso,id=foo

iso: kernel etc
	ls build/drive.iso || dd if=/dev/zero of=build/drive.iso bs=1M count=2048
	mkfs.ext2 -F build/drive.iso
	mkdir -p mnt

mount: iso
	sudo mount build/drive.iso mnt
	sudo chown -R $(shell whoami) mnt
	cp -r build/root/* mnt/
	sudo umount mnt

kernel:
	cd $(CODE)boot/kernel/ && $(MAKE)
	mkdir -p build/boot
	mv $(CODE)boot/kernel/kernel build/boot/

etc:
	mkdir -p build/root/etc
	echo -e "/dev/virt-blk7\t/\text2\trw" > build/root/etc/fstab

clean:
	-rm -r obj/ build/
	-cd src/boot/kernel && $(MAKE) clean

