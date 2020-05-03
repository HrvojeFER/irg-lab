#ifndef IRGLAB_THREE_DIMENSIONAL_PRIMITIVES_HPP
#define IRGLAB_THREE_DIMENSIONAL_PRIMITIVES_HPP

#include "pch.hpp"

namespace irglab::three_dimensional
{
	using cartesian_coordinates = glm::vec3;
	using homogeneous_coordinates = glm::vec4;

	using point = homogeneous_coordinates;
	using line = homogeneous_coordinates;
	using plane = homogeneous_coordinates;
	using plane_normal = glm::vec3;

	using transformation_matrix = glm::mat4;

	[[nodiscard]] constexpr homogeneous_coordinates to_homogeneous_coordinates(
		const cartesian_coordinates& cartesian_coordinates)
	{
		return
		{
			cartesian_coordinates.x,
			cartesian_coordinates.y,
			cartesian_coordinates.z,
			1.0f
		};
	}

	[[nodiscard]] constexpr cartesian_coordinates to_cartesian_coordinates(
		const homogeneous_coordinates& homogeneous_coordinates)
	{
		return
		{
			homogeneous_coordinates.x / homogeneous_coordinates.w,
			homogeneous_coordinates.y / homogeneous_coordinates.w,
			homogeneous_coordinates.z / homogeneous_coordinates.w
		};
	}

	constexpr void normalize(homogeneous_coordinates& homogeneous_coordinates)
	{
		homogeneous_coordinates.x = homogeneous_coordinates.x / homogeneous_coordinates.w;
		homogeneous_coordinates.y = homogeneous_coordinates.y / homogeneous_coordinates.w;
		homogeneous_coordinates.z = homogeneous_coordinates.z / homogeneous_coordinates.w;
		homogeneous_coordinates.w = 1.0f;
	}

	[[nodiscard]] inline plane get_common_plane(
		point first_point,
		point second_point,
		point third_point)
	{
		normalize(first_point);
		normalize(second_point);
		normalize(third_point);

		const plane_normal normal
		{
			(second_point.y - first_point.y) * (third_point.z - first_point.z) -
				(second_point.z - first_point.z) * (third_point.y - first_point.y),

			-(second_point.x - first_point.x) * (third_point.z - first_point.z) +
				(second_point.z - first_point.z) * (third_point.x - first_point.x),

			(second_point.x - first_point.x) * (third_point.y - first_point.y) -
				(second_point.y - first_point.y) * (third_point.x - first_point.x),
		};

		return
		{
			normal.x,
			normal.y,
			normal.z,
			-first_point.x * normal.x - first_point.y * normal.y - first_point.z * normal.z
		};
	}
}

#endif
