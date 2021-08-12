#![no_std]
#![feature(lang_items, asm, prelude_import, ptr_as_uninit, maybe_uninit_extra, alloc_error_handler, panic_info_message , alloc_prelude)]

pub extern crate alloc;

pub mod allocators;
pub mod syscalls;

#[prelude_import]
pub use prelude::rust_2018::*;

pub mod prelude {
    pub mod rust_2018 {
        pub use core::prelude::v1::*;
        pub use alloc::prelude::v1::*;
    }
}

#[macro_export]
macro_rules! print {
    ($($arg: tt)*) => {
        use core::write;
        use core::fmt::Write;
        let _ = write!($crate::syscalls::FileDescriptor::from(1), $($arg)*);
    };
}

#[macro_export]
macro_rules! println {
    () => { print!("\n") };
    ($($arg: tt)*) => {
        print!("{}\n", format_args!($($arg)*))
    };
}

#[macro_export]
macro_rules! eprint {
    ($($arg: tt)*) => {
        use core::write;
        use core::fmt::Write;
        let _ = write!($crate::syscalls::FileDescriptor::from(2), $($arg)*);
    };
}

#[macro_export]
macro_rules! eprintln {
    () => { eprint!("\n") };
    ($($arg: tt)*) => {
        eprint!("{}\n", format_args!($($arg)*))
    };
}

#[panic_handler]
fn panic(info: &core::panic::PanicInfo) -> ! {
    if let Some(p) = info.location() {
        eprintln!("line {}, file {}: {}", p.line(), p.file(), info.message().unwrap());
    } else {
        eprintln!("no information available.");
    }

    syscalls::exit(1);
}

mod entry {
    #[no_mangle]
    unsafe extern "C" fn _start() {
        extern {
            fn main();
        }

        main();
        super::syscalls::exit(0);
    }

    #[lang = "start"]
    fn lang_start<T>(main: fn() -> T, _argc: isize, _argv: *const *const u8) -> isize {
        main();
        0
    }
}
