const std = @import("std");
const perf = @import("perf");
const tokenize_mod = @import("tokenize.zig");
const syntax_mod = @import("syntax.zig");

const tokenize = tokenize_mod.tokenize;
const parse_syntax = syntax_mod.parse_syntax;

pub const Json = syntax_mod.Json;
pub const deinit_json = syntax_mod.deinit_json;
pub const Value = syntax_mod.Value;

pub fn parse(al: std.mem.Allocator, src: []const u8) !Json {
    const s_tok = perf.beginProfile(@src(), "json tokenize");
    const tokens = try tokenize(al, src);
    s_tok.end(null);
    defer al.free(tokens);

    const s_syn = perf.beginProfile(@src(), "json parse syntax");
    const result = try parse_syntax(al, tokens);
    s_syn.end(null);
    return result;
}

test {
    _ = @import("tokenize_test.zig");
    _ = @import("syntax_test.zig");
}

test "shit" {
    const gpa = std.testing.allocator;
    const result = try parse(gpa, "{\"shit\": \"some\"}");
    defer deinit_json(gpa, result);
    var Object = result.value.Object;
    _ = try Object.put(gpa, "govno", .{ .boolean = false });
    _ = try Object.get("shit");
    _ = try Object.get("govno");
}
