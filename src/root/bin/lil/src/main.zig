const std = @import("stdowo");
const write = std.write;

pub fn main() anyerror!void {
    const slice = "uwu\n";
    write(1, slice, slice.len);
}

export fn _start() callconv(.C) noreturn {
    main() catch unreachable;
    const exit_syscall: u64 = 60;
    asm volatile ("ecall"
        :
        : [exit_syscall] "{a7}" (exit_syscall)
    );
    unreachable;
}
