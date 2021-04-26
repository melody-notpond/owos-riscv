CODE=src/
CC=riscv64-unknown-elf-gcc
CFLAGS=-march=rv64g -mabi=lp64 -static -mcmodel=medany -fvisibility=hidden -nostdlib -nostartfiles -Tlink.ld
EMU=qemu-system-riscv64
EFLAGS=--machine sifive_u --nographic --bios none

all: boot
	$(EMU) $(EFLAGS) --kernel build/boot

boot: $(CODE)boot/*.s
	mkdir -p obj/boot/
	mkdir -p build
	$(CC) $(CFLAGS) -c $?
	mv *.o obj/boot/
	$(CC) $(CFLAGS) obj/boot/*.o -o build/boot

clean:
	-rm -r obj/ build/
