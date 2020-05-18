#ifndef IRGLAB_PRIMITIVES_HPP
#define IRGLAB_PRIMITIVES_HPP

#include "pch.hpp"

namespace irglab
{
	inline constexpr glm::qualifier precision = glm::defaultp;


	
	using natural_number = unsigned long long int;
	inline constexpr natural_number natural_number_max = UINT64_MAX;
	inline constexpr natural_number natural_number_min = 0;
	inline constexpr natural_number zero = 0;
	inline constexpr natural_number one = 1;

	using small_natural_number = unsigned short int;
	inline constexpr small_natural_number small_natural_number_max = USHRT_MAX;
	inline constexpr small_natural_number small_natural_number_min = 0;
	inline constexpr small_natural_number small_zero = 0;
	inline constexpr small_natural_number small_one = 1;
	inline constexpr small_natural_number small_two = 2;
	inline constexpr small_natural_number small_three = 3;
	
	using rational_number = float;
	inline constexpr rational_number rational_number_max = FLT_MAX;
	inline constexpr rational_number rational_number_min = -FLT_MAX;
	inline constexpr rational_number rational_zero = 0.0f;
	inline constexpr rational_number rational_one = 1.0f;
	
	
	using angle = rational_number;

	
	template<small_natural_number Size>
	struct is_vector_size_supported : std::bool_constant<
		Size == 1 || Size == 2 || Size == 3 || Size == 4> {};

	template<small_natural_number Size>
	inline constexpr bool is_vector_size_supported_v = is_vector_size_supported<Size>::value;
	
	template<small_natural_number Size, std::enable_if_t<
		is_vector_size_supported_v<Size>,
	int> = 0>
	using vector = glm::vec<Size, rational_number, precision>;

	
	template<small_natural_number RowCount, small_natural_number ColumnCount>
	struct is_matrix_size_supported : std::bool_constant<
		(RowCount == 2 || RowCount == 3 || RowCount == 4) &&
		(ColumnCount == 2 || ColumnCount == 3 || ColumnCount == 4)> {};

	template<small_natural_number RowCount, small_natural_number ColumnCount>
	inline constexpr bool is_matrix_size_supported_v =
		is_matrix_size_supported<RowCount, ColumnCount>::value;

	template<small_natural_number RowCount, small_natural_number ColumnCount, std::enable_if_t<
		is_matrix_size_supported_v<RowCount, ColumnCount>,
	int> = 0>
	using matrix = glm::mat<ColumnCount, RowCount, rational_number, precision>;


	
	template<small_natural_number DimensionCount, std::enable_if_t<
		is_vector_size_supported_v<DimensionCount>,
	int> = 0>
	using cartesian_coordinates = vector<DimensionCount>;

	template<small_natural_number DimensionCount, std::enable_if_t<
		is_vector_size_supported_v<DimensionCount + small_one>,
	int> = 0>
	using homogeneous_coordinates = vector<DimensionCount + 1>;

	
	template<small_natural_number DimensionCount, std::enable_if_t<
		is_vector_size_supported_v<DimensionCount + small_one>,
	int> = 0>
	using point = vector<DimensionCount + small_one>;

	template<small_natural_number DimensionCount, std::enable_if_t<
		is_vector_size_supported_v<DimensionCount + small_one>,
	int> = 0>
	using line = vector<DimensionCount + small_one>;


	
	template<small_natural_number DimensionCount>
	struct are_primitive_operations_supported : std::bool_constant<
		DimensionCount == 2 || DimensionCount == 3> {};

	template<small_natural_number DimensionCount>
	inline constexpr bool is_primitive_operation_supported_v =
		are_primitive_operations_supported<DimensionCount>::value;

	
	template<small_natural_number DimensionCount, std::enable_if_t<
		is_primitive_operation_supported_v<DimensionCount>,
	int> = 0>
	// ReSharper disable once CppFunctionIsNotImplemented
	[[nodiscard]] constexpr homogeneous_coordinates<DimensionCount> to_homogeneous_coordinates(
		const cartesian_coordinates<DimensionCount>& cartesian_coordinates) noexcept;

	template<small_natural_number DimensionCount, std::enable_if_t<
		is_primitive_operation_supported_v<DimensionCount>,
	int> = 0>
	// ReSharper disable once CppFunctionIsNotImplemented
	[[nodiscard]] constexpr cartesian_coordinates<DimensionCount> to_cartesian_coordinates(
		const homogeneous_coordinates<DimensionCount>& homogeneous_coordinates);

	template<small_natural_number DimensionCount, std::enable_if_t<
		is_primitive_operation_supported_v<DimensionCount>,
	int> = 0>
	// ReSharper disable once CppFunctionIsNotImplemented
	constexpr void normalize(homogeneous_coordinates<DimensionCount>& homogeneous_coordinates);


	
	namespace two_dimensional { inline constexpr small_natural_number dimension_count = 2; }
	namespace three_dimensional { inline constexpr small_natural_number dimension_count = 3; }

	
	template<>
	[[nodiscard]] inline homogeneous_coordinates<two_dimensional::dimension_count>
		to_homogeneous_coordinates<two_dimensional::dimension_count>(
		
		const cartesian_coordinates<two_dimensional::dimension_count>& cartesian_coordinates) noexcept
	{
		return { cartesian_coordinates.x, cartesian_coordinates.y, 1.0f };
	}
	template<>
	[[nodiscard]] inline cartesian_coordinates<two_dimensional::dimension_count>
		to_cartesian_coordinates<two_dimensional::dimension_count>(
		
		const homogeneous_coordinates<two_dimensional::dimension_count>& homogeneous_coordinates)
	{
		return
		{
			homogeneous_coordinates.x / homogeneous_coordinates.z,
			homogeneous_coordinates.y / homogeneous_coordinates.z
		};
	}

	template<>
	constexpr void normalize<two_dimensional::dimension_count>(
		homogeneous_coordinates<two_dimensional::dimension_count>& homogeneous_coordinates)
	{
		homogeneous_coordinates.x = homogeneous_coordinates.x / homogeneous_coordinates.z;
		homogeneous_coordinates.y = homogeneous_coordinates.y / homogeneous_coordinates.z;
		homogeneous_coordinates.z = 1.0f;
	}


	template<>
	[[nodiscard]] inline homogeneous_coordinates<three_dimensional::dimension_count>
		to_homogeneous_coordinates<three_dimensional::dimension_count>(
		
		const cartesian_coordinates<three_dimensional::dimension_count>& cartesian_coordinates) noexcept
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
	[[nodiscard]] inline cartesian_coordinates<three_dimensional::dimension_count>
		to_cartesian_coordinates<three_dimensional::dimension_count>(
		
		const homogeneous_coordinates<three_dimensional::dimension_count>& homogeneous_coordinates)
	{
		return
		{
			homogeneous_coordinates.x / homogeneous_coordinates.w,
			homogeneous_coordinates.y / homogeneous_coordinates.w,
			homogeneous_coordinates.z / homogeneous_coordinates.w
		};
	}

	template<>
	constexpr void normalize<three_dimensional::dimension_count>(
		homogeneous_coordinates<three_dimensional::dimension_count>& homogeneous_coordinates)
	{
		homogeneous_coordinates.x = homogeneous_coordinates.x / homogeneous_coordinates.w;
		homogeneous_coordinates.y = homogeneous_coordinates.y / homogeneous_coordinates.w;
		homogeneous_coordinates.z = homogeneous_coordinates.z / homogeneous_coordinates.w;
		homogeneous_coordinates.w = 1.0f;
	}
}


namespace irglab::two_dimensional
{
	
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

	
	[[nodiscard]] inline line get_line_at_y(const rational_number y_coordinate) noexcept
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
	using cartesian_coordinates = irglab::cartesian_coordinates<dimension_count>;
	using homogeneous_coordinates = irglab::homogeneous_coordinates<dimension_count>;

	using point = irglab::point<dimension_count>;
	using line = irglab::line<dimension_count>;

	
	using plane = vector<dimension_count + 1>;
	using plane_normal = vector<dimension_count>;

	
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
