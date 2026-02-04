#include "DebugAllocationsForTests.h"
#define SL_IMPLEMENTATION
#include "../Sl/Sl.hpp"

#error "Under construction"

using namespace Sl;

void test_hashmap_init_cleanup(void) {
    HashMap<int, int> map = { 0 };
    hashmap_cleanup(&map);
    ASSERT_EQUALS(map.items, NULL);
    ASSERT_EQUALS(map.count, 0);
    ASSERT_EQUALS(map.capacity, 0);
}

void test_hashmap_basic_put_get(void) {
    HashMap map = {0};
    hashmap_init(&map);

    int key = 42;
    int value = 100;
    hashmap_put(&map, &key, &value);

    int* retrieved = hashmap_get(&map, &key);
    ASSERT_NOT_NULL(retrieved);
    ASSERT_EQUALS(*retrieved, 100);

    hashmap_cleanup(&map);
}

void test_hashmap_cstr_put_get(void) {
    HashMap map = {0};
    hashmap_init(&map);

    hashmap_put_cstr(&map, "hello", 123);
    void* value = hashmap_get_cstr(&map, "hello");
    ASSERT_NOT_NULL(value);
    ASSERT_EQUALS(value, 123);

    hashmap_cleanup(&map);
}

void test_hashmap_lit_put_get(void) {
    HashMap map = {0};
    hashmap_init(&map);

    hashmap_put_lit(&map, "test", 999);
    void* value = hashmap_get_lit(&map, "test");
    ASSERT_NOT_NULL(value);
    ASSERT_EQUALS(value, 999);

    hashmap_cleanup(&map);
}

void test_hashmap_overwrite(void) {
    HashMap map = {0};
    hashmap_init(&map);

    int key = 1;
    hashmap_put(&map, &key, 10);
    hashmap_put(&map, &key, 20);

    int* value = hashmap_get(&map, &key);
    ASSERT_EQUALS(value, 20);

    hashmap_cleanup(&map);
}

void test_hashmap_remove(void) {
    HashMap map = {0};
    hashmap_init(&map);

    int key = 123;
    hashmap_put(&map, &key, 456);
    void* removed = hashmap_remove(&map, &key);
    ASSERT_EQUALS(removed, 456);
    ASSERT_TRUE(hashmap_get(&map, &key) == NULL);

    hashmap_cleanup(&map);
}

void test_hashmap_resize(void) {
    HashMap map = {0};
    hashmap_init(&map);

    for (int i = 0; i < 100; i++) {
        hashmap_put_lit(&map, i, (i * 10));
    }

    hashmap_resize(&map, 200);
    ASSERT_NOT_NULL(hashmap_get_lit(&map, 50));

    hashmap_cleanup(&map);
}

void test_hashmap_get_missing(void) {
    HashMap map = {0};
    hashmap_init(&map);

    int key = 999;
    ASSERT_TRUE(hashmap_get(&map, &key) == NULL);

    hashmap_cleanup(&map);
}

void test_hashmap_collisions(void) {
    HashMap map = {0};
    hashmap_init(&map);

    // Keys that likely hash collide
    int keys[] = { 1, 1001, 2001, 3001, 4001, 5001, 6001, 7001, 8001, 9001 };
    int values[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };
    static_assert(ARRAY_LEN(keys) == ARRAY_LEN(values), "");

    for (int i = 0; i < ARRAY_LEN(keys); i++) {
        hashmap_put(&map, &keys[i], &values[i]);
    }

    for (int i = 0; i < ARRAY_LEN(keys); i++) {
        ASSERT_EQUALS(*(int*)hashmap_get(&map, &keys[i]), values[i]);
    }

    hashmap_cleanup(&map);
}

void test_hashmap_empty_after_remove_all(void) {
    HashMap map = {0};
    hashmap_init(&map);

    int key1 = 1, key2 = 2;
    hashmap_put(&map, &key1, 10);
    hashmap_put(&map, &key2, 20);

    hashmap_remove(&map, &key1);
    hashmap_remove(&map, &key2);
    ASSERT_EQUALS(map.count, 0);

    hashmap_cleanup(&map);
}

void test_hashmap_double_cleanup(void) {
    HashMap map = {0};
    hashmap_init(&map);
    hashmap_cleanup(&map);
    hashmap_cleanup(&map); // should be safe
}

void test_hashmap_different_key_sizes(void) {
    HashMap map = {0};
    hashmap_init(&map);

    int int_key = 42;
    char char_key = 'A';

    hashmap_put(&map, &int_key, 100);
    hashmap_put(&map, &char_key, 200);

    ASSERT_EQUALS(hashmap_get(&map, &int_key), 100);
    ASSERT_EQUALS(hashmap_get(&map, &char_key), 200);

    hashmap_cleanup(&map);
}

void test_hashmap_zero_key_len(void) {
    HashMap map = {0};
    hashmap_init(&map);

    // Edge case: zero length key
    hashmap_put_impl(&map, NULL, 0, 999);
    ASSERT_EQUALS(map.count, 0);

    hashmap_cleanup(&map);
}

void test_hashmap_init_different_sizes(void) {
    HashMap map1 = {0}, map2 = {0};
    hashmap_init(&map1, .capacity = 16);
    hashmap_init(&map2, .capacity = 1024);

    ASSERT_TRUE(map1.capacity = 16);
    ASSERT_TRUE(map2.capacity = 1024);

    hashmap_cleanup(&map1);
    hashmap_cleanup(&map2);
}

void test_hashmap_massive_insertion(void) {
    HashMap map;
    hashmap_init(&map);

    usize count = 5000;
    for (usize i = 0; i < count; i++) {
        hashmap_put_lit(&map, i, (i * 100));
    }

    ASSERT_EQUALS(map.count, count);
    for (usize i = 0; i < count; i++) {
        usize result = (usize)hashmap_get_lit(&map, i);
        ASSERT_EQUALS(result, i * 100);
    }

    hashmap_cleanup(&map);
}

void test_hashmap_resize_multiple_times(void) {
    HashMap map;
    hashmap_init(&map, .capacity = 16);

    for (int i = 0; i < 1000; i++) {
        hashmap_put_lit(&map, i, i);
    }

    // Multiple resizes
    hashmap_resize(&map, 32);
    hashmap_resize(&map, 1024 * 1);
    hashmap_resize(&map, 1024 * 2);
    hashmap_resize(&map, 1024 * 16);

    ASSERT_EQUALS(hashmap_get_lit(&map, 1), 1);
    ASSERT_EQUALS(hashmap_get_lit(&map, 500), 500);
    ASSERT_EQUALS(hashmap_get_lit(&map, 999), 999);

    hashmap_cleanup(&map);
}

void test_hashmap_collision_stress(void) {
    HashMap map;
    hashmap_init(&map, .capacity = 8);

    for (usize i = 0; i < 999999; i++) {
        usize key = (i << 16) | 0xFFFF; // Same low 16 bits
        hashmap_put_lit(&map, key, (i * 10));
    }

    for (usize i = 0; i < 999999; i++) {
        usize key = (i << 16) | 0xFFFF;
        usize result = (usize) hashmap_get_lit(&map, key);
        ASSERT_EQUALS(result, i * 10);
    }

    hashmap_cleanup(&map);
}

void test_hashmap_remove_during_iteration_pattern(void) {
    HashMap map;
    hashmap_init(&map);

    for (int i = 0; i < 50; i++) {
        hashmap_put(&map, &i, i);
    }

    // Remove every other key
    for (int i = 0; i < 50; i += 2) {
        void* removed = hashmap_remove(&map, &i);
        ASSERT_EQUALS(removed, i);
    }

    ASSERT_EQUALS(map.count, 25);
    for (int i = 1; i < 50; i += 2) {
        ASSERT_NOT_NULL(hashmap_get(&map, &i));
    }

    hashmap_cleanup(&map);
}

void test_hashmap_put_null_value(void) {
    HashMap map;
    hashmap_init(&map);

    int key = 42;
    hashmap_put(&map, &key, NULL);
    void* value = hashmap_get(&map, &key);
    ASSERT_TRUE(value == NULL);

    hashmap_cleanup(&map);
}

void test_hashmap_duplicate_keys_concurrent_style(void) {
    HashMap map;
    hashmap_init(&map);

    int key = 1;
    for (int i = 0; i < 100; i++) {
        hashmap_put(&map, &key, i);
    }

    void* final = hashmap_get(&map, &key);
    ASSERT_EQUALS(final, 99); // Last write wins

    hashmap_cleanup(&map);
}

void test_hashmap_large_key_structs(void) {
    HashMap map;
    hashmap_init(&map);

    // 128-byte struct keys
    struct { char data[128]; } key1 = { {0} }, key2 = { {1} };
    hashmap_put_impl(&map, &key1, sizeof(key1), 100);
    hashmap_put_impl(&map, &key2, sizeof(key2), 200);

    ASSERT_EQUALS(hashmap_get_impl(&map, &key1, sizeof(key1)), 100);
    ASSERT_EQUALS(hashmap_get_impl(&map, &key2, sizeof(key2)), 200);

    hashmap_cleanup(&map);
}

void test_hashmap_resize_empty(void) {
    HashMap map;
    hashmap_init(&map);

    hashmap_resize(&map, 4096);
    ASSERT_EQUALS(map.count, 0);

    int key = 42;
    hashmap_put(&map, &key, 100);
    ASSERT_NOT_NULL(hashmap_get(&map, &key));

    hashmap_cleanup(&map);
}

void test_hashmap_cleanup_uninit(void) {
    HashMap map = { 0 }; // Uninitialized
    hashmap_cleanup(&map);
    hashmap_cleanup(&map); // Should be safe
}

typedef struct TestStruct
{
    int x;
    float y;
} TestStruct;

void test_hashmap_all_macro_variants(void) {
    HashMap map;
    hashmap_init(&map);

    int int_key = 1;
    char* cstr_key = "hello";
    TestStruct test_struct = {0};
    test_struct.x = 123;
    test_struct.y = 333.0;

    hashmap_put(&map, &int_key, 10);
    hashmap_put(&map, &test_struct, 30);
    hashmap_put_cstr(&map, cstr_key, 20);

    ASSERT_EQUALS(hashmap_get(&map, &int_key), 10);
    ASSERT_EQUALS(hashmap_get_cstr(&map, cstr_key), 20);
    ASSERT_EQUALS(hashmap_get(&map, &test_struct), 30);

    hashmap_cleanup(&map);
}

void test_hashmap_remove_nonexistent(void) {
    HashMap map;
    hashmap_init(&map);

    int key = 999;
    void* result = hashmap_remove(&map, &key);
    ASSERT_TRUE(result == NULL);

    hashmap_cleanup(&map);
}


int main()
{
    test_hashmap_init_cleanup();
    test_hashmap_basic_put_get();
    test_hashmap_cstr_put_get();
    test_hashmap_lit_put_get();
    test_hashmap_overwrite();
    test_hashmap_resize();
    test_hashmap_get_missing();
    test_hashmap_collisions();
    test_hashmap_double_cleanup();
    test_hashmap_different_key_sizes();
    test_hashmap_zero_key_len();
    test_hashmap_init_different_sizes();
    test_hashmap_massive_insertion();
    test_hashmap_resize_multiple_times();
    test_hashmap_collision_stress();
    test_hashmap_put_null_value();
    test_hashmap_duplicate_keys_concurrent_style();
    test_hashmap_large_key_structs();
    test_hashmap_resize_empty();
    test_hashmap_cleanup_uninit();
    test_hashmap_all_macro_variants();
    //test_hashmap_remove();
    //test_hashmap_remove_nonexistent();
    //test_hashmap_remove_during_iteration_pattern();
    //test_hashmap_empty_after_remove_all();

    ASSERT_FALSE(is_memory_leak_detected());

    printf("Every test passed\n");
}
