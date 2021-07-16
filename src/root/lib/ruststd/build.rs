extern crate cc;

fn main() {
    cc::Build::new()
        .flag("-march=rv64gc")
        .flag("-mabi=lp64d")
        .flag("-c")
        .file("src/syscalls.s")
        .compile("syscalls.o");
}
