#ifndef SL_RELATIVEPTR_H
#define SL_RELATIVEPTR_H

#include "Sl_Defines.hpp"

namespace Sl
{
    // T must be signed numeric type, since you want to know offset from left and right
	template <typename T = int32_t>
	struct RelativePtr
	{
		T relative_index = 0;
		RelativePtr(T relative_index)
		    : relative_index(relative_index)
		{}

		static RelativePtr<T> create(const void* offset_ptr, const void* ptr) noexcept
		{
			const ptrdiff_t offset = ((ptrdiff_t)ptr) - ((ptrdiff_t)offset_ptr);
			const s64 max_offset = (1llu << (sizeof(T) * 8));
			const s64 min_offset = -max_offset;
			ASSERT(offset <= max_offset && offset >= min_offset,
			    "Offset exceded max size of relative_index type, change type to a bigger size");
           	RelativePtr<T> rel_ptr(offset ^ (1 << (sizeof(T) * 8 - 1)));
			return rel_ptr;
		}

		void* to_normal(const void* offset_ptr) noexcept
		{
			return (void*) (((ptrdiff_t) offset_ptr + (ptrdiff_t)relative_index) ^ (1 << (sizeof(T) * 8 - 1)));
		}
	};
} // namespace Sl

#endif // SL_RELATIVEPTR_H
