#ifndef SL_ALLOCATORS_H
#define SL_ALLOCATORS_H

#include "Sl_Defines.hpp"

namespace Sl
{
    // If you want to use snapshot()/rewind() feature you create your custom snapshot and extend from this class
    struct Snapshot
    {
        virtual ~Snapshot() = default;
        // When you making snapshot() you must initilize this member variable
        //  so that you can ensure that this size equals to your custom Snapshot during rewind()
        // This is needed to make it more safe, so that you can ensure no memory corruption
        usize size_of_allocated_snapshot = 0;
    };

    // If you want to use custom allocator, you must extend from this base class
    //  or you can use already implemented allocators bellow
    struct Allocator
    {
        virtual ~Allocator() = default;

        virtual void* allocate(usize size) = 0;
        virtual void* allocate_aligned(usize size, u16 alignment) = 0;
        virtual void* reallocate(void* ptr, usize old_size, usize new_size) = 0;
        virtual Snapshot* snapshot() = 0;
        virtual void rewind(Snapshot* snapshot) = 0;
        virtual void reset() = 0;
        virtual void cleanup() = 0;
        virtual void display_content() = 0;
    };
}
#include "Sl_Array.hpp"
namespace Sl
{
    Allocator* get_global_allocator() noexcept;
    void set_global_allocator(Allocator& allocator) noexcept;
    // Deallocates global allocator. You don't need to call it before calling set_global_allocator(), it will be done for you
    void cleanup_global_allocator() noexcept;
    // Allocate some memory from the global allocator.
    void* temp_allocate(usize size) noexcept;
    // Allocate some memory with alignment from the global allocator. 0 alignment equals no alignment
    void* temp_allocate_aligned(usize size, u16 alignment = sizeof(void*)) noexcept;
    // Make a "snapshot" of current state of the global allocator,
    //  in order to restore it later (basically deallocates used resources) by calling temp_end(snapshot);
    Snapshot* temp_begin() noexcept;
    // Restores state of the global allocator of when temp_begin() was called
    //  (!!! This doesnt actually free's any resources)
    void temp_end(Snapshot* snapshot) noexcept;
    // Reset's (but not free's!!) global allocator
    //  Only recomended if you know that you not uses resources allocated by global allocator.
    //  Some ezbuild.h functions uses global_allocator, for example to strdup file names.
    // Better do:
    //   Snapshot* snapshot = temp_begin();
    //   {
    //     // do some temp allocations...
    //   }
    //   temp_end(snapshot);
    //
    // Or:
    //   {
    //      ScopedAllocator temp; // will call temp_end() at the end of the scope automatically
    //      // do some temp allocations...
    //   }
    void temp_reset() noexcept;

    // Saves state of the local/global allocator in the Constructor and restores state at the Destructor.
    struct ScopedAllocator
    {
        Allocator* _allocator_ref;
        Snapshot* _snapshot;

        ScopedAllocator(Allocator& allocator)
        {
            _allocator_ref = &allocator;
            _snapshot = _allocator_ref->snapshot();
        }
        ScopedAllocator()
        {
            _allocator_ref = get_global_allocator();
            _snapshot = _allocator_ref->snapshot();
        }
        ~ScopedAllocator()
        {
            _allocator_ref->rewind(_snapshot);
        }
    };

    struct ArenaSnapshot : Snapshot
    {
        usize region_index = 0;
        usize index = 0;
    };

    struct ArenaRegion
    {
        char* data = nullptr;
        usize cursor = 0;
        usize capacity = 0;
    };

    struct ArenaAllocator : Allocator
    {
    private:
        Array<ArenaRegion, false> _regions;
        usize _current_region_index;
        usize _region_size;
    public:
        ArenaAllocator(usize default_region_size = ALLOCATOR_INITIAL_SIZE)
        {
            _region_size = default_region_size;
            _current_region_index = 0;
        }

        inline Array<ArenaRegion, false>& regions() { return _regions; }
        inline usize current_region_index() { return _current_region_index; }
        inline usize region_size() { return _region_size; }

        void* allocate(usize size) override;
        void* allocate_aligned(usize size, u16 alignment) override;
        void* reallocate(void* ptr, usize old_size, usize new_size) override;
        Snapshot* snapshot() override;
        void rewind(Snapshot* snapshot) override;
        void reset() override;
        void cleanup() override;
        void display_content() override;
    };

    struct LinearSnapshot : Snapshot
    {
        usize index = 0;
    };
    struct LinearAllocator : Allocator
    {
        char* data;
        usize cursor;
        usize total_size;

        LinearAllocator(usize total_size = ALLOCATOR_INITIAL_SIZE)
        {
            this->data = nullptr;
            this->cursor = 0;
            this->total_size = total_size;
        }

        void* allocate(usize size) override;
        void* allocate_aligned(usize size, u16 alignment) override;
        void* reallocate(void* ptr, usize old_size, usize new_size) override;
        Snapshot* snapshot() override;
        void rewind(Snapshot* snapshot) override;
        void reset() override;
        void cleanup() override;
        void display_content() override;
    };

    struct StackSnapshot : Snapshot
    {
        void* current = nullptr;
    };
    struct StackAllocator : Allocator
    {
        void* begin;
        void* end;
        void* current;
        usize total_size;

        StackAllocator(usize total_size = ALLOCATOR_INITIAL_SIZE)
        {
            this->begin = nullptr;
            this->current = nullptr;
            this->end = nullptr;
            this->total_size = total_size;
        }
        void* allocate(usize size) override;
        void* allocate_aligned(usize size, u16 alignment) override;
        void* reallocate(void* ptr, usize old_size, usize new_size) override;
        Snapshot* snapshot() override;
        void rewind(Snapshot* snapshot) override;
        void reset() override;
        void cleanup() override;
        void display_content() override;
    };

    typedef struct PoolChunk {
        struct PoolChunk* next;
    } PoolChunk;
    struct PoolSnapshot : Snapshot
    {
        PoolChunk* current_chunk = nullptr;
    };
    struct PoolAllocator : Allocator
    {
        PoolChunk* root;
        PoolChunk* pool;
        usize chunk_count;
        usize chunk_size;

        PoolAllocator(usize chunk_count = 32, usize chunk_size = ALLOCATOR_INITIAL_SIZE / 32)
        {
            this->root = nullptr;
            this->pool = nullptr;
            this->chunk_count = chunk_count;
            this->chunk_size = chunk_size;
        }

        void* allocate(usize size) override;
        void* allocate_aligned(usize size, u16 alignment) override;
        void* reallocate(void* ptr, usize old_size, usize new_size) override;
        Snapshot* snapshot() override;
        void rewind(Snapshot* snapshot) override;
        void reset() override;
        void cleanup() override;
        void display_content() override;
        // void* pool_alloc_ex(PoolAllocator* a, usize size);
        // void* pool_allocate_aligned(PoolAllocator* a, usize size, s16 alignment);
    };
} // namespace Sl
#endif // !SL_ALLOCATORS_H

#ifdef SL_IMPLEMENTATION
namespace Sl
{
    static SL_THREAD_LOCAL Allocator* _global_alloc = nullptr;

    Allocator* get_global_allocator() noexcept
    {
        if (_global_alloc == nullptr) {
            static SL_THREAD_LOCAL ArenaAllocator _default_global_alloc(GLOBAL_ALLOCATOR_INITIAL_SIZE);
            _global_alloc = &_default_global_alloc;
        }
        return _global_alloc;
    }

    void set_global_allocator(Allocator& new_allocator) noexcept
    {
        get_global_allocator()->cleanup();
        _global_alloc = &new_allocator;
    }

    void cleanup_global_allocator() noexcept
    {
        get_global_allocator()->cleanup();
    }

    void* temp_allocate(usize size) noexcept
    {
        return get_global_allocator()->allocate(size);
    }

    void* temp_allocate_aligned(usize size, u16 alignment) noexcept
    {
        return get_global_allocator()->allocate_aligned(size, alignment);
    }

    Snapshot* temp_begin() noexcept
    {
        return get_global_allocator()->snapshot();
    }

    void temp_end(Snapshot* snapshot) noexcept
    {
        get_global_allocator()->rewind(snapshot);
    }

    void temp_reset() noexcept
    {
        get_global_allocator()->reset();
    }

    void memory_copy(void* dest, usize dest_size, const void* src, usize src_size) noexcept;

    void log_empty(const char* const format, ...);

    void* LinearAllocator::allocate_aligned(usize size, u16 alignment)
    {
        size = ALIGNMENT(size, alignment);
        if (data == nullptr) {
            if (total_size == 0) total_size = GLOBAL_ALLOCATOR_INITIAL_SIZE;
            const auto allocated_size = MAX(total_size, size);
            this->total_size = allocated_size;
            this->data = (char*)ALLOCATOR_MALLOC(allocated_size);
            ASSERT_DEBUG(data != nullptr);
        }
        ASSERT(cursor <= total_size, "Cursor went past total size (possible memory corruption)");

        if (cursor + size > total_size)
            return (void*)NULL;

        void* new_ptr = data + cursor;
        cursor += size;
        return new_ptr;
    }

    void* LinearAllocator::allocate(usize size)
    {
        return allocate_aligned(size, ALLOCATOR_DEFAULT_ALIGNMENT);
    }

    void* LinearAllocator::reallocate(void* ptr, usize old_size, usize new_size)
    {
        void* new_ptr = allocate(new_size);
        if (ptr == nullptr) return new_ptr;

        memory_copy(new_ptr, new_size, ptr, old_size);
        return new_ptr;
    }

    void LinearAllocator::reset()
    {
        cursor = 0;
    }

    void LinearAllocator::cleanup()
    {
        ALLOCATOR_FREE(data);
        data = nullptr;
        cursor = 0;
        total_size = 0;
    }

    void LinearAllocator::rewind(Snapshot* snapshot)
    {
        auto* snapshot_ptr = static_cast<LinearSnapshot*>(snapshot);
        ASSERT(snapshot_ptr != nullptr, "Invalid type of snapshot");
        ASSERT(snapshot_ptr->size_of_allocated_snapshot == sizeof(LinearSnapshot), "Wrong snapshot type");
        this->cursor = snapshot_ptr->index;
    }

    Snapshot* LinearAllocator::snapshot()
    {
        auto current_cursor = cursor;
        auto* snapshot = (LinearSnapshot*) temp_allocate(sizeof(LinearSnapshot));
        snapshot->size_of_allocated_snapshot = sizeof(LinearSnapshot);
        snapshot->index = current_cursor;
        return snapshot;
    }

    void LinearAllocator::display_content()
    {
        log_empty("|---------------------\n");
        log_empty("|Linear allocator:\n");
        log_empty("|---------------------\n");
        log_empty("|  capacity: %zu\n", total_size);
        log_empty("|  cursor: %zu\n", cursor);
        log_empty("|---------------------\n");
    }

    void* StackAllocator::allocate_aligned(usize size, u16 alignment)
    {
        size = ALIGNMENT(size, alignment);
        if (begin == nullptr) {
            if (total_size < 1) total_size = GLOBAL_ALLOCATOR_INITIAL_SIZE;
            begin = (char*)ALLOCATOR_MALLOC(total_size);
            this->current = this->begin;
            this->end = ((char*)this->begin) + total_size;
        }

        if ((char*)current + size > end)
            return nullptr;

        void* ptr = (void*)current;
        current = (char*)current + size;
        return ptr;
    }

    void* StackAllocator::allocate(usize size)
    {
        return allocate_aligned(size, ALLOCATOR_DEFAULT_ALIGNMENT);
    }

    void* StackAllocator::reallocate(void* ptr, usize old_size, usize new_size)
    {
        void* new_ptr = allocate(new_size);
        if (ptr == nullptr) return new_ptr;

        memory_copy(new_ptr, new_size, ptr, old_size);
        return new_ptr;
    }

    Snapshot* StackAllocator::snapshot()
    {
        auto* current_ptr = this->current;
        auto* snapshot = (StackSnapshot*) temp_allocate(sizeof(StackSnapshot));
        if (current_ptr == nullptr) current_ptr = this->begin;

        snapshot->size_of_allocated_snapshot = sizeof(StackSnapshot);
        snapshot->current = current_ptr;
        return snapshot;
    }

    void StackAllocator::rewind(Snapshot* snapshot)
    {
        auto* snapshot_ptr = static_cast<StackSnapshot*>(snapshot);
        ASSERT(snapshot_ptr != nullptr, "Invalid type of snapshot");
        ASSERT(snapshot_ptr->size_of_allocated_snapshot == sizeof(StackSnapshot), "Wrong snapshot type");
        ASSERT_TRUE(snapshot_ptr->current >= (void*)this->begin);
        ASSERT_TRUE(snapshot_ptr->current <= (void*)this->end);
        this->current = snapshot_ptr->current;
    }

    void StackAllocator::reset()
    {
        current = begin;
    }

    void StackAllocator::cleanup()
    {
        ALLOCATOR_FREE(begin);
        begin = nullptr;
        end = nullptr;
        current = nullptr;
    }

    void StackAllocator::display_content()
    {
        log_empty("|---------------------\n");
        log_empty("|Stack allocator:\n");
        log_empty("|---------------------\n");
        log_empty("|  current: %p\n", current);
        log_empty("|  begin: %p\n", begin);
        log_empty("|  end: %p\n", end);
        log_empty("|---------------------\n");
    }

    void* PoolAllocator::allocate(usize size)
    {
        if (this->root == nullptr) {
            const usize total_size = (usize)chunk_count * (usize)chunk_size;

            if(this->chunk_count < 1) this->chunk_count = 32;
            if(this->chunk_size < 1) this->chunk_size = GLOBAL_ALLOCATOR_INITIAL_SIZE / 32;
            this->pool = nullptr;
            this->root = (PoolChunk*)ALLOCATOR_MALLOC(total_size);
            ASSERT_DEBUG(this->root != nullptr);

            reset();
        }
        ASSERT(chunk_size >= size, "Requested size bigger than chunk size of Pool");
        ASSERT(chunk_count > 0, "Chunk count must be bigger than 0");
        ASSERT(chunk_size > 0, "Chunk size must be bigger than 0");

        if (pool == NULL
            || size > chunk_size
            || chunk_count == 0
            || chunk_size == 0
        )
            return (void*)NULL;

        PoolChunk* ptr = pool;
        pool = ptr->next;
        return ptr;
    }

    void* PoolAllocator::allocate_aligned(usize size, u16 alignment)
    {
        UNUSED(alignment);
        return allocate(size);
    }

    void* PoolAllocator::reallocate(void* ptr, usize old_size, usize new_size)
    {
        void* new_ptr = allocate(new_size);
        if (ptr == nullptr) return new_ptr;

        memory_copy(new_ptr, new_size, ptr, old_size);
        return new_ptr;
    }

    Snapshot* PoolAllocator::snapshot()
    {
        auto* current_chunk = this->pool;
        auto* snapshot = (PoolSnapshot*) temp_allocate(sizeof(PoolSnapshot));
        if (current_chunk == nullptr) current_chunk = this->root;

        snapshot->size_of_allocated_snapshot = sizeof(PoolSnapshot);
        snapshot->current_chunk = current_chunk;
        return snapshot;
    }

    void PoolAllocator::rewind(Snapshot* snapshot)
    {
        auto* snapshot_ptr = static_cast<PoolSnapshot*>(snapshot);
        ASSERT(snapshot_ptr != nullptr, "Invalid type of snapshot");
        ASSERT(snapshot_ptr->size_of_allocated_snapshot == sizeof(PoolSnapshot), "Wrong snapshot type");

        printf("Snapshot %p\n", snapshot_ptr->current_chunk);
        this->pool = snapshot_ptr->current_chunk;
    }

    void PoolAllocator::reset()
    {
        const usize total_size = chunk_count * chunk_size;
        ASSERT(chunk_count > 0, "Chunk count must be bigger than 0");
        ASSERT(chunk_size > 0, "Chunk size must be bigger than 0");

        pool = root;
        PoolChunk* current_block = pool;

        for (usize i = chunk_size; i < total_size; i += chunk_size)
        {
            current_block->next = (PoolChunk*)((char*)root + i);
            current_block = current_block->next;
        }
        current_block->next = NULL;
    }

    void PoolAllocator::cleanup()
    {
        ALLOCATOR_FREE(root);
        root = nullptr;
        pool = nullptr;
    }

    void PoolAllocator::display_content()
    {
        // @TODO not really useful, must traverse all chunks
        log_empty("|---------------------\n");
        log_empty("|Pool allocator:\n");
        log_empty("|---------------------\n");
        log_empty("|  chunk_count: %zu\n", chunk_count);
        log_empty("|  chunk_size: %zu\n", chunk_size);
        log_empty("|  current_chunk: %p\n", pool);
        log_empty("|  root_chunk: %p\n", root);
        log_empty("|---------------------\n");
    }

    void* ArenaAllocator::allocate_aligned(usize size, u16 alignment)
    {
        const usize aligned_size = ALIGNMENT(size, alignment);

        ArenaRegion* region = NULL;
        bool found_region = false;
        for (usize i = this->_current_region_index; i < _regions.count; ++i) {
            region = &_regions.get(i);
            if (region->cursor + aligned_size <= region->capacity || region->data == NULL) {
                found_region = true;
                this->_current_region_index = i;
                break;
            }
        }
        if (!found_region) {
            _regions.push(ArenaRegion{});
            region = &_regions.last();
            this->_current_region_index = _regions.count - 1;
        }

        if (region->data == NULL) {
            const usize allocated_size = MAX(aligned_size, _region_size);
            region->cursor = 0;
            region->capacity = allocated_size;
            region->data = (char*)ALLOCATOR_MALLOC(allocated_size);
            ASSERT_DEBUG(region->data != nullptr);
        }
        void* new_ptr = (void*)(region->data + region->cursor);
        region->cursor += aligned_size;
        return new_ptr;
    }

    void* ArenaAllocator::allocate(usize size)
    {
        return ArenaAllocator::allocate_aligned(size, ALLOCATOR_DEFAULT_ALIGNMENT);
    }

    void* ArenaAllocator::reallocate(void* ptr, usize old_size, usize new_size)
    {
        void* new_ptr = allocate(new_size);
        if (ptr == nullptr) return new_ptr;

        memory_copy(new_ptr, new_size, ptr, old_size);
        return new_ptr;
    }

    void ArenaAllocator::reset()
    {
        for (auto& region : _regions) {
            region.cursor = 0;
        }
        _current_region_index = 0;
    }

    void ArenaAllocator::cleanup()
    {
        for (auto& region : _regions) {
            ALLOCATOR_FREE(region.data);
        }
        _regions.cleanup();
        _current_region_index = 0;
    }

    void ArenaAllocator::rewind(Snapshot* _snapshot)
    {
        if (_snapshot == nullptr) return;
        auto& snapshot = *static_cast<ArenaSnapshot*>(_snapshot);
        ASSERT(snapshot.size_of_allocated_snapshot == sizeof(ArenaSnapshot), "Wrong snapshot type");
        if (snapshot.region_index < _regions.count)
        {
            auto& region = _regions[snapshot.region_index];
            region.cursor = snapshot.index;
            _current_region_index = snapshot.region_index;
            for (usize i = snapshot.region_index + 1; i < _regions.count; ++i)
                _regions[i].cursor = 0;
        }
    }

    Snapshot* ArenaAllocator::snapshot()
    {
        if (_regions.count < 1) allocate_aligned(0, 0);

        const auto current_index = _regions[_current_region_index].cursor;
        ArenaSnapshot* snapshot = (ArenaSnapshot*) temp_allocate(sizeof(ArenaSnapshot));
        snapshot->size_of_allocated_snapshot = sizeof(ArenaSnapshot);
        snapshot->region_index = _current_region_index;
        snapshot->index = current_index;
        return snapshot;
    }

    void ArenaAllocator::display_content()
    {
        u32 iter = 0;
        log_empty("|---------------------\n");
        log_empty("|Arena allocator:\n");
        log_empty("|---------------------\n");
        for (auto& region : _regions) {
            log_empty("|-region %d\n", iter++);
            log_empty("|  capacity: %zu\n", region.capacity);
            log_empty("|  cursor: %zu\n", region.cursor);
            log_empty("|---------------------\n");
        }
    }
} // namespace Sl

#endif // !SL_IMPLEMENTATION
