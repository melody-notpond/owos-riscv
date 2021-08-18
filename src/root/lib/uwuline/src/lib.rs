use std::syscalls::{FileDescriptor, read, write};
use core::{fmt::Write, panic, write, writeln};

use std::alloc::vec;
use std::alloc::collections::VecDeque;

pub struct Editor {
    in_fd: FileDescriptor,
    out_fd: FileDescriptor,
    buffer: VecDeque<u8>,
}

impl Editor {
    pub fn new() -> Editor {
        Editor {
            in_fd: FileDescriptor::from(0),
            out_fd: FileDescriptor::from(1),
            buffer: VecDeque::new(),
        }
    }

    fn perform_movement(&mut self, cursor_pos: &mut usize, mut dist: isize, buffer: &[u8]) {
        let change = *cursor_pos as isize + dist;
        if 0 <= change && change < buffer.len() as isize {
            *cursor_pos = change as usize;
        } else if change < 0 {
            dist = *cursor_pos as isize;
            *cursor_pos = 0;
        } else {
            dist = (buffer.len() - *cursor_pos - 1) as isize;
            *cursor_pos += dist as usize;
        }

        while dist != 0 {
            let fetch_pos = [0x1b, b'[', b'6', b'n'];
            write(self.out_fd, fetch_pos.as_ptr(), fetch_pos.len());

            let mut in_ansi_escape = false;
            let mut c = [b'\0'];
            let mut escape = vec![];
            while !in_ansi_escape || c[0] != b'R' {
                read(self.in_fd, c.as_mut_ptr(), c.len());

                // if you type in ^[[R or something then screw you >:(
                if in_ansi_escape {
                    escape.push(c[0]);
                    in_ansi_escape = !(0x40 <= c[0] && c[0] <= 0x7e);
                } else {
                    if !escape.is_empty() {
                        self.buffer.extend(escape);
                        escape = vec![];
                    }

                    if c[0] == 0x1b {
                        in_ansi_escape = true;
                        escape.push(c[0]);
                    }
                }
            }

            let escape = String::from_utf8(escape).unwrap();
            let mut split = escape[2..escape.len() - 1].split(';');
            let row: isize = match split.next() {
                Some(v) => v.parse().unwrap(),
                None => panic!("oh no"),
            };

            if dist < 0 {
                if row < -dist {
                    // TODO
                    println!("uwu");
                } else {
                    write!(self.out_fd, "\x1b[{}D", -dist).unwrap();
                    dist = 0;
                }
            } else {
                // TODO
            }
        }
    }

    pub fn readline(&mut self, prompt: &str) -> String {
        write!(self.out_fd, "{}", prompt).unwrap();

        let mut buffer = vec![];
        let mut cursor_pos = 0usize;

        loop {
            let mut c = [0u8];
            if !self.buffer.is_empty() {
                c[0] = self.buffer.pop_front().unwrap();
            } else {
                read(self.in_fd, c.as_mut_ptr(), c.len());
            }

            if c[0] == b'\r' || c[0] == b'\n' {
                writeln!(self.out_fd).unwrap();

                match String::from_utf8(buffer) {
                    Ok(v) => break v,
                    Err(e) => panic!("error creating string: {}", e),
                }
            } else if c[0] == 0x0c {
                write!(self.out_fd, "\x1b[2J\x1b[;H");
                write!(self.out_fd, "{}", prompt).unwrap();
                write(self.out_fd, buffer.as_ptr(), buffer.len());
            } else if c[0] == 0x1b {
                read(self.in_fd, c.as_mut_ptr(), c.len());
                if c[0] == b'[' {
                    let mut parameters = vec![];
                    let mut intermediate = vec![];
                    let mut final_byte = b'\0';

                    while final_byte == b'\0' {
                        read(self.in_fd, c.as_mut_ptr(), c.len());
                        if 0x30 <= c[0] && c[0] <= 0x3f && intermediate.is_empty() {
                            parameters.push(c[0]);
                        } else if 0x20 <= c[0] && c[0] <= 0x2f {
                            intermediate.push(c[0]);
                        } else if 0x40 <= c[0] && c[0] <= 0x7e {
                            final_byte = c[0];
                        } else {
                            break;
                        }
                    }

                    if final_byte == b'\0' {
                    } else {
                        match final_byte {
                            b'C' if intermediate.is_empty() => {
                                let mut dist = String::from_utf8(parameters).unwrap();
                                if let Ok(dist) = dist.parse() {
                                    self.perform_movement(&mut cursor_pos, dist, &buffer);
                                } else if dist.is_empty() {
                                    self.perform_movement(&mut cursor_pos, 1, &buffer);
                                }
                            }

                            b'D' if intermediate.is_empty() => {
                                let mut dist = String::from_utf8(parameters).unwrap();
                                if let Ok(dist) = dist.parse::<isize>() {
                                    self.perform_movement(&mut cursor_pos, -dist, &buffer);
                                } else if dist.is_empty() {
                                    self.perform_movement(&mut cursor_pos, -1, &buffer);
                                }
                            }

                            _ => ()
                        }
                    }
                }
            } else if c[0] == 0x7f {
                write!(self.out_fd, "\x1b[2K\x1b[G");
                buffer.pop();
                write!(self.out_fd, "{}", prompt).unwrap();
                write(self.out_fd, buffer.as_ptr(), buffer.len());
                cursor_pos -= 1;
            } else {
                buffer.push(c[0]);
                write(self.out_fd, c.as_ptr(), c.len());
                cursor_pos += 1;
            }
        }
    }
}
