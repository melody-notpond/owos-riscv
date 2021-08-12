use core::fmt::Write;

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

#[derive(Copy, Clone)]
pub struct FileDescriptor(u32);

impl From<u32> for FileDescriptor {
    fn from(i: u32) -> Self {
        Self(i)
    }
}

impl Write for FileDescriptor {
    fn write_str(&mut self, s: &str) -> core::fmt::Result {
        write(*self, s.as_ptr(), s.len());
        Ok(())
    }
}

/// Writes data to the given file descriptor.
pub fn write<T>(fd: FileDescriptor, data: *const T, size: usize) {
    unsafe {
        syscall(1, fd.0 as u64, data as u64, size as u64, 0, 0, 0);
    }
}

pub const PROT_READ: u8 = 1;
pub const PROT_WRITE: u8 = 2;
pub const PROT_EXEC: u8 = 4;

pub unsafe fn mmap<T>(length: usize, prot: u8) -> *mut T {
    syscall(9, 0, length as u64, prot as u64, 0, 0, 0) as *mut T
}

pub unsafe fn munmap<T>(addr: *mut T, length: usize) {
    syscall(11, addr as u64, length as u64, 0, 0, 0, 0);
}

/// Exits the current program and passes the given memory to the parent process.
pub fn exit(status: u64) -> ! {
    unsafe {
        syscall(60, status, 0, 0, 0, 0, 0);
    }

    // This is never actually reached, but Rust doesn't know that
    loop {
        // Suppress clippy saying loop { } is wasting CPU cycles
        let _ = 0;
    }
}
