#define SL_IMPLEMENTATION
#include "../Sl.h"

void test_sv_from_string(void) {
    const char* str = "hello";
    StrView sv = sv_from_string(str, 5);
    ASSERT_EQUALS(sv.data, str);
    ASSERT_EQUALS(sv.size, 5);
}

void test_sv_from_cstring(void) {
    const char* str = "hello";
    StrView sv = sv_from_cstring(str);
    ASSERT_EQUALS(sv.data, str);
    ASSERT_EQUALS(sv.size, 5);
}

void test_sv_find_last_char(void) {
    const char* str = "hello world";
    StrView sv = sv_from_string(str, 11);
    ASSERT_EQUALS(sv_find_last_char(sv, 'l'), 9);
    ASSERT_EQUALS(sv_find_last_char(sv, 'z'), SV_INVALID_INDEX);
}

void test_sv_trim_left_char(void) {
    const char* str = "   hello";
    StrView sv = sv_from_string(str, 8);
    StrView trimmed = sv_trim_left_char(sv, ' ');
    ASSERT_EQUALS(trimmed.data, str + 3);
    ASSERT_EQUALS(trimmed.size, 5);
}

void test_sv_trim_right_char(void) {
    const char* str = "hello   ";
    StrView sv = sv_from_string(str, 8);
    StrView trimmed = sv_trim_right_char(sv, ' ');
    ASSERT_EQUALS(trimmed.data, str);
    ASSERT_EQUALS(trimmed.size, 5);
}

void test_sv_trim(void) {
    const char* str = "   hello   ";
    StrView sv = sv_from_string(str, 11);
    StrView trimmed = sv_trim(sv);
    ASSERT_EQUALS(trimmed.data, str + 3);
    ASSERT_EQUALS(trimmed.size, 5);
}

void test_sv_compare(void) {
    const char* str1 = "hello";
    const char* str2 = "world";
    StrView sv1 = sv_from_string(str1, 5);
    StrView sv2 = sv_from_string(str2, 5);
    ASSERT_TRUE(sv_compare(sv1, sv2) < 0);
    ASSERT_TRUE(sv_compare(sv2, sv1) > 0);
    ASSERT_TRUE(sv_compare(sv1, sv1) == 0);
}

void test_sv_equals(void) {
    const char* str = "hello";
    StrView sv1 = sv_from_string(str, 5);
    StrView sv2 = sv_from_string(str, 5);
    ASSERT_TRUE(sv_equals(sv1, sv2));
}

void test_sv_at(void) {
    const char* str = "hello";
    StrView sv = sv_from_string(str, 5);
    ASSERT_EQUALS(sv_at(sv, 0), 'h');
    ASSERT_EQUALS(sv_at(sv, 4), 'o');
}

void test_sv_chop_left(void) {
    const char* str = "hello world";
    StrView sv = sv_from_string(str, 11);
    StrView chopped = sv_chop_left(sv, 6);
    ASSERT_EQUALS(chopped.data, str + 6);
    ASSERT_EQUALS(chopped.size, 5);
}

void test_sv_empty_string(void) {
    const char* str = "";
    StrView sv = sv_from_string(str, 0);
    ASSERT_EQUALS(sv.data, str);
    ASSERT_EQUALS(sv.size, 0);
}

void test_sv_single_char_string(void) {
    const char* str = "a";
    StrView sv = sv_from_string(str, 1);
    ASSERT_EQUALS(sv.data, str);
    ASSERT_EQUALS(sv.size, 1);
    ASSERT_EQUALS(sv_at(sv, 0), 'a');
    ASSERT_EQUALS(sv_find_last_char(sv, 'a'), 0);
    ASSERT_EQUALS(sv_find_last_char(sv, 'b'), SV_INVALID_INDEX);
}

void test_sv_find_last_char_empty_string(void) {
    const char* str = "";
    StrView sv = sv_from_string(str, 0);
    ASSERT_EQUALS(sv_find_last_char(sv, 'x'), SV_INVALID_INDEX);
}

void test_sv_trim_left_no_match(void) {
    const char* str = "hello";
    StrView sv = sv_from_string(str, 5);
    StrView trimmed = sv_trim_left_char(sv, ' ');
    // No characters trimmed, so data and size unchanged
    ASSERT_EQUALS(trimmed.data, sv.data);
    ASSERT_EQUALS(trimmed.size, sv.size);
}

void test_sv_trim_right_no_match(void) {
    const char* str = "hello";
    StrView sv = sv_from_string(str, 5);
    StrView trimmed = sv_trim_right_char(sv, ' ');
    // No characters trimmed, so data and size unchanged
    ASSERT_EQUALS(trimmed.data, sv.data);
    ASSERT_EQUALS(trimmed.size, sv.size);
}

void test_sv_trim_all_chars(void) {
    const char* str = "     ";
    StrView sv = sv_from_string(str, 5);
    StrView trimmed = sv_trim(sv);
    // After trimming all ' ', should be empty string view
    ASSERT_EQUALS(trimmed.size, 0);
    ASSERT_TRUE(trimmed.data == str + 5);
}

void test_sv_at_out_of_bounds(void) {
    const char* str = "hello";
    StrView sv = sv_from_string(str, 5);
    ASSERT_EQUALS(sv_at(sv, 0), str[0]);
    ASSERT_EQUALS(sv_at(sv, 1), str[1]);
    ASSERT_EQUALS(sv_at(sv, 2), str[2]);
    ASSERT_EQUALS(sv_at(sv, 3), str[3]);
    ASSERT_EQUALS(sv_at(sv, 4), str[4]);
    ASSERT_EQUALS(sv_at(sv, 5), SV_INVALID_INDEX);
    ASSERT_EQUALS(sv_at(sv, 100), SV_INVALID_INDEX);
    ASSERT_EQUALS(sv_at(sv, 99999999999), SV_INVALID_INDEX);
}

void test_sv_from_string_null(void) {
    // Test behavior when passing a NULL pointer
    StrView sv = sv_from_string(NULL, 5);
    ASSERT_EQUALS(sv.data, NULL);
    ASSERT_EQUALS(sv.size, 0);
}

void test_sv_find_last_char_multiple(void) {
    const char* str = "abracadabra";
    StrView sv = sv_from_string(str, 11);
    // Find last occurrence of 'a', which should be at index 10
    ASSERT_EQUALS(sv_find_last_char(sv, 'a'), 10);
}

void test_sv_trim_all_none(void) {
    const char* str = "test";
    StrView sv = sv_from_string(str, 4);
    StrView trimmed = sv_trim_left_char(sv, 'x');
    // Trim char 'x' which does not exist; result should be unmodified
    ASSERT_EQUALS(trimmed.data, sv.data);
    ASSERT_EQUALS(trimmed.size, sv.size);
}

void test_sv_chop_left_zero(void) {
    const char* str = "hello";
    StrView sv = sv_from_string(str, 5);
    StrView chopped = sv_chop_left(sv, 0);
    // Chop zero characters means unchanged string view
    ASSERT_EQUALS(chopped.data, sv.data);
    ASSERT_EQUALS(chopped.size, sv.size);
}

void test_sv_compare_empty_vs_nonempty(void) {
    const char* str = "hello";
    StrView sv1 = sv_from_string(NULL, 0);
    StrView sv2 = sv_from_string(str, 5);
    ASSERT_TRUE(sv_compare(sv1, sv2) < 0);
    ASSERT_TRUE(sv_compare(sv2, sv1) > 0);
}

void test_sv_equals_different_sizes(void) {
    const char* str1 = "hello";
    const char* str2 = "hello world";
    StrView sv1 = sv_from_string(str1, 5);
    StrView sv2 = sv_from_string(str2, 11);
    ASSERT_FALSE(sv_equals(sv1, sv2));
}

void test_sv_starts_with(void) {
    const char* str = "hello world";
    StrView sv = sv_from_string(str, 11);

    StrView prefix = sv_from_cstring("hello");
    StrView non_prefix = sv_from_cstring("world");
    ASSERT_TRUE(sv_starts_with(sv, prefix));
    ASSERT_FALSE(sv_starts_with(sv, non_prefix));
}

void test_sv_ends_with(void) {
    const char* str = "hello world";
    StrView sv = sv_from_string(str, 11);

    StrView suffix = sv_from_cstring("world");
    StrView non_suffix = sv_from_cstring("hello");
    ASSERT_TRUE(sv_ends_with(sv, suffix));
    ASSERT_FALSE(sv_ends_with(sv, non_suffix));
}

int main(void)
{
    test_sv_from_string();
    test_sv_from_cstring();
    test_sv_trim_left_char();
    test_sv_trim_right_char();
    test_sv_trim();
    test_sv_compare();
    test_sv_equals();
    test_sv_at();
    test_sv_chop_left();
    test_sv_empty_string();
    test_sv_trim_left_no_match();
    test_sv_trim_right_no_match();
    test_sv_trim_all_chars();
    test_sv_at_out_of_bounds();
    test_sv_from_string_null();
    test_sv_trim_all_none();
    test_sv_chop_left_zero();
    test_sv_compare_empty_vs_nonempty();
    test_sv_equals_different_sizes();
    test_sv_starts_with();
    test_sv_ends_with();
    //test_sv_find_last_char_multiple();
    //test_sv_find_last_char();
    //test_sv_single_char_string();
    //test_sv_find_last_char_empty_string();

    printf("Every test passed\n");
}
