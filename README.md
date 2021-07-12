# owos-riscv
Simple operating system for QEMU's virt board.

## Build requirements
Make sure you have [gcc for RISC-V](https://github.com/riscv/riscv-gnu-toolchain) installed.

## Build and run
```bash
make mount && make run
```

You must have `sudo` access to run `make mount, as it will attempt to mount a disk image to load files onto it. If you made changes to the code but don't want to rebuild the disk image, you can run `make` instead of `make mount`.
