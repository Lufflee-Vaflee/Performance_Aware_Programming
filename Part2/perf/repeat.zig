const std = @import("std");
const profiler = @import("profiler.zig");

const print = std.debug.print;

pub const RepeatStats = struct {
    first_cycles: u64,
    last_cycles:  u64,
    min_cycles:   u64,
    max_cycles:   u64,
    total_cycles: u64,
    count:        u64,
    byte_count:   usize,

    pub fn avg(self: RepeatStats) u64 {
        if (self.count == 0) return 0;
        return self.total_cycles / self.count;
    }
};

/// Run `func` with `args` for `duration_seconds`, collecting per-call cycle stats.
/// `byte_count` is the number of bytes processed per call (for throughput); pass 0 to skip.
/// `cpu_freq` is used for printing ms/throughput; pass 0 to show raw cycles only.
pub fn run(
    duration_seconds: u64,
    cpu_freq: u64,
    byte_count: usize,
    comptime label: []const u8,
    func: anytype,
    args: anytype,
) RepeatStats {
    profiler.startProfileSession();

    const os_freq = profiler.getOSTimerFreq();
    const deadline = profiler.getOSTime() + os_freq * duration_seconds;

    var stats = RepeatStats{
        .first_cycles = 0,
        .last_cycles  = 0,
        .min_cycles   = std.math.maxInt(u64),
        .max_cycles   = 0,
        .total_cycles = 0,
        .count        = 0,
        .byte_count   = byte_count,
    };

    print("\n[{s}]\n", .{label});
    // Print placeholder block so the cursor is in the right position before the loop.
    print("  fst:             -\n", .{});
    print("  min:             -\n", .{});
    print("  max:             -\n", .{});
    print("  avg:             -\n", .{});
    print("  lst:             -\n", .{});

    while (profiler.getOSTime() < deadline) {
        const s = profiler.beginProfile(@src(), label);
        _ = @call(.auto, func, args);
        s.end(byte_count);

        const elapsed = profiler.rdtsc() - s.start;
        if (stats.count == 0)        stats.first_cycles = elapsed;
        if (elapsed < stats.min_cycles) stats.min_cycles = elapsed;
        if (elapsed > stats.max_cycles) stats.max_cycles = elapsed;
        stats.last_cycles  = elapsed;
        stats.total_cycles += elapsed;
        stats.count        += 1;

        // \x1b[5F: move cursor to beginning of line 5 lines up, then rewrite the block.
        print("\x1b[5F", .{});
        printBlockLine("fst", stats.first_cycles, stats, cpu_freq);
        printBlockLine("min", stats.min_cycles,   stats, cpu_freq);
        printBlockLine("max", stats.max_cycles,   stats, cpu_freq);
        printBlockLine("avg", stats.avg(),         stats, cpu_freq);
        printBlockLine("lst", stats.last_cycles,  stats, cpu_freq);
    }

    print("\n", .{});
    profiler.printSamples(cpu_freq);

    return stats;
}

fn cyclesTo(cycles: u64, cpu_freq: u64) f64 {
    return @as(f64, @floatFromInt(cycles)) / @as(f64, @floatFromInt(cpu_freq)) * 1000.0;
}

fn throughputGBs(cycles: u64, byte_count: usize, cpu_freq: u64) f64 {
    if (cpu_freq == 0 or cycles == 0 or byte_count == 0) return 0;
    const seconds = @as(f64, @floatFromInt(cycles)) / @as(f64, @floatFromInt(cpu_freq));
    return @as(f64, @floatFromInt(byte_count)) / seconds / (1024.0 * 1024.0 * 1024.0);
}

fn printBlockLine(comptime tag: []const u8, cycles: u64, stats: RepeatStats, cpu_freq: u64) void {
    if (cpu_freq > 0) {
        print("  " ++ tag ++ ":{d:>14}cy {d:>10.4}ms", .{ cycles, cyclesTo(cycles, cpu_freq) });
        if (stats.byte_count > 0)
            print(" {d:>8.4}GB/s", .{throughputGBs(cycles, stats.byte_count, cpu_freq)});
        print("\n", .{});
    } else {
        print("  " ++ tag ++ ":{d:>14}cy\n", .{cycles});
    }
}
