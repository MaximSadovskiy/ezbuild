#define EZBUILD_IMPLEMENTATION
#include "../../ezbuild.hpp"

using namespace Sl;

int main(int argc, char **argv)
{
    rebuild_itself((ExecutableOptions{.debug = true, .std = FlagsSTD::CPP20}),
                    argc, argv, "../../ezbuild.hpp");

    // Check if this build.cpp script was rebuilt.
    bool force_rebuilt = was_script_rebuilt(argc, argv);
    bool debug_set = is_argument_set("--debug", argc, argv);

    // Enable colored logging
    ScopedLogger _(logger_colored);

    
    // Create cmd object in which we gonna configure our build...
    Cmd cmd {};
    cmd.start_build(ExecutableOptions{.debug = debug_set});
    // It will call end_build at the end of the scope, convinient
    defer(cmd.end_build(true, force_rebuilt));
    
    // It will create different exe file depending if you passed "--debug" argument to build script
    if (debug_set) {
        cmd.output_file("output_debug");
        cmd.output_folder(".debug");
        // It will add -DDEBUG_CONFIG
        cmd.add_define("DEBUG_CONFIG");
    } else {
        cmd.output_file("output_release");
        cmd.output_folder(".release");
    }
    cmd.add_source_file("main.cpp");
}
 