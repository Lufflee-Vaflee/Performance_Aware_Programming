const prof = @import("profiler.zig");
const std = @import("std");
// --- tests ---

var recursion_test_id: usize = 0;
fn recursive_helper(depth: usize) void {
    const s = prof.beginProfile(@src(), "recursive_helper");
    recursion_test_id = s.id;
    if (depth > 0) recursive_helper(depth - 1);
    s.end(null);
}

test "single sample: hit count=1, time>0, stack clean" {
    prof.startProfileSession();
    const s = prof.beginProfile(@src(), "single sample");
    var x: u64 = 0;
    for (0..10000) |i| x +%= i;
    std.mem.doNotOptimizeAway(&x);
    s.end(null);

    const b = prof.profile_registry.profile_buckets[s.id];
    try std.testing.expectEqual(@as(u64, 1), b.hit_count);
    try std.testing.expect(b.total_cpu_time > 0);
    try std.testing.expectEqual(b.total_cpu_time, b.relative_cpu_time);
}

test "repeated calls: hit count accumulates" {
    prof.startProfileSession();
    var last_id: usize = 0;
    for (0..7) |_| {
        const s = prof.beginProfile(@src(), "repeated calls test");
        last_id = s.id;
        s.end(null);
    }
    try std.testing.expectEqual(@as(u64, 7), prof.profile_registry.profile_buckets[last_id].hit_count);
}

test "nested: outer relative < outer total, inner relative == inner total" {
    prof.startProfileSession();
    const outer = prof.beginProfile(@src(), "nested test outer");
    var x: u64 = 0;
    for (0..5000) |i| x +%= i;
    std.mem.doNotOptimizeAway(&x);
    const inner = prof.beginProfile(@src(), "nested test inner");
    var y: u64 = 0;
    for (0..10000) |i| y +%= i;
    std.mem.doNotOptimizeAway(&y);
    inner.end(null);
    outer.end(null);

    const ob = prof.profile_registry.profile_buckets[outer.id];
    const ib = prof.profile_registry.profile_buckets[inner.id];

    try std.testing.expectEqual(@as(u64, 1), ob.hit_count);
    try std.testing.expectEqual(@as(u64, 1), ib.hit_count);
    try std.testing.expect(ob.total_cpu_time > ib.total_cpu_time);
    try std.testing.expect(ob.relative_cpu_time < ob.total_cpu_time);
    try std.testing.expectEqual(ib.total_cpu_time, ib.relative_cpu_time);
}

test "sequential profiles: independent, stack clean" {
    prof.startProfileSession();
    const s1 = prof.beginProfile(@src(), "sequential test indempendent first");
    var a: u64 = 0;
    for (0..5000) |i| a +%= i;
    std.mem.doNotOptimizeAway(&a);
    s1.end(null);

    const s2 = prof.beginProfile(@src(), "sequential test indempendent second");
    var b: u64 = 0;
    for (0..5000) |i| b +%= i;
    std.mem.doNotOptimizeAway(&b);
    s2.end(null);

    try std.testing.expectEqual(@as(u64, 1), prof.profile_registry.profile_buckets[s1.id].hit_count);
    try std.testing.expectEqual(@as(u64, 1), prof.profile_registry.profile_buckets[s2.id].hit_count);
    try std.testing.expect(s1.id != s2.id);
}

test "recursion: no panic, hit count equals call depth, stack clean" {
    prof.startProfileSession();
    recursive_helper(4);  // depth 4 -> 5 total calls
    const b = prof.profile_registry.profile_buckets[recursion_test_id];
    try std.testing.expectEqual(@as(u64, 5), b.hit_count);
    try std.testing.expect(b.total_cpu_time > 0);
}

test "startProfileSession clears stats and stack" {
    prof.startProfileSession();
    const s = prof.beginProfile(@src(), "startProfileSession clears test");
    s.end(null);
    const id = s.id;
    try std.testing.expect(prof.profile_registry.profile_buckets[id].hit_count > 0);

    prof.startProfileSession();
    try std.testing.expectEqual(@as(u64, 0), prof.profile_registry.profile_buckets[id].hit_count);
    try std.testing.expectEqual(@as(u64, 0), prof.profile_registry.profile_buckets[id].total_cpu_time);
}

