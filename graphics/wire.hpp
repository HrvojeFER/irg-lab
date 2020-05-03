#ifndef IRGLAB_WIRE_HPP
#define IRGLAB_WIRE_HPP


#include "pch.hpp"

#include "primitives.hpp"
#include "bounds.hpp"
#include "transformations.hpp"


namespace irglab
{
	template<size DimensionCount>
	struct wire
	{
		using point = point<DimensionCount>;
		point begin, end;

		
		constexpr explicit wire(const point& begin, const point& end) noexcept :
			begin{ begin }, end{ end } { }

		
		constexpr void normalize()
		{
			irglab::normalize<DimensionCount>(begin), irglab::normalize<DimensionCount>(end);
		}

		
		// Not using std::swap because it is not constexpr and noexcept.
		constexpr void operator~() noexcept
		{
			const auto temp = begin;
			begin = end;
			end = temp;
		}

		constexpr bool operator==(const wire& other) noexcept
		{
			return
				(this->begin == other.begin && this->end == other.end) ||
				(this->begin == other.end && this->end == other.begin);
		}

		friend constexpr void operator|=(bounds<DimensionCount>& bounds, const wire& wire) noexcept
		{
			bounds |= wire.begin, bounds |= wire.end;
		}

		constexpr void operator*=(const transformation<DimensionCount>& transformation) noexcept
		{
			begin = transformation * begin, end = transformation * end;
		}

		
		friend std::ostream& operator<<(std::ostream& output_stream, const wire& wire)
		{
			return output_stream << 
				glm::to_string(wire.begin) << ", " << 
				glm::to_string(wire.end) << std::endl;
		}
	};
}

namespace irglab::two_dimensional
{
	using wire = irglab::wire<dimension_count>;
}

namespace irglab::three_dimensional
{
	using wire = irglab::wire<dimension_count>;
}

#endif
