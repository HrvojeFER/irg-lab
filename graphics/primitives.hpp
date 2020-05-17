#ifndef IRGLAB_PRIMITIVES_HPP
#define IRGLAB_PRIMITIVES_HPP

#include "pch.hpp"

namespace irglab
{
	inline const glm::qualifier precision = glm::defaultp;
	using size = size_t;

	using number = float;
	inline const number number_max = FLT_MAX;
	inline const number number_min = -FLT_MAX;

	using angle = number;

	template<size Size>
	using vector = glm::vec<Size, number, precision>;

	template<size RowCount, size ColumnCount>
	using matrix = glm::mat<RowCount, ColumnCount, number, precision>;

	
	template<size DimensionCount>
	using cartesian_coordinates = vector<DimensionCount>;

	template<size DimensionCount>
	using homogeneous_coordinates = vector<DimensionCount + 1>;

	
	template<size DimensionCount>
	using point = homogeneous_coordinates<DimensionCount>;

	template<size DimensionCount>
	using line = homogeneous_coordinates<DimensionCount>;

	
	template<size DimensionCount>
	// ReSharper disable once CppFunctionIsNotImplemented
	[[nodiscard]] constexpr homogeneous_coordinates<DimensionCount> to_homogeneous_coordinates(
		const cartesian_coordinates<DimensionCount>& cartesian_coordinates) noexcept;

	template<size DimensionCount>
	// ReSharper disable once CppFunctionIsNotImplemented
	[[nodiscard]] constexpr cartesian_coordinates<DimensionCount> to_cartesian_coordinates(
		const homogeneous_coordinates<DimensionCount>& homogeneous_coordinates);

	template<size DimensionCount>
	// ReSharper disable once CppFunctionIsNotImplemented
	constexpr void normalize(homogeneous_coordinates<DimensionCount>& homogeneous_coordinates);

	
	template<>
	[[nodiscard]] inline homogeneous_coordinates<2> to_homogeneous_coordinates<2>(
		const cartesian_coordinates<2>& cartesian_coordinates) noexcept
	{
		return { cartesian_coordinates.x, cartesian_coordinates.y, 1.0f };
	}
	template<>
	[[nodiscard]] inline cartesian_coordinates<2> to_cartesian_coordinates<2>(
		const homogeneous_coordinates<2>& homogeneous_coordinates)
	{
		return
		{
			homogeneous_coordinates.x / homogeneous_coordinates.z,
			homogeneous_coordinates.y / homogeneous_coordinates.z
		};
	}

	template<>
	constexpr void normalize<2>(homogeneous_coordinates<2>& homogeneous_coordinates)
	{
		homogeneous_coordinates.x = homogeneous_coordinates.x / homogeneous_coordinates.z;
		homogeneous_coordinates.y = homogeneous_coordinates.y / homogeneous_coordinates.z;
		homogeneous_coordinates.z = 1.0f;
	}


	template<>
	[[nodiscard]] inline homogeneous_coordinates<3> to_homogeneous_coordinates<3>(
		const cartesian_coordinates<3>& cartesian_coordinates) noexcept
	{
		return
		{
			cartesian_coordinates.x,
			cartesian_coordinates.y,
			cartesian_coordinates.z,
			1.0f
		};
	}

	template<>
	[[nodiscard]] inline cartesian_coordinates<3> to_cartesian_coordinates<3>(
		const homogeneous_coordinates<3>& homogeneous_coordinates)
	{
		return
		{
			homogeneous_coordinates.x / homogeneous_coordinates.w,
			homogeneous_coordinates.y / homogeneous_coordinates.w,
			homogeneous_coordinates.z / homogeneous_coordinates.w
		};
	}

	template<>
	constexpr void normalize<3>(homogeneous_coordinates<3>& homogeneous_coordinates)
	{
		homogeneous_coordinates.x = homogeneous_coordinates.x / homogeneous_coordinates.w;
		homogeneous_coordinates.y = homogeneous_coordinates.y / homogeneous_coordinates.w;
		homogeneous_coordinates.z = homogeneous_coordinates.z / homogeneous_coordinates.w;
		homogeneous_coordinates.w = 1.0f;
	}
}


namespace irglab::two_dimensional
{
	inline const size dimension_count = 2;

	
	using cartesian_coordinates = irglab::cartesian_coordinates<dimension_count>;
	using homogeneous_coordinates = irglab::homogeneous_coordinates<dimension_count>;

	using point = irglab::point<dimension_count>;
	using line = irglab::line<dimension_count>;

	
	[[nodiscard]] inline homogeneous_coordinates to_homogeneous_coordinates(
		const cartesian_coordinates& cartesian_coordinates) noexcept
	{
		return irglab::to_homogeneous_coordinates<dimension_count>(cartesian_coordinates);
	}

	[[nodiscard]] inline cartesian_coordinates to_cartesian_coordinates(
		const homogeneous_coordinates& homogeneous_coordinates)
	{
		return irglab::to_cartesian_coordinates<dimension_count>(homogeneous_coordinates);
	}

	constexpr void normalize(homogeneous_coordinates& homogeneous_coordinates)
	{
		irglab::normalize<dimension_count>(homogeneous_coordinates);
	}

	
	[[nodiscard]] inline line get_line_at_y(const number y_coordinate) noexcept
	{
		return { 0, 1, -y_coordinate };
	}

	[[nodiscard]] inline point get_intersection(const line& line_a, const line& line_b) noexcept
	{
		return cross(line_a, line_b);
	}

	[[nodiscard]] inline line get_connecting_line(const point& first, const point& second) noexcept
	{
		return cross(first, second);
	}
}

namespace irglab::three_dimensional
{
	inline const size dimension_count = 3;

	
	using cartesian_coordinates = irglab::cartesian_coordinates<dimension_count>;
	using homogeneous_coordinates = irglab::homogeneous_coordinates<dimension_count>;

	using point = irglab::point<dimension_count>;
	using line = irglab::line<dimension_count>;

	
	using plane = homogeneous_coordinates;
	using plane_normal = cartesian_coordinates;

	
	[[nodiscard]] inline homogeneous_coordinates to_homogeneous_coordinates(
		const cartesian_coordinates& cartesian_coordinates) noexcept
	{
		return irglab::to_homogeneous_coordinates<dimension_count>(cartesian_coordinates);
	}

	[[nodiscard]] inline cartesian_coordinates to_cartesian_coordinates(
		const homogeneous_coordinates& homogeneous_coordinates)
	{
		return irglab::to_cartesian_coordinates<dimension_count>(homogeneous_coordinates);
	}

	constexpr void normalize(homogeneous_coordinates& homogeneous_coordinates)
	{
		irglab::normalize<dimension_count>(homogeneous_coordinates);
	}

	[[nodiscard]] inline plane_normal get_plane_normal(
		const cartesian_coordinates& first_point,
		const cartesian_coordinates& second_point,
		const cartesian_coordinates& third_point)
	{
		return
		{
			(second_point.y - first_point.y) * (third_point.z - first_point.z) -
				(second_point.z - first_point.z) * (third_point.y - first_point.y),

			-(second_point.x - first_point.x) * (third_point.z - first_point.z) +
				(second_point.z - first_point.z) * (third_point.x - first_point.x),

			(second_point.x - first_point.x) * (third_point.y - first_point.y) -
				(second_point.y - first_point.y) * (third_point.x - first_point.x)
		};
	}

	[[nodiscard]] inline plane get_common_plane(
		const cartesian_coordinates& first_point,
		const cartesian_coordinates& second_point,
		const cartesian_coordinates& third_point)
	{
		const auto normal = get_plane_normal(first_point, second_point, third_point);

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
