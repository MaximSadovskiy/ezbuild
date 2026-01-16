#define SL_IMPLEMENTATION
#include "Sl.h"  // your header
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define ASSERT(condition, text) if(!(condition)) {*((int*)0)=0;}

// libFuzzer entry point
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (data == nullptr || size == 0) return 0;
    using namespace Sl;

    // Create StrView from fuzzer input
    Sl::StrView sv((const char*)data, size);

    // Fuzz all public methods with the input data

    // Chopping (modifies sv - create copies)
    {
        Sl::StrView copy1 = sv;
        copy1.chop_left(size/4);
        copy1.chop_right(size/4);
    }

    // Static methods
    StrView copy2 = sv;
    sv.equals(copy2);
    sv.starts_with(copy2);

    // Instance methods
    sv.ends_with(sv);
    sv.trim();
    sv.trim_left();
    sv.trim_right();
    sv.contains_non_ascii_char();

    sv.trim_left_char(data[0]);
    sv.trim_right_char(data[0]);
    sv.find_first_occurrence_word("xasdfd");

    // Find methods
    sv.find_first_occurrence_char_until(data[0], ' ');
    sv.find_first_occurrence_char(data[0]);
    sv.find_last_occurrence_char(data[0]);
    sv.find_first_of_chars(sv);

    // Split (needs Array - mock minimal version)
    Sl::Array<Sl::StrView> arr;  // assumes minimal Array impl available
    sv.split_by_char(arr, data[0]);
    arr.cleanup();

    // Comparison
    sv.compare(copy2);
    sv == copy2;

    // Bounds checking via at()
    if (size > 0) {
        sv.at(0);
        sv.at(size);  // triggers bounds check
    }

    return 0;
}
