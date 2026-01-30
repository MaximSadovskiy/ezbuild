#include <time.h>
#define EZBUILD_IMPLEMENTATION
#include "../../ezbuild.hpp"

using namespace Sl;

int main(int argc, char **argv)
{
    rebuild_itself(ExecutableOptions {}, argc, argv, "../../ezbuild.hpp");

    // Set random seed
    srand(time(NULL));

    // Select from 2 to 5 proccesses
    auto number_of_procs = (rand() % 4) + 2;
    log("Running %d procs:\n", number_of_procs);
    defer(log("End\n"));

    const auto system = get_system();

    // Change this variable to run syncronously
    bool run_asynchronously = true;

    Processes procs {};
    Cmd cmd {};
    for (int i = 1; i <= number_of_procs; ++i) {
        cmd.clear();
        if (system == FlagsSystem::WINDOWS)
            cmd.push("sleep.bat", i);
        else
            cmd.push("bash", "sleep.sh", i);

        CmdOptions options{};
        if (run_asynchronously) options.async = &procs;
        cmd.execute(options);
    }

    // Wait on all proccesses
    if (!procs.wait_all())
        return EXIT_FAILURE;
}
