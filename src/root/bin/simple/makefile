CODE=./
CC=riscv64-unknown-elf-gcc
CFLAGS=-march=rv64g -mabi=lp64 -static -mcmodel=medany -fvisibility=hidden -nostdlib -nostartfiles -Tlink.ld -g -Wall -Wextra

.PHONY: all clean

all: $(CODE)*.c $(CODE)*.s
	$(CC) $(CFLAGS) $? -o simple

clean:
	-rm -r obj/
	-rm *.o

