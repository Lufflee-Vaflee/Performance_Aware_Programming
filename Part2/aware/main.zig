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
    const start = perf.rdtsc();
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const al = arena.allocator();

    var it = init.minimal.args.iterate();
    _ = it.next();
    const filename = it.next() orelse {
        std.debug.print("Usage: aware <file.json>\n", .{});
        return;
    };

    const prepare  = perf.rdtsc() - start;

    const file = try Io.Dir.openFile(Io.Dir.cwd(), init.io, filename, .{});
    defer file.close(init.io);

    const stat = try file.stat(init.io);
    var buf: [65536]u8 = undefined;
    var file_reader = file.reader(init.io, &buf);
    const src = try file_reader.interface.readAlloc(al, stat.size);

    const read = perf.rdtsc() - start;

    var root = try json.parse(al, src);

    const parse = perf.rdtsc() - start;

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

    const calculate = perf.rdtsc() - start;
    const TotalTime = @as(f64, @floatFromInt(calculate)) / @as(f64, @floatFromInt(perf.estimateCPUFreq(10)));

    const prepare_percent = @as(f64, @floatFromInt(prepare)) / @as(f64, @floatFromInt(calculate)) * 100;
    const read_percent = @as(f64, @floatFromInt(read - prepare)) / @as(f64, @floatFromInt(calculate)) * 100;
    const parse_percent = @as(f64, @floatFromInt(parse - read)) / @as(f64, @floatFromInt(calculate)) * 100;
    const calculate_percent = @as(f64, @floatFromInt(calculate - parse)) / @as(f64, @floatFromInt(calculate)) * 100;

    std.debug.print("\nInput size: {d}\n", .{stat.size});
    std.debug.print("Pair count: {d}\n", .{pairs.items.len});
    std.debug.print("Haverstine sum: {d}\n\n", .{sum});

    std.debug.print("Total time(seconds): {d}\n", .{TotalTime});
    std.debug.print("Startup: {d}, ({d}%)\n", .{prepare, prepare_percent});
    std.debug.print("Read: {d}, ({d}%)\n", .{read - prepare, read_percent});
    std.debug.print("Json Parse: {d}, ({d}%)\n", .{parse - read, parse_percent});
    std.debug.print("Calculate: {d}, ({d}%)\n", .{calculate - parse, calculate_percent});
}


