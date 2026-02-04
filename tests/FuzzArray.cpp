#define SL_IMPLEMENTATION
#include "../Sl/Sl.hpp"
#include <cstdint>
#include <cstring>
#include <string>
#include <cstddef>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 16) return 0;

    Sl::Array<int> arr;

    // Use first 8 bytes as seed for operations
    uint64_t seed = *(uint64_t*)data;
    size_t available_bytes = size - 8;  // After seed
    size_t max_elements = available_bytes / 4;  // How many ints fit?
    size_t max_pushes = std::min({size_t(seed % 100), max_elements, size_t(100)});

    // SAFE: Only read bytes we actually have
    for (size_t i = 0; i < max_pushes; ++i) {
        size_t offset = 8 + i * 4;
        if (offset + 3 >= size) break;  // Not enough bytes left

        int value = (data[offset] << 24) | (data[offset+1] << 16) |
                    (data[offset+2] << 8) | data[offset+3];
        arr.push(value);
    }

    // Test various operations based on input
    uint8_t op = data[0] % 10;
    switch (op) {
        case 0: case 1:
            if (!arr.is_empty()) {
                arr.remove_unordered(seed % arr.count);
            }
            break;
        case 2:
            if (!arr.is_empty()) {
                volatile int& ref = arr[seed % arr.count];
                (void)ref;
            }
            break;
        case 3:
            arr.clear();
            break;
        case 4:
            arr.memzero();
            break;
        case 5:
            if (!arr.is_empty()) {
                volatile int& first = arr.first();
                volatile int& last = arr.last();
                (void)first; (void)last;
            }
            break;
        case 6:
            arr.resize(seed % 1000);
            break;
        case 7:
            arr.reserve(seed % 1000);
            break;
        case 8:
            // Test iteration
            arr.forEach([](int x) {
                volatile int v = x * 2;
                (void)v;
            });
            break;
        case 9:
            // Test pop
            for (size_t i = 0; i < 10 && !arr.is_empty(); ++i) {
                arr.pop();
            }
            break;
    }

    // Cleanup
    arr.cleanup();
    return 0;
}

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv) {
    return 0;
}
