#ifndef IRGLAB_TRANSFORMATIONS_HPP
#define IRGLAB_TRANSFORMATIONS_HPP


#include "external/external.hpp"

#include "primitives.hpp"



/*
 * In GLM and GLSL, everything is set column by column, so, in order to make sense of the transformations visually,
 * I set them row by row and then transposed all of them. This doesn't affect performance a lot because,
 * usually, transposing a transformation is way faster than calculating the transformation and in most
 * calculations you would calculate the transformation and apply it to all points.
 */



// Types

namespace il
{
    template<
            small_natural_number DimensionCount, ENABLE_IF(
                    is_matrix_size_supported(DimensionCount + small_one, DimensionCount + small_one))>
    using transformation [[maybe_unused]] = il::matrix<DimensionCount + small_one, DimensionCount + small_one>;



    template<
            small_natural_number DimensionCount, ENABLE_IF(
                    is_matrix_size_supported(DimensionCount + small_one, DimensionCount + small_one))>
    // Columns are orthonormal vectors.
    using orthonormal_base [[maybe_unused]] = il::matrix<DimensionCount + small_one, DimensionCount + small_one>;

    template<
            small_natural_number DimensionCount, ENABLE_IF(
                    is_vector_size_supported(is_vector_size_supported(DimensionCount)))>
    using axis [[maybe_unused]] = il::vector<DimensionCount>;
}



// 2D definitions

namespace il::d2
{
    using transformation [[maybe_unused]] = il::transformation<dimension_count>;
    using orthonormal_base [[maybe_unused]] = il::orthonormal_base<dimension_count>;
    using axis [[maybe_unused]] = il::axis<dimension_count>;

    [[nodiscard, maybe_unused]] inline transformation get_scale_transformation(
            const rational_number scaling_factor) noexcept
    {
        return
                transpose(
                        transformation
                                {
                                        scaling_factor, 0.0f, 0.0f,
                                        0.0f, scaling_factor, 0.0f,
                                        0.0f, 0.0f, 1.0f
                                });
    }

    [[nodiscard, maybe_unused]] inline transformation get_translation(
            const rational_number x_translation,
            const rational_number y_translation) noexcept
    {
        return
                transpose(
                        transformation
                                {
                                        1.0f, 0.0f, 0.0f,
                                        0.0f, 1.0f, 0.0f,
                                        x_translation, y_translation, 1.0f
                                });
    }

    [[nodiscard, maybe_unused]] inline transformation get_rotation(
            const angle angle) noexcept
    {
        // To avoid multiple calculations.
        const auto sine = glm::sin(angle);
        const auto cosine = glm::cos(angle);

        return
                transpose(
                        transformation
                                {
                                        cosine, sine, 0.0f,
                                        -sine, cosine, 0.0f,
                                        0.0f, 0.0f, 1.0f
                                });
    }

    [[nodiscard, maybe_unused]] inline transformation get_shear_transformation(
            const angle x_angle, const angle y_angle) noexcept
    {
        return
                transpose(
                        transformation
                                {
                                        1.0f, glm::tan(x_angle), 0.0f,
                                        glm::tan(y_angle), 1.0f, 0.0f,
                                        0.0f, 0.0f, 1.0f
                                });
    }

    [[nodiscard, maybe_unused]] inline transformation get_perspective_transformation(
            const angle angle,
            const rational_number x_translation,
            const rational_number y_translation) noexcept
    {
        return get_rotation(angle) * get_translation(x_translation, y_translation);
    }
}

namespace il::d3
{
    using transformation [[maybe_unused]] = il::transformation<dimension_count>;
    using orthonormal_base [[maybe_unused]] = il::orthonormal_base<dimension_count>;
    using axis [[maybe_unused]] = il::axis<dimension_count>;

    [[nodiscard, maybe_unused]] inline transformation get_scale_transformation(
            const rational_number scaling_factor) noexcept
    {
        return
                transpose(
                        transformation
                                {
                                        scaling_factor, 0.0f, 0.0f, 0.0f,
                                        0.0f, scaling_factor, 0.0f, 0.0f,
                                        0.0f, 0.0f, scaling_factor, 0.0f,
                                        0.0f, 0.0f, 0.0f, 1.0f
                                });
    }

    [[nodiscard, maybe_unused]] inline transformation get_translation(
            const rational_number x_translation,
            const rational_number y_translation,
            const rational_number z_translation) noexcept
    {
        return
                transpose(
                        transformation
                                {
                                        1.0f, 0.0f, 0.0f, 0.0f,
                                        0.0f, 1.0f, 0.0f, 0.0f,
                                        0.0f, 0.0f, 1.0f, 0.0f,
                                        x_translation, y_translation, z_translation, 1.0f
                                });
    }

    [[nodiscard, maybe_unused]] inline transformation get_x_rotation(
            const angle angle) noexcept
    {
        // To avoid multiple calculations.
        const auto sine = glm::sin(angle);
        const auto cosine = glm::cos(angle);

        return
                transpose(
                        transformation
                                {
                                        1.0f, 0.0f, 0.0f, 0.0f,
                                        0.0f, cosine, sine, 0.0f,
                                        0.0f, -sine, cosine, 0.0f,
                                        0.0f, 0.0f, 0.0f, 1.0f
                                });
    }

    [[nodiscard, maybe_unused]] inline transformation get_y_rotation(const angle angle) noexcept
    {
        // To avoid multiple calculations.
        const auto sine = glm::sin(angle);
        const auto cosine = glm::cos(angle);

        return
                transpose(
                        transformation
                                {
                                        cosine, 0.0f, -sine, 0.0f,
                                        0.0f, 1.0f, 0.0f, 0.0f,
                                        sine, 0.0f, cosine, 0.0f,
                                        0.0f, 0.0f, 0.0f, 1.0f
                                });
    }

    [[nodiscard, maybe_unused]] inline transformation get_z_rotation(const angle angle) noexcept
    {
        // To avoid multiple calculations.
        const auto sine = glm::sin(angle);
        const auto cosine = glm::cos(angle);

        return
                transpose(
                        transformation
                                {
                                        cosine, sine, 0.0f, 0.0f,
                                        -sine, cosine, 0.0f, 0.0f,
                                        0.0f, 0.0f, 1.0f, 0.0f,
                                        0.0f, 0.0f, 0.0f, 1.0f
                                });
    }

    // Following the Rodrigues' rotation formula.
    [[nodiscard, maybe_unused]] inline transformation get_rotation(const angle angle, const axis& axis) noexcept
    {
        // To avoid multiple calculations.
        const auto sine = glm::sin(angle);
        const auto cosine = glm::cos(angle);
        const auto cosine_inverse = 1 - cosine;

        return
                transpose(
                        transformation
                                {
                                        cosine + axis.x * axis.x * cosine_inverse,
                                        axis.x * axis.y * cosine_inverse - axis.z * sine,
                                        axis.y * sine + axis.x * axis.z * cosine_inverse,
                                        0.0f,

                                        axis.z * sine + axis.x * axis.y * cosine_inverse,
                                        cosine + axis.y * axis.y * cosine_inverse,
                                        -axis.x * sine + axis.y * axis.z * cosine_inverse,
                                        0.0f,

                                        -axis.y * sine + axis.x * axis.z * cosine_inverse,
                                        axis.x * sine + axis.y * axis.z * cosine_inverse,
                                        cosine + axis.z * axis.z * cosine_inverse,
                                        0.0f,

                                        0.0f, 0.0f, 0.0f, 1.0f
                                });
    }

    [[nodiscard, maybe_unused]] inline transformation get_shear_transformation(
            const angle x_angle,
            const angle y_angle,
            const angle z_angle) noexcept
    {
        // To avoid multiple calculations.
        const auto x_tan = glm::tan(x_angle);
        const auto y_tan = glm::tan(y_angle);
        const auto z_tan = glm::tan(z_angle);

        return
                transpose(
                        transformation
                                {
                                        1.0f, x_tan, x_tan, 0.0f,
                                        y_tan, 1.0f, y_tan, 0.0f,
                                        z_tan, z_tan, 1.0f, 0.0f,
                                        0.0f, 0.0f, 0.0f, 1.0f
                                });
    }
}

#endif
