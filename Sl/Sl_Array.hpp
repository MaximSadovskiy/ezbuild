#ifndef SL_ARRAY_H
#define SL_ARRAY_H

#include <memory>
#include "Sl_Defines.hpp"

namespace Sl
{
    struct Allocator;

    template <typename T, bool cpp_compliant_and_slow = ARRAY_CPP_COMPLIANT>
    struct Array
    {
        T* data;
        usize capacity;
        usize count;
        Allocator* _allocator; // Optional, if set: it will allocate memory from this allocator istead of ARRAY_REALLOC

        static const usize INVALID_INDEX = -1;

        Array(Allocator* allocator = nullptr)
        {
            this->_allocator = allocator;
            this->data = nullptr;
            this->capacity = 0;
            this->count = 0;
        }

        template<typename... Args>
        void push(Args&&... args) noexcept
        {
            resize(count + 1);
            ::new (data + count++) T(std::forward<Args>(args)...);
        }

        void push_many(const T* new_data, usize new_count) noexcept
        {
            ASSERT(new_data != nullptr, "Cannot push null array");
            if (new_data == nullptr) return;

            resize(this->count + new_count);
            for (usize i = 0; i < new_count; ++i)
                data[this->count++] = new_data[i];
        }

        void remove_unordered(usize index) noexcept
        {
            ASSERT(count > 0 && index <= count, "Index out of range");
            if (count == 0 || index >= count) return;

            IF_CONSTEXPR (cpp_compliant_and_slow) {
                // I don't know what's std::vector devs were smoking
                if (index == count - 1) {
                    std::swap(data[index], data[--count]);
                    data[count].~T();
                }
                else
                    data[index] = std::move(data[--count]);
            }
            else
                data[index] = std::move(data[--count]);
        }

        T& get(usize index) noexcept
        {
            ASSERT(count > index, "Index out of range");
            return data[index];
        }

        T& get_unsafe(usize index) noexcept
        {
            return data[index];
        }

        template<typename Function>
        void forEach(Function&& func) noexcept {
            for (usize i = 0; i < count; ++i) {
                func(data[i]);
            }
        }

        template<typename Function>
        void forEachIndexed(Function&& func) noexcept {
            for (usize i = 0; i < count; ++i) {
                func(i, data[i]);
            }
        }

        T& first() noexcept { return get(0); }
        T& last() noexcept { return get(count - 1); }
        bool is_empty() noexcept { return count < 1; }

        usize find_first(T val) const
        {
            for (usize i = 0; i < count; ++i) {
                if (data[i] == val) return i;
            }
            return INVALID_INDEX;
        }
        usize find_last(T val) const
        {
            usize index = INVALID_INDEX;
            for (usize i = 0; i < count; ++i) {
                if (data[i] == val) index = i;
            }
            return index;
        }

        bool contains(T val) const { return find_first(val) != INVALID_INDEX; }
        T& operator[](usize index) noexcept { return get(index); }
        T* begin() noexcept { return data; }
        T* end() noexcept { return data + count; }

        void pop() noexcept
        {
            ASSERT(count > 0, "Cannot pop from empty array");
            if (count < 1) return;

            IF_CONSTEXPR (cpp_compliant_and_slow) {
                data[count - 1].~T();
            }
            count -= 1;
        }

        // Does not shrink, if needed: use reserve()
        void resize(usize needed_capacity) noexcept
        {
            if (needed_capacity > capacity)
            {
                auto old_capacity = capacity;
                if (capacity == 0) capacity = 32;
                while (capacity < needed_capacity) capacity *= 2;

                if (_allocator)
                    data = (T*)_allocator->reallocate(data, old_capacity * ALIGNMENT(sizeof(T), alignof(T)), capacity * ALIGNMENT(sizeof(T), alignof(T)));
                else
                    data = (T*)ARRAY_REALLOC(data, capacity * ALIGNMENT(sizeof(T), alignof(T)));
                ASSERT_DEBUG(data != nullptr);
            }
        }

        void reserve(usize needed_capacity) noexcept
        {
            if (needed_capacity > capacity) {
                if (_allocator)
                    data = (T*)_allocator->reallocate(data, capacity * ALIGNMENT(sizeof(T), alignof(T)), needed_capacity * ALIGNMENT(sizeof(T), alignof(T)));
                else
                    data = (T*)ARRAY_REALLOC(data, needed_capacity * ALIGNMENT(sizeof(T), alignof(T)));
                capacity = needed_capacity;
                ASSERT_DEBUG(data != nullptr);
                IF_CONSTEXPR (cpp_compliant_and_slow) {
                    for (usize i = count; i < capacity; ++i)
                        ::new (data + i) T(static_cast<T&&>(data[i]));
                }
            } else {
                const auto needed_capacity_bytes = needed_capacity * ALIGNMENT(sizeof(T), alignof(T));
                T* new_data;
                if (_allocator)
                    new_data = (T*)_allocator->allocate(needed_capacity_bytes);
                else
                    new_data = (T*)ARRAY_REALLOC(nullptr, needed_capacity_bytes);
                ASSERT_DEBUG(new_data != nullptr);
                const auto new_count = needed_capacity < count ? needed_capacity : count;
                IF_CONSTEXPR (cpp_compliant_and_slow) {
                   for (usize i = 0; i < new_count; ++i)
                       new_data[i] = std::move(data[i]);
                } else {
                    void memory_copy(void* dest, usize dest_size, const void* src, usize src_size) noexcept;
                    memory_copy((void*)new_data, needed_capacity_bytes, (void*)data, needed_capacity_bytes);
                }
                if (!_allocator) ARRAY_FREE(data);
                data = new_data;
                capacity = needed_capacity;
                count = new_count;
            }
        }

        void memzero() noexcept
        {
            if (data) memory_zero(data, capacity * ALIGNMENT(sizeof(T), alignof(T)));
        }

        void clear() noexcept
        {
            IF_CONSTEXPR (cpp_compliant_and_slow) {
                for (usize i = 0; i < count; ++i) {
                    data[i].~T();
                }
            }
            count = 0;
        }

        void cleanup() noexcept
        {
            clear();
            if (!_allocator) ARRAY_FREE(data);
            data = nullptr;
            capacity = 0;
        }

        Array& operator=(Array& arr) noexcept
        {
            if ((void*) this != (void*) &arr) {
                cleanup();
                this->data = arr.data;
                this->count = arr.count;
                this->capacity = arr.capacity;
                arr.data = nullptr;
                arr.count = 0;
                arr.capacity = 0;
            }
            return *this;
        }

        Array& operator=(Array&& arr) noexcept
        {
            return *this = arr;
        }

        Array& operator=(Array<T> arr) = delete;
        Array& operator=(const Array<T>&& arr) = delete;
        Array& operator=(const Array& arr) = delete;
    // ----------------------------------------------
    // Const functions
    // ----------------------------------------------
        T& get(usize index) const noexcept
        {
            ASSERT(count > index, "Index out of range");
            return data[index];
        }
        T& get_unsafe(usize index) const noexcept
        {
            return data[index];
        }

        T& operator[](usize index) const noexcept { return get(index); }
        const T* begin() const noexcept { return data; }
        const T* end() const noexcept { return data + count; }

        T& first() const noexcept { return get(0); }
        T& last() const noexcept { return get(count - 1); }

        template<typename Function>
        void forEach(Function&& func) const noexcept {
            for (usize i = 0; i < count; ++i) {
                func(data[i]);
            }
        }

        template<typename Function>
        void forEachIndexed(Function&& func) const noexcept {
            for (usize i = 0; i < count; ++i) {
                func(i, data[i]);
            }
        }
    };


    // * LocalArray: Stack array for trivial types (no init).
    // * Uses LOCAL_ARRAY_INITIAL_SIZE stack storage initially.
    // * This allows to speed it up on small scale sizes.
    // * Allocates heap/custom allocator when exceeded.
    static_assert((LOCAL_ARRAY_INITIAL_SIZE) > 0ll, "Provide correct size");

    template<typename T>
    class LocalArray
    {
    private:
        T* _data;
        T _storage[LOCAL_ARRAY_INITIAL_SIZE];
        usize _count;
        usize _allocated_capacity;
        bool _is_heap_allocated; // True if allocated with ARRAY_REALLOC or Allocator
    public:
        LocalArray()
        {
            _data = (T*) &_storage;
            _count = 0;
            _allocated_capacity = 0;
            _is_heap_allocated = false;
        }

        inline T* data() { return _data; }
        inline usize count() const { return _count; }
        inline usize allocated_capacity() const { return _allocated_capacity; }
        inline bool is_heap_allocated() const { return _is_heap_allocated; }

        template<typename... Args>
        void push(Args&&... args) noexcept
        {
            if (_count + 1 > LOCAL_ARRAY_INITIAL_SIZE && !is_heap_allocated()) {
                 _is_heap_allocated = true;
                _allocated_capacity = LOCAL_ARRAY_INITIAL_SIZE * 2;
                auto* new_data = (T*)ARRAY_REALLOC(nullptr, size_of_t() * _allocated_capacity);
                memory_copy(new_data, size_of_t() * LOCAL_ARRAY_INITIAL_SIZE, _data, size_of_t() * LOCAL_ARRAY_INITIAL_SIZE);
                _data = new_data;
            } else if (_count + 1 > _allocated_capacity && is_heap_allocated()) {
                resize(_count + 1);
            }
            ::new (_data + _count++) T(std::forward<Args>(args)...);
        }

        T& get(usize index) noexcept
        {
            ASSERT(_count > index, "Index out of range");
            return _data[index];
        }

        T& get_unsafe(usize index) noexcept
        {
            return _data[index];
        }
        T& operator[](usize index) noexcept { return get(index); }

        bool is_empty() noexcept { return _count < 1; }
        T& first() noexcept { return get(0); }
        T& last() noexcept { return get(_count - 1); }

        // Does not shrink
        void resize(usize needed_capacity) noexcept
        {
            if (needed_capacity > _allocated_capacity)
            {
                if (_allocated_capacity == 0) _allocated_capacity = 32;
                while (_allocated_capacity < needed_capacity) _allocated_capacity *= 2;

                _data = (T*)ARRAY_REALLOC(_data, _allocated_capacity * size_of_t());
                ASSERT_DEBUG(_data != nullptr);
            }
        }

        void pop() noexcept
        {
            ASSERT(_count > 0, "Cannot pop from empty array");
            if (_count < 1) return;
            --_count;
        }

        void remove_unordered(usize index) noexcept
        {
            ASSERT(_count > 0 && index <= _count, "Index out of range");
            if (_count == 0 || index >= _count) return;

            _data[index] = std::move(_data[--_count]);
        }

        template<typename Function>
        void forEach(Function&& func) noexcept {
            for (usize i = 0; i < _count; ++i) {
                func(_data[i]);
            }
        }

        template<typename Function>
        void forEachIndexed(Function&& func) noexcept {
            for (usize i = 0; i < _count; ++i) {
                func(i, _data[i]);
            }
        }

        void clear() noexcept
        {
            _count = 0;
        }

        void cleanup() noexcept
        {
            clear();
            if (is_heap_allocated()) {
                ARRAY_FREE(_data);
                _data = &_storage;
                _is_heap_allocated = false;
            }
        }

        void memzero() noexcept
        {
            ASSERT(_data != nullptr, "Data cannot be null, since it must point to _storage or allocated block");
            usize size = size_of_t();
            if (is_heap_allocated())
                size *= _allocated_capacity;
            else
                size *= LOCAL_ARRAY_INITIAL_SIZE;
            memory_zero(_data, size);
        }
    // ----------------------------------------------
    // Const functions
    // ----------------------------------------------
        T& get(usize index) const noexcept
        {
            ASSERT(_count > index, "Index out of range");
            return _data[index];
        }
        T& get_unsafe(usize index) const noexcept
        {
            return _data[index];
        }

        T& operator[](usize index) const noexcept { return get(index); }
        const T* begin() const noexcept { return _data; }
        const T* end() const noexcept { return _data + _count; }

        T& first() const noexcept { return get(0); }
        T& last() const noexcept { return get(_count - 1); }

        template<typename Function>
        void forEach(Function&& func) const noexcept {
            for (usize i = 0; i < _count; ++i) {
                func(_data[i]);
            }
        }

        template<typename Function>
        void forEachIndexed(Function&& func) const noexcept {
            for (usize i = 0; i < _count; ++i) {
                func(i, _data[i]);
            }
        }
        usize size_of_t() const noexcept { return ALIGNMENT(sizeof(T), alignof(T)); }
    };
} // namescape Sl
#endif // SL_ARRAY_H
