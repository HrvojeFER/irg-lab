#ifndef GRAPHICS_BODY_HPP
#define GRAPHICS_BODY_HPP


#include "pch.hpp"


namespace irglab::three_dimensional
{
	using cartesian_coordinates = glm::vec3;
	using homogeneous_coordinates = glm::vec4;

	using point = homogeneous_coordinates;
	using line = homogeneous_coordinates;
	using plane = homogeneous_coordinates;
	using parametric_plane = glm::mat3x4;
	using plane_normal = glm::vec3;

	using transform_matrix = glm::mat4;

	[[nodiscard]] constexpr homogeneous_coordinates to_homogeneous_coordinates(
		const cartesian_coordinates& cartesian_coordinates)
	{
		return
		{
			cartesian_coordinates.x,
			cartesian_coordinates.y,
			cartesian_coordinates.z,
			1.0f
		};
	}

	[[nodiscard]] constexpr cartesian_coordinates to_cartesian_coordinates(
		const homogeneous_coordinates& homogeneous_coordinates)
	{
		return
		{
			homogeneous_coordinates.x / homogeneous_coordinates.w,
			homogeneous_coordinates.y / homogeneous_coordinates.w,
			homogeneous_coordinates.z / homogeneous_coordinates.w
		};
	}

	constexpr void normalize(homogeneous_coordinates& homogeneous_coordinates)
	{
		homogeneous_coordinates.x = homogeneous_coordinates.x / homogeneous_coordinates.w;
		homogeneous_coordinates.y = homogeneous_coordinates.y / homogeneous_coordinates.w;
		homogeneous_coordinates.z = homogeneous_coordinates.z / homogeneous_coordinates.w;
		homogeneous_coordinates.w = 1.0f;
	}

	[[nodiscard]] inline plane get_common_plane(
		point first_point,
		point second_point,
		point third_point)
	{
		normalize(first_point);
		normalize(second_point);
		normalize(third_point);
		
		const plane_normal normal
		{
			(second_point.y - first_point.y) * (third_point.z - first_point.z) -
				(second_point.z - first_point.z) * (third_point.y - first_point.y),

			-(second_point.x - first_point.x) * (third_point.z - first_point.z) +
				(second_point.z - first_point.z) * (third_point.x - first_point.x),

			(second_point.x - first_point.x) * (third_point.y - first_point.y) -
				(second_point.y - first_point.y) * (third_point.x - first_point.x),
		};

		return
		{
			normal.x,
			normal.y,
			normal.z,
			-first_point.x * normal.x - first_point.y * normal.y - first_point.z * normal.z
		};
	}

	[[nodiscard]] inline transform_matrix get_scale_transform_matrix(const float scale_factor)
	{
		return
		{
			scale_factor, 0.0f, 0.0f, 0.0f,
			0.0f, scale_factor, 0.0f, 0.0f,
			0.0f, 0.0f, scale_factor, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
	}

	[[nodiscard]] inline transform_matrix get_translate_transform_matrix(
		const float dx, 
		const float dy, 
		const float dz)
	{
		return
		{
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			dx, dy, dz, 1.0f
		};
	}

	
	struct triangle
	{
		using vertex = point;

		explicit triangle(const std::array<vertex, 3>&& vertices) :
			vertices_{ vertices },
			plane_{ get_common_plane(
				vertices[0],
				vertices[1],
				vertices[2]) } { }


		friend std::ostream& operator <<(std::ostream& output_stream,
			const triangle& triangle);

		
		[[nodiscard]] std::array<vertex, 3> get_vertices() const
		{
			return vertices_;
		}

		[[nodiscard]] plane get_plane() const
		{
			return plane_;
		}

		void transform(const transform_matrix& transform_matrix)
		{
			for (auto& vertex : vertices_)
			{
				vertex = transform_matrix * vertex;
			}

			plane_ = get_common_plane(
				vertices_[0],
				vertices_[1],
				vertices_[2]);
		}

	private:
		std::array<vertex, 3> vertices_;
		plane plane_;
	};

	
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
		
		void normalize (const float limit)
		{
			std::cout << "Normalizing body..." << std::endl;
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

			const auto dx = x_max - x_min;
			const auto dy = y_max - y_min;
			const auto dz = z_max - z_min;
			
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

			std::cout << "Body successfully normalized!" << std::endl;
			std::cout << "Translated by dx: " << -dx << " dy: " << -dy << " dz: " << -dz << std::endl;
			std::cout << "Scaled by a factor of: " << scaling_factor << std::endl <<
				std::endl;
		}

		
		[[nodiscard]] static body parse(const std::vector<std::string>&& lines)
		{
			std::cout << "Parsing body..." << std::endl;
			
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

			std::cout << "Body successfully parsed!" << std::endl <<
				"Lines read: " << lines.size() << std::endl <<
				"Comment lines count: " << comment_count << std::endl <<
				"Vertex count: " << vertices.size() << std::endl <<
				"Triangle count: " << triangles.size() << std::endl << std::endl;
			
			return body{ triangles };
		}

		[[nodiscard]] std::vector<triangle> get_triangles() const
		{
			return triangles_;
		}

	private:
		std::vector<triangle> triangles_;
	};

	inline std::ostream& operator <<(std::ostream& output_stream,
		const triangle& triangle)
	{
		output_stream << "\t Vertices -> " << std::endl;
		for (const auto& vertex : triangle.vertices_)
		{
			output_stream << "x: " << vertex.x / vertex.w <<
				" y: " << vertex.y / vertex.w <<
				" z: " << vertex.z / vertex.w <<
				std::endl;
		}

		output_stream << "\t Plane -> a: " << triangle.plane_.x <<
			" b: " << triangle.plane_.y <<
			" c: " << triangle.plane_.z <<
			" d: " << triangle.plane_.w << std::endl;

		return output_stream;
	}
}

#endif
