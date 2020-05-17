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
		using triangle = tracking_triangle<DimensionCount>;
		using shared_triangle = std::shared_ptr<triangle>;
		using vertex = typename triangle::vertex;
		using tracked_vertex = typename triangle::tracked_vertex;

	protected:
		std::unordered_set<tracked_vertex> vertices_;
		std::unordered_set<shared_triangle> triangles_;


		constexpr explicit convex_body_internal(
			const std::initializer_list<tracked_vertex>& vertices,
			const std::initializer_list<shared_triangle>& triangles) noexcept :

			vertices_{ vertices },
			triangles_{ triangles } { }

		
		template<typename Iterator>
		static constexpr bool is_proper_vertex_iterator = is_iterator_v<Iterator> &&
			std::is_same_v<typename std::iterator_traits<Iterator>::value_type, tracked_vertex>;

		template<typename Iterator>
		static constexpr bool is_proper_triangle_iterator = is_iterator_v<Iterator> &&
			std::is_same_v<typename std::iterator_traits<Iterator>::value_type, shared_triangle>;

		template<typename VertexIterator, typename TriangleIterator, std::enable_if_t<
			is_proper_vertex_iterator<VertexIterator> &&
			is_proper_triangle_iterator<TriangleIterator>,
		int> = 0>
		explicit convex_body_internal(
			const VertexIterator& vertices_begin, const VertexIterator& vertices_end,
			const TriangleIterator& triangles_begin, const TriangleIterator& triangles_end) :

			vertices_{ vertices_begin, vertices_end },
			triangles_{ triangles_begin, triangles_end } { }

		
	public:
		[[nodiscard]] const std::unordered_set<tracked_vertex>& vertices() const
		{
			return vertices_;
		}

		[[nodiscard]] const std::unordered_set<shared_triangle>& triangles() const
		{
			return triangles_;
		}
		
		
		friend constexpr void operator|=(
			bounds<DimensionCount>& bounds, const convex_body_internal& body) noexcept
		{
			for (const auto& triangle : body.triangles_) bounds |= *triangle;
		}

		friend void operator+=(irglab::tracking_wireframe<DimensionCount>& wireframe, 
			const convex_body_internal& body)
		{
			for (const auto& triangle : body.triangles_) wireframe += *triangle;
		}

		
		friend std::ostream& operator<<(std::ostream& output_stream, const convex_body_internal& body)
		{
			output_stream << "Triangles:" << std::endl;

			for (const auto& triangle : body.triangles_)
			{
				output_stream << *triangle;
			}

			return output_stream << std::endl;
		}

		constexpr void normalize() const
		{
			for (auto& triangle : triangles_) triangle->normalize();
		}

		constexpr void operator*=(const transformation<DimensionCount>& transformation) const noexcept
		{
			for (auto& triangle : triangles_) *triangle *= transformation;
		}
	};


	
	template<size DimensionCount>
	struct convex_body final : convex_body_internal<DimensionCount> { convex_body() = delete; };

	template<>
	struct convex_body<3> : convex_body_internal<3>
	{
	private:
		using base = convex_body_internal<3>;

		constexpr explicit convex_body(
			const std::initializer_list<tracked_vertex>& vertices,
			const std::initializer_list<shared_triangle>& triangles) noexcept :

			base{ vertices, triangles } { }

		template<typename VertexIterator, typename TriangleIterator, std::enable_if_t<
			is_proper_vertex_iterator<VertexIterator>&&
			is_proper_triangle_iterator<TriangleIterator>,
			int> = 0>
		explicit convex_body(
			const VertexIterator& vertices_begin, const VertexIterator& vertices_end,
			const TriangleIterator& triangles_begin, const TriangleIterator& triangles_end) :

			base{ vertices_begin, vertices_end, triangles_begin, triangles_end } { }

	public:
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
			for (const auto& triangle : body.triangles_)
			{
				if (not (point < *triangle))
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

		std::vector<tracked_vertex> vertices;
		std::vector<shared_triangle> triangles;

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

				vertices.emplace_back(tracked_vertex{ 
					std::make_shared<vertex>(x, y, z, 1.0f) });
			}
			else if (first == 'f')
			{
				size_t first_index, second_index, third_index;
				line_stream >> first_index >> second_index >> third_index;

				// Numbering in .obj files starts with 1.
				const auto& first_vertex = vertices[first_index - 1];
				const auto& second_vertex = vertices[second_index - 1];
				const auto& third_vertex = vertices[third_index - 1];
				
				const auto shared_triangle = std::make_shared<triangle>(
					first_vertex, second_vertex, third_vertex);
				
				first_vertex += shared_triangle;
				second_vertex += shared_triangle;
				third_vertex += shared_triangle;
				
				triangles.emplace_back(shared_triangle);
			}
			
			else if (first != 'g')
			{
				error_count++;
			}
		}

		return parsing_report
		{
			convex_body{
				vertices.begin(), vertices.end(),
				triangles.begin(), triangles.end() },

			lines.size(),
			comment_count,
			error_count,
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
