#ifndef IRGLAB_PRIMITIVE_OPERATIONS_HPP
#define IRGLAB_PRIMITIVE_OPERATIONS_HPP

#include "pch.hpp"

namespace irglab
{
	template<unsigned int DimensionCount>
	using point = glm::vec<DimensionCount + 1, float>;

	template<unsigned int DimensionCount>
	using line = glm::vec<DimensionCount + 1, float>;

	
	template<unsigned int DimensionCount>
	[[nodiscard]] point<DimensionCount> get_intersection(
		const line<DimensionCount>& line_a,
		const line<DimensionCount>& line_b)
	{
		return glm::cross(line_a, line_b);
	}

	template<unsigned int DimensionCount>
	[[nodiscard]] line<DimensionCount> get_connecting_line(
		const point<DimensionCount>& first,
		const point<DimensionCount>& second)
	{
		return glm::cross(first, second);
	}

	template<unsigned int DimensionCount>
	[[nodiscard]] std::vector<line<DimensionCount>> get_successive_point_lines(
		const std::vector<point<DimensionCount>>& points)
	{
		std::vector<line<DimensionCount>> result{ points.size() };

		for (size_t index = 0; index < points.size(); ++index)
		{
			result[index] = get_connecting_line(
				points[index],
				points[(index + 1) % points.size()]);
		}

		return result;
	}

	template<unsigned int DimensionCount>
	[[nodiscard]] std::vector<point<DimensionCount>> get_intersections(
		const std::vector<line<DimensionCount>>& lines,
		const line<DimensionCount>& line)
	{
		std::vector<point<DimensionCount>> result{ lines.size() };

		std::transform(lines.begin(), lines.end(), result.begin(),
			[line](const line<DimensionCount>& other)
			{
				return get_intersection(line, other);
			});

		return result;
	}
}

#endif

