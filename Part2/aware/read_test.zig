const std      = @import("std");
const Io       = std.Io;
const profiler = @import("perf").profiler;
const repeat   = @import("perf").repeat;

// All variants share this inner read: open, read into dest, close.
fn doRead(io: std.Io, filename: []const u8, dest: []u8) void {
    const file = Io.Dir.openFile(Io.Dir.cwd(), io, filename, .{}) catch return;
    defer file.close(io);
    var iobuf: [65536]u8 = undefined;
    var rdr = file.reader(io, &iobuf);
    var fba = std.heap.FixedBufferAllocator.init(dest);
    _ = rdr.interface.readAlloc(fba.allocator(), dest.len) catch return;
}

// --- variant contexts -------------------------------------------------

const ArenaCtx = struct {
    io:       std.Io,
    filename: []const u8,
    size:     usize,
    arena:    *std.heap.ArenaAllocator,
};

const HeapCtx = struct {
    io:       std.Io,
    filename: []const u8,
    size:     usize,
};

const PreAllocCtx = struct {
    io:       std.Io,
    filename: []const u8,
    dest:     []u8,
};

// --- variant functions ------------------------------------------------

// 1. Arena: reset (retain pages) + alloc each iteration.
fn readArena(ctx: ArenaCtx) void {
    _ = ctx.arena.reset(.retain_capacity);
    const dest = ctx.arena.allocator().alloc(u8, ctx.size) catch return;
    doRead(ctx.io, ctx.filename, dest);
}

// 3. Heap: malloc + free each iteration.
fn readHeap(ctx: HeapCtx) void {
    const dest = std.heap.c_allocator.alloc(u8, ctx.size) catch return;
    defer std.heap.c_allocator.free(dest);
    doRead(ctx.io, ctx.filename, dest);
}

// 2 & 4. Pre-allocated: same buffer reused every iteration, no alloc cost.
fn readPreAlloc(ctx: PreAllocCtx) void {
    doRead(ctx.io, ctx.filename, ctx.dest);
}

// --- main -------------------------------------------------------------

pub fn main(init: std.process.Init) !void {
    var outer_arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer outer_arena.deinit();
    const al = outer_arena.allocator();

    var it = init.minimal.args.iterate();
    _ = it.next();
    const filename = it.next() orelse {
        std.debug.print("Usage: read_test <file>\n", .{});
        return;
    };

    const probe = try Io.Dir.openFile(Io.Dir.cwd(), init.io, filename, .{});
    const stat  = try probe.stat(init.io);
    probe.close(init.io);
    const size = stat.size;

    const cpu_freq = profiler.estimateCPUFreq(100);

    // 1. Arena alloc — reset + alloc each iteration
    {
        var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
        defer arena.deinit();
        _ = repeat.run(30, cpu_freq, size, "arena alloc (reset each iter)",
            readArena, .{ArenaCtx{ .io = init.io, .filename = filename, .size = size, .arena = &arena }});
    }

    // 2. Arena pre-alloc — allocate once with arena, reuse same buffer
    {
        const dest = try al.alloc(u8, size);
        _ = repeat.run(30, cpu_freq, size, "arena pre-alloc (no realloc)",
            readPreAlloc, .{PreAllocCtx{ .io = init.io, .filename = filename, .dest = dest }});
    }

    // 3. Heap alloc — malloc + free each iteration
    _ = repeat.run(30, cpu_freq, size, "heap alloc+free (each iter)",
        readHeap, .{HeapCtx{ .io = init.io, .filename = filename, .size = size }});

    // 4. Heap pre-alloc — allocate once with malloc, reuse same buffer
    {
        const dest = try std.heap.c_allocator.alloc(u8, size);
        defer std.heap.c_allocator.free(dest);
        _ = repeat.run(30, cpu_freq, size, "heap pre-alloc (no realloc)",
            readPreAlloc, .{PreAllocCtx{ .io = init.io, .filename = filename, .dest = dest }});
    }
}
