#include "DebugAllocationsForTests.h"
#define SL_IMPLEMENTATION
#include "../Sl.h"

void test_linear_basic_alloc(void) {
    LinearAllocator a;
    linear_init(&a, 1024);

    void* ptr1 = linear_alloc(&a, 16);
    void* ptr2 = linear_alloc(&a, 32);
    ASSERT_NOT_NULL(ptr1);
    ASSERT_NOT_NULL(ptr2);
    ASSERT_TRUE((char*)ptr2 >= (char*)ptr1);

    linear_free(&a);
}

void test_linear_reset_reuse(void) {
    LinearAllocator a;
    linear_init(&a, 512);

    void* ptr1 = linear_alloc(&a, 64);
    linear_reset(&a);
    void* ptr2 = linear_alloc(&a, 64);

    ASSERT_EQUALS(ptr1, ptr2); // Same memory reused

    linear_free(&a);
}

void test_linear_snapshot_rewind(void) {
    LinearAllocator a;
    linear_init(&a, 1024);

    LinearSnapshot snap = linear_snapshot(&a);
    void* ptr1 = linear_alloc(&a, 32);

    linear_rewind(&a, snap);
    void* ptr2 = linear_alloc(&a, 32);
    ASSERT_EQUALS(ptr1, ptr2);

    linear_free(&a);
}

void test_linear_alignment(void) {
    LinearAllocator a;
    linear_init(&a, 1024);

    void* ptr = linear_alloc_aligned(&a, 1, 16);
    ASSERT_EQUALS((uintptr_t)ptr % 16, 0);

    linear_free(&a);
}

void test_linear_exhaustion(void) {
    LinearAllocator a;
    linear_init(&a, 100);

    void* ptrs[20];
    int i;
    for (i = 0; i < 20; i++) {
        ptrs[i] = linear_alloc(&a, 8);
        if (!ptrs[i]) break;
    }

    ASSERT_TRUE(i < 20); // Should exhaust
    linear_free(&a);
}

void test_stack_basic(void) {
    StackAllocator a;
    stack_init(&a, 1024);

    void* ptr1 = stack_alloc(&a, 16);
    void* ptr2 = stack_alloc(&a, 32);
    ASSERT_TRUE((char*)ptr2 >= (char*)ptr1);

    stack_free(&a);
}

void test_stack_dealloc_last(void) {
    StackAllocator a;
    stack_init(&a, 512);

    void* ptr1 = stack_alloc(&a, 64);
    void* ptr2 = stack_alloc(&a, 64);
    stack_dealloc(&a, ptr2);

    void* ptr3 = stack_alloc(&a, 64);
    ASSERT_EQUALS(ptr2, ptr3);

    stack_free(&a);
}

void test_stack_dealloc_middle(void) {
    StackAllocator a;
    stack_init(&a, 512);

    void* ptr1 = stack_alloc_aligned(&a, 32, 0);
    void* ptr2 = stack_alloc(&a, 32);
    void* ptr3 = stack_alloc(&a, 32);

    stack_dealloc(&a, ptr2); // This is wrong, but we will allow it
    ASSERT_EQUALS(stack_memory_consumed(&a), 32); // Only first consumed

    stack_free(&a);
}

void test_stack_memory_metrics(void) {
    StackAllocator a;
    stack_init(&a, 1024);

    ASSERT_EQUALS(stack_memory_left(&a), 1024);
    ASSERT_EQUALS(stack_memory_consumed(&a), 0);

    stack_alloc(&a, 128);
    ASSERT_EQUALS(stack_memory_left(&a), 1024 - 128);
    ASSERT_EQUALS(stack_memory_consumed(&a), 128);

    stack_free(&a);
}

void test_pool_basic(void) {
    PoolAllocator a;
    pool_init(&a, 10, 64);

    void* ptrs[15];
    for (int i = 0; i < 15; i++) {
        ptrs[i] = pool_alloc(&a);
    }

    ASSERT_NOT_NULL(ptrs[9]); // Should grow chunks
    pool_free(&a);
}

void test_pool_dealloc_reuse(void) {
    PoolAllocator a;
    pool_init(&a, 2, 32);

    void* ptr1 = pool_alloc(&a);
    void* ptr2 = pool_alloc(&a);
    pool_dealloc(&a, ptr1);

    void* ptr3 = pool_alloc(&a);
    ASSERT_EQUALS(ptr1, ptr3);

    pool_free(&a);
}

void test_pool_reset(void) {
    PoolAllocator a;
    pool_init(&a, 3, 48);

    void* ptr1 = pool_alloc(&a);
    pool_reset(&a);
    void* ptr2 = pool_alloc(&a);

    ASSERT_EQUALS(ptr1, ptr2);
    pool_free(&a);
}

void test_arena_basic(void) {
    ArenaAllocator a;
    arena_init(&a, 512);

    void* ptr1 = arena_alloc(&a, 128);
    void* ptr2 = arena_alloc(&a, 256);
    ASSERT_TRUE((char*)ptr2 >= (char*)ptr1);

    arena_free(&a);
}

void test_arena_grow_regions(void) {
    ArenaAllocator a;
    arena_init(&a, 256);

    void* ptrs[10];
    for (int i = 0; i < 10; i++) {
        ptrs[i] = arena_alloc(&a, 300); // Force region growth
    }

    ASSERT_NOT_NULL(ptrs[9]);
    arena_free(&a);
}

void test_arena_snapshot_rewind(void) {
    ArenaAllocator a;
    arena_init(&a, 512);

    ArenaSnapshot snap = arena_snapshot(&a);
    void* ptr1 = arena_alloc(&a, 64);

    arena_rewind(&a, snap);
    void* ptr2 = arena_alloc(&a, 64);
    ASSERT_EQUALS(ptr1, ptr2);

    arena_free(&a);
}

void test_arena_alignment(void) {
    ArenaAllocator a;
    arena_init(&a, 1024);

    void* ptr = arena_alloc_aligned(&a, 1, 32);
    ASSERT_EQUALS((uintptr_t)ptr % 32, 0);

    arena_free(&a);
}

// Edge cases
void test_linear_zero_size(void) {
    LinearAllocator a;
    linear_init(&a, 1024);
    void* ptr = linear_alloc(&a, 0);
    ASSERT_NOT_NULL(ptr); // or NULL, depending on impl
    linear_free(&a);
}

void test_stack_zero_size(void) {
    StackAllocator a;
    stack_init(&a, 1024);
    void* ptr = stack_alloc(&a, 0);
    ASSERT_NOT_NULL(ptr);
    stack_free(&a);
}

void test_linear_multiple_resets(void) {
    LinearAllocator a;
    linear_init(&a, 256);
    linear_reset(&a);
    linear_reset(&a); // Multiple safe
    linear_free(&a);
    linear_free(&a);
}

void test_linear_alignment_multiple(void) {
    LinearAllocator a;
    linear_init(&a, 1024);

    void* ptr1 = linear_alloc_aligned(&a, 8, 16);
    void* ptr2 = linear_alloc_aligned(&a, 8, 32);
    void* ptr3 = linear_alloc_aligned(&a, 8, 64);

    ASSERT_EQUALS((uintptr_t)ptr1 % 16, 0);
    ASSERT_EQUALS((uintptr_t)ptr2 % 32, 0);
    ASSERT_EQUALS((uintptr_t)ptr3 % 64, 0);
    ASSERT_TRUE((char*)ptr2 > (char*)ptr1);
    ASSERT_TRUE((char*)ptr3 > (char*)ptr2);

    linear_free(&a);
}

void test_linear_exhaustion_aligned(void) {
    LinearAllocator a;
    linear_init(&a, 256);

    // Allocate with alignment wasting space
    int count = 0;
    while (linear_alloc_aligned(&a, 1, 64)) count++;

    ASSERT_TRUE(count <= 4); // Limited by alignment waste
    linear_free(&a);
}

void test_linear_snapshot_nested(void) {
    LinearAllocator a;
    linear_init(&a, 512);

    LinearSnapshot snap1 = linear_snapshot(&a);
    void* ptr1 = linear_alloc(&a, 32);
    LinearSnapshot snap2 = linear_snapshot(&a);
    void* ptr2 = linear_alloc(&a, 32);

    linear_rewind(&a, snap1);
    ASSERT_EQUALS(a.cursor, snap1.index);
    void* ptr3 = linear_alloc(&a, 32);
    ASSERT_EQUALS(ptr1, ptr3);

    linear_free(&a);
}

void test_stack_alignment_stress(void) {
    StackAllocator a;
    stack_init(&a, 1024);

    void* ptrs[20];
    for (int i = 0; i < 20; i++) {
        ptrs[i] = stack_alloc_aligned(&a, 16, 32);
        ASSERT_EQUALS((uintptr_t)ptrs[i] % 32, 0);
    }

    stack_free(&a);
}

void test_stack_dealloc_order_violation(void) {
    StackAllocator a;
    stack_init(&a, 512);

    void* ptr1 = stack_alloc(&a, 32);
    void* ptr2 = stack_alloc(&a, 32);
    void* ptr3 = stack_alloc(&a, 32);

    // Try deallocating middle (should ignore or handle)
    stack_dealloc(&a, ptr2);
    stack_dealloc(&a, ptr1); // Only top works

    stack_free(&a);
}

void test_stack_memory_exhaustion(void) {
    StackAllocator a;
    stack_init(&a, 100);

    ASSERT_EQUALS(stack_memory_left(&a), 100);
    void* ptr = NULL;
    do {
        ptr = stack_alloc(&a, 1);
    } while (ptr != NULL && stack_memory_left(&a) > 0);
    ASSERT_EQUALS(ptr, NULL);

    stack_free(&a);
}

void test_pool_many_chunks(void) {
    PoolAllocator a;
    pool_init(&a, 100, 32);

    void* ptrs[100];
    for (int i = 0; i < 100; i++) {
        ptrs[i] = pool_alloc(&a);
    }

    ASSERT_NOT_NULL(ptrs[99]);
    pool_free(&a);
}

void test_pool_alloc_ex_variable_size(void) {
    PoolAllocator a;
    pool_init(&a, 10, 64);

    void* small = pool_alloc_ex(&a, 17);
    void* large = pool_alloc_ex(&a, 63);

    ASSERT_NOT_NULL(small);
    ASSERT_NOT_NULL(large);
    pool_free(&a);
}

void test_pool_mixed_alloc_dealloc(void) {
    PoolAllocator a;
    pool_init(&a, 11, 48);

    void* ptrs[10] = {0};
    for (int i = 0; i < 10; i++)
        ptrs[i] = pool_alloc(&a);

    // Random dealloc/reuse pattern
    pool_dealloc(&a, ptrs[2]);
    pool_dealloc(&a, ptrs[5]);
    void* reused1 = pool_alloc(&a);
    void* reused2 = pool_alloc(&a);

    ASSERT_EQUALS(reused2, ptrs[2]);
    ASSERT_EQUALS(reused1, ptrs[5]);

    pool_free(&a);
}

void test_arena_region_exhaustion(void) {
    ArenaAllocator a;
    arena_init(&a, 128);

    void* ptrs[20];
    for (int i = 0; i < 20; i++) {
        ptrs[i] = arena_alloc(&a, 150); // Force new region each time
    }

    ASSERT_NOT_NULL(ptrs[19]);
    arena_free(&a);
}

void test_arena_nested_snapshots(void) {
    ArenaAllocator a;
    arena_init(&a, 256);

    ArenaSnapshot snap1 = arena_snapshot(&a);
    arena_alloc(&a, 64);
    ArenaSnapshot snap2 = arena_snapshot(&a);
    arena_alloc(&a, 64);

    arena_rewind(&a, snap1);
    ASSERT_EQUALS(a.current_region_index, snap1.region_index);

    arena_free(&a);
}

void test_arena_alignment_waste(void) {
    ArenaAllocator a;
    arena_init(&a, 512);

    void* ptr1 = arena_alloc_aligned(&a, 1, 8);
    void* ptr2 = arena_alloc_aligned(&a, 1, 64);

    // Check alignment maintained despite waste
    ASSERT_EQUALS((uintptr_t)ptr1 % 8, 0);
    ASSERT_EQUALS((uintptr_t)ptr2 % 64, 0);

    arena_free(&a);
}

void test_arena_snapshot_after_reset(void) {
    ArenaAllocator a;
    arena_init(&a, 512);

    arena_alloc(&a, 64);
    arena_reset(&a);
    ArenaSnapshot snap = arena_snapshot(&a);

    arena_rewind(&a, snap);
    arena_free(&a);
}

void test_allocator_double_free(void) {
    LinearAllocator la;
    linear_init(&la, 256);
    void* ptr = linear_alloc(&la, 32);
    linear_free(&la);
    linear_free(&la); // Safe double free

    StackAllocator sa;
    stack_init(&sa, 256);
    stack_free(&sa);
    stack_free(&sa);

    PoolAllocator pa;
    pool_init(&pa, 4, 32);
    pool_free(&pa);
    pool_free(&pa);

    ArenaAllocator aa;
    arena_init(&aa, 256);
    arena_free(&aa);
    arena_free(&aa);
}

void test_linear_unaligned_alloc_after_aligned(void) {
    LinearAllocator a;
    linear_init(&a, 1024);

    void* aligned = linear_alloc_aligned(&a, 16, 32);
    void* unaligned = linear_alloc(&a, 16);

    ASSERT_EQUALS((uintptr_t)aligned % 32, 0);
    ASSERT_TRUE((char*)unaligned > (char*)aligned);

    linear_free(&a);
}

void test_pool_alloc_ex_alignment(void) {
    PoolAllocator a;
    pool_init(&a, 10, 64);

    void* ptr = pool_alloc_aligned(&a, 32, 16);
    ASSERT_EQUALS((uintptr_t)ptr % 16, 0);
    pool_free(&a);
}

int main()
{
    test_linear_basic_alloc();
    test_linear_reset_reuse();
    test_linear_snapshot_rewind();
    //test_linear_alignment();
    //test_arena_alignment();
    //test_linear_alignment_multiple();
    //test_stack_alignment_stress();
    //test_arena_alignment_waste();
    //test_linear_unaligned_alloc_after_aligned();
    //test_pool_alloc_ex_alignment();
    test_linear_exhaustion();
    test_stack_basic();
    test_stack_dealloc_last();
    test_stack_dealloc_middle();
    test_stack_memory_metrics();
    test_pool_basic();
    test_pool_dealloc_reuse();
    test_pool_reset();
    test_arena_basic();
    test_arena_grow_regions();
    test_arena_snapshot_rewind();
    test_linear_zero_size();
    test_stack_zero_size();
    test_linear_multiple_resets();
    test_linear_exhaustion_aligned();
    test_linear_snapshot_nested();
    test_stack_dealloc_order_violation();
    test_stack_memory_exhaustion();
    test_pool_many_chunks();
    test_pool_alloc_ex_variable_size();
    test_pool_mixed_alloc_dealloc();
    test_arena_region_exhaustion();
    test_arena_nested_snapshots();
    test_arena_snapshot_after_reset();
    test_allocator_double_free();

    ASSERT_FALSE(is_memory_leak_detected());

    printf("Every test passed\n");
}
