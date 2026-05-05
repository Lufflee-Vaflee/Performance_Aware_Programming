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
    estimateCPUFreq(10);
}

