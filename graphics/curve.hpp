#ifndef IRGLAB_CURVE_HPP
#define IRGLAB_CURVE_HPP


#include "pch.hpp"
#include "extensions.hpp"

#include "primitives.hpp"


namespace irglab
{
	template<small_natural_number DimensionCount>
	struct [[maybe_unused]] is_curve_description_supported : is_vector_size_supported<DimensionCount> {};

	template<small_natural_number DimensionCount>
	[[maybe_unused]] inline constexpr bool is_curve_description_supported_v =
		is_curve_description_supported<DimensionCount>::value;

	
	template<small_natural_number DimensionCount, std::enable_if_t<
		is_curve_description_supported_v<DimensionCount>,
	int> = 0>
	struct [[maybe_unused]] curve
	{
		static constexpr small_natural_number dimension_count = DimensionCount;
		
		using control_point = cartesian_coordinates<dimension_count>;

	private:
		std::vector<control_point> control_points_;

		
	public:
		explicit curve(std::initializer_list<control_point> control_points) :
			control_points_{ control_points } { }

		
		[[nodiscard]] cartesian_coordinates<dimension_count> operator()(
			const rational_number parameter)
		{
			cartesian_coordinates<dimension_count> result{};

			for (natural_number i = 0 ; i < control_points_.size() ; ++i)
				result += control_points_[i] * get_bernstein_polynomial_result(
					i, control_points_.size() - 1, parameter);

			return result;
		}

	private:
		[[nodiscard]] static rational_number get_bernstein_polynomial_result(
			const natural_number index, 
			const natural_number control_point_count, 
			const rational_number parameter)
		{
			return static_cast<rational_number>(
				number_of_combinations(control_point_count, index) * 
				glm::pow(parameter, index) *
				glm::pow(1 - parameter, control_point_count - index));
		}
	};
}

namespace irglab::two_dimensional
{
	using curve [[maybe_unused]] = irglab::curve<dimension_count>;
}

namespace irglab::three_dimensional
{
	using curve [[maybe_unused]] = irglab::curve<dimension_count>;
}

#endif
