#ifndef IRGLAB_TRANSFORMS_HPP
#define IRGLAB_TRANSFORMS_HPP


#include "pch.hpp"

#include "primitives.hpp"

/*
 * In GLM, everything is set column by column, so, in order to make sense of the transformations visually,
 * I set them row by row and then transposed all of them. This doesn't affect performance a lot because,
 * usually, transposing a transformation is way faster than calculating the transformation and in most
 * calculations you would calculate the transformation and apply it to all points.
 */

namespace irglab
{
	template<small_natural_number DimensionCount, std::enable_if_t<
		is_matrix_size_supported_v<DimensionCount + 1, DimensionCount + 1>,
	int> = 0>
	using transformation = irglab::matrix<DimensionCount + 1, DimensionCount + 1>;

	
	template<small_natural_number DimensionCount, std::enable_if_t<
		is_matrix_size_supported_v<DimensionCount + 1, DimensionCount + 1>,
	int> = 0>
	// Columns are orthonormal vectors.
	using orthonormal_base = irglab::matrix<DimensionCount + 1, DimensionCount + 1>;

	
	template<small_natural_number DimensionCount, std::enable_if_t<
		is_vector_size_supported_v<DimensionCount>,
	int> = 0>
	using axis = irglab::vector<DimensionCount>;
}

namespace irglab::two_dimensional
{
	using transformation = irglab::transformation<dimension_count>;
	using orthonormal_base = irglab::orthonormal_base<dimension_count>;
	using axis = irglab::axis<dimension_count>;

	[[nodiscard]] inline transformation get_scale_transformation(
		const rational_number scaling_factor) noexcept
	{
		return
			transpose(transformation
				{
					scaling_factor, 0.0f, 0.0f,
					0.0f, scaling_factor, 0.0f,
					0.0f, 0.0f, 1.0f
				});
	}

	[[nodiscard]] inline transformation get_translation(
		const rational_number x_translation,
		const rational_number y_translation) noexcept
	{
		return
			transpose(transformation
				{
					1.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f,
					x_translation, y_translation, 1.0f
				});
	}

	[[nodiscard]] inline transformation get_rotation(
		const angle angle) noexcept
	{
		// To avoid multiple calculations.
		const auto sine = glm::sin(angle);
		const auto cosine = glm::cos(angle);
		
		return
			transpose(transformation
				{
					cosine, sine, 0.0f,
					-sine, cosine, 0.0f,
					0.0f, 0.0f, 1.0f
				});
	}

	[[nodiscard]] inline transformation get_shear_transformation(
		const angle x_angle, const angle y_angle) noexcept
	{
		return
			transpose(transformation
				{
					1.0f, glm::tan(x_angle), 0.0f,
					glm::tan(y_angle), 1.0f, 0.0f,
					0.0f, 0.0f, 1.0f
				});
	}

	[[nodiscard]] inline transformation get_perspective_transformation(
		const angle angle, 
		const rational_number x_translation, 
		const rational_number y_translation) noexcept
	{
		return get_rotation(angle) * get_translation(x_translation, y_translation);
	}
}

namespace irglab::three_dimensional
{
	using transformation = irglab::transformation<dimension_count>;
	using orthonormal_base = irglab::orthonormal_base<dimension_count>;
	using axis = irglab::axis<dimension_count>;

	[[nodiscard]] inline transformation get_scale_transformation(
		const rational_number scaling_factor) noexcept
	{
		return
			transpose(transformation
				{
					scaling_factor, 0.0f, 0.0f, 0.0f,
					0.0f, scaling_factor, 0.0f, 0.0f,
					0.0f, 0.0f, scaling_factor, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f
				});
	}
	
	[[nodiscard]] inline transformation get_translation(
		const rational_number x_translation,
		const rational_number y_translation,
		const rational_number z_translation) noexcept
	{
		return
			transpose(transformation
				{
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					x_translation, y_translation, z_translation, 1.0f
				});
	}

	[[nodiscard]] inline transformation get_x_rotation(
		const angle angle) noexcept
	{
		// To avoid multiple calculations.
		const auto sine = glm::sin(angle);
		const auto cosine = glm::cos(angle);
		
		return
			transpose(transformation
				{
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, cosine, sine, 0.0f,
					0.0f, -sine, cosine, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f
				});
	}

	[[nodiscard]] inline transformation get_y_rotation(const angle angle) noexcept
	{
		// To avoid multiple calculations.
		const auto sine = glm::sin(angle);
		const auto cosine = glm::cos(angle);

		return
			transpose(transformation
				{
					cosine, 0.0f, -sine, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					sine, 0.0f, cosine, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f
				});
	}

	[[nodiscard]] inline transformation get_z_rotation(const angle angle) noexcept
	{
		// To avoid multiple calculations.
		const auto sine = glm::sin(angle);
		const auto cosine = glm::cos(angle);

		return
			transpose(transformation
				{
					cosine, sine, 0.0f, 0.0f,
					-sine, cosine, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f
				});
	}

	// Following the Rodrigues' rotation formula.
	[[nodiscard]] inline transformation get_rotation(const angle angle, const axis& axis) noexcept
	{
		// To avoid multiple calculations.
		const auto sine = glm::sin(angle);
		const auto cosine = glm::cos(angle);

		return
			transpose(transformation
				{
					cosine + axis.x * axis.x * (1 - cosine),
					axis.x * axis.y * (1 - cosine) - axis.z * sine,
					axis.y * sine + axis.x * axis.z * (1 - cosine),
					0.0f,

					axis.z * sine + axis.x * axis.y * (1 - cosine),
					cosine + axis.y * axis.y * (1 - cosine),
					-axis.x * sine + axis.y * axis.z * (1 - cosine),
					0.0f,

					-axis.y * sine + axis.x * axis.z * (1 - cosine),
					axis.x * sine + axis.y * axis.z * (1 - cosine),
					cosine + axis.z * axis.z * (1 - cosine),
					0.0f,

					0.0f, 0.0f, 0.0f, 1.0f
				});
	}

	[[nodiscard]] inline transformation get_shear_transformation(
		const angle x_angle, 
		const angle y_angle, 
		const angle z_angle) noexcept
	{
		// To avoid multiple calculations.
		const auto x_tan = glm::tan(x_angle);
		const auto y_tan = glm::tan(y_angle);
		const auto z_tan = glm::tan(z_angle);
		
		return
			transpose(transformation
				{
					1.0f, x_tan, x_tan, 0.0f,
					y_tan, 1.0f, y_tan, 0.0f,
					z_tan, z_tan, 1.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f
				});
	}
}

#endif
