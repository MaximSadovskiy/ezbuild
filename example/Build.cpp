#define EZBUILD_DEBUG
#define EZBUILD_IMPLEMENTATION
#include "../ezbuild.hpp"

using namespace Sl;

bool sleep_example(Cmd& cmd, u32 number_of_procs)
{
    Processes procs = {};
    auto system = get_system();

    for (u32 i = 1; i <= number_of_procs; ++i) {
        cmd.clear();
        if (system == FlagsSystem::WINDOWS)
            cmd.appendf("sleep.bat %d", i);
        else
            cmd.appendf("bash sleep.sh %d", i);
        cmd.execute({.async = &procs});
    }
    return procs.wait_all();
}

bool file_example(const char* file_path)
{
    FileHandle file;
    if (!create_file(file_path, file, false, FlagsFile::FILE_OPEN_WRITE)) return false;

    StrBuilder buffer;
    buffer.append("file_example:\n");
    buffer.append("  123\n");
    buffer.append("  test");
    if (!write_to_file(file, buffer.data, buffer.count)) {
        buffer.clear();
        return false;
    }
    buffer.clear();

    if (!read_entire_file(file_path, buffer)) return false;
    printf("Created, writen and readed file \"%s\":\n" SB_FORMAT "\n", file_path, SB_ARG(buffer));
    return true;
}

bool read_folder_example(const char* folder_path)
{
    Array<FileEntry> files;
    if (!read_folder(folder_path, files))
        return false;

    printf("Readed folder \"%s\":\n", folder_path);
    for (auto& file : files)
        printf("  %s: (%s)\n", file.name.data, file.get_type_name());
    return true;
}

bool colored_logger_example()
{
    ScopedLogger _(logger_colored);

    int count = 0;
    printf("Colored logger example:\n");
    log("  "); log_trace("Trace message! %d\n", ++count);
    log("  "); log_info("Info message! %d\n", ++count);
    log("  "); log_warning("Warning message! %d\n", ++count);
    log("  "); log_error("Error message! %d\n", ++count);

    return true;
}

bool check_flags(const char* flag_str)
{
    ScopedLogger mute(logger_muted);
    printf("flag %s is%s supported\n", flag_str, is_flag_supported_cpp(flag_str) ? "" : " not");

    Array<StrView> flags = {};
    defer(flags.cleanup());
    if (!get_supported_flags(flags)) return false;
    // for (auto& flag : flags) printf(SV_FORMAT"\n", SV_ARG(flag));
    return true;
}

bool build_cpp(Cmd& cmd, bool force_rebuilt)
{
    cmd.start_cpp(ExecutableOptions {.is_cpp = true,
                           .incremental_build = true,
                           .optimize = FlagsOptimization::BASIC,
                           .warnings = FlagsWarning::ALL,
                           .std = FlagsSTD::CPPLatest
                         });

    cmd.add_include_path("inner/");
    cmd.output_file("fff");

    cmd.include_sources_from_folder("inner/");
    cmd.add_source_file("Test3.cpp");
    cmd.add_source_file("Test33.cpp");
    DISABLE cmd.add_source_file("Test333.cpp");

    ScopedLogger _(logger_colored);
    bool run = true;
    return cmd.build(run, force_rebuilt);
}

int main(int argc, char **argv)
{
    rebuild_itself((ExecutableOptions {
                        .optimize = FlagsOptimization::NONE,
                        .warnings = FlagsWarning::ALL_FATAL,
                        .std = FlagsSTD::CPP20
                    }), argc, argv, "../ezbuild.hpp");
    bool force_rebuilt = was_script_rebuilt(argc, argv);

    printf("Start:\n");
    defer(printf("End\n"));
    printf("--------------------------------------\n");

    Cmd cmd = {};

    if (!file_example("test_file.txt"))
        return 1;

    printf("--------------------------------------\n");

    if (!check_flags("-fwrapv"))
        return 1;

    printf("--------------------------------------\n");

    if (!colored_logger_example())
        return 1;

    printf("--------------------------------------\n");

    auto rand_number_from_2_to_5 = 2 + rand() % 4;
    if (!sleep_example(cmd, rand_number_from_2_to_5))
        return 1;

    printf("--------------------------------------\n");

    if (!build_cpp(cmd, force_rebuilt))
        return 1;

    printf("--------------------------------------\n");

    if (!read_folder_example("./test_folder"))
        return 1;

    printf("--------------------------------------\n");
    return 0;
}
