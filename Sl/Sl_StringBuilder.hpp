#ifndef SL_STRINGBUILDER_H
#define SL_STRINGBUILDER_H

#include "Sl_Defines.hpp"
#include "Sl_Array.hpp"

#define SB_FORMAT "%.*s"
#define SB_ARG(sv) (int)((sv).count), (sv).data

namespace Sl
{
    struct StrView;
    struct StrBuilder : Array<char, false>
    {
        StrBuilder(Allocator* allocator = nullptr)
        {
            current_allocator(allocator);
        }

        void append(char ch) noexcept;
        void append(const char* str, usize size) noexcept;
        void append(StrView str) noexcept;
        void append_escaped(StrView command, bool force_escape = false); // Usefull for paths
        void append_null(bool update_count = true) noexcept;
        bool appendf(const char* format, ...) noexcept SL_PRINTF_FORMATER(2, 3);
        void reset() noexcept;
        void align(u16 alignment) noexcept;
        void cleanup() noexcept;
        void current_allocator(Allocator* allocator) noexcept;
        Allocator* current_allocator() const noexcept;
        char* to_cstring_alloc() const noexcept;
        StrView to_string_view(bool is_null_terminated = false, bool is_wide = false) const noexcept;
    };
} // namespace Sl
#endif // !SL_STRINGBUILDER_H

#if defined(SL_IMPLEMENTATION)
namespace Sl
{
    void StrBuilder::append(const char* str, usize size) noexcept
    {
        if (size > 0 && str) {
            push_many(str, size);
        }
    }

    void StrBuilder::append(char ch) noexcept
    {
        append(&ch, sizeof(ch));
    }

    void StrBuilder::append_null(bool update_count) noexcept
    {
        append("\0", 1);
        if (!update_count) count -= 1;
    }

    void StrBuilder::append(StrView str) noexcept
    {
        if (str.data) {
            append(str.data, str.size);
        }
    }

    void StrBuilder::append_escaped(StrView command, bool force_escape)
    {
        if (command.size == 0) return;
        if (!force_escape && command.find_first_of_chars(" \t\n\v\"") == StrView::INVALID_INDEX) {
            // no need to quote
            append(command.data, command.size);
        } else {
            // we need to escape:
            // 1. double quotes in the original arg
            // 2. consequent backslashes before a double quote
            usize backslashes = 0;
            append("\"", 1);
            for (usize i = 0; i < command.size; ++i) {
                char x = command.data[i];
                if (x == '\\') {
                    backslashes += 1;
                } else {
                    if (x == '\"') {
                        // escape backslashes (if any) and the double quote
                        for (usize k = 0; k < 1+backslashes; ++k) {
                            append('\\');
                        }
                    }
                    backslashes = 0;
                }
                append(x);
            }
            // escape backslashes (if any)
            for (usize i = 0; i < backslashes; ++i) {
                append('\\');
            }
            append('"');
        }
    }

    void StrBuilder::align(u16 alignment) noexcept
    {
        usize aligned_size = ALIGNMENT(count, alignment) - count;
        for (; aligned_size > 0; --aligned_size) {
            append_null();
        }
    }

    void StrBuilder::reset() noexcept
    {
        Array::clear();
    }

    void StrBuilder::cleanup() noexcept
    {
        Array::cleanup();
    }

    bool StrBuilder::appendf(const char* format, ...) noexcept
    {
        ASSERT_DEBUG(format);
        if (!format) return false;
        bool result = false;

        va_list args;
        va_start(args, format);
        if (_allocator) {
            auto size = vsnprintf(nullptr, 0, format, args);
            if (size > 0) {
                auto* str = (char*)_allocator->allocate(size);
                append(str, static_cast<usize>(size));
                result = true;
            }
        } else {
            char buf[4096];
            auto size = vsnprintf(buf, sizeof(buf), format, args);
            if (size > 0) {
                append(buf, static_cast<usize>(size));
                result = true;
            }
        }
        va_end(args);
        return result;
    }

    Allocator* StrBuilder::current_allocator() const noexcept
    {
        return _allocator;
    }

    void StrBuilder::current_allocator(Allocator* allocator) noexcept
    {
        _allocator = allocator;
    }

    char* StrBuilder::to_cstring_alloc() const noexcept
    {
        usize size = count * sizeof(char);
        char* str;
        if (_allocator) str = (char*)_allocator->allocate(size + 1);
        else return nullptr;
        ASSERT_DEBUG(str);

        if (this->data && size > 0)
            memory_copy(str, size, this->data, size);
        str[size] = '\0';
        return str;
    }

    Sl::StrView StrBuilder::to_string_view(bool is_null_terminated, bool is_wide) const noexcept
    {
        return Sl::StrView(data, count * sizeof(char), is_null_terminated, is_wide);
    }
} // namespace Sl
#endif // !SL_IMPLEMENTATION
