#ifndef CONVEX_POLYGON_HPP
#define CONVEX_POLYGON_HPP


#include "pch.hpp"


namespace irglab
{
	using cartesian_coordinates_type = glm::vec2;
	using homogeneous_coordinates_type = glm::vec3;

	using homogeneous_point_type = homogeneous_coordinates_type;
	using homogeneous_line_type = homogeneous_coordinates_type;

	[[nodiscard]] constexpr homogeneous_coordinates_type to_homogeneous_coordinates(
		const cartesian_coordinates_type& cartesian_coordinates)
	{
		return {cartesian_coordinates.x, cartesian_coordinates.y, 1.0f};
	}

	[[nodiscard]] constexpr cartesian_coordinates_type to_cartesian_coordinates(
		const homogeneous_coordinates_type& homogeneous_coordinates)
	{
		return {
			homogeneous_coordinates.x / homogeneous_coordinates.z,
			homogeneous_coordinates.y / homogeneous_coordinates.z
		};
	}

	[[nodiscard]] constexpr homogeneous_line_type get_line_at_y(const float y_coordinate) noexcept
	{
		return {0, 1, -y_coordinate};
	}

	[[nodiscard]] constexpr void normalize(homogeneous_coordinates_type& homogeneous_coordinates)
	{
		homogeneous_coordinates.x = homogeneous_coordinates.x / homogeneous_coordinates.z;
		homogeneous_coordinates.y = homogeneous_coordinates.y / homogeneous_coordinates.z;
		homogeneous_coordinates.z = 1.0f;
	}

	[[nodiscard]] inline homogeneous_point_type get_intersection(
		const homogeneous_line_type& line_a,
		const homogeneous_line_type& line_b)
	{
		return cross(line_a, line_b);
	}

	[[nodiscard]] inline homogeneous_line_type get_connecting_line(
		const homogeneous_point_type& first,
		const homogeneous_point_type& second)
	{
		return cross(first, second);
	}

	[[nodiscard]] inline std::vector<homogeneous_line_type> get_successive_point_lines(
		std::vector<homogeneous_point_type> points)
	{
		std::vector<homogeneous_line_type> result{points.size()};

		for (size_t index = 0; index < points.size(); ++index)
		{
			result[index] = get_connecting_line(
				points[index],
				points[(index + 1) % points.size()]);
		}

		return result;
	}

	[[nodiscard]] inline std::vector<homogeneous_point_type> get_intersections(
		const std::vector<homogeneous_line_type>& lines,
		const homogeneous_line_type& line)
	{
		std::vector<homogeneous_point_type> result{lines.size()};

		std::transform(lines.begin(), lines.end(), result.begin(),
		               [line](const homogeneous_line_type& other)
		               {
			               return get_intersection(line, other);
		               });

		return result;
	}
	
	struct convex_polygon
	{
		using vertex_type = homogeneous_point_type;
		using edge_type = homogeneous_line_type;


		using direction_type = bool;

		static inline const direction_type clockwise = true;
		static inline const direction_type counterclockwise = false;

		static inline const direction_type right = false;
		static inline const direction_type left = true;

		static inline const direction_type top = true;
		static inline const direction_type bottom = false;


		explicit convex_polygon(const std::vector<homogeneous_point_type>& points) :
			vertices_{ take_prefix_forming_convex_polygon(points) },
			direction_{ get_direction(vertices_[0], vertices_[1], vertices_[2]) },
			edges_{ get_successive_point_lines(vertices_) },
			left_edges_{ get_edges_from_direction(left) },
			right_edges_{ get_edges_from_direction(right) } { }


		friend std::ostream& operator <<(std::ostream& output_stream,
			const convex_polygon& convex_polygon);


		[[nodiscard]] bool is_inside(const homogeneous_point_type& point) const
		{
			for (const auto& edge : edges_)
			{
				if (direction_ != get_direction(edge, point))
				{
					return false;
				}
			}

			return true;
		}


		struct edge_intersections
		{
			const homogeneous_point_type left;
			const homogeneous_point_type right;
		};

		[[nodiscard]] edge_intersections get_edge_intersections_at_y(const float y_coordinate) const
		{
			const auto y_line = get_line_at_y(y_coordinate);

			homogeneous_point_type max_left{-FLT_MAX, 0, 1};
			for (const auto& left_intersection : 
				get_intersections(left_edges_, y_line))
			{
				if (left_intersection.x / left_intersection.z > max_left.x / max_left.z)
				{
					max_left = left_intersection;
				}
			}

			homogeneous_point_type min_right{FLT_MAX, 0, 1};
			for (const auto& right_intersection : 
				get_intersections(right_edges_, y_line))
			{
				if (right_intersection.x / right_intersection.z < min_right.x / min_right.z)
				{
					min_right = right_intersection;
				}
			}

			return { max_left, min_right };
		}

		[[nodiscard]] std::vector<homogeneous_point_type> get_all_edge_line_intersections_at_y(
			const float y_coordinate) const
		{
			return get_intersections(edges_, get_line_at_y(y_coordinate));
		}

		[[nodiscard]] vertex_type get_vertex_on(const direction_type top_or_bottom) const
		{
			auto result = vertices_[0];

			for (const auto& vertex : vertices_)
			{
				if (vertex.y / vertex.z > result.y / result.z == top_or_bottom)
				{
					result = vertex;
				}
			}

			return result;
		}

		[[nodiscard]] std::vector<vertex_type> get_vertices() const
		{
			return vertices_;
		}

	private:
		std::vector<vertex_type> vertices_;

		direction_type direction_;

		std::vector<edge_type> edges_;
		// stored for fast intersection calculations
		std::vector<edge_type> left_edges_;
		std::vector<edge_type> right_edges_;


		[[nodiscard]] std::vector<edge_type> get_edges_from_direction(
			const direction_type left_or_right) const
		{
			std::vector<edge_type> result{};

			for (size_t index = 0; index < vertices_.size(); ++index)
			{
				const auto& first_vertex = vertices_[index];
				const auto& second_vertex = 
					vertices_[(index + 1) % vertices_.size()];
				
				if (first_vertex.y / first_vertex.z > 
					second_vertex.y / second_vertex.z ==
					direction_ == left_or_right)
				{
					result.push_back(edges_[index]);
				}
			}

			return result;
		}


		[[nodiscard]] static direction_type get_direction(
			const edge_type& edge,
			const homogeneous_point_type& point)
		{
			if (const auto relation = dot(point, edge);
				relation != 0)
			{
				return relation > 0;
			}

			throw std::invalid_argument("Point is collinear with the edge.");
		}

		[[nodiscard]] static direction_type get_direction(
			const homogeneous_point_type& first,
			const homogeneous_point_type& second,
			const homogeneous_point_type& third)
		{
			if (const auto relation = dot(third, cross(first, second));
				relation != 0)
			{
				return relation > 0;
			}

			throw std::invalid_argument("Points are collinear.");
		}

		[[nodiscard]] static bool are_convex_polygon_in_direction(
			const std::vector<vertex_type>& points,
			const direction_type circular_direction)
		{
			if (points.size() < 3)
			{
				throw std::range_error("Expected at least three points.");
			}

			for (size_t index = 0; index < points.size(); ++index)
			{
				const auto current_direction = get_direction(
					points[index],
					points[(index + 1) % points.size()],
					points[(index + 2) % points.size()]);
				
				if (circular_direction != current_direction)
				{
					return false;
				}
			}

			return true;
		}

		[[nodiscard]] static std::vector<vertex_type> take_prefix_forming_convex_polygon(
			const std::vector<homogeneous_point_type>& points)
		{
			if (points.size() < 3)
			{
				throw std::range_error("Expected at least three points.");
			}

			std::vector<glm::vec3> convex_polygon =
			{
				points[0],
				points[1],
				points[2]
			};

			auto direction = get_direction(
				convex_polygon[0],
				convex_polygon[1],
				convex_polygon[2]);

			for (size_t index = 3; index < points.size(); ++index)
			{
				convex_polygon.push_back(points[index]);

				if (!are_convex_polygon_in_direction(convex_polygon, direction))
				{
					std::reverse(convex_polygon.begin(), convex_polygon.end());
					direction = !direction;

					if (!are_convex_polygon_in_direction(convex_polygon, direction))
					{
						convex_polygon =
						{
							convex_polygon.begin(),
							convex_polygon.end() - 1
						};
						break;
					}
				}
			}

			return convex_polygon;
		}
	};
	
	inline std::ostream& operator <<(std::ostream& output_stream,
		const convex_polygon& convex_polygon)
	{
		output_stream << "Direction: " <<
			(convex_polygon.direction_ ? "clockwise" : "counterclockwise") << std::endl;

		output_stream << "Vertices: " << std::endl;
		for (const auto& vertex : convex_polygon.vertices_)
		{
			const auto cartesian_coordinates = to_cartesian_coordinates(vertex);
			
			output_stream <<
				"x: " << cartesian_coordinates.x << 
				" y: " << cartesian_coordinates.y << std::endl;
		}

		output_stream << "Edges: " << std::endl;
		for (const auto& edge : convex_polygon.edges_)
		{
			output_stream << "a: " << edge.x << " b: " << edge.y << " c: " << edge.z << std::endl;
		}

		return output_stream;
	}
}

#endif
