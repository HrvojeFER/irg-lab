#ifndef GRAPHICS_BODY_HPP
#define GRAPHICS_BODY_HPP


#include "pch.hpp"

#include "three_dimensional_primitives.hpp"

#include "three_dimensional_triangle.hpp"


namespace irglab::three_dimensional
{
	struct body
	{
		explicit body(std::vector<triangle> triangles) : triangles_{ std::move(triangles) }
		{
			for (const auto& triangle : triangles_)
			{
				std::cout << triangle << std::endl;
			}
		}

		[[nodiscard]] bool is_inside(const point& point) const
		{
			for (const auto& triangle : triangles_)
			{
				if (dot(point, triangle.get_plane()) > 0)
				{
					return false;
				}
			}

			return true;
		}

		struct bounds
		{
			const float x_min;
			const float x_max;

			const float y_min;
			const float y_max;

			const float z_min;
			const float z_max;
		};

		[[nodiscard]] bounds get_bounds()
		{

			auto
				x_min = FLT_MAX, x_max = -FLT_MAX,
				y_min = FLT_MAX, y_max = -FLT_MAX,
				z_min = FLT_MAX, z_max = -FLT_MAX;

			for (const auto& triangle : triangles_)
			{
				for (const auto& vertex : triangle.get_vertices())
				{
					if (vertex.x / vertex.w < x_min)
					{
						x_min = vertex.x;
					}
					else if (vertex.x / vertex.w > x_max)
					{
						x_max = vertex.x;
					}

					if (vertex.y / vertex.w < y_min)
					{
						y_min = vertex.y;
					}
					else if (vertex.y / vertex.w > y_max)
					{
						y_max = vertex.y;
					}

					if (vertex.z / vertex.w < z_min)
					{
						z_min = vertex.z;
					}
					else if (vertex.z / vertex.w > z_max)
					{
						z_max = vertex.z;
					}
				}
			}

			return bounds
			{
				x_min,
				x_max,

				y_min,
				y_max,

				z_min,
				z_max
			};
		}
		
		void normalize (const float limit)
		{
#if !defined(NDEBUG)
			std::cout << "Normalizing body..." << std::endl;
#endif

			const auto bounds = get_bounds();

			const auto dx = bounds.x_max - bounds.x_min;
			const auto dy = bounds.y_max - bounds.y_min;
			const auto dz = bounds.z_max - bounds.z_min;
			
			const auto&& translate_transform_matrix = 
				get_translate_transform_matrix(-dx, -dy, -dz);

			const auto scaling_factor =
				limit / (dx > dy ? (dx > dz ? dx : dz) : dy > dz ? dy : dz);
			
			const auto&& scale_transform_matrix = 
				get_scale_transform_matrix(scaling_factor);

			const auto transform_matrix = 
				scale_transform_matrix * translate_transform_matrix;
			
			for (auto& triangle : triangles_)
			{
				triangle.transform(transform_matrix);

				std::cout << triangle << std::endl;
			}
			
#if !defined(NDEBUG)
			std::cout << "Body successfully normalized!" << std::endl;
			std::cout << "Translated by dx: " << -dx << " dy: " << -dy << " dz: " << -dz << std::endl;
			std::cout << "Scaled by a factor of: " << scaling_factor << std::endl <<
				std::endl;
#endif
		}

		struct parsing_report;
		[[nodiscard]] static parsing_report parse(const std::vector<std::string>&& lines);

		[[nodiscard]] std::vector<triangle> get_triangles() const
		{
			return triangles_;
		}

	private:
		std::vector<triangle> triangles_;
	};

	inline std::ostream& operator<<(std::ostream& output_stream, const body::bounds body_bounds)
	{
		output_stream << "";
	}

	inline std::ostream& operator<<(std::ostream& output_stream, const body& body)
	{
		output_stream << "Body triangles:" << std::endl;

		for (const auto& triangle : body.get_triangles())
		{
			output_stream << triangle << std::endl;
		}

		return output_stream;
	}

	struct body::parsing_report
	{
		const body body;
		const unsigned long long line_count;
		const unsigned long long comment_count;
		const unsigned long long vertex_count;
		const unsigned long long triangle_count;
	};

	inline std::ostream& operator<<(std::ostream& output_stream,
		const body::parsing_report& parsing_report)
	{
		return output_stream << "Body successfully parsed!" << std::endl <<
			"Lines read: " << parsing_report.line_count << std::endl <<
			"Comment lines count: " << parsing_report.comment_count << std::endl <<
			"Vertex count: " << parsing_report.vertex_count << std::endl <<
			"Triangle count: " << parsing_report.triangle_count << std::endl << std::endl;
	}

	[[nodiscard]] inline body::parsing_report body::parse(const std::vector<std::string>&& lines)
	{
		unsigned long long comment_count = 0;

		std::vector<triangle> triangles;
		std::vector<triangle::vertex> vertices;

		for (const auto& line : lines)
		{
			if (line[0] == '#')
			{
				comment_count++;
				continue;
			}

			std::istringstream line_stream{ line };

			char first;
			line_stream >> first;

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
						std::array<triangle::vertex, 3>
						{
					// Numbering in text starts with 1.
					vertices[a - 1],
					vertices[b - 1],
					vertices[c - 1]
				}
					});
			}
			// The tetrahedron file has a line starting with 'g'.
			// I'm not sure if that's correct.
			else if (first != 'g')
			{
				throw std::runtime_error("Unexpected character.");
			}
		}

		return parsing_report
		{
			body { triangles },
			lines.size(),
			comment_count,
			vertices.size(),
			triangles.size()
		};
	}
}

#endif
