fn main() {
    let s = "uwu\n";
    std::syscalls::write(1, s.as_ptr(), s.len());
}
