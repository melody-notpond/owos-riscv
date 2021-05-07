CODE=src/root/
CC=riscv64-unknown-elf-gcc
CFLAGS=-march=rv64g -mabi=lp64 -static -mcmodel=medany -fvisibility=hidden -nostdlib -nostartfiles -Tlink.ld -g -Wall -Wextra
EMU=qemu-system-riscv64
EFLAGS=-machine virt -m 256m -nographic -device virtio-blk-device,scsi=off,drive=foo -bios none -global virtio-mmio.force-legacy=false -device virtio-gpu-device # -serial stdio

all: kernel #iso

run:
	$(EMU) $(EFLAGS) -kernel build/root/kernel -drive if=none,format=raw,file=build/drive.iso,id=foo

iso: kernel
	ls build/drive.iso || dd if=/dev/zero of=build/drive.iso bs=1M count=2048
	mkfs.ext2 -F build/drive.iso
	mkdir -p mnt

mount: iso
	sudo mount build/drive.iso mnt
	sudo chown -R $(shell whoami) mnt
	cp -r build/root/* mnt/
	sudo umount mnt


kernel: $(CODE)kernel/boot.s $(CODE)kernel/*.s $(CODE)kernel/*.c $(CODE)kernel/virtio/*.c $(CODE)kernel/filesystems/*.c
	mkdir -p build/root
	mkdir -p obj/
	$(CC) $(CFLAGS) $? -o kernel
	mv kernel build/root

clean:
	-rm -r obj/ build/

