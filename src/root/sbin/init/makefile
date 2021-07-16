CODE=./
CC=riscv64-unknown-elf-gcc
CFLAGS=-march=rv64gc -mabi=lp64d -static -mcmodel=medany -fvisibility=hidden -nostdlib -nostartfiles -Tlink.ld -g -Wall -Wextra

.PHONY: all clean

all: $(CODE)*.c $(CODE)*.s
	$(CC) $(CFLAGS) $? -o init

clean:
	-rm -r obj/
	-rm *.o

