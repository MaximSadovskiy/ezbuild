#define EZBUILD_IMPLEMENTATION
#include "../../ezbuild.hpp"

using namespace Sl;

int main(int argc, char **argv)
{
    rebuild_itself(ExecutableOptions{.std = FlagsSTD::CPP20},
                    argc, argv, "../../ezbuild.hpp");

    // Check if this build.cpp script was rebuilt.
    bool force_rebuilt = was_script_rebuilt(argc, argv);

    // Enable colored logging
    ScopedLogger _(logger_colored);

    // Create cmd object in which we gonna configure our build...
    Cmd cmd {};
    cmd.start_build(ExecutableOptions {.is_cpp = true,
                                       .incremental_build = true,
                                       .optimize = FlagsOptimization::BASIC,
                                       .warnings = FlagsWarning::ALL,
                                       .std = FlagsSTD::CPPLatest
                                      });

    // -Iinner/
    cmd.add_include_path("inner/");

    // You don't need to put extension in output file, it will be done for you.
    // -o output.[exe/out]
    cmd.output_file("output");

    // -DTEST_DEFINE
    cmd.add_define("TEST_DEFINE");

    // Set build folder, where all temporary output files would be created,
    //  (By default it is ".build", and you don't need to create it yourself, unlike other build systems)
    cmd.output_folder(".ezbuild");

    cmd.add_source_file("Main.cpp");
    cmd.add_source_file("Second.cpp");
    // Include all .c/.cpp files in "inner/" folder
    cmd.include_sources_from_folder("inner/");

    StrView custom_flag = "-fwrapv";
    if (is_flag_supported_cpp(custom_flag))
        cmd.add_cpp_flag(custom_flag);

    if (get_system() == FlagsSystem::WINDOWS) {
        // You can add custom linker flags
        cmd.add_linker_flag("/SUBSYSTEM:CONSOLE");
        cmd.add_linker_flag("/DEBUG");
        // You can link with libraries
        cmd.link_library("kernel32.lib");
            // If you want
            // cmd.link_common_win_libraries();
        // Add library search path
        cmd.add_library_path("./");
    }

    // Add argument that would be passed to executable, when runned.
    cmd.add_run_argument("test123");

    bool run = true;
    if (!cmd.end_build(run, force_rebuilt))
        return EXIT_FAILURE;
}
