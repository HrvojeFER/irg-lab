#ifndef IRGLAB_PRIMITIVES_HPP
#define IRGLAB_PRIMITIVES_HPP


#include "external/external.hpp"


namespace il
{
    // Precision

    [[maybe_unused]] inline constexpr glm::qualifier precision = glm::defaultp;



    // Scalars

    using natural_number [[maybe_unused]] = unsigned long long int;

    [[maybe_unused]] inline constexpr natural_number natural_number_max = UINT64_MAX;

    [[maybe_unused]] inline constexpr natural_number natural_number_min = 0;

    [[maybe_unused]] inline constexpr natural_number zero = 0;

    [[maybe_unused]] inline constexpr natural_number one = 1;


    using small_natural_number [[maybe_unused]] = unsigned short int;

    [[maybe_unused]] inline constexpr small_natural_number small_natural_number_max = USHRT_MAX;

    [[maybe_unused]] inline constexpr small_natural_number small_natural_number_min = 0;

    [[maybe_unused]] inline constexpr small_natural_number small_zero = 0;

    [[maybe_unused]] inline constexpr small_natural_number small_one = 1;

    [[maybe_unused]] inline constexpr small_natural_number small_two = 2;

    [[maybe_unused]] inline constexpr small_natural_number small_three = 3;


    using rational_number [[maybe_unused]] = float;

    [[maybe_unused]] inline constexpr rational_number rational_number_max = FLT_MAX;

    [[maybe_unused]] inline constexpr rational_number rational_number_min = -FLT_MAX;

    [[maybe_unused]] inline constexpr rational_number rational_zero = 0.0f;

    [[maybe_unused]] inline constexpr rational_number rational_one = 1.0f;


    using angle [[maybe_unused]] = rational_number;



    // Vectors and matrices

    [[nodiscard, maybe_unused]] constexpr bool is_vector_size_supported(small_natural_number size)
    {
        return size == 1 || size == 2 || size == 3 || size == 4;
    }

    template<small_natural_number Size, ENABLE_IF(is_vector_size_supported(Size))>
    using vector [[maybe_unused]] = glm::vec<Size, rational_number, precision>;


    [[nodiscard, maybe_unused]] constexpr bool is_matrix_size_supported(
            small_natural_number row_count, small_natural_number column_count)
    {
        return (row_count == 2 || row_count == 3 || row_count == 4) &&
               (column_count == 2 || column_count == 3 || column_count == 4);
    }

    template<
            small_natural_number RowCount, small_natural_number ColumnCount,
            ENABLE_IF(is_matrix_size_supported(RowCount, ColumnCount))>
    using matrix [[maybe_unused]] = glm::mat<ColumnCount, RowCount, rational_number, precision>;



    // Primitives

    template<small_natural_number DimensionCount, ENABLE_IF(is_vector_size_supported(DimensionCount))>
    using cartesian_coordinates [[maybe_unused]] = vector<DimensionCount>;

    template<
            small_natural_number DimensionCount,
            ENABLE_IF(is_vector_size_supported(DimensionCount + small_one))>
    using homogeneous_coordinates [[maybe_unused]] = vector<DimensionCount + 1>;


    template<
            small_natural_number DimensionCount,
            ENABLE_IF(is_vector_size_supported(DimensionCount + small_one))>
    using point [[maybe_unused]] = vector<DimensionCount + small_one>;



    // Primitive operations

    [[nodiscard, maybe_unused]] constexpr bool are_primitive_operations_supported(
            small_natural_number dimension_count)
    {
        return dimension_count == 2 || dimension_count == 3;
    }

    template<
            small_natural_number DimensionCount,
            ENABLE_IF(are_primitive_operations_supported(DimensionCount))>
    [[nodiscard, maybe_unused]] constexpr homogeneous_coordinates<DimensionCount> to_homogeneous_coordinates(
            const cartesian_coordinates<DimensionCount>& cartesian_coordinates) noexcept;

    template<
            small_natural_number DimensionCount,
            ENABLE_IF(are_primitive_operations_supported(DimensionCount))>
    [[nodiscard, maybe_unused]] constexpr cartesian_coordinates<DimensionCount> to_cartesian_coordinates(
            const homogeneous_coordinates<DimensionCount>& homogeneous_coordinates);

    template<
            small_natural_number DimensionCount,
            ENABLE_IF(are_primitive_operations_supported(DimensionCount))>
    [[maybe_unused]] constexpr void normalize(homogeneous_coordinates<DimensionCount>& homogeneous_coordinates);



    // Supported dimension namespaces

    namespace d2
    {
        [[maybe_unused]] inline constexpr small_natural_number dimension_count = 2;
    }
    namespace d3
    {
        [[maybe_unused]] inline constexpr small_natural_number dimension_count = 3;
    }



    // Primitive operation implementations

    template<>
    [[nodiscard, maybe_unused]] inline homogeneous_coordinates<d2::dimension_count>
    to_homogeneous_coordinates<d2::dimension_count>(

            const cartesian_coordinates <d2::dimension_count>& cartesian_coordinates) noexcept
    {
        return {cartesian_coordinates.x, cartesian_coordinates.y, 1.0f};
    }

    template<>
    [[nodiscard, maybe_unused]] inline cartesian_coordinates<d2::dimension_count>
    to_cartesian_coordinates<d2::dimension_count>(

            const homogeneous_coordinates <d2::dimension_count>& homogeneous_coordinates)
    {
        return
                {
                        homogeneous_coordinates.x / homogeneous_coordinates.z,
                        homogeneous_coordinates.y / homogeneous_coordinates.z
                };
    }

    template<>
    [[maybe_unused]] constexpr void normalize<d2::dimension_count>(
            homogeneous_coordinates<d2::dimension_count>& homogeneous_coordinates)
    {
        homogeneous_coordinates.x = homogeneous_coordinates.x / homogeneous_coordinates.z;
        homogeneous_coordinates.y = homogeneous_coordinates.y / homogeneous_coordinates.z;
        homogeneous_coordinates.z = 1.0f;
    }


    template<>
    [[nodiscard, maybe_unused]] inline homogeneous_coordinates<d3::dimension_count>
    to_homogeneous_coordinates<d3::dimension_count>(

            const cartesian_coordinates <d3::dimension_count>& cartesian_coordinates) noexcept
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
    [[nodiscard, maybe_unused]] inline cartesian_coordinates<d3::dimension_count>
    to_cartesian_coordinates<d3::dimension_count>(

            const homogeneous_coordinates <d3::dimension_count>& homogeneous_coordinates)
    {
        return
                {
                        homogeneous_coordinates.x / homogeneous_coordinates.w,
                        homogeneous_coordinates.y / homogeneous_coordinates.w,
                        homogeneous_coordinates.z / homogeneous_coordinates.w
                };
    }

    template<>
    [[maybe_unused]] constexpr void normalize<d3::dimension_count>(
            homogeneous_coordinates<d3::dimension_count>& homogeneous_coordinates)
    {
        homogeneous_coordinates.x = homogeneous_coordinates.x / homogeneous_coordinates.w;
        homogeneous_coordinates.y = homogeneous_coordinates.y / homogeneous_coordinates.w;
        homogeneous_coordinates.z = homogeneous_coordinates.z / homogeneous_coordinates.w;
        homogeneous_coordinates.w = 1.0f;
    }
}



// 2D namespace aliases

namespace il::d2
{
    using cartesian_coordinates [[maybe_unused]] = il::cartesian_coordinates<dimension_count>;
    using homogeneous_coordinates [[maybe_unused]] = il::homogeneous_coordinates<dimension_count>;

    using point [[maybe_unused]] = il::point<dimension_count>;

    using line [[maybe_unused]] = il::vector<dimension_count + small_one>;


    [[nodiscard, maybe_unused]] inline homogeneous_coordinates to_homogeneous_coordinates(
            const cartesian_coordinates& cartesian_coordinates) noexcept
    {
        return il::to_homogeneous_coordinates<dimension_count>(cartesian_coordinates);
    }

    [[nodiscard, maybe_unused]] inline cartesian_coordinates to_cartesian_coordinates(
            const homogeneous_coordinates& homogeneous_coordinates)
    {
        return il::to_cartesian_coordinates<dimension_count>(homogeneous_coordinates);
    }

    [[maybe_unused]] constexpr void normalize(homogeneous_coordinates& homogeneous_coordinates)
    {
        il::normalize<dimension_count>(homogeneous_coordinates);
    }


    [[nodiscard, maybe_unused]] inline line get_line_at_y(const rational_number y_coordinate) noexcept
    {
        return {0, 1, -y_coordinate};
    }

    [[nodiscard, maybe_unused]] inline point get_intersection(const line& line_a, const line& line_b) noexcept
    {
        return cross(line_a, line_b);
    }

    [[nodiscard, maybe_unused]] inline line get_connecting_line(const point& first, const point& second) noexcept
    {
        return cross(first, second);
    }
}



// 3D namespace aliases

namespace il::d3
{
    using cartesian_coordinates [[maybe_unused]] = il::cartesian_coordinates<dimension_count>;
    using homogeneous_coordinates [[maybe_unused]] = il::homogeneous_coordinates<dimension_count>;

    using point [[maybe_unused]] = il::point<dimension_count>;

    using plane [[maybe_unused]] = vector<dimension_count + 1>;
    using plane_normal [[maybe_unused]] = vector<dimension_count>;


    [[nodiscard, maybe_unused]] inline homogeneous_coordinates to_homogeneous_coordinates(
            const cartesian_coordinates& cartesian_coordinates) noexcept
    {
        return il::to_homogeneous_coordinates<dimension_count>(cartesian_coordinates);
    }

    [[nodiscard, maybe_unused]] inline cartesian_coordinates to_cartesian_coordinates(
            const homogeneous_coordinates& homogeneous_coordinates)
    {
        return il::to_cartesian_coordinates<dimension_count>(homogeneous_coordinates);
    }

    [[maybe_unused]] constexpr void normalize(homogeneous_coordinates& homogeneous_coordinates)
    {
        il::normalize<dimension_count>(homogeneous_coordinates);
    }

    [[nodiscard, maybe_unused]] inline plane_normal get_plane_normal(
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

    [[nodiscard, maybe_unused]] inline plane get_common_plane(
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
