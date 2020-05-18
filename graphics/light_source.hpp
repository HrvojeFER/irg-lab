#ifndef IRGLAB_LIGHT_SOURCE_HPP
#define IRGLAB_LIGHT_SOURCE_HPP


#include "pch.hpp"

#include "primitives.hpp"


namespace irglab
{
	template<small_natural_number DimensionCount>
	struct [[maybe_unused]] is_light_source_description_supported :
		are_primitive_operations_supported<DimensionCount> {};

	template<small_natural_number DimensionCount>
	[[maybe_unused]] inline constexpr bool is_light_source_description_supported_v =
		is_light_source_description_supported<DimensionCount>::value;
	
	
	template<
	small_natural_number DimensionCount, std::enable_if_t<
		is_light_source_description_supported_v<DimensionCount>,
	int> = 0>
	struct [[maybe_unused]] light_source
	{
		static constexpr small_natural_number dimension_count = DimensionCount;
		
		using point = point<dimension_count>;
		using cartesian_coordinates = cartesian_coordinates<dimension_count>;

		using vector = vector<dimension_count>;

		static constexpr small_natural_number color_component_count = 3;
		using color = irglab::vector<color_component_count>;

	private:
		static constexpr rational_number min_normal_coefficient = rational_zero;
		
		point position_;

		rational_number ambient_intensity_;
		rational_number diffuse_intensity_;
		rational_number specular_intensity_;

		small_natural_number shine_;

		color hue_;


	public:
		[[nodiscard]] const point& position() const
		{
			return position_;
		}

		
		explicit light_source(
			point position,
			
			const rational_number ambient_intensity = 0.2f,
			const rational_number diffuse_intensity = 0.3f,
			const rational_number specular_intensity = 3.0f,

			const small_natural_number shine = 10,

			color hue = { 1.0f, 0.5f, 0.5f }) :

			position_{ std::move(position) },

			ambient_intensity_{ ambient_intensity },
			diffuse_intensity_{ diffuse_intensity },
			specular_intensity_{ specular_intensity },

			shine_{ shine },

			hue_{ std::move(hue) } { }


		[[nodiscard]] color get_lighting(
			const point& viewpoint,
			const point& vertex,
			vector normal,
			const rational_number ambient_coefficient = 0.1f,
			const rational_number diffuse_coefficient = 0.3f,
			const rational_number specular_coefficient = 2.0f) const
		{
			return hue_ * light_source::get_light_intensity(viewpoint, vertex, normal,
				ambient_coefficient, diffuse_coefficient, specular_coefficient);
		}

	private:
		[[nodiscard]] rational_number get_light_intensity(
			const point& viewpoint,
			const point& vertex,
			vector normal,
			const rational_number ambient_coefficient,
			const rational_number diffuse_coefficient,
			const rational_number specular_coefficient) const
		{
			const auto position_cartesian = to_cartesian_coordinates<dimension_count>(position_);
			const auto vertex_cartesian = to_cartesian_coordinates<dimension_count>(vertex);

			normal = glm::normalize(normal);

			
			const auto light_direction = light_source::get_light_direction(
				position_cartesian, vertex_cartesian);
			const auto normal_dot_light_direction = glm::dot(normal, light_direction);
			
			const auto reflection_direction = light_source::get_reflection_direction(
				light_direction, normal_dot_light_direction, normal);

			const auto viewpoint_direction = light_source::get_viewpoint_direction(
				vertex, viewpoint);

			
			return
				this->get_ambient_component(ambient_coefficient) +
				this->get_diffuse_component(diffuse_coefficient, normal_dot_light_direction) +
				this->get_specular_component(
					specular_coefficient, reflection_direction, viewpoint_direction);
		}
		
		[[nodiscard]] rational_number get_ambient_component(const rational_number coefficient) const
		{
			return ambient_intensity_ * coefficient;
		}

		[[nodiscard]] rational_number get_diffuse_component(
			const rational_number coefficient,
			rational_number normal_coefficient) const
		{
			normal_coefficient = normal_coefficient > min_normal_coefficient ? 
				normal_coefficient : min_normal_coefficient;
			
			return diffuse_intensity_ * coefficient * normal_coefficient;
		}

		[[nodiscard]] rational_number get_specular_component(
			const rational_number coefficient,
			const vector& reflection_direction,
			const vector& viewpoint_direction) const
		{
			auto shine_coefficient = glm::dot(reflection_direction, viewpoint_direction);
			shine_coefficient = shine_coefficient > min_normal_coefficient ?
				shine_coefficient : min_normal_coefficient;

			return specular_intensity_ * coefficient * 
				static_cast<rational_number>(glm::pow(shine_coefficient, shine_));
		}

		[[nodiscard]] static vector get_light_direction(
			const cartesian_coordinates& position,
			const cartesian_coordinates& vertex)
		{
			return glm::normalize(position - vertex);
		}

		[[nodiscard]] static vector get_reflection_direction(
			const vector& light_direction,
			const rational_number normal_dot_light_direction,
			const vector& normal)
		{
			return glm::normalize(normal * (2 * normal_dot_light_direction) - light_direction);
		}

		[[nodiscard]] static vector get_viewpoint_direction(
			const cartesian_coordinates& vertex,
			const cartesian_coordinates& viewpoint)
		{
			return glm::normalize(viewpoint - vertex);
		}
	};
}


namespace irglab::two_dimensional
{
	using light_source = irglab::light_source<dimension_count>;
}


namespace irglab::three_dimensional
{
	using light_source = irglab::light_source<dimension_count>;
}


#endif
