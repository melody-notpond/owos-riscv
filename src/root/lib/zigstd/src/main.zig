pub fn write(fd: u32, buffer: []const u8, size: usize) void {
    const write_syscall: u64 = 1;
    asm volatile ("ecall"
        :
        : [write_syscall] "{a7}" (write_syscall),
          [fd] "{a0}" (fd),
          [buffer] "{a1}" (buffer.ptr),
          [size] "{a2}" (size)
    );
}
