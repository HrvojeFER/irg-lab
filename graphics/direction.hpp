#ifndef IRGLAB_DIRECTION_HPP
#define IRGLAB_DIRECTION_HPP


#include "pch.hpp"

#include "primitives.hpp"


// It would be great if this was an enum of some king,
// but I'm not sure what the semantics of it would have to be.

namespace irglab
{
	using direction = bool;

	constexpr direction counterclockwise = true;
	constexpr direction clockwise = false;

	constexpr direction above = true;
	constexpr direction below = false;
	
	constexpr direction right = true;
	constexpr direction left = false;

	constexpr direction bottom = true;
	constexpr direction top = false;

	constexpr direction direction_far = true;
	constexpr direction direction_near = false;
}

namespace irglab::two_dimensional
{
	[[nodiscard]] inline direction get_direction(
		const line& line,
		const point& point)
	{
		if (const auto relation = dot(point, line); relation != 0) return relation < 0;

		throw std::invalid_argument("Point is on the line.");
	}

	[[nodiscard]] inline direction get_direction(
		const point& first,
		const point& second,
		const point& third)
	{
		if (const auto relation = dot(third, get_connecting_line(first, second)); 
			relation != 0)
		{
			return relation < 0;
		}

		throw std::invalid_argument("Points are collinear.");
	}
}

namespace irglab::three_dimensional
{
	[[nodiscard]] inline direction get_direction(const plane& plane, const point& point) 
	{
		if (const auto relation = dot(point, plane); relation != 0) return relation < 0;

		throw std::invalid_argument("Point is on the plane.");
	}

	[[nodiscard]] inline direction get_direction(
		const point& first,
		const point& second,
		const point& third,
		const point& fourth)
	{
		if (const auto relation = dot(fourth, get_common_plane(first, second, third));
			relation != 0)
		{
			return relation < 0;
		}

		throw std::invalid_argument("Points are coplanar.");
	}
}

#endif
