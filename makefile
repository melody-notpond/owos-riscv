CODE=src/
EMU=qemu-system-riscv64
EFLAGS=-machine virt -cpu rv64 -bios opensbi-riscv64-generic-fw_dynamic.bin -m 256m -nographic -device virtio-blk-device,scsi=off,drive=foo -global virtio-mmio.force-legacy=false -device virtio-gpu-device -s #-S

all: kernel

run:
	$(EMU) $(EFLAGS) -kernel build/boot/kernel -drive if=none,format=raw,file=build/drive.iso,id=foo

iso: kernel etc bin sbin
	ls build/drive.iso || dd if=/dev/zero of=build/drive.iso bs=1M count=2048
	mkfs.ext2 -F build/drive.iso
	mkdir -p mnt

mount: iso
	sudo mount build/drive.iso mnt
	sudo chown -R $(shell whoami) mnt
	cp -r build/root/* mnt/
	sudo chown root:root -R mnt/sbin
	sudo umount mnt

kernel: dirs
	cd $(CODE)boot/kernel/ && $(MAKE)
	mv $(CODE)boot/kernel/kernel build/boot/

etc: dirs
	echo -e "/dev/virt-blk7\t/\text2\trw" > build/root/etc/fstab

bin: simple

sbin: init

simple: dirs
	cd $(CODE)root/bin/simple && $(MAKE)
	mv $(CODE)root/bin/simple/simple build/root/bin/

init: dirs
	cd $(CODE)root/sbin/init && $(MAKE)
	mv $(CODE)root/sbin/init/init build/root/sbin

dirs:
	mkdir -p build/root/bin/ build/boot/ build/root/etc/ build/root/sbin

clean:
	-rm -r build/
	-cd src/boot/kernel && $(MAKE) clean

