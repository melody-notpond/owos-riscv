use std::syscalls::{FileDescriptor, read};
use core::{fmt::Write, panic, write, writeln};

pub struct Editor {
    in_fd: FileDescriptor,
    out_fd: FileDescriptor,
}

impl Editor {
    pub fn new() -> Editor {
        Editor {
            in_fd: FileDescriptor::from(0),
            out_fd: FileDescriptor::from(1),
        }
    }

    pub fn readline(&mut self, prompt: &str) -> String {
        write!(self.out_fd, "{}", prompt).unwrap();

        let mut buffer = Vec::new();

        loop {
            let mut c = [0u8];
            read(self.in_fd, c.as_mut_ptr(), c.len());

            if c[0] == b'\r' || c[0] == b'\n' {
                writeln!(self.out_fd).unwrap();

                match String::from_utf8(buffer) {
                    Ok(v) => break v,
                    Err(e) => panic!("error creating string: {}", e),
                }
            } else if c[0] == 0x0c {
                let clear = [0x1b, b'[', b'2', b'J', 0x1b, b'[', b';', b'H'];
                std::syscalls::write(self.out_fd, clear.as_ptr(), clear.len());
                write!(self.out_fd, "{}", prompt).unwrap();
                std::syscalls::write(self.out_fd, buffer.as_ptr(), buffer.len());
            } else if c[0] == 0x1b {
                write!(self.out_fd, "^[").unwrap();
            } else if c[0] == 0x7f {
                let clear = [0x1b, b'[', b'2', b'K', 0x1b, b'[', b'G'];
                std::syscalls::write(self.out_fd, clear.as_ptr(), clear.len());
                buffer.pop();
                write!(self.out_fd, "{}", prompt).unwrap();
                std::syscalls::write(self.out_fd, buffer.as_ptr(), buffer.len());
            } else {
                buffer.push(c[0]);
                std::syscalls::write(self.out_fd, c.as_ptr(), c.len());
            }
        }
    }
}
