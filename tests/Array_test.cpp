# include "DebugAllocationsForTests.h"
#define SL_IMPLEMENTATION
# include "../Sl.h"

using namespace Sl;

struct Test_array : Array<int>
{
    u64 secret_data1;
    s64 secret_data2;
};

struct Test_array_float : Array<float>
{
    u64 secret_data1;
    s64 secret_data2;
};

struct Employee
{
    int id;
    char name[16];
    float salary;
};

void set_secret_data(Test_array* arr, u64 expected_data1, s64 expected_data2)
{
    arr->secret_data1 = expected_data1;
    arr->secret_data2 = expected_data2;
}

void check_secret_data(Test_array* arr, u64 expected_data1, s64 expected_data2)
{
    ASSERT_EQUALS(arr->secret_data1, expected_data1);
    ASSERT_EQUALS(arr->secret_data2, expected_data2);
}

// void test_arr_custom_struct(void) {
//     Test_array_custom_struct arr = { 0 };
//     set_secret_data(&arr, 500, -500);

//     Employee e1 = { 1, "Alice", 50000.0f };
//     Employee e2 = { 2, "Bob", 60000.0f };

//     arr.push(e1);
//     arr.push(e2);
//     arr.push(((Employee) { .id = 3, .name = "Charlie", .salary = 70000.0f }));

//     ASSERT_EQUALS(arr.count, 3);
//     ASSERT_EQUALS(arr.first().id, 1);
//     ASSERT_EQUALS(strcmp(arr.get(1).name, "Bob"), 0);
//     ASSERT_TRUE(arr.last().salary == 70000.0f);

//     // Test update operation
//     arr.get(1).salary = 65000.0f;
//     ASSERT_TRUE(arr.get(1).salary == 65000.0f);

//     // Remove second employee unordered
//     arr.remove_unordered(1);
//     ASSERT_EQUALS(arr.count, 2);

//     // After removal, the last employee should have replaced the removed
//     ASSERT_TRUE(strcmp(arr.get(1).name, "Charlie") == 0);

//     check_secret_data(&arr, 500, -500);
//     arr.cleanup();
// }

void test_arr_initial_state(void) {
    Test_array arr = {};
    set_secret_data(&arr, 123, -123);
    ASSERT_EQUALS(arr.count, 0);
    ASSERT_EQUALS(arr.capacity, 0);
    ASSERT_TRUE(arr.data == NULL);
    check_secret_data(&arr, 123, -123);
}

void test_array_push_and_resize(void) {
    const auto initial_cap = 123;
    Test_array arr = {};
    arr.resize(initial_cap);
    set_secret_data(&arr, 333, -333);

    for (int i = 0; i < initial_cap; i++) {
        arr.push(i);
        ASSERT_EQUALS(arr.last(), i);
    }
    arr.push(100);
    ASSERT_TRUE(arr.count == initial_cap + 1);
    ASSERT_TRUE(arr.capacity >= arr.count);
    ASSERT_EQUALS(arr.last(), 100);
    arr.cleanup();
    check_secret_data(&arr, 333, -333);
}

void test_array_get(void) {
    Test_array arr = {};
    set_secret_data(&arr, 665, -665);

    arr.push(10);
    arr.push(20);
    ASSERT_EQUALS(arr.get(0), 10);
    ASSERT_EQUALS(arr.get(1), 20);
    arr.cleanup();
    check_secret_data(&arr, 665, -665);
}

// void test_arr_pop(void) {
//     Test_array arr = {0};
//     set_secret_data(&arr, 93125214, -93125214);

//     arr.push(5);
//     arr.push(10);
//     int popped = arr.pop();
//     ASSERT_EQUALS(popped, 10);
//     ASSERT_EQUALS(arr.count, 1);
//     array_pop(&arr);
//     ASSERT_EQUALS(arr.count, 0);
//     arr.cleanup();
//     check_secret_data(&arr, 93125214, -93125214);
// }

void test_arr_remove_unordered(void) {
    Test_array arr = {};
    set_secret_data(&arr, 565934528, -565934528);

    for (int i = 1; i <= 5; i++) {
        arr.push(i);
    }
    size_t orig_count = arr.count;
    arr.remove_unordered(2);  // remove index 2 (value 3)
    ASSERT_EQUALS(arr.count, orig_count - 1);
    // value 3 should be gone, last element moved into index 2
    ASSERT_TRUE(memchr(arr.data, 3, arr.count * sizeof(int)) == NULL);
    ASSERT_EQUALS(arr.get(2), 5);
    arr.cleanup();
    check_secret_data(&arr, 565934528, -565934528);
}

void test_arr_clear(void) {
    Test_array arr = {};
    set_secret_data(&arr, 123456789, -123456789);

    arr.push(1);
    arr.push(2);
    arr.clear();
    ASSERT_EQUALS(arr.count, 0);
    ASSERT_TRUE(arr.capacity >= 2);
    ASSERT_TRUE(arr.data != NULL);
    arr.cleanup();
    check_secret_data(&arr, 123456789, -123456789);
}

void test_arr_resize_and_reserve(void) {
    Test_array arr = {};
    set_secret_data(&arr, 345690123, -345690123);

    arr.resize(10);
    ASSERT_TRUE(arr.capacity >= 10);
    size_t old_capacity = arr.capacity;
    arr.reserve(old_capacity + 5);
    ASSERT_TRUE(arr.capacity >= old_capacity + 5);
    arr.cleanup();
    check_secret_data(&arr, 345690123, -345690123);
}

void test_array_push_many(void) {
    Test_array arr = {};
    set_secret_data(&arr, 9911199, -9911199);

    int batch[] = { 10, 20, 30, 40 };
    arr.push_many(batch, 4);
    ASSERT_EQUALS(arr.count, 4);
    for (size_t i = 0; i < 4; i++) {
        ASSERT_EQUALS(arr.get(i), batch[i]);
    }
    arr.cleanup();
    check_secret_data(&arr, 9911199, -9911199);
}

void test_arr_free(void) {
    Test_array arr = {};
    set_secret_data(&arr, 222222221, -222222221);

    arr.push(1);
    arr.cleanup();
    ASSERT_TRUE(arr.data == NULL);
    ASSERT_EQUALS(arr.count, 0);
    ASSERT_EQUALS(arr.capacity, 0);
    check_secret_data(&arr, 222222221, -222222221);
}

void test_arr_large_push(void) {
    Test_array arr = {};
    set_secret_data(&arr, 777, -777);

    size_t max_test = 100000; // large but practical test size
    for (size_t i = 0; i < max_test; i++) {
        arr.push((int)i);
    }
    ASSERT_EQUALS(arr.count, max_test);
    ASSERT_TRUE(arr.capacity >= max_test);
    ASSERT_EQUALS(arr.first(), 0);
    ASSERT_EQUALS(arr.last(), (int)(max_test - 1));
    arr.cleanup();
    check_secret_data(&arr, 777, -777);
}

void test_arr_null_push(void) {
    Test_array arr = {};
    set_secret_data(&arr, 6581, -6581);

    arr.push(NULL);
    ASSERT_EQUALS(arr.count, 1);
    ASSERT_TRUE(arr.first() == NULL);
    arr.cleanup();
    check_secret_data(&arr, 6581, -6581);
}

void test_arr_remove_all_unordered(void) {
    Test_array arr = {};
    set_secret_data(&arr, 3332211, -3332211);

    for (int i = 0; i < 10; i++) {
        arr.push(i * 2);
    }
    while (arr.count > 0) {
        arr.remove_unordered(0);
    }
    ASSERT_EQUALS(arr.count, 0);
    arr.cleanup();
    check_secret_data(&arr, 3332211, -3332211);
}

void test_arr_resize_stress(void) {
    Test_array arr = {};
    set_secret_data(&arr, 912323576, -912323576);

    for (int i = 0; i < 50; i++) {
        arr.resize(i);
        // ASSERT_TRUE(arr.capacity >= (size_t)i || arr.capacity >= ARRAY_INITIAL_CAPACITY);
        ASSERT_TRUE(arr.capacity >= (size_t)i);
    }
    arr.cleanup();
    check_secret_data(&arr, 912323576, -912323576);
}

int main(void)
{
    test_arr_initial_state();
    test_array_push_and_resize();
    test_array_get();
    // test_arr_pop();
    test_arr_remove_unordered();
    test_arr_remove_all_unordered();
    test_arr_clear();
    test_arr_resize_and_reserve();
    test_array_push_many();
    test_arr_free();
    test_arr_large_push();
    test_arr_null_push();
    test_arr_resize_stress();
    // test_arr_custom_struct();

    ASSERT(!is_memory_leak_detected(), "Memory leak detected!");

    printf("Every test passed\n");
}
