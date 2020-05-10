#ifndef IRGLAB_TWO_DIMENSIONAL_PRIMITIVES_HPP
#define IRGLAB_TWO_DIMENSIONAL_PRIMITIVES_HPP

#include "pch.hpp"

namespace irglab::two_dimensional
{
	using cartesian_coordinates = glm::vec2;
	using homogeneous_coordinates = glm::vec3;

	using point = homogeneous_coordinates;
	using line = homogeneous_coordinates;

	[[nodiscard]] constexpr homogeneous_coordinates to_homogeneous_coordinates(
		const cartesian_coordinates& cartesian_coordinates)
	{
		return { cartesian_coordinates.x, cartesian_coordinates.y, 1.0f };
	}

	[[nodiscard]] constexpr cartesian_coordinates to_cartesian_coordinates(
		const homogeneous_coordinates& homogeneous_coordinates)
	{
		return
		{
			homogeneous_coordinates.x / homogeneous_coordinates.z,
			homogeneous_coordinates.y / homogeneous_coordinates.z
		};
	}

	[[nodiscard]] constexpr void normalize(homogeneous_coordinates& homogeneous_coordinates)
	{
		homogeneous_coordinates.x = homogeneous_coordinates.x / homogeneous_coordinates.z;
		homogeneous_coordinates.y = homogeneous_coordinates.y / homogeneous_coordinates.z;
		homogeneous_coordinates.z = 1.0f;
	}

	[[nodiscard]] constexpr line get_line_at_y(const float y_coordinate) noexcept
	{
		return { 0, 1, -y_coordinate };
	}

	[[nodiscard]] inline point get_intersection(
		const line& line_a,
		const line& line_b)
	{
		return cross(line_a, line_b);
	}

	[[nodiscard]] inline line get_connecting_line(
		const point& first,
		const point& second)
	{
		return cross(first, second);
	}

	[[nodiscard]] inline std::vector<line> get_successive_point_lines(
		std::vector<point> points)
	{
		std::vector<line> result{ points.size() };

		for (size_t index = 0; index < points.size(); ++index)
		{
			result[index] = get_connecting_line(
				points[index],
				points[(index + 1) % points.size()]);
		}

		return result;
	}

	[[nodiscard]] inline std::vector<point> get_intersections(
		const std::vector<line>& lines,
		const line& line)
	{
		std::vector<point> result{ lines.size() };

		std::transform(lines.begin(), lines.end(), result.begin(),
			[line](const two_dimensional::line& other)
			{
				return get_intersection(line, other);
			});

		return result;
	}
}


#endif
