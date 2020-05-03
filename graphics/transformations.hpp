#ifndef IRGLAB_TRANSFORMS_HPP
#define IRGLAB_TRANSFORMS_HPP


#include "pch.hpp"

#include "primitives.hpp"


namespace irglab
{
	template<size DimensionCount>
	using transformation = irglab::matrix<DimensionCount + 1, DimensionCount + 1>;
}

namespace irglab::two_dimensional
{
	using transformation = irglab::transformation<dimension_count>;

	[[nodiscard]] constexpr transformation get_scale_transformation(
		const number scaling_factor) noexcept
	{
		return
		{
			scaling_factor, 0.0f, 0.0f,
			0.0f, scaling_factor, 0.0f,
			0.0f, 0.0f, 1.0f
		};
	}

	[[nodiscard]] constexpr transformation get_translation(
		const number x_translation,
		const number y_translation) noexcept
	{
		return
		{
			1.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			x_translation, y_translation, 1.0f
		};
	}

	[[nodiscard]] inline transformation get_rotation(
		const angle angle) noexcept
	{
		// To avoid multiple calculations.
		const auto sine = glm::sin(angle);
		const auto cosine = glm::cos(angle);
		
		return
		{
			cosine, sine, 0.0f,
			-sine, cosine, 0.0f,
			0.0f, 0.0f, 1.0f
		};
	}

	[[nodiscard]] inline transformation get_shear_transformation(
		const angle x_angle, const angle y_angle) noexcept
	{
		return
		{
			1.0f, glm::tan(x_angle), 0.0f,
			glm::tan(y_angle), 1.0f, 0.0f,
			0.0f, 0.0f, 1.0f
		};
	}

	[[nodiscard]] inline transformation get_perspective_transformation(
		const angle angle, 
		const number x_translation, 
		const number y_translation) noexcept
	{
		return get_rotation(angle) * get_translation(x_translation, y_translation);
	}
}

namespace irglab::three_dimensional
{
	using transformation = irglab::transformation<dimension_count>;

	[[nodiscard]] constexpr transformation get_scale_transformation(
		const number scaling_factor) noexcept
	{
		return
		{
			scaling_factor, 0.0f, 0.0f, 0.0f,
			0.0f, scaling_factor, 0.0f, 0.0f,
			0.0f, 0.0f, scaling_factor, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
	}
	
	[[nodiscard]] constexpr transformation get_translation(
		const number x_translation,
		const number y_translation,
		const number z_translation) noexcept
	{
		return
		{
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			x_translation, y_translation, z_translation, 1.0f
		};
	}

	[[nodiscard]] inline transformation get_x_rotation(
		const angle angle) noexcept
	{
		// To avoid multiple calculations.
		const auto sine = glm::sin(angle);
		const auto cosine = glm::cos(angle);
		
		return
		{
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, cosine, sine, 0.0f,
			0.0f, -sine, cosine, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
	}

	[[nodiscard]] inline transformation get_y_rotation(const angle angle) noexcept
	{
		// To avoid multiple calculations.
		const auto sine = glm::sin(angle);
		const auto cosine = glm::cos(angle);

		return
		{
			cosine, 0.0f, -sine, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			sine, 0.0f, cosine, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
	}

	[[nodiscard]] inline transformation get_z_rotation(const angle angle) noexcept
	{
		// To avoid multiple calculations.
		const auto sine = glm::sin(angle);
		const auto cosine = glm::cos(angle);

		return
		{
			cosine, sine, 0.0f, 0.0f,
			-sine, cosine, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
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
		{
			1.0f, x_tan, x_tan, 0.0f,
			y_tan, 1.0f, y_tan, 0.0f,
			z_tan, z_tan, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
	}
}

#endif
