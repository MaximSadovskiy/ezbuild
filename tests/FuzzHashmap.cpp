// Compile: clang++ -g -O1 -fsanitize=address,fuzzer,address fuzzer.cpp -o fuzzer
// Run:     ./fuzzer corpus/ -jobs=8 -workers=8 -max_len=1024

#define SL_ARRAY_CPP_COMPLIANT true
#define SL_IMPLEMENTATION
#include "../Sl/Sl.hpp"
#include <cstdint>
#include <cstring>
#include <string>
#include <cassert>

using namespace Sl;

// template<typename Map>
void parse_and_execute(const uint8_t* data, size_t size, HashMap<StrView, int>& map) {
    size_t pos = 0;

    while (pos + 16 <= size) {
        uint8_t op = data[pos++];
        // std::string key(reinterpret_cast<const char*>(data + pos), 15);
        StrView key((const char*)data + pos, 15);
        pos += 15;
        int xd = 0;

        switch (op % 8) {
            case 0: // insert new value
                xd = rand() % 1000;
                map.insert(std::move(key), xd);
                break;
            case 1: // get
                (void)map[key];
                break;
            case 2: // contains
                (void)map.contains(key);
                break;
            case 3: // remove
                map.remove(key);
                break;
            case 4: // foreach (stress iterator)
                map.forEach([](auto& k, auto& v){ (void)k; (void)v; });
                break;
            case 5: // clear
                map.clear();
                break;
            case 6: // double grow trigger
                for (int i = 0; i < 100; ++i)
                    map.insert("GROW_", i);
                break;
            case 7: // edge: empty key
                map.insert("", 42);
                break;
        }
    }
}

void parse_and_execute_str(const uint8_t* data, size_t size, HashMap<StrView, StrView>& map) {
    size_t pos = 0;

    while (pos + 16 <= size) {
        uint8_t op = data[pos++];
        // std::string key(reinterpret_cast<const char*>(data + pos), 15);
        StrView key((const char*)data + pos, 15);
        pos += 15;
        StrView xd("", 0);

        switch (op % 8) {
            case 0: // insert new value
                if (rand() % 2 == 0) xd = key.chop_left(rand() % key.size);
                else xd = key.chop_right(rand() % key.size);
                map.insert(std::move(key), std::move(xd));
                break;
            case 1: // get
                (void)map[key];
                break;
            case 2: // contains
                (void)map.contains(key);
                break;
            case 3: // remove
                map.remove(key);
                break;
            case 4: // foreach (stress iterator)
                map.forEach([](auto& k, auto& v){ (void)k; (void)v; });
                break;
            case 5: // clear
                map.clear();
                break;
            case 6: // double grow trigger
                for (int i = 0; i < 100; ++i)
                    map.insert("GROW_", "GROW_");
                break;
            case 7: // edge: empty key
                map.insert("", "");
                break;
        }
    }
}

void parse_and_execute_int(const uint8_t* data, size_t size, HashMap<int, int>& map) {
    size_t pos = 0;
    while (pos + 8 <= size) {  // Check 1-byte op + 4-byte key + 4-byte val
        uint8_t op = data[pos++];

        // Bounds check BEFORE reading ints
        if (pos + 7 >= size) break;  // Need 4+4 bytes

        int key = *(int*)(data + pos);
        pos += 4;
        int val = *(int*)(data + pos);
        pos += 4;

        switch (op % 5) {
            case 0: map.insert(std::move(key), val); break;      // Fixed: no std::move on int
            case 1: (void)map[key]; break;
            case 2: (void)map.contains(key); break;
            case 3: map.remove(key); break;
            case 4: map.forEach([](int k, int v){ (void)k; (void)v; }); break;
        }
    }
}

// libFuzzer entry point
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size) {
    if (Size < 8) return 0;

    // Parse fuzzer input: [seed:8][ops...]
    const uint8_t* input = Data;
    usize seed = *(u64*)input;
    input += 8;
    Size -= 8;

    try {
        // Test multiple template instantiations for max coverage
        {
            HashMap<StrView, int> m({.seed = seed});
            parse_and_execute(input, Size, m);
            m.cleanup();
        }
        {
            HashMap<StrView, StrView> m({.seed = seed});
            parse_and_execute_str(input, Size, m);
            m.cleanup();
        }
        {
            HashMap<int, int> m({.seed = seed});  // cpp_compliant=true
            parse_and_execute_int(input, Size, m);
            m.cleanup();
        }

    } catch (...) {
        std::abort();  // Crash on ANY exception
    }

    return 0;
}
