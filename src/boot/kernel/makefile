CODE=src/
CC=riscv64-unknown-elf-gcc
CFLAGS=-march=rv64g -mabi=lp64 -static -mcmodel=medany -fvisibility=hidden -nostdlib -nostartfiles -Tkernel.ld -g -Wall -Wextra

.PHONY: all clean

all: main lib userspace drivers
	mkdir -p obj/
	mv *.o obj/
	mv obj/boot.o obj/1.o
	$(CC) $(CFLAGS) obj/*.o -o kernel

main: $(CODE)*.c $(CODE)*.s
	$(CC) $(CFLAGS) -c $?

lib: $(CODE)lib/*.c
	$(CC) $(CFLAGS) -c $?

userspace: $(CODE)userspace/*.c $(CODE)userspace/*.s
	$(CC) $(CFLAGS) -c $?

drivers: drivers/filesystems drivers/uart drivers/virtio

drivers/filesystems: $(CODE)drivers/filesystems/*.c
	$(CC) $(CFLAGS) -c $?

drivers/uart: $(CODE)drivers/uart/*.c $(CODE)drivers/uart/*.s
	$(CC) $(CFLAGS) -c $?

drivers/virtio: $(CODE)drivers/virtio/*.c
	$(CC) $(CFLAGS) -c $?

clean:
	-rm -r obj/
	-rm *.o

