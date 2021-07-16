unsafe fn syscall(syscall: u64, a0: u64, a1: u64, a2: u64, a3: u64, a4: u64, a5: u64) -> u64 {
    let result;
    asm!("ecall",
        in ("a7") syscall,
        in ("a0") a0,
        in ("a1") a1,
        in ("a2") a2,
        in ("a3") a3,
        in ("a4") a4,
        in ("a5") a5,
        lateout("a0") result);
    result
}

pub type FileDescriptor = u32;

/// Writes data to the given file descriptor.
pub fn write<T>(fd: FileDescriptor, data: *const T, size: usize) {
    unsafe {
        syscall(1, fd as u64, data as u64, size as u64, 0, 0, 0);
    }
}

/// Exits the current program and passes the given memory to the parent process.
pub fn exit<T>(returned: *const T, size: usize) -> ! {
    unsafe {
        syscall(60, returned as u64, size as u64, 0, 0, 0, 0);
    }

    // This is never actually reached, but Rust doesn't know that
    loop {
        // Suppress clippy saying loop { } is wasting CPU cycles
        let _ = 0;
    }
}
