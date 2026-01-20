#define EZBUILD_DEBUG
#define EZBUILD_IMPLEMENTATION
#define SL_ARRAY_CPP_COMPLIANT false
#include "../ezbuild.hpp"

using namespace Sl;

int main(int argc, char **argv)
{
    rebuild_itself(({.optimize = FlagsOptimization::NONE,
                    .warnings = FlagsWarning::ALL_FATAL,
                    .std = FlagsSTD::CPP20}),
                    argc, argv, "../ezbuild.hpp");

    bool force_rebuilt = was_cpp_rebuilt(argc, argv);

    printf("--------------------------------------\n");
    Array<File> files;
    if (!read_folder(".", files)) return 1;


    Cmd cmd = {};
    for (usize i = 0; i < files.count; ++i)
    {
        auto& file = files[i];
        bool skip_build_file = memory_equals(file.name.data, file.name.size, "Test.cpp", 8);
        if (!file.name.ends_with(".cpp") || skip_build_file)
            continue;
        cmd.create_executable({.incremental_build = true,
                               .debug = true,
                               .std = FlagsSTD::CPP20,
                              });
        cmd.source_file(file.name);

        // cmd.add_include_path("./inner/");
        // cmd.include_sources_from_folder("./inner/");

        StrBuilder output(get_global_allocator());
        output.append(file.name.data, file.name.size - 4);
        output.append_null(false);
        cmd.output_file(output.data);

        bool run = true;
        if (!cmd.build(run, force_rebuilt)) return 1;
    }
    return 0;
}
