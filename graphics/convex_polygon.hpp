#ifndef CONVEX_POLYGON_HPP
#define CONVEX_POLYGON_HPP


#include "pch.hpp"
#include "extensions.hpp"

#include "primitives.hpp"
#include "direction.hpp"
#include "bounds.hpp"

#include "wire.hpp"
#include "wireframe.hpp"


namespace irglab
{
	template<size DimensionCount, typename Inheritor>
	struct convex_polygon_internal
	{
		using vertex = point<DimensionCount>;

		void normalize()
		{
			for (auto& vertex : vertices_) irglab::normalize<DimensionCount>(vertex);
		}

		friend void operator+=(
			owning_wireframe<DimensionCount>& wireframe, const convex_polygon_internal& convex_polygon)
		{
			for (auto i = convex_polygon.vertices_.begin() + 1; i != convex_polygon.vertices_.end(); ++i)
				wireframe += owning_wire<DimensionCount>{ *(i - 1), *i };

			wireframe += owning_wire<DimensionCount>
			{
				*(convex_polygon.vertices_.end() - 1),
				*convex_polygon.vertices_.begin()
			};
		}


		friend std::ostream& operator<<(
			std::ostream& output_stream,
			const convex_polygon_internal& convex_polygon)
		{
			output_stream << "Vertices: " << std::endl;
			for (const auto& vertex : convex_polygon.vertices_)
				output_stream <<
				glm::to_string(to_cartesian_coordinates<DimensionCount>(vertex)) << std::endl;

			return output_stream << std::endl;
		}

	protected:
		template<typename Iterator>
		static constexpr bool is_proper_vertex_iterator_v = is_iterator_v<Iterator> &&
			std::is_same_v<typename std::iterator_traits<Iterator>::value_type, vertex> &&
			std::is_convertible_v<typename std::iterator_traits<Iterator>::iterator_category,
				std::input_iterator_tag>;
		
		template<typename Iterator, std::enable_if_t<is_proper_vertex_iterator_v<Iterator>, int> = 0>
		convex_polygon_internal(Iterator begin, Iterator end) :
			vertices_{ convex_polygon_internal::take_prefix_forming_convex_polygon(
				std::move(begin), std::move(end)) } { }

		convex_polygon_internal(const std::initializer_list<vertex>& vertices) :
			vertices_{ convex_polygon_internal::take_prefix_forming_convex_polygon(
					vertices.begin(), vertices.end()) } { }

		std::vector<vertex> vertices_;


	private:
		template<typename Iterator, std::enable_if_t<is_proper_vertex_iterator_v<Iterator>, int> = 0>
		[[nodiscard]] static std::vector<vertex> take_prefix_forming_convex_polygon(
			const Iterator& begin, const Iterator& end)
		{
			if (static_cast<size_t>(std::distance(begin, end)) < 3)
				throw std::range_error("Expected at least three points.");

			std::vector<vertex> convex_polygon =
			{
				*begin,
				*(begin + 1),
				*(begin + 2)
			};

			for (auto i = begin + 3; i != end; ++i)
			{
				convex_polygon.emplace_back(*i);

				if (!Inheritor::form_convex_polygon(convex_polygon))
				{
					std::reverse(convex_polygon.begin(), convex_polygon.end());

					if (!Inheritor::form_convex_polygon(convex_polygon))
					{
						convex_polygon.pop_back();
						break;
					}
				}
			}
			
			return convex_polygon;
		}
	};

	template<size DimensionCount>
	struct convex_polygon final :
		convex_polygon_internal<DimensionCount, convex_polygon<DimensionCount>>
	{
		convex_polygon() = delete;
	};

	template<>
	struct convex_polygon<2> final : convex_polygon_internal<2, convex_polygon<2>>
	{
		using vertex = two_dimensional::point;
		using edge = two_dimensional::line;

		explicit convex_polygon(const std::initializer_list<vertex>& vertices) :
			convex_polygon_internal{ vertices },
			edges_{ get_successive_point_lines(vertices_.begin(), vertices_.end()) },
			direction_{ two_dimensional::get_direction(edges_[0], vertices_[2]) },
			left_edges_{ get_edges_from_direction(left) },
			right_edges_{ get_edges_from_direction(right) } { }

		template<typename Iterator, std::enable_if_t<is_proper_vertex_iterator_v<Iterator>, int> = 0>
		explicit convex_polygon(Iterator begin, Iterator end) :
			convex_polygon_internal{ begin, end },
			edges_{ get_successive_point_lines(vertices_.begin(), vertices_.end()) },
			direction_{ two_dimensional::get_direction(edges_[0], vertices_[2]) },
			left_edges_{ get_edges_from_direction(left) },
			right_edges_{ get_edges_from_direction(right) } { }


		void operator~()
		{
			std::reverse(vertices_.begin(), vertices_.end()), direction_ = !direction_;
		}

		void operator%=(const direction direction)
		{
			if (direction != direction_) ~*this;
		}


		[[nodiscard]] friend bool operator<(
			const two_dimensional::point& point, const convex_polygon& convex_polygon)
		{
			for (const auto& edge : convex_polygon.edges_)
				if (convex_polygon.direction_ != two_dimensional::get_direction(edge, point))
					return false;

			return true;
		}

		struct edge_intersections
		{
			two_dimensional::point left;
			two_dimensional::point right;
		};

		[[nodiscard]] edge_intersections get_edge_intersections_at_y(const float y_coordinate) const
		{
			const auto y_line = two_dimensional::get_line_at_y(y_coordinate);

			two_dimensional::point max_left{ number_min, 0, 1 };
			for (const auto& left_intersection :
				get_intersections(left_edges_.begin(), left_edges_.end(), y_line))
			{
				if (left_intersection.x / left_intersection.z >
					max_left.x / max_left.z)
				{
					max_left = left_intersection;
				}
			}

			two_dimensional::point min_right{ number_max, 0, 1 };
			for (const auto& right_intersection :
				get_intersections(right_edges_.begin(), right_edges_.end(), y_line))
			{
				if (right_intersection.x / right_intersection.z < min_right.x / min_right.z)
				{
					min_right = right_intersection;
				}
			}

			return { max_left, min_right };
		}

		[[nodiscard]] std::vector<two_dimensional::point> get_all_edge_line_intersections_at_y(
			const float y_coordinate) const
		{
			return get_intersections(edges_.begin(), edges_.end(), 
				two_dimensional::get_line_at_y(y_coordinate));
		}

		[[nodiscard]] vertex get_vertex_on(const direction bottom_or_top) const
		{
			vertex result{ 0.0f, bottom_or_top ? number_max : number_min, 1.0f };

			for (const auto& vertex : vertices_)
				if (vertex.y / vertex.z < result.y / result.z == bottom_or_top) result = vertex;

			return result;
		}


		[[nodiscard]] static bool form_convex_polygon(const std::vector<vertex>& points)
		{
			if (points.size() < 3)
			{
				throw std::range_error("Expected at least three points.");
			}

			const auto direction =
				two_dimensional::get_direction(
					points[0],
					points[1],
					points[2]);

			for (size index = 1; index < points.size(); ++index)
			{
				const auto&& current_direction =
					two_dimensional::get_direction(
						points[index],
						points[(index + 1) % points.size()],
						points[(index + 2) % points.size()]);

				if (direction != current_direction) return false;
			}

			return true;
		}


	private:
		std::vector<edge> edges_;

		direction direction_;

		// stored for faster intersection calculations
		std::vector<edge> left_edges_;
		std::vector<edge> right_edges_;


		template<typename Iterator, std::enable_if_t<is_proper_vertex_iterator_v<Iterator>, int> = 0>
		[[nodiscard]] static std::vector<two_dimensional::line> get_successive_point_lines(
			const Iterator& begin, const Iterator& end)
		{
			if (static_cast<int>(std::distance(begin, end)) < 0)
				throw std::runtime_error{ "Iterator distance is less than 0." };

			std::vector<two_dimensional::line> result{ };

			auto i = begin;
			for (; i + 1 != end; ++i)
				result.emplace_back(two_dimensional::get_connecting_line(
					*i, *(i + 1)));

			result.emplace_back(two_dimensional::get_connecting_line(
				*i, *begin));
			return result;
		}

		template<typename Iterator, std::enable_if_t<is_proper_vertex_iterator_v<Iterator>, int> = 0>
		[[nodiscard]] static std::vector<two_dimensional::point> get_intersections(
			const Iterator& begin, const Iterator& end,
			const two_dimensional::line& line)
		{
			if (static_cast<int>(std::distance(begin, end)) < 0)
				throw std::runtime_error{ "Iterator distance is less than 0." };

			std::vector<two_dimensional::point> result{ static_cast<size>(std::distance(begin, end)) };

			std::transform(begin, end, result.begin(),
				[line](const irglab::two_dimensional::line& other)
				{
					return two_dimensional::get_intersection(line, other);
				});

			return result;
		}


		[[nodiscard]] std::vector<edge> get_edges_from_direction(
			const direction right_or_left) const
		{
			std::vector<edge> result{};

			for (size_t index = 0; index < vertices_.size(); ++index)
			{
				const auto& first_vertex = vertices_[index];
				const auto& second_vertex =
					vertices_[(index + 1) % vertices_.size()];

				if (first_vertex.y / first_vertex.z >
					second_vertex.y / second_vertex.z == direction_ == right_or_left)
				{
					result.emplace_back(edges_[index]);
				}
			}

			return result;
		}
	};
}

namespace irglab::two_dimensional
{
	using convex_polygon = irglab::convex_polygon<dimension_count>;
}

#endif
