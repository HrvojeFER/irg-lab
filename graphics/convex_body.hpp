#ifndef GRAPHICS_BODY_HPP
#define GRAPHICS_BODY_HPP


#include "pch.hpp"
#include "extensions.hpp"

#include "primitives.hpp"
#include "transformations.hpp"

#include "bounds.hpp"

#include "triangle.hpp"


namespace irglab
{
	template<size DimensionCount>
	struct convex_body_internal
	{
		using triangle = triangle<DimensionCount>;
		std::vector<triangle> triangles;

		
		constexpr explicit convex_body_internal(
			const std::initializer_list<triangle>& triangles) noexcept :
			triangles{ triangles } { }

		template<typename Iterator>
		explicit convex_body_internal(const Iterator& begin, const Iterator& end) :
			triangles{ begin, end } { }


		constexpr void normalize()
		{
			for (auto& triangle : triangles) triangle.normalize();
		}
		
		constexpr void operator*=(const transformation<DimensionCount>& transformation) noexcept
		{
			for (auto& triangle : triangles) triangle *= transformation;
		}

		friend constexpr void operator|=(
			bounds<DimensionCount>& bounds, const convex_body_internal& body) noexcept
		{
			for (const auto& triangle : body.triangles) bounds |= triangle;
		}

		friend void operator+=(wireframe<DimensionCount>& wireframe, const convex_body_internal& body)
		{
			for (const auto& triangle : body.triangles) wireframe += triangle;
		}

		
		friend std::ostream& operator<<(std::ostream& output_stream, const convex_body_internal& body)
		{
			output_stream << "Triangles:" << std::endl;

			for (const auto& triangle : body.triangles)
			{
				output_stream << triangle;
			}

			return output_stream << std::endl;
		}
	};


	
	template<size DimensionCount>
	struct convex_body final : convex_body_internal<DimensionCount> { convex_body() = delete; };

	
	template<>
	struct convex_body<2> final : convex_body_internal<2>
	{
		constexpr explicit convex_body(
			const std::initializer_list<two_dimensional::triangle>& triangles) noexcept :
			convex_body_internal<2>{ triangles } { }

		template<typename Iterator>
		explicit convex_body(const Iterator& begin, const Iterator& end) :
			convex_body_internal{ begin, end } { }
	};


	template<>
	struct convex_body<3> : convex_body_internal<3>
	{
		using triangle = three_dimensional::triangle;

		constexpr explicit convex_body(
			const std::initializer_list<three_dimensional::triangle>& triangles) noexcept :
			convex_body_internal<3>{ triangles } { }

		template<typename Iterator>
		explicit convex_body(Iterator begin, Iterator end) :
			convex_body_internal<3>{ begin, end } { }

		struct parsing_report;
		[[nodiscard]] static parsing_report parse(const std::vector<std::string>& lines);

		struct bound_fit_report
		{
			float dx{};
			float dy{};
			float dz{};

			float scaling_factor{};


			friend std::ostream& operator<<(std::ostream& output_stream,
				const bound_fit_report& normalization_report)
			{
				output_stream <<
					"dx: " << normalization_report.dx << " " <<
					"dy: " << normalization_report.dy << " " <<
					"dz: " << normalization_report.dz << std::endl <<
					"scaling factor: " << normalization_report.scaling_factor << std::endl <<
					std::endl;

				return output_stream;
			}
		};

		bound_fit_report operator&=(const three_dimensional::bounds& bounds) noexcept
		{
			three_dimensional::bounds current_bounds{};
			current_bounds |= *this;

			const auto translation_difference =
				bounds.get_center() - current_bounds.get_center();
			const auto&& translation =
				three_dimensional::get_translation(
					translation_difference.x,
					translation_difference.y,
					translation_difference.z);

			const auto scaling_factors =
				bounds.get_difference() / current_bounds.get_difference();
			const auto min_scaling_factor =
				scaling_factors.x < scaling_factors.y ?
				(scaling_factors.x < scaling_factors.z ? scaling_factors.x : scaling_factors.y) :
				scaling_factors.y < scaling_factors.z ? scaling_factors.y : scaling_factors.z;

			const auto&& scale_transformation =
				three_dimensional::get_scale_transformation(min_scaling_factor);

			*this *= translation * scale_transformation;

			return
			{
				translation_difference.x,
				translation_difference.y,
				translation_difference.z,

				min_scaling_factor
			};
		}

		bound_fit_report operator&=(const float limit) noexcept
		{
			return *this &= get_normalization_bounds(limit);
		}

		[[nodiscard]] friend bool operator<(
			const three_dimensional::point& point, const convex_body& body)
		{
			for (const auto& triangle : body.triangles)
			{
				if (not (point < triangle))
				{
					return false;
				}
			}

			return true;
		}

	private:
		[[nodiscard]] static constexpr three_dimensional::bounds get_normalization_bounds(
			const number limit) noexcept
		{
			return
			{
				-limit,
				limit,

				-limit,
				limit,

				-limit,
				limit
			};
		}
	};

	struct convex_body<3>::parsing_report
	{
		convex_body body;
		size line_count{};
		size comment_count{};
		size error_count{};
		size vertex_count{};
		size triangle_count{};

		friend std::ostream& operator<<(std::ostream& output_stream,
			const parsing_report& parsing_report) noexcept
		{
			return output_stream <<
				"Lines read: " << parsing_report.line_count << std::endl <<
				"Comment lines count: " << parsing_report.comment_count << std::endl <<
				"Error lines count: " << parsing_report.error_count << std::endl << 
				"Vertex count: " << parsing_report.vertex_count << std::endl <<
				"Triangle count: " << parsing_report.triangle_count << std::endl <<
				std::endl << 
				"Body:" << std::endl <<
				parsing_report.body << std::endl;
		}
	};

	[[nodiscard]] inline auto convex_body<3>::parse(
		const std::vector<std::string>& lines) -> parsing_report
	{
		size comment_count = 0;
		size error_count = 0;

		std::vector<three_dimensional::triangle> triangles;
		std::vector<three_dimensional::triangle::vertex> vertices;

		for (const auto& line : lines)
		{
			std::istringstream line_stream{ line };

			char first;
			line_stream >> first;

			if (first == '#')
			{
				comment_count++;
				continue;
			}

			if (first == 'v')
			{
				float x, y, z;
				line_stream >> x >> y >> z;

				vertices.emplace_back(triangle::vertex{ x, y, z, 1.0f });
			}
			else if (first == 'f')
			{
				size_t a, b, c;
				line_stream >> a >> b >> c;

				triangles.emplace_back(
					triangle
					{
						// Numbering in text starts with 1.
						vertices[a - 1],
						vertices[b - 1],
						vertices[c - 1]
					});
			}
			
			else if (first != 'g')
			{
				error_count++;
			}
		}

		return parsing_report
		{
			convex_body(triangles.begin(), triangles.end()),
			lines.size(),
			comment_count,
			vertices.size(),
			triangles.size()
		};
	}

	[[nodiscard]] inline convex_body<3> operator""_body(const char* chars, size)
	{
		std::istringstream char_stream{ chars };

		std::vector<std::string> result;
		for (std::string line; std::getline(char_stream, line); )
		{
			// Check if empty or contains only whitespace
			if (!line.empty() &&
				!(line.find_first_not_of(' ') == std::string::npos) &&
				!(line.find_first_not_of('\t') == std::string::npos))
			{
				trim(line);
				result.emplace_back(line);
			}
		}

		return convex_body<3>::parse(result).body;
	}
}

namespace irglab::two_dimensional
{
	using convex_body = irglab::convex_body<2>;
}

namespace irglab::three_dimensional
{
	using convex_body = irglab::convex_body<3>;
}

#endif
