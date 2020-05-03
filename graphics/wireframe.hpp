#ifndef IRGLAB_WIREFRAME_HPP
#define IRGLAB_WIREFRAME_HPP


#include "pch.hpp"

#include "primitives.hpp"
#include "bounds.hpp"
#include "transformations.hpp"

#include "wire.hpp"


namespace irglab
{
	template<size DimensionCount>
	struct wireframe
	{
		using wire = wire<DimensionCount>;
		std::vector<wire> wires;

		constexpr void normalize()
		{
			for (auto& wire : wires) wire.normalize();
		}
		
		constexpr explicit wireframe(const std::initializer_list<wire>& points) noexcept :
			wires{ points } { }

		template<typename Iterator>
		wireframe(const Iterator first, const Iterator last) :
			wires{ std::distance(first, last) }
		{
			for (auto i = first; i != last; ++i) wires.emplace_back(wire{ *i, *(i + 1) });
			wires.emplace_back(wire{ *last, *first });
		}
		
		void remove_duplicate_wires()
		{
			for (size i = 0; i < wires.size(); ++i)
				for (auto j = i + 1; j < wires.size(); ++j)
					if (wires[i] == wires[j])
					{
						wires.erase(wires.begin() + j);
						--j;
					}
		}

		[[nodiscard]] std::vector<point<DimensionCount>> get_points() const
		{
			std::vector<point<DimensionCount>> points{ };

			for (const auto& wire : wires)
				points.emplace_back(wire.begin),
				points.emplace_back(wire.end);

			return points;
		}
		
		
		constexpr void operator*=(const transformation<DimensionCount>& transformation) noexcept
		{
			for (auto& [start, end] : wires) start *= transformation, end *= transformation;
		}

		
		void operator+=(const wire& wire)
		{
			wires.emplace_back(wire);
		}

		void operator+=(const wireframe& other)
		{
			this->wires.insert(this->wires.end(), other.wires.begin(), other.wires.end());
		}

		
		friend constexpr  void operator|=(
			bounds<DimensionCount>& bounds, const wireframe& wireframe) noexcept
		{
			for (const auto& wire : wireframe.wires) bounds |= wire;
		}

		
		friend std::ostream& operator<<(std::ostream& output_stream,
			const wireframe<DimensionCount>& wireframe)
		{
			output_stream << "Wires:" << std::endl;

			for (const auto& wire : wireframe.wires) output_stream << wire;

			return output_stream;
		}
	};
}

namespace irglab::two_dimensional
{
	using wireframe = irglab::wireframe<dimension_count>;
}

namespace irglab::three_dimensional
{
	using wireframe = irglab::wireframe<dimension_count>;
}


#endif
