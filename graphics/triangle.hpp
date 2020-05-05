#ifndef IRGLAB_THREE_DIMENSIONAL_TRIANGLE_HPP
#define IRGLAB_THREE_DIMENSIONAL_TRIANGLE_HPP


#include "pch.hpp"

#include "primitives.hpp"
#include "transformations.hpp"
#include "direction.hpp"
#include "bounds.hpp"

#include "wire.hpp"
#include "wireframe.hpp"


namespace irglab
{
	template<size DimensionCount>
	struct triangle_internal
	{
		using vertex = point<DimensionCount>;
		vertex first, second, third;

		
		constexpr void normalize()
		{
			irglab::normalize<DimensionCount>(first),
			irglab::normalize<DimensionCount>(second),
			irglab::normalize<DimensionCount>(third);
		}

		// Not using std::swap because it is not constexpr and noexcept.
		constexpr void operator~() noexcept
		{
			const auto temp = first;
			first = third;
			third = temp;
		}

		constexpr void operator*=(const transformation<DimensionCount>& transformation) noexcept
		{
			first = first * transformation,
			second = second * transformation,
			third = third * transformation;
		}


		friend constexpr void operator|=(
			bounds<DimensionCount>& bounds, const triangle_internal& triangle) noexcept
		{
			bounds |= triangle.first, bounds |= triangle.second, bounds |= triangle.third;
		}


		friend void operator+=(
			wireframe<DimensionCount>& wireframe, const triangle_internal& triangle)
		{
			wireframe += wire<DimensionCount>{ triangle.first, triangle.second },
			wireframe += wire<DimensionCount>{ triangle.second, triangle.third },
			wireframe += wire<DimensionCount>{ triangle.third, triangle.first };
		}


		using barycentric_coordinates = vector<DimensionCount>;

		[[nodiscard]] barycentric_coordinates get_barycentric_coordinates(
			const point<DimensionCount>& point) const
		{
			return point * inverse(
				glm::mat<DimensionCount + 1, DimensionCount + 1, number, precision>
			{
				first, second, third
			});
		}


		friend std::ostream& operator<<(std::ostream& output_stream, const triangle_internal& triangle)
		{
			return output_stream <<
				"Vertices:" << std::endl <<
				glm::to_string(triangle.first) << std::endl <<
				glm::to_string(triangle.second) << std::endl <<
				glm::to_string(triangle.third) << std::endl;
		}

		
	protected:
		constexpr explicit triangle_internal(
			const vertex& first, const vertex& second, const vertex& third) noexcept :
			first{ first }, second{ second }, third{ third } { }
	};


	template<size DimensionCount>
	struct triangle : triangle_internal<DimensionCount> { triangle() = delete; };

	
	template<>
	struct triangle<2> final : triangle_internal<2>
	{
		constexpr explicit triangle(
			const vertex& first, const vertex& second, const vertex& third) noexcept :
			triangle_internal<2>{ first, second, third } { }


		[[nodiscard]] direction get_direction() const
		{
			return two_dimensional::get_direction(first, second, third);
		}

		void operator%=(const direction direction)
		{
			if (this->get_direction() != direction) ~*this;
		}

		[[nodiscard]] friend bool operator<(
			const two_dimensional::point& point, const triangle& triangle)
		{
			const auto triangle_direction = triangle.get_direction();

			return
				two_dimensional::get_direction(
					triangle.first, triangle.second, point
				) == triangle_direction &&

				two_dimensional::get_direction(
					triangle.second, triangle.third, point
				) == triangle_direction &&

				two_dimensional::get_direction(
					triangle.third, triangle.first, point
				) == triangle_direction;
		}
	};

	
	template<>
	struct triangle<3> final : triangle_internal<3>
	{
		constexpr explicit triangle(
			const vertex& first, const vertex& second, const vertex& third) noexcept :
			triangle_internal<3>{ first, second, third } { }


		[[nodiscard]] three_dimensional::plane get_plane() const
		{
			return three_dimensional::get_common_plane(
				three_dimensional::to_cartesian_coordinates(this->first), 
				three_dimensional::to_cartesian_coordinates(this->second), 
				three_dimensional::to_cartesian_coordinates(this->third));
		}

		[[nodiscard]] three_dimensional::plane_normal get_plane_normal() const
		{
			return three_dimensional::get_plane_normal(
				three_dimensional::to_cartesian_coordinates(this->first),
				three_dimensional::to_cartesian_coordinates(this->second),
				three_dimensional::to_cartesian_coordinates(this->third));
		}

		[[nodiscard]] three_dimensional::cartesian_coordinates get_center() const
		{
			return three_dimensional::to_cartesian_coordinates(this->first) +
				three_dimensional::to_cartesian_coordinates(this->second) +
				three_dimensional::to_cartesian_coordinates(this->third) / 3.0f;
		}
		
		[[nodiscard]] friend bool operator<(
			const three_dimensional::point& point, const triangle& triangle)
		{
			return three_dimensional::get_direction(point, triangle.get_plane());
		}
	};
}

namespace irglab::two_dimensional
{
	using triangle = irglab::triangle<dimension_count>;
}

namespace irglab::three_dimensional
{
	using triangle = irglab::triangle<dimension_count>;
}

#endif
