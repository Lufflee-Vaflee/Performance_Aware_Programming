const c = @cImport({
    @cInclude("sys/time.h");
});

const std = @import("std");

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

pub const profile_frame = struct {
    pos: std.builtin.SourceLocation = undefined,
    label: []const u8 = "",
    hit_count: u64 = 0,
    total_cpu_time: u64 = 0,
    relative_cpu_time: u64 = 0,
    recursion_depth: u64 = 0,
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

const profile_registry = struct {
    var start_of_time: u64 = 0;
    var id_counter: usize = 0;
    var profile_buckets: [4096]profile_frame = undefined;

    var relative_stack: [65536]usize = undefined;
    var current_stack_num: usize = 0;

    pub fn print_samples(cpu_freq: u64) void {
        const total_time = rdtsc() - profile_registry.start_of_time;

        var max_name: usize = overall_label.len;
        var max_hits: usize = 1;
        for (profile_registry.profile_buckets[0..profile_registry.id_counter]) |frame| {
            if (frame.hit_count == 0) continue;
            const name = if (frame.label.len > 0) frame.label else frame.pos.fn_name;
            if (name.len > max_name) max_name = name.len;
            const d = countDigits(frame.hit_count);
            if (d > max_hits) max_hits = d;
        }

        print("\n--- Profile Results ---\n", .{});
        for (profile_registry.profile_buckets[0..profile_registry.id_counter]) |frame| {
            if (frame.hit_count == 0) continue;
            const name = if (frame.label.len > 0) frame.label else frame.pos.fn_name;
            const total_pct = @as(f64, @floatFromInt(frame.total_cpu_time)) / @as(f64, @floatFromInt(total_time)) * 100.0;
            const self_pct  = @as(f64, @floatFromInt(frame.relative_cpu_time)) / @as(f64, @floatFromInt(total_time)) * 100.0;

            print("  {s}", .{name});
            printPad(name.len, max_name);
            print(" | hits: ", .{});
            printPad(countDigits(frame.hit_count), max_hits);
            print("{d}", .{frame.hit_count});

            if (cpu_freq > 0) {
                const total_ms = @as(f64, @floatFromInt(frame.total_cpu_time)) / @as(f64, @floatFromInt(cpu_freq)) * 1000.0;
                print(" | total: {d:>12.4}ms ({d:>6.2}%) | self: {d:>6.2}%\n", .{ total_ms, total_pct, self_pct });
            } else {
                print(" | total: {d:>12} ({d:>6.2}%) | self: {d:>6.2}%\n", .{ frame.total_cpu_time, total_pct, self_pct });
            }
        }

        print("  {s}", .{overall_label});
        printPad(overall_label.len, max_name);
        print(" | ", .{});
        printPad(0, 6 + max_hits); // fill "hits: NNN" space to align total column
        if (cpu_freq > 0) {
            const overall_ms = @as(f64, @floatFromInt(total_time)) / @as(f64, @floatFromInt(cpu_freq)) * 1000.0;
            print(" | total: {d:>12.4}ms (100.00%)\n", .{overall_ms});
        } else {
            print(" | total: {d:>12} (100.00%)\n", .{total_time});
        }
        print("-----------------------\n", .{});
    }
};

const sample_termination_handler = struct {
    id: usize,
    start: u64,
    prev_stack_depth: usize,

    pub fn end(self: sample_termination_handler) void {
        const end_time = rdtsc();
        const elapsed = end_time - self.start;

        if (profile_registry.current_stack_num == 0 or
            profile_registry.relative_stack[profile_registry.current_stack_num - 1] != self.id)
        {
            const frame = profile_registry.profile_buckets[self.id];
            std.debug.panic(
                "profile nesting violation: trying to close '{s}:{d}' but it is not at the top of the stack",
                .{ frame.pos.fn_name, frame.pos.line },
            );
        }

        profile_registry.profile_buckets[self.id].recursion_depth -= 1;
        if (profile_registry.profile_buckets[self.id].recursion_depth == 0) {
            profile_registry.profile_buckets[self.id].total_cpu_time += elapsed;
        }
        profile_registry.profile_buckets[self.id].relative_cpu_time +%= elapsed;

        profile_registry.current_stack_num -= 1;

        if (profile_registry.current_stack_num != self.prev_stack_depth) {
            const frame = profile_registry.profile_buckets[self.id];
            std.debug.panic(
                "profile stack depth mismatch on close of '{s}:{d}': expected depth {d}, got {d}",
                .{ frame.pos.fn_name, frame.pos.line, self.prev_stack_depth, profile_registry.current_stack_num },
            );
        }

        if (profile_registry.current_stack_num > 0) {
            const parent_id = profile_registry.relative_stack[profile_registry.current_stack_num - 1];
            profile_registry.profile_buckets[parent_id].relative_cpu_time -%= elapsed;
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

    if (Site.id.? >= 4096) {
        @panic("max amount of samples is 4096");
    }

    return Site.id.?;
}

pub fn beginProfile(comptime src: std.builtin.SourceLocation, comptime label: []const u8) sample_termination_handler {
    const id = getMeasurementId(src);
    profile_registry.profile_buckets[id].pos = src;
    profile_registry.profile_buckets[id].label = label;
    profile_registry.profile_buckets[id].hit_count += 1;
    profile_registry.profile_buckets[id].recursion_depth += 1;

    const prev_depth = profile_registry.current_stack_num;
    profile_registry.relative_stack[profile_registry.current_stack_num] = id;
    profile_registry.current_stack_num += 1;

    return .{ .id = id, .start = rdtsc(), .prev_stack_depth = prev_depth };
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
    }
    profile_registry.current_stack_num = 0;
    profile_registry.start_of_time = rdtsc();
}

// --- tests ---

var recursion_test_id: usize = 0;
fn recursive_helper(depth: usize) void {
    const s = beginProfile(@src(), "recursive_helper");
    recursion_test_id = s.id;
    if (depth > 0) recursive_helper(depth - 1);
    s.end();
}

test "single sample: hit count=1, time>0, stack clean" {
    startProfileSession();
    const s = beginProfile(@src(), "single sample");
    var x: u64 = 0;
    for (0..10000) |i| x +%= i;
    std.mem.doNotOptimizeAway(&x);
    s.end();

    const b = profile_registry.profile_buckets[s.id];
    try std.testing.expectEqual(@as(u64, 1), b.hit_count);
    try std.testing.expect(b.total_cpu_time > 0);
    try std.testing.expectEqual(b.total_cpu_time, b.relative_cpu_time);
    try std.testing.expectEqual(@as(usize, 0), profile_registry.current_stack_num);
}

test "repeated calls: hit count accumulates" {
    startProfileSession();
    var last_id: usize = 0;
    for (0..7) |_| {
        const s = beginProfile(@src(), "repeated calls test");
        last_id = s.id;
        s.end();
    }
    try std.testing.expectEqual(@as(u64, 7), profile_registry.profile_buckets[last_id].hit_count);
    try std.testing.expectEqual(@as(usize, 0), profile_registry.current_stack_num);
}

test "nested: outer relative < outer total, inner relative == inner total" {
    startProfileSession();
    const outer = beginProfile(@src(), "nested test outer");
    var x: u64 = 0;
    for (0..5000) |i| x +%= i;
    std.mem.doNotOptimizeAway(&x);
    const inner = beginProfile(@src(), "nested test inner");
    var y: u64 = 0;
    for (0..10000) |i| y +%= i;
    std.mem.doNotOptimizeAway(&y);
    inner.end();
    outer.end();

    const ob = profile_registry.profile_buckets[outer.id];
    const ib = profile_registry.profile_buckets[inner.id];

    try std.testing.expectEqual(@as(u64, 1), ob.hit_count);
    try std.testing.expectEqual(@as(u64, 1), ib.hit_count);
    try std.testing.expect(ob.total_cpu_time > ib.total_cpu_time);
    try std.testing.expect(ob.relative_cpu_time < ob.total_cpu_time);
    try std.testing.expectEqual(ib.total_cpu_time, ib.relative_cpu_time);
    try std.testing.expectEqual(@as(usize, 0), profile_registry.current_stack_num);
}

test "sequential profiles: independent, stack clean" {
    startProfileSession();
    const s1 = beginProfile(@src(), "sequential test indempendent first");
    var a: u64 = 0;
    for (0..5000) |i| a +%= i;
    std.mem.doNotOptimizeAway(&a);
    s1.end();

    const s2 = beginProfile(@src(), "sequential test indempendent second");
    var b: u64 = 0;
    for (0..5000) |i| b +%= i;
    std.mem.doNotOptimizeAway(&b);
    s2.end();

    try std.testing.expectEqual(@as(u64, 1), profile_registry.profile_buckets[s1.id].hit_count);
    try std.testing.expectEqual(@as(u64, 1), profile_registry.profile_buckets[s2.id].hit_count);
    try std.testing.expect(s1.id != s2.id);
    try std.testing.expectEqual(@as(usize, 0), profile_registry.current_stack_num);
}

test "recursion: no panic, hit count equals call depth, stack clean" {
    startProfileSession();
    recursive_helper(4);  // depth 4 -> 5 total calls
    const b = profile_registry.profile_buckets[recursion_test_id];
    try std.testing.expectEqual(@as(u64, 5), b.hit_count);
    try std.testing.expect(b.total_cpu_time > 0);
    try std.testing.expectEqual(@as(usize, 0), profile_registry.current_stack_num);
}

test "startProfileSession clears stats and stack" {
    startProfileSession();
    const s = beginProfile(@src(), "startProfileSession clears test");
    s.end();
    const id = s.id;
    try std.testing.expect(profile_registry.profile_buckets[id].hit_count > 0);

    startProfileSession();
    try std.testing.expectEqual(@as(u64, 0), profile_registry.profile_buckets[id].hit_count);
    try std.testing.expectEqual(@as(u64, 0), profile_registry.profile_buckets[id].total_cpu_time);
    try std.testing.expectEqual(@as(usize, 0), profile_registry.current_stack_num);
}

