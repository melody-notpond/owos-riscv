CODE=src/
CC=riscv64-unknown-elf-gcc
CFLAGS=-march=rv64g -mabi=lp64 -static -mcmodel=medany -fvisibility=hidden -nostdlib -nostartfiles -Tlink.ld
EMU=qemu-system-riscv64
EFLAGS=--machine sifive_u --bios none --nographic -m 6g

all: boot iso
	$(EMU) $(EFLAGS) --kernel build/boot --drive format=raw,file=build/sd.iso

run:
	$(EMU) $(EFLAGS) --kernel build/boot

build: boot iso

boot: $(CODE)boot/*.s
	mkdir -p obj/boot/
	mkdir -p build
	$(CC) $(CFLAGS) -c $?
	mv *.o obj/boot/
	$(CC) $(CFLAGS) obj/boot/*.o -o build/boot

iso: kernel
	dd if=/dev/zero of=build/sd.iso bs=1M count=2048
	mkfs.ext2 -F build/sd.iso
	mkdir -p mnt
	mount build/sd.iso mnt
	cp -r build/root/* mnt/
	umount mnt

kernel: src/root/kernel.c
	mkdir -p build/root
	$(CC) $(CFLAGS) $? -o kernel
	mv kernel build/root

clean:
	-rm -r obj/ build/

