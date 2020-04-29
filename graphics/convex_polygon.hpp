#ifndef CONVEX_POLYGON_HPP
#define CONVEX_POLYGON_HPP


#include "pch.hpp"
#include "two_dimensional_primitives.hpp"


namespace irglab::two_dimensional
{
	struct convex_polygon
	{
		using vertex = point;
		using edge = line;

		using direction = bool;

		static inline const direction clockwise = true;
		static inline const direction counterclockwise = false;

		static inline const direction right = false;
		static inline const direction left = true;

		static inline const direction top = true;
		static inline const direction bottom = false;


		explicit convex_polygon(const std::vector<point>& points) :
			vertices_{ take_prefix_forming_convex_polygon(points) },
			direction_{ get_direction(vertices_[0], vertices_[1], vertices_[2]) },
			edges_{ get_successive_point_lines(vertices_) },
			left_edges_{ get_edges_from_direction(left) },
			right_edges_{ get_edges_from_direction(right) } { }


		friend std::ostream& operator <<(std::ostream& output_stream,
			const convex_polygon& convex_polygon);


		[[nodiscard]] bool is_inside(const point& point) const
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
			const point left;
			const point right;
		};

		[[nodiscard]] edge_intersections get_edge_intersections_at_y(const float y_coordinate) const
		{
			const auto y_line = get_line_at_y(y_coordinate);

			point max_left{ -FLT_MAX, 0, 1 };
			for (const auto& left_intersection :
				get_intersections(left_edges_, y_line))
			{
				if (left_intersection.x / left_intersection.z > max_left.x / max_left.z)
				{
					max_left = left_intersection;
				}
			}

			point min_right{ FLT_MAX, 0, 1 };
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

		[[nodiscard]] std::vector<point> get_all_edge_line_intersections_at_y(
			const float y_coordinate) const
		{
			return get_intersections(edges_, get_line_at_y(y_coordinate));
		}

		[[nodiscard]] vertex get_vertex_on(const direction top_or_bottom) const
		{
			vertex result{ 0.0f, top_or_bottom ? -FLT_MAX : FLT_MAX, 1.0f };

			for (const auto& vertex : vertices_)
			{
				if (vertex.y / vertex.z > result.y / result.z == top_or_bottom)
				{
					result = vertex;
				}
			}

			return result;
		}

		[[nodiscard]] std::vector<vertex> get_vertices() const
		{
			return vertices_;
		}

	private:
		std::vector<vertex> vertices_;

		direction direction_;

		std::vector<edge> edges_;
		// stored for faster intersection calculations
		std::vector<edge> left_edges_;
		std::vector<edge> right_edges_;


		[[nodiscard]] std::vector<edge> get_edges_from_direction(
			const direction left_or_right) const
		{
			std::vector<edge> result{};

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


		[[nodiscard]] static direction get_direction(
			const edge& edge,
			const point& point)
		{
			if (const auto relation = dot(point, edge);
				relation != 0)
			{
				return relation > 0;
			}

			throw std::invalid_argument("Point is collinear with the edge.");
		}

		[[nodiscard]] static direction get_direction(
			const point& first,
			const point& second,
			const point& third)
		{
			if (const auto relation = dot(third, cross(first, second));
				relation != 0)
			{
				return relation > 0;
			}

			throw std::invalid_argument("Points are collinear.");
		}

		[[nodiscard]] static bool are_convex_polygon_in_direction(
			const std::vector<vertex>& points,
			const direction circular_direction)
		{
			if (points.size() < 3)
			{
				throw std::range_error("Expected at least three points.");
			}

			for (size_t index = 0; index < points.size(); ++index)
			{
				const auto&& current_direction = get_direction(
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

		[[nodiscard]] static std::vector<vertex> take_prefix_forming_convex_polygon(
			const std::vector<point>& points)
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

			auto&& direction = get_direction(
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
						convex_polygon.pop_back();
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
			const auto&& cartesian_coordinates =
				to_cartesian_coordinates(vertex);

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
