#ifndef IRGLAB_THREE_DIMENSIONAL_TRIANGLE_HPP
#define IRGLAB_THREE_DIMENSIONAL_TRIANGLE_HPP

#include "pch.hpp"

#include "three_dimensional_primitives.hpp"


namespace irglab::three_dimensional
{
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
