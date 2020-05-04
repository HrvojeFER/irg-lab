#ifndef IRGLAB_CURVE_HPP
#define IRGLAB_CURVE_HPP


#include "pch.hpp"
#include "extensions.hpp"

#include "primitives.hpp"


namespace irglab
{
	template<size DimensionCount>
	struct curve
	{
		using point = cartesian_coordinates<DimensionCount>;
		std::vector<point> control_points;

		explicit curve(std::initializer_list<point> control_points) :
			control_points{ control_points } { }

		[[nodiscard]] point operator()(const number parameter)
		{
			point result{};

			for (size i = 0; i < control_points.size(); ++i)
				result += control_points[i] * get_bernstein_polynomial_result(
					i, control_points.size(), parameter);

			return result;
		}
		
		[[nodiscard]] static number get_bernstein_polynomial_result(
			const size index, const size control_point_count, const number parameter)
		{
			return number_of_combinations(control_point_count, index) * 
				glm::pow(parameter, index) *
				glm::pow(1 - parameter, control_point_count - index);
		}
	};
}

namespace irglab::two_dimensional
{
	using curve = irglab::curve<dimension_count>;
}

namespace irglab::three_dimensional
{
	using curve = irglab::curve<dimension_count>;
}

#endif
