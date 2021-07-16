#![no_std]
#![feature(lang_items, asm, prelude_import)]

pub mod syscalls;

#[prelude_import]
pub use prelude::rust_2018::*;

pub mod prelude {
    pub mod rust_2018 {
        pub use core::prelude::v1::*;
    }
}

#[panic_handler]
fn panic(_info: &core::panic::PanicInfo) -> ! {
    /*
    if let Some(p) = info.location() {
      println!("line {}, file {}: {}", p.line(), p.file(), info.message().unwrap());
    } else {
      println!("no information available.");
    }
    */

    syscalls::exit::<u8>(core::ptr::null(), 0);
}

mod entry {
    #[no_mangle]
    unsafe extern "C" fn _start() {
        extern { 
            fn main();
        }

        main();
        super::syscalls::exit::<u8>(core::ptr::null(), 0);
    }

    #[lang = "start"]
    fn lang_start<T>(main: fn() -> T, _argc: isize, _argv: *const *const u8) -> isize {
        main();
        0
    }
}
