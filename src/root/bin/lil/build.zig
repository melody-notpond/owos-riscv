const std = @import("std");

pub fn build(b: *std.build.Builder) void {
    // Standard release options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
    const mode = b.standardReleaseOptions();

    const exe = b.addExecutable("lil", "src/main.zig");
    exe.addPackagePath("stdowo", "../../lib/zigstd/src/main.zig");
    exe.setTarget(.{
        .cpu_arch = .riscv64,
        .os_tag = .freestanding,
        .abi = .none,
    });
    exe.setBuildMode(mode);
    exe.install();
}
