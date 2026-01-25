#ifndef SL_HASHMAP_H
#define SL_HASHMAP_H

#include "Sl_Defines.hpp"
#include "Sl_Array.hpp"

//// 0 = Not equals, 1.. = Equals
typedef u64(*Hasher_fn) (u64 seed, const void* key, usize key_len);

namespace Sl
{
    u64 hasher_fn_default(usize seed, const void* key, usize key_len);

    struct HashMapOptions
    {
        // Capacity of hashmap is always power of 2
        u32 initial_size = 32;
        float max_load = 0.80f;
        float grow_factor = 2.0f;
        Hasher_fn hasher = hasher_fn_default;
        usize seed = 0;
        // Optional. If set: it will allocate memory from this allocator for Array table
        Allocator* allocator = nullptr;
    };

    template<typename K, typename V, bool cpp_compliant_and_slow = SL_ARRAY_CPP_COMPLIANT>
    class HashMap {
        struct Entry {
            K key;
            V value;
            bool occupied;

            Entry(K&& k, V&& v) : key(std::move(k)), value(std::move(v)), occupied(true) {}
        };
    public:
        Array<Entry, cpp_compliant_and_slow> table;
        size_t count;
        Hasher_fn hasher;
        usize seed;
        float max_load;
        float grow_factor;

        HashMap(HashMapOptions opt = {}) noexcept
        {
            const auto new_capacity = is_power_of_two(opt.initial_size) ? opt.initial_size : next_power_of_two(opt.initial_size);
            table.count = new_capacity;
            table.reserve(new_capacity);
            for (auto& slot : table) slot.occupied = false;

            count = 0;
            hasher = opt.hasher;
            seed = opt.seed;
            max_load = opt.max_load;
            grow_factor = opt.grow_factor;
            table._allocator = opt.allocator;
        }

        bool is_empty() const noexcept { return count == 0; }

        bool contains(const K& key) const noexcept { return get(key) != nullptr; }

        template<typename... Args>
        void insert(K&& key, Args&&... args) noexcept
        {
            if (++count > table.capacity * max_load) grow();

            const usize index = hash(key, table.capacity);
            for (usize i = index; i < table.capacity; ++i) {
                auto& slot = table[i];
                if (!slot.occupied) {
                    ::new (&slot.value) V(std::forward<Args>(args)...);
                    slot.key = key;
                    slot.occupied = true;
                    return;
                }
            }
            // Wrap if not found
            for (usize i = 0; i < index; ++i) {
                auto& slot = table[i];
                if (!slot.occupied) {
                    ::new (&slot.value) V(std::forward<Args>(args)...);
                    slot.key = key;
                    slot.occupied = true;
                    return;
                }
            }
            grow();
            insert(std::move(key), std::forward<Args>(args)...);
        }

        template<typename... Args>
        void insert(K& key, Args&&... args) noexcept
        {
            if (++count > table.capacity * max_load) grow();

            const usize index = hash(key, table.capacity);
            for (usize i = index; i < table.capacity; ++i) {
                auto& slot = table[i];
                if (!slot.occupied) {
                    ::new (&slot.value) V(std::forward<Args>(args)...);
                    slot.key = key;
                    slot.occupied = true;
                    return;
                }
            }
            // Wrap if not found
            for (usize i = 0; i < index; ++i) {
                auto& slot = table[i];
                if (!slot.occupied) {
                    ::new (&slot.value) V(std::forward<Args>(args)...);
                    slot.key = key;
                    slot.occupied = true;
                    return;
                }
            }
            grow();
            insert(std::move(key), std::forward<Args>(args)...);
        }

        V* operator[](const K& key) noexcept
        {
            return get(key);
        }

        V* get(const K& key) const noexcept
        {
            const usize index = hash(key, table.capacity);

            for (usize i = index; i < table.count; ++i) {
                auto& slot = table[i];

                if (!slot.occupied) return nullptr;
                if (slot.key == key) return &slot.value;
            }
            return nullptr;
        }

        bool remove(const K& key) noexcept
        {
            const usize index = hash(key, table.capacity);

            for (size_t i = index; i < table.capacity; ++i) {
                Entry& slot = table[i];

                if (!slot.occupied) return false;
                if (slot.key == key) {
                    slot.occupied = false;
                    --count;
                    return true;
                }
            }
            return false;
        }

        template<typename Function>
        void forEach(Function&& func) const noexcept {
            for (size_t i = 0; i < table.capacity; ++i) {
                Entry& slot = table[i];

                if (slot.occupied) func(slot.key, slot.value);
            }
        }

        inline usize hash(const K& key, usize capacity) const noexcept
        {
            return hasher(seed, &key, sizeof(key)) & (capacity - 1);
        }

        void grow() noexcept
        {
            u64 new_capacity = table.capacity * grow_factor;
            if (!is_power_of_two(new_capacity)) {
                new_capacity = next_power_of_two(new_capacity);
            }
            Array<Entry, cpp_compliant_and_slow> new_table;
        restart:
            new_table.count = new_capacity;
            new_table.reserve(new_capacity);
            for (auto& slot : new_table) slot.occupied = false;

            this->count = 0;
            for (usize i = 0; i < this->table.capacity; ++i) {
                auto& slot = this->table[i];
                if (slot.occupied) {
                    if (!insert_inner(new_table, std::move(slot.key), std::move(slot.value)))
                    {
                        new_table.count = 0;
                        new_capacity = next_power_of_two(new_capacity * grow_factor);
                        goto restart;
                    }
                    ++this->count;
                }
            }
            this->table.cleanup();
            table.data = new_table.data;
            table.count = new_table.count;
            table.capacity = new_table.capacity;
        }

        void clear() noexcept
        {
            for (usize i = this->table.capacity; i > 0; --i) {
                auto& slot = this->table[i - 1];
                if (slot.occupied) {
                    slot.occupied = false;
                    IF_CONSTEXPR (cpp_compliant_and_slow) {
                        slot.value.~V();
                    }
                }
            }
        }

        void cleanup() noexcept
        {
            clear();
            table.count = 0;
            table.cleanup();
        }
    private:
        static constexpr bool is_power_of_two(usize n) noexcept
        {
            return n > 0 && (n & (n - 1)) == 0;
        }

        static constexpr usize next_power_of_two(usize n) noexcept
        {
            if (n <= 1) return 2;
            n--;
            n |= n >> 1;
            n |= n >> 2;
            n |= n >> 4;
            n |= n >> 8;
            n |= n >> 16;
            n |= n >> 32;
            return n + 1;
        }
        bool insert_inner(Array<Entry, cpp_compliant_and_slow>& _table, K&& key, V&& value) noexcept
        {
            ++count;
            ASSERT_TRUE(count < _table.capacity);

            const usize index = hash(key, _table.capacity);
            for (usize i = index; i < _table.capacity; ++i) {
                Entry& slot = _table[i];

                if (!slot.occupied) {
                    slot.key = std::move(key);
                    slot.value = std::move(value);
                    slot.occupied = true;
                    return true;
                }
            }
            for (usize i = 0; i < index; ++i) {
                Entry& slot = _table[i];

                if (!slot.occupied) {
                    slot.key = std::move(key);
                    slot.value = std::move(value);
                    slot.occupied = true;
                    return true;
                }
            }
            return false;
        }
    };

} // namespace Sl
#endif // !SL_HASHMAP_H

#ifdef SL_IMPLEMENTATION
namespace Sl
{
    u64 hasher_fn_default(usize seed, const void* key, usize key_len)
    {
        const u64 prime = 1099511628211ull;
        u64 hash = 14695981039346656037ull;
        auto* bytes = (const u8*)key;

        hash ^= seed;
        hash *= prime;

        for (usize i = 0; i < key_len; ++i) {
            hash ^= (u64)bytes[i];
            hash *= prime;
        }

        return hash;
    }
} // namespace Sl
#endif
