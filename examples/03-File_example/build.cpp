#define EZBUILD_IMPLEMENTATION
#include "../../ezbuild.hpp"

using namespace Sl;

bool file_example(const char* file_path)
{
    FileHandle file;
    if (!create_file(file_path, file, false, FlagsFile::FILE_OPEN_WRITE))
        return false;

    StrBuilder buffer;
    buffer.append("file_example:\n");
    buffer.append("  123\n");
    buffer.append("  test");
    if (!write_to_file(file, buffer.data(), buffer.count())) {
        buffer.clear();
        return false;
    }
    buffer.clear();
    if (!read_entire_file(file_path, buffer))
        return false;
    log_info("Created, writen and readed file \"%s\":\n" SB_FORMAT "\n",
            file_path,
            SB_ARG(buffer));
    return true;
}

int main(int argc, char **argv)
{
    rebuild_itself(ExecutableOptions {}, argc, argv, "../../ezbuild.hpp");
    bool force_rebuilt = was_script_rebuilt(argc, argv);

    if (!file_example("test_file.txt"))
        return EXIT_FAILURE;
}
