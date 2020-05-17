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
	template<
	size DimensionCount, bool IsConvex, bool IsTracking>
	struct is_body_description_supported : std::bool_constant<
		(DimensionCount == 3 || DimensionCount == 2) && 
		IsTracking> {};

	template<size DimensionCount, bool IsConvex, bool IsTracking>
	inline constexpr bool is_body_description_supported_v = 
			is_body_description_supported<DimensionCount, IsConvex, IsTracking>::value;
	
	template<
	size DimensionCount, bool IsConvex, bool IsTracking, std::enable_if_t<
		is_body_description_supported_v<DimensionCount, IsConvex, IsTracking>,
	int> = 0>
	struct body
	{
		static constexpr size dimension_count = DimensionCount;
		static constexpr bool is_convex = IsConvex;
		static constexpr bool is_tracking = IsTracking;
		
		using triangle = tracking_triangle<dimension_count>;
		using shared_triangle = std::shared_ptr<triangle>;
		using vertex = typename triangle::vertex;
		using tracked_vertex = typename triangle::tracked_vertex;

		
	private:
		std::unordered_set<tracked_vertex> vertices_;
		std::unordered_set<shared_triangle> triangles_;

		
		constexpr explicit body(
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

		template<
		typename VertexIterator, typename TriangleIterator, std::enable_if_t<
			is_proper_vertex_iterator<VertexIterator> &&
			is_proper_triangle_iterator<TriangleIterator>,
		int> = 0>
		explicit body(
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
			bounds<dimension_count>& bounds, const body& body) noexcept
		{
			for (const auto& triangle : body.triangles_) bounds |= *triangle;
		}

		friend constexpr bounds<dimension_count>& operator|(
			bounds<dimension_count>& old_bounds, const body& body) noexcept
		{
			bounds<dimension_count> new_bounds{ old_bounds };
			new_bounds |= body;
			return new_bounds;
		}
		

		friend void operator+=(irglab::tracking_wireframe<dimension_count>& wireframe,
			const body& body)
		{
			for (const auto& triangle : body.triangles_) wireframe += *triangle;
		}

		friend irglab::tracking_wireframe<dimension_count>& operator+(
			irglab::tracking_wireframe<dimension_count>& old_wireframe,
			const body& body)
		{
			irglab::tracking_wireframe<dimension_count> new_wireframe{ old_wireframe };
			new_wireframe |= body;
			return new_wireframe;
		}
		
		
		friend std::ostream& operator<<(std::ostream& output_stream, const body& body)
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

		constexpr void operator*=(
			const transformation<dimension_count>& transformation) const noexcept
		{
			for (auto& triangle : triangles_) *triangle *= transformation;
		}


		
		// Convex

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void>&& is_convex == true,
			int> = 0>
			[[nodiscard]] friend bool operator<(
				const three_dimensional::point& point, const body& body)
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


		
		// Three dimensional

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && dimension_count == 3,
			int> = 0>
		[[nodiscard]] static body<3, IsConvex, true> parse(
			const std::vector<std::string>& lines)
		{
			std::vector<tracked_vertex> vertices;
			std::vector<shared_triangle> triangles;

			for (const auto& line : lines)
			{
				std::istringstream line_stream{ line };

				char first;
				line_stream >> first;

				if (first == '#')
				{
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
			}

			return body
			{
				vertices.begin(), vertices.end(),
				triangles.begin(), triangles.end()
			};
		}


		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && dimension_count == 3,
			int> = 0>
		void operator&=(const three_dimensional::bounds& bounds) noexcept
		{
			static_assert(dimension_count == 3);
			
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
		}

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && dimension_count == 3,
			int> = 0>
		void operator&=(const float limit) noexcept
		{
			return *this &= get_normalization_bounds(limit);
		}

	private:
		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void>&& dimension_count == 3,
			int> = 0>
			[[nodiscard]] static three_dimensional::bounds get_normalization_bounds(
				const number limit) noexcept
		{
			return three_dimensional::bounds
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

	template<size DimensionCount, bool IsConvex>
	using tracking_body = body<DimensionCount, IsConvex, true>;

	template<size DimensionCount, bool IsTracking>
	using concave_body = body<DimensionCount, false, IsTracking>;

	template<size DimensionCount, bool IsTracking>
	using convex_body = body<DimensionCount, true, IsTracking>;

	template<size DimensionCount>
	using convex_tracking_body = body<DimensionCount, true, true>;


	
	template<
	size DimensionCount, bool IsConvex, bool IsTracking, std::enable_if_t<
		is_body_description_supported_v<DimensionCount, IsConvex, IsTracking>,
	int> = 0>
		struct body_descriptor
	{
		static constexpr size dimension_count = DimensionCount;
		static constexpr bool is_convex = IsConvex;
		static constexpr bool is_tracking = IsTracking;
	};

	template<
	typename CheckType, typename = void>
	struct is_body_descriptor : std::false_type {};

	template<
		typename CheckType>
	struct is_body_descriptor<CheckType, std::enable_if_t<
		std::is_same_v<decltype(CheckType::dimension_count), const size> &&
		std::is_same_v<decltype(CheckType::is_convex), const bool> &&
		std::is_same_v<decltype(CheckType::is_tracking), const bool>,
		void>> : std::true_type {};

	template<typename CheckType>
	inline constexpr bool is_body_descriptor_v = is_body_descriptor<CheckType>::value;

	
	template<typename Descriptor, std::enable_if_t<is_body_descriptor_v<Descriptor>, int> = 0>
	using described_body = body<
		Descriptor::dimension_count, Descriptor::is_convex, Descriptor::is_tracking>;
	
}


namespace irglab::three_dimensional
{
	template<bool IsConvex, bool IsTracking>
	using body = irglab::body<dimension_count, IsConvex, IsTracking>;
	
	template<bool IsConvex>
	using tracking_body = irglab::tracking_body<dimension_count, IsConvex>;

	template<bool IsTracking>
	using concave_body = irglab::concave_body<dimension_count, IsTracking>;

	template<bool IsTracking>
	using convex_body = irglab::convex_body<dimension_count, IsTracking>;

	using convex_tracking_body = irglab::convex_tracking_body<dimension_count>;


	[[nodiscard]] inline convex_tracking_body operator""_body(const char* chars, size)
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

		return convex_tracking_body::parse(result);
	}
}

#endif
