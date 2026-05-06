const std   = @import("std");
const Io    = std.Io;
const json  = @import("json");
const haver = @import("haverstine_ref").haverstine_ref;
const perf  = @import("perf");

fn to_f64(v: json.Value) f64 {
    return switch (v) {
        .float   => |f| f,
        .integer => |i| @floatFromInt(i),
        else     => unreachable,
    };
}

pub fn main(init: std.process.Init) !void {
    perf.startProfileSession();
    const s_startup = perf.beginProfile(@src(), "Startup");
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const al = arena.allocator();

    var it = init.minimal.args.iterate();
    _ = it.next();
    const filename = it.next() orelse {
        std.debug.print("Usage: aware <file.json>\n", .{});
        return;
    };
    s_startup.end();

    const s_read = perf.beginProfile(@src(), "Read");
    const file = try Io.Dir.openFile(Io.Dir.cwd(), init.io, filename, .{});
    defer file.close(init.io);
    const stat = try file.stat(init.io);
    var buf: [65536]u8 = undefined;
    var file_reader = file.reader(init.io, &buf);
    const src = try file_reader.interface.readAlloc(al, stat.size);
    s_read.end();

    const s_parse = perf.beginProfile(@src(), "Parse");
    var root = try json.parse(al, src);
    s_parse.end();

    const s_calc = perf.beginProfile(@src(), "Calculate");
    const pairs = (try root.value.Object.get("pairs")).array;
    var sum: f64 = 0;
    const coef = 1.0 / @as(f64, @floatFromInt(pairs.items.len));
    for (pairs.items) |item| {
        var pair = item.Object;
        sum += coef * haver(
            to_f64(try pair.get("x1")),
            to_f64(try pair.get("y1")),
            to_f64(try pair.get("x2")),
            to_f64(try pair.get("y2")),
            6372.8,
        );
    }
    s_calc.end();

    std.debug.print("\nInput size: {d}\n", .{stat.size});
    std.debug.print("Pair count: {d}\n", .{pairs.items.len});
    std.debug.print("Haverstine sum: {d}\n", .{sum});

    const cpu_freq = perf.estimateCPUFreq(10);
    perf.printSamples(cpu_freq);
}
