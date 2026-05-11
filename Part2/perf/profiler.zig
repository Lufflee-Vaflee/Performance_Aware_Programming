const c = @cImport({
    @cInclude("sys/time.h");
    @cInclude("sys/resource.h");
});

const std = @import("std");
const config = @import("perf_config");

const print = std.debug.print;

pub fn getOSTimerFreq() u64 {
    return 1000000;
}

/// Returns the current value of the processor's time-stamp counter.
pub fn rdtsc() u64 {
    var lo: u32 = undefined;
    var hi: u32 = undefined;
    asm volatile ("rdtsc"
        : [lo] "={eax}" (lo),
          [hi] "={edx}" (hi),
    );
    return (@as(u64, hi) << 32) | lo;
}

pub fn getOSTime() u64 {
    var val: c.timeval = undefined;
    _ = c.gettimeofday(@ptrCast(&val), null);

    return getOSTimerFreq() * @as(u64, @intCast(val.tv_sec)) + @as(u64, @intCast(val.tv_usec));
}

pub fn estimateCPUFreq(mil: u64) u64 {
    const OSFreq = getOSTimerFreq();
    print("OS Freq: {}\n", .{OSFreq});

    const OSstart = getOSTime();
    var OSEnd: u64 = 0;
    var OSElapsed: u64 = 0;

    const CPUstart = rdtsc();
    while(OSElapsed < OSFreq * mil / 1000) {
        OSEnd = getOSTime();
        OSElapsed = OSEnd - OSstart;
    }

    const CPUend = rdtsc();
    const CPUelapsed = CPUend - CPUstart;
    var CPUFreq: u64 = 0;
    if(OSElapsed != 0) {
        CPUFreq = OSFreq * CPUelapsed / OSElapsed;
    }


    print("OS Timer: {}, {}, {}\n", .{OSstart, OSEnd, OSElapsed});
    print("CPU Timer: {}, {}, {}\n", .{CPUstart, CPUend, CPUelapsed});
    print("OS Seconds: {}\n", .{@as(f64, @floatFromInt(OSElapsed))/@as(f64,@floatFromInt(OSFreq))});
    print("CPU Freq(guessed, invariant): {}\n", .{CPUFreq});

    return CPUFreq;
}

test "estimate" {
    _ = estimateCPUFreq(10);
}

fn getPageFaults() u64 {
    const usage = std.posix.getrusage(0); // 0 = RUSAGE_SELF
    return @as(u64, @intCast(usage.minflt)) + @as(u64, @intCast(usage.majflt));
}

pub const profile_frame = struct {
    pos: std.builtin.SourceLocation = undefined,
    label: []const u8 = "",
    hit_count: u64 = 0,
    total_cpu_time: u64 = 0,
    relative_cpu_time: u64 = 0,
    recursion_depth: u64 = 0,
    process_byte_count: usize = 0,
    page_fault_count: u64 = 0,
};

fn countDigits(n: u64) usize {
    if (n == 0) return 1;
    var x = n;
    var d: usize = 0;
    while (x > 0) : (x /= 10) d += 1;
    return d;
}

fn printPad(current: usize, target: usize) void {
    var i = current;
    while (i < target) : (i += 1) print(" ", .{});
}

const overall_label = "Overall CPU time";

pub const profile_registry = struct {
    pub var start_of_time: u64 = 0;
    pub var id_counter: usize = 1; // slot 0 is the implicit root/session frame
    pub var profile_buckets: [4096]profile_frame = undefined;
    pub var current_parent_id: usize = 0;

    pub fn print_samples(cpu_freq: u64) void {
        const total_time = rdtsc() - profile_registry.start_of_time;

        var relative_check: f64 = 0.0;

        var max_name: usize = overall_label.len;
        var max_hits: usize = 1;
        var max_pf:   usize = 1;
        for (profile_registry.profile_buckets[1..profile_registry.id_counter]) |frame| {
            if (frame.hit_count == 0) continue;
            const name = if (frame.label.len > 0) frame.label else frame.pos.fn_name;
            if (name.len > max_name) max_name = name.len;
            const d = countDigits(frame.hit_count);
            if (d > max_hits) max_hits = d;
            const dp = countDigits(frame.page_fault_count);
            if (dp > max_pf) max_pf = dp;
        }

        print("\n--- Profile Results ---\n", .{});
        for (profile_registry.profile_buckets[1..profile_registry.id_counter]) |frame| {
            if (frame.hit_count == 0) continue;
            const total_s  = @as(f64, @floatFromInt(frame.total_cpu_time)) / @as(f64, @floatFromInt(cpu_freq + 1));
            const total_ms = total_s * 1000;

            const relative_s = @as(f64, @floatFromInt(frame.relative_cpu_time)) / @as(f64, @floatFromInt(cpu_freq + 1));
            const relative_ms = relative_s * 1000;

            const name = if (frame.label.len > 0) frame.label else frame.pos.fn_name;
            const total_pct = @as(f64, @floatFromInt(frame.total_cpu_time)) / @as(f64, @floatFromInt(total_time)) * 100.0;
            const self_pct  = @as(f64, @floatFromInt(frame.relative_cpu_time)) / @as(f64, @floatFromInt(total_time)) * 100.0;
            const MB        = @as(f64, @floatFromInt(frame.process_byte_count)) / (1024 * 1024);
            const MBPerSec  = MB / total_s;
            relative_check += self_pct;

            print("  {s}", .{name});
            printPad(name.len, max_name);
            print(" | hits: ", .{});
            printPad(countDigits(frame.hit_count), max_hits);
            print("{d}", .{frame.hit_count});

            print(" | inclusive: {d:>12.4}ms ({d:>6.2}%) | exclusive: {d:>12.4}ms ({d:>6.2}%) | {d:>6.2}MB at {d:>10.4}MB/S | pf: ", .{ total_ms, total_pct, relative_ms, self_pct, MB, MBPerSec });
            printPad(countDigits(frame.page_fault_count), max_pf);
            print("{d}", .{frame.page_fault_count});
            if (frame.page_fault_count > 0 and frame.process_byte_count > 0) {
                print(" | {d:>6}B/pf", .{frame.process_byte_count / frame.page_fault_count});
            } else {
                print(" |          ", .{});
            }
            print(" |\n", .{});
        }

        // Slot 0 is the root frame: relative_cpu_time accumulates via wrapping subtraction,
        // so total_time +% relative_cpu_time[0] = unaccounted time outside any profiled block.
        const unaccounted = total_time +% profile_registry.profile_buckets[0].relative_cpu_time;
        const unaccounted_pct = @as(f64, @floatFromInt(unaccounted)) / @as(f64, @floatFromInt(total_time)) * 100.0;
        relative_check += unaccounted_pct;
        print("  {s}", .{overall_label});
        printPad(overall_label.len, max_name);
        print(" | ", .{});
        printPad(0, 6 + max_hits);
        if (cpu_freq > 0) {
            const overall_ms = @as(f64, @floatFromInt(total_time)) / @as(f64, @floatFromInt(cpu_freq)) * 1000.0;
            print(" | total: {d:>12.4}ms (100.00%) | self: {d:>6.2}%\n", .{ overall_ms, unaccounted_pct });
        } else {
            print(" | total: {d:>12} (100.00%) | self: {d:>6.2}%\n", .{ total_time, unaccounted_pct });
        }
        print(" Relative pct check: {}\n", .{relative_check});
        print("-----------------------\n", .{});
    }
};

const sample_termination_handler = struct {
    id: usize,
    start: u64,
    prev_parent_id: usize,
    start_page_faults: u64,

    pub inline fn end(self: sample_termination_handler, bytes: ?usize) void {
        if (config.enabled) {
            const end_time = rdtsc();
            const elapsed = end_time - self.start;

            if (profile_registry.current_parent_id != self.id) {
                const frame = profile_registry.profile_buckets[self.id];
                std.debug.panic(
                    "profile nesting violation: trying to close '{s}' but it is not the active frame",
                    .{if (frame.label.len > 0) frame.label else frame.pos.fn_name},
                );
            }

            profile_registry.profile_buckets[self.id].recursion_depth -= 1;
            profile_registry.profile_buckets[self.id].process_byte_count += bytes orelse 0;
            const bool_mult = @intFromBool(profile_registry.profile_buckets[self.id].recursion_depth == 0);
            profile_registry.profile_buckets[self.id].total_cpu_time    += elapsed * bool_mult;
            profile_registry.profile_buckets[self.id].page_fault_count  += (getPageFaults() - self.start_page_faults) * bool_mult;
            profile_registry.profile_buckets[self.id].relative_cpu_time             +%= elapsed;
            profile_registry.profile_buckets[self.prev_parent_id].relative_cpu_time -%= elapsed;

            profile_registry.current_parent_id = self.prev_parent_id;
        }
    }
};

fn getMeasurementId(comptime src: std.builtin.SourceLocation) usize {
    const Site = struct {
        const src_: std.builtin.SourceLocation = src;
        var id: ?usize = null;
    };

    if (Site.id == null) {
        Site.id = profile_registry.id_counter;
        profile_registry.id_counter += 1;
    }

    return Site.id.?;
}

pub inline fn beginProfile(comptime src: std.builtin.SourceLocation, comptime label: []const u8) sample_termination_handler {
    if (config.enabled) {
        const id = getMeasurementId(src);
        profile_registry.profile_buckets[id].pos = src;
        profile_registry.profile_buckets[id].label = label;
        profile_registry.profile_buckets[id].hit_count += 1;
        profile_registry.profile_buckets[id].recursion_depth += 1;

        const prev_parent_id = profile_registry.current_parent_id;
        profile_registry.current_parent_id = id;

        return .{ .id = id, .start = rdtsc(), .prev_parent_id = prev_parent_id, .start_page_faults = getPageFaults() };
    }

    return .{ .id = 0, .start = 0, .prev_parent_id = 0, .start_page_faults = 0 };
}

pub fn printSamples(cpu_freq: u64) void {
    profile_registry.print_samples(cpu_freq);
}

pub fn startProfileSession() void {
    for (profile_registry.profile_buckets[0..profile_registry.id_counter]) |*bucket| {
        bucket.hit_count         = 0;
        bucket.total_cpu_time    = 0;
        bucket.relative_cpu_time = 0;
        bucket.recursion_depth   = 0;
        bucket.page_fault_count  = 0;
    }
    profile_registry.current_parent_id = 0;
    profile_registry.start_of_time = rdtsc();
}

