const std = @import("std");
pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const json_mod = b.addModule("json", .{
        .root_source_file = b.path("json/root.zig"),
        .target = target,
        .optimize = optimize,
    });

    const json_test = b.addTest(.{
        .name = "json-test",
        .root_module = json_mod,
        .use_llvm = true,
    });

    const run_test = b.addRunArtifact(json_test);

    const test_step = b.step("test", "Run all tests");
    test_step.dependOn(&run_test.step);

    const install_test = b.addInstallArtifact(json_test, .{});
    const test_install_step = b.step("test-install", "Install test binary for gdb");
    test_install_step.dependOn(&install_test.step);

    const haverstine_ref_mod = b.addModule("haverstine_ref", .{
        .root_source_file = b.path("haverstine/haverstine_ref.zig"),
        .target = target,
        .optimize = optimize,
    });

    const HaverstineGeneratorExe = b.addExecutable(.{
        .name = "haverstine_gen",
        .use_llvm = true,
        .root_module = b.createModule(.{
            .root_source_file = b.path("gen/main.zig"),
            .target = target,
            .optimize = optimize,
        }),
    });

    HaverstineGeneratorExe.root_module.addImport("haverstine_ref", haverstine_ref_mod);

    b.installArtifact(HaverstineGeneratorExe);


    const run_step = b.step("run", "Run the app");
    const run_cmd = b.addRunArtifact(HaverstineGeneratorExe);
    run_step.dependOn(&run_cmd.step);

    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

      // 1. Add the command line option (Usage: zig build -Dprofile=true)
    const enable_prof = b.option(bool, "profile", "Enable detailed profiling") orelse false;

    // 2. Create the options module
    const perf_options = b.addOptions();
    perf_options.addOption(bool, "enabled", enable_prof);

    const perf_mod = b.addModule("perf", .{
        .root_source_file = b.path("perf/root.zig"),
        .target = target,
        .optimize = optimize
    });

    perf_mod.addOptions("perf_config", perf_options);

    const perf_test = b.addTest(.{
        .name = "perf-test",
        .root_module = perf_mod,
        .use_llvm = true,
    });

    const run_perf_test = b.addRunArtifact(perf_test);
    const perf_step = b.step("test-perf", "Run perf test");
    perf_step.dependOn(&run_perf_test.step);
    test_step.dependOn(&run_perf_test.step);


    perf_mod.link_libc = true;

    json_mod.addImport("perf", perf_mod);
    json_mod.link_libc = true;

    const AwareExe = b.addExecutable(.{
        .name = "aware",
        .use_llvm = true,
        .root_module = b.createModule(.{
            .root_source_file = b.path("aware/main.zig"),
            .target = target,
            .optimize = optimize,
        }),
    });

    AwareExe.root_module.addImport("json",           json_mod);
    AwareExe.root_module.addImport("haverstine_ref", haverstine_ref_mod);
    AwareExe.root_module.addImport("perf",       perf_mod);

    b.installArtifact(AwareExe);
}
