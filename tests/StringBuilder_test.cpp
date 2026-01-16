# include "DebugAllocationsForTests.h"
#define SL_IMPLEMENTATION
# include "../Sl.h"

using namespace Sl;

int is_string_equals(StrBuilder* builder, const char* cstr, usize size)
{
    ASSERT_NOT_NULL(builder);
    ASSERT_NOT_NULL(cstr);
    return memory_equals(builder->data, builder->count, cstr, size);
}

void test_sb_append_basic(void) {
    StrBuilder builder = {};
    builder.reserve(100);

    builder.append("hello", 5);
    ASSERT_EQUALS(builder.count, 5);
    ASSERT_TRUE(is_string_equals(&builder, "hello", 5));

    builder.cleanup();
}

void test_sb_append_multiple(void) {
    StrBuilder builder = {};
    builder.reserve(100);

    builder.append("hello", 5);
    builder.append(" world", 6);

    ASSERT_EQUALS(builder.count, 11);
    ASSERT_TRUE(is_string_equals(&builder, "hello world", 11));

    builder.cleanup();
}

void test_sb_append_cstr(void) {
    StrBuilder builder = {};
    const char* test_str = "test string";
    builder.reserve(100);

    builder.append(test_str);

    ASSERT_EQUALS(builder.count, 11);
    ASSERT_TRUE(is_string_equals(&builder, test_str, 11));

    builder.cleanup();
}

void test_sb_reset(void) {
    StrBuilder builder = {};
    builder.reserve(100);

    builder.append("initial", 7);
    ASSERT_EQUALS(builder.count, 7);

    builder.reset();
    ASSERT_EQUALS(builder.count, 0);

    builder.cleanup();
}

void test_sb_appendf(void) {
    StrBuilder builder = {};
    builder.reserve(100);

    bool success = builder.appendf("Value: %d, name: %s", 42, "test");
    ASSERT_EQUALS(builder.count, 21);
    ASSERT_TRUE(success);
    ASSERT_TRUE(is_string_equals(&builder, "Value: 42, name: test", 21));

    builder.cleanup();
}

void test_sb_to_string_view(void) {
    StrBuilder builder = {};
    builder.reserve(100);

    builder.append("hello world");

    StrView sv = builder.to_string_view();
    ASSERT_EQUALS(sv.size, 11);
    ASSERT_TRUE(memory_equals(sv.data, sv.size, "hello world", 11));

    builder.cleanup();
}

void test_sb_to_cstring(void) {
    StrBuilder builder = {};
    builder.reserve(100);

    builder.append("test");

    char* result = builder.to_cstring_alloc();

    ASSERT_NOT_NULL(result);
    ASSERT_TRUE(memory_equals(result, builder.count, "test", 4));

    builder.cleanup();
    free(result);
}

void test_sb_align(void) {
    StrBuilder builder = {};
    builder.reserve(100);

    builder.append("abc", 3);
    usize original_count = builder.count;

    builder.align(4);
    ASSERT_EQUALS(builder.count, 4);
    ASSERT_EQUALS(builder.data[3], 0);

    builder.cleanup();
}

void test_sb_capacity_growth(void) {
    StrBuilder builder = {};
    builder.reserve(5);
    ASSERT_EQUALS(builder.capacity, 5);
    ASSERT_NOT_NULL(builder.data);

    builder.append("1234567890", 10);

    ASSERT_EQUALS(builder.count, 10);
    ASSERT_TRUE(builder.capacity >= 10);

    builder.cleanup();
}

void test_sb_cleanup(void) {
    StrBuilder builder = {};
    builder.reserve(100);

    builder.append("test", 4);

    builder.cleanup();
    ASSERT_EQUALS(builder.data, NULL);
    ASSERT_EQUALS(builder.count, 0);
    ASSERT_EQUALS(builder.capacity, 0);
}

void test_sb_append_empty_string(void) {
    StrBuilder builder = {};

    builder.append("", 0);
    ASSERT_EQUALS(builder.count, 0);

    builder.cleanup();
}

void test_sb_append_null_cstr(void) {
    StrBuilder builder = {};

    // Should handle NULL gracefully (no crash)
    builder.append(NULL);
    ASSERT_EQUALS(builder.count, 1);

    builder.cleanup();
}

void test_sb_append_zero_capacity(void) {
    StrBuilder builder = {}; // No reserve

    builder.append("hello", 5);
    ASSERT_EQUALS(builder.count, 5);
    ASSERT_TRUE(builder.capacity >= 5);

    builder.cleanup();
}

void test_sb_append_max_size(void) {
    StrBuilder builder = {};

    builder.append("12345678901234567890", 20); // exceeds capacity
    ASSERT_EQUALS(builder.count, 20);
    ASSERT_TRUE(builder.capacity >= 20);

    builder.cleanup();
}

void test_sb_reset_multiple_times(void) {
    StrBuilder builder = {};
    builder.reserve(20);

    builder.append("first", 5);
    builder.reset();
    builder.append("second", 6);
    builder.reset();

    ASSERT_EQUALS(builder.count, 0);
    ASSERT_TRUE(builder.data != NULL); // buffer preserved

    builder.cleanup();
}

void test_sb_to_string_view_empty(void) {
    StrBuilder builder = {};

    StrView sv = builder.to_string_view();
    ASSERT_EQUALS(sv.size, 0);
    ASSERT_EQUALS(sv.data, builder.data);

    builder.cleanup();
}

void test_sb_to_cstring_empty(void) {
    StrBuilder builder = {};

    char* result = builder.to_cstring_alloc();

    ASSERT_NOT_NULL(result);
    ASSERT_EQUALS(builder.count, 0);
    ASSERT_EQUALS(result[0], '\0');

    builder.cleanup();
    free(result);
}

void test_sb_align_zero(void) {
    StrBuilder builder = {};
    builder.reserve(10);

    builder.append("abc", 3);
    builder.align(0);
    ASSERT_EQUALS(builder.count, 3); // no change

    builder.cleanup();
}

void test_sb_align_already_aligned(void) {
    StrBuilder builder = {};
    builder.reserve(10);

    builder.append("abcd", 4);
    usize count = builder.count;
    builder.align(4);
    ASSERT_EQUALS(builder.count, 4); // already aligned

    builder.cleanup();
}

void test_sb_cleanup_unreserved(void) {
    StrBuilder builder = {}; // no allocation

    builder.cleanup();
    ASSERT_EQUALS(builder.data, NULL);
    ASSERT_EQUALS(builder.count, 0);
    ASSERT_EQUALS(builder.capacity, 0);
}

void test_sb_append_binary_data(void) {
    StrBuilder builder = {};

    u8 binary[] = { 0x00, 0xFF, 'A', 0x7F };
    builder.append((char*)binary, 4);

    ASSERT_EQUALS(builder.count, 4);
    ASSERT_EQUALS((u8)builder.data[0], 0x00);
    ASSERT_EQUALS((u8)builder.data[1], 0xFF);
    ASSERT_EQUALS((u8)builder.data[2], 'A');
    ASSERT_EQUALS((u8)builder.data[3], 0x7F);

    builder.cleanup();
}

// void test_sb_appendf_aloc(void) {
//     StrBuilder builder = {};
//     ArenaAllocator arena = {};
//     arena_init(&arena, 100);

//     bool success = sb_appendf_aloc(&builder, &arena, "num=%d str=%s", 123, "abc");
//     ASSERT_TRUE(success);
//     ASSERT_EQUALS(builder.count, 15);

//     arena_free(&arena);
//     builder.cleanup();
// }

void test_sb_append_size_zero_nonnull(void) {
    StrBuilder builder = {};

    builder.append("hello", 0);
    ASSERT_EQUALS(builder.count, 0);

    builder.cleanup();
}

void test_sb_append_null_buffer(void) {
    StrBuilder builder = {};

    for (int i = 0; i < 5; ++i) {
        builder.append_null();
    }
    ASSERT_EQUALS(builder.count, 5);
    ASSERT_EQUALS(builder.data[0], NULL);
    ASSERT_EQUALS(builder.data[1], NULL);
    ASSERT_EQUALS(builder.data[2], NULL);
    ASSERT_EQUALS(builder.data[3], NULL);
    ASSERT_EQUALS(builder.data[4], NULL);

    builder.clear();
    ASSERT_EQUALS(builder.count, 0);
    for (int i = 0; i < 5; ++i) {
        builder.append_null(false);
    }
    ASSERT_EQUALS(builder.count, 0);

    builder.cleanup();
}

void test_sb_append_after_cleanup(void) {
    StrBuilder builder = {};
    builder.reserve(10);
    builder.cleanup();

    builder.append("test", 4);
    ASSERT_EQUALS(builder.count, 4);
    ASSERT_TRUE(builder.capacity >= 4);

    builder.cleanup();
}

void test_sb_reserve_exact_capacity(void) {
    StrBuilder builder = {};
    builder.reserve(10);

    ASSERT_EQUALS(builder.capacity, 10);
    ASSERT_EQUALS(builder.count, 0);

    builder.cleanup();
}

void test_sb_reserve_zero(void) {
    StrBuilder builder = {};

    ASSERT_EQUALS(builder.capacity, 0);
    ASSERT_EQUALS(builder.data, NULL);

    builder.cleanup();
}

void test_sb_appendf_null_format(void) {
    StrBuilder builder = {};

    bool success = builder.appendf(NULL);
    ASSERT_FALSE(success);

    builder.cleanup();
}

void test_sb_appendf_too_many_args(void) {
    StrBuilder builder = {};

    bool success = builder.appendf("%d %d", 1, 2, 3); // extra arg
    ASSERT_TRUE(success);
    ASSERT_EQUALS(builder.count, 3); // "1 2"

    builder.cleanup();
}

void test_sb_align_overshoot(void) {
    StrBuilder builder = {};

    builder.append("ab", 2);
    builder.align(8);
    ASSERT_EQUALS(builder.count, 8);
    ASSERT_EQUALS(builder.data[2], 0);
    ASSERT_EQUALS(builder.data[3], 0);

    builder.cleanup();
}

void test_sb_to_string_view_after_reset(void) {
    StrBuilder builder = {};

    builder.append("hello", 5);
    builder.reset();

    StrView sv = builder.to_string_view();
    ASSERT_EQUALS(sv.size, 0);
    ASSERT_EQUALS(sv.data, builder.data);

    builder.cleanup();
}

void test_sb_nested_operations(void) {
    StrBuilder builder = {};

    builder.append("start", 5);
    builder.appendf(" %d", 42);
    builder.append(" end");
    builder.align(16);

    ASSERT_EQUALS(builder.count, 16);
    ASSERT_TRUE(memory_equals(builder.data, 12, "start 42 end", 12));

    builder.cleanup();
}

void test_sb_repeated_growth(void) {
    StrBuilder builder = {};

    builder.append("1234");
    builder.append("5678");
    builder.append("90");

    ASSERT_EQUALS(builder.count, 10);
    ASSERT_TRUE(builder.capacity >= 10);

    builder.cleanup();
}

void test_sb_double_cleanup(void) {
    StrBuilder builder = {};
    builder.reserve(10);

    builder.cleanup();
    builder.cleanup(); // should not be a problem
    ASSERT_EQUALS(builder.data, NULL);
    ASSERT_EQUALS(builder.count, 0);
    ASSERT_EQUALS(builder.capacity, 0);
}

int main()
{
    test_sb_append_basic();
    test_sb_append_multiple();
    test_sb_append_cstr();
    test_sb_reset();
    test_sb_appendf();
    test_sb_to_string_view();
    test_sb_to_cstring();
    test_sb_align();
    test_sb_capacity_growth();
    test_sb_cleanup();
    test_sb_append_empty_string();
    test_sb_append_null_cstr();
    test_sb_append_zero_capacity();
    test_sb_append_max_size();
    test_sb_reset_multiple_times();
    test_sb_to_string_view_empty();
    test_sb_to_cstring_empty();
    test_sb_align_zero();
    test_sb_align_already_aligned();
    test_sb_cleanup_unreserved();
    test_sb_append_binary_data();
    // test_sb_appendf_aloc();
    test_sb_append_size_zero_nonnull();
    test_sb_append_null_buffer();
    test_sb_append_after_cleanup();
    test_sb_reserve_exact_capacity();
    test_sb_reserve_zero();
    test_sb_appendf_null_format();
    test_sb_appendf_too_many_args();
    test_sb_align_overshoot();
    test_sb_to_string_view_after_reset();
    test_sb_nested_operations();
    test_sb_repeated_growth();
    test_sb_double_cleanup();

    ASSERT_FALSE(is_memory_leak_detected());

    printf("Every test passed\n");
}
