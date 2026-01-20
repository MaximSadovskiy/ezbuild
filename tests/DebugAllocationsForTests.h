#pragma once

#include <stdlib.h>
#include <stdint.h>

void* custom_realloc(void* ptr, size_t size);
void custom_free(void* ptr);
static int is_memory_leak_detected(void);

#define SL_ARRAY_REALLOC(ptr, size) custom_realloc((ptr), (size))
#define ARRAY_FREE(ptr) custom_free((ptr))
#define SL_ALLOCATOR_MALLOC(size) custom_realloc(NULL, (size))
#define SL_ALLOCATOR_FREE(ptr) custom_free((ptr))

#ifdef SL_CORE_H
#error Must be included before <Sl.h>
#endif

static int64_t alloc_count = 0;
static int64_t total_allocated_size = 0;

#define ALLOC_HEADER_SIZE sizeof(size_t)

static void* custom_realloc(void* ptr, size_t size) {
    if (!ptr) {
        // New allocation: allocate extra space to store size
        void* base = malloc(size + ALLOC_HEADER_SIZE);
        if (!base) return NULL;
        // Store size at start
        *((size_t*)base) = size;
        alloc_count++;
        total_allocated_size += size;
        // Return pointer after size header
        return (char*)base + ALLOC_HEADER_SIZE;
    }
    if (size == 0) {
        // realloc with size zero is free
        // get base pointer
        void* base = (char*)ptr - ALLOC_HEADER_SIZE;
        size_t old_size = *((size_t*)base);
        free(base);
        alloc_count--;
        total_allocated_size -= old_size;
        return NULL;
    }
    // Reallocate existing block
    void* base_old = (char*)ptr - ALLOC_HEADER_SIZE;
    size_t old_size = *((size_t*)base_old);
    void* base_new = realloc(base_old, size + ALLOC_HEADER_SIZE);
    if (!base_new) return NULL; // realloc failure, original block intact
    *((size_t*)base_new) = size;

    // Update total allocated size
    if (size > old_size) {
        total_allocated_size += (size - old_size);
    }
    else if (old_size > size) {
        total_allocated_size -= (old_size - size);
    }
    // Return pointer after header
    return (char*)base_new + ALLOC_HEADER_SIZE;
}

static void custom_free(void* ptr) {
    if (ptr) {
        void* base = (char*)ptr - ALLOC_HEADER_SIZE;
        size_t old_size = *((size_t*)base);
        free(base);
        alloc_count--;
        total_allocated_size -= old_size;
    }
}

static int is_memory_leak_detected(void) {
    if (alloc_count != 0 || total_allocated_size != 0)
        return 1;
    return 0;
}
