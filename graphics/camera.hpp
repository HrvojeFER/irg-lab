#ifndef IRGLAB_CAMERA_HPP
#define IRGLAB_CAMERA_HPP


#include "pch.hpp"

#include "primitives.hpp"
#include "transformations.hpp"


namespace irglab
{
	template<size DimensionCount>
	struct [[maybe_unused]] camera_internal
	{
		using point = point<DimensionCount>;
		using orthonormal_base = orthonormal_base<DimensionCount>;

		point viewpoint;
		orthonormal_base viewpoint_base;

	protected:
		constexpr explicit camera_internal(
			const point& viewpoint,
			const orthonormal_base& viewpoint_base) noexcept :
			viewpoint{ viewpoint },
			viewpoint_base{ viewpoint_base }{ }
	};

	
	template<size DimensionCount>
	struct camera final : camera_internal<DimensionCount> { camera() = delete; };


	template<>
	struct [[maybe_unused]] camera<2> final : camera_internal<2>
	{
		constexpr explicit camera(
			const point& viewpoint = { 0.0f, 0.0f, 1.0f },
			const orthonormal_base& viewpoint_base = orthonormal_base{ 1.0f }) noexcept :
			camera_internal<2>{ viewpoint, viewpoint_base } { }
	};

	
	template<>
	struct [[maybe_unused]] camera<3> final : camera_internal<3>
	{
		static inline const point origin = { 0.0f, 0.0f, 0.0f, 1.0f };
		
		number projection_plane_distance;
		
		constexpr explicit camera(
			const point& viewpoint = { -0.1f, 0.1f, -2.0f, 1.0f },
			const orthonormal_base& viewpoint_base = orthonormal_base { 1.0f },
			const number projection_plane_distance = 1.0f) noexcept :
			camera_internal<3>{ viewpoint, viewpoint_base },
			projection_plane_distance{ projection_plane_distance } { }


		[[nodiscard]] three_dimensional::transformation get_view_transformation()
		{
			three_dimensional::normalize(viewpoint);

			return
				three_dimensional::get_translation(
				-viewpoint.x,
				-viewpoint.y,
				-viewpoint.z) *
				viewpoint_base;
		}

		using projection = two_dimensional::cartesian_coordinates;
		
		[[nodiscard]] projection get_projection(
			const three_dimensional::cartesian_coordinates& point) const
		{
			return
			{
				point.x * projection_plane_distance / point.z,
				point.y * projection_plane_distance / point.z
			};
		}


		void point_to(const point& point)
		{
			point_to(point, three_dimensional::axis
				{ viewpoint_base[1].x, viewpoint_base[1].y, viewpoint_base[1].z });
		}
		

		void point_to(const point& point, const three_dimensional::axis& view_down)
		{
			three_dimensional::normalize(viewpoint);


			const auto z_norm = glm::normalize(
				three_dimensional::to_cartesian_coordinates(point) -  
				three_dimensional::cartesian_coordinates
					{ viewpoint.x, viewpoint.y, viewpoint.z });

			const auto x_norm = glm::normalize(cross(view_down, z_norm));

			const auto y_norm = cross(z_norm, x_norm);

			
			viewpoint_base =
				transpose(orthonormal_base
					{
						x_norm.x, y_norm.x, z_norm.x, 0.0f,
						x_norm.y, y_norm.y, z_norm.y, 0.0f,
						x_norm.z, y_norm.z, z_norm.z, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f
					});
		}

		
		void move_inward(const number step_size)
		{
			three_dimensional::normalize(viewpoint);
			viewpoint = viewpoint + viewpoint_base[2] * step_size;
		}

		void move_outward(const number step_size)
		{
			three_dimensional::normalize(viewpoint);
			viewpoint = viewpoint - viewpoint_base[2] * step_size;
		}

		void move_right(const number step_size)
		{
			three_dimensional::normalize(viewpoint);
			viewpoint = viewpoint + viewpoint_base[0] * step_size;
		}
		
		void move_left(const number step_size)
		{
			three_dimensional::normalize(viewpoint);
			viewpoint = viewpoint - viewpoint_base[0] * step_size;
		}


		void view_up(const angle angle)
		{
			viewpoint_base = 
				three_dimensional::get_rotation(angle, viewpoint_base[0]) * viewpoint_base;
		}

		void view_down(const angle angle)
		{
			viewpoint_base = 
				three_dimensional::get_rotation(-angle, viewpoint_base[0]) * viewpoint_base;
		}

		void view_right(const angle angle)
		{
			viewpoint_base = 
				three_dimensional::get_rotation(angle, viewpoint_base[1]) * viewpoint_base;
		}

		void view_left(const angle angle)
		{
			viewpoint_base = 
				three_dimensional::get_rotation(-angle, viewpoint_base[1]) * viewpoint_base;
		}
	};
}

namespace irglab::two_dimensional
{
	using camera [[maybe_unused]] = irglab::camera<dimension_count>;
}

namespace irglab::three_dimensional
{
	using camera [[maybe_unused]] = irglab::camera<dimension_count>;
}

#endif
