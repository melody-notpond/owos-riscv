const owo = @import("stdowo");
const write = owo.write;

const std = @import("std");

comptime {
    @export(owo._start, .{ .name = "_start" });
}

export fn main() void {
    const slice = "uwu\n";
    write(1, slice, slice.len);
}
