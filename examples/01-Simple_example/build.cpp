// This is your simpliest main build script
//
// First, you need to bootstrap builder by doing command:
//   "g++/clang++/cl -o build build.cpp"
// After that you can just run "./build" it will detect changes and rebuild itself (if needed)
//
#define EZBUILD_IMPLEMENTATION
#include "../../ezbuild.hpp"

using namespace Sl;

int main(int argc, char** argv)
{
    rebuild_itself(ExecutableOptions{}, argc, argv, "../../ezbuild.hpp");

    log_warning("Hello %s (%s)\n", get_compiler_name(), get_system_name());

    return EXIT_SUCCESS;
}
