#define EZBUILD_IMPLEMENTATION
#include "../../ezbuild.hpp"

using namespace Sl;

int main(int argc, char **argv)
{
    rebuild_itself(ExecutableOptions{}, argc, argv, "../../ezbuild.hpp");
    bool force_rebuilt = was_script_rebuilt(argc, argv);

    const char* folder_path = "test_folder";

    Array<FileEntry> files;
    if (!read_folder(folder_path, files))
        return EXIT_FAILURE;

    log("Readed folder \"%s\":\n", folder_path);
    for (auto& file : files)
        log_info("  %s: (%s)\n", file.name.data, file.get_type_name());
}
