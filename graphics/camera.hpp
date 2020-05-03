#ifndef IRGLAB_SCENE_HPP
#define IRGLAB_SCENE_HPP


#include "pch.hpp"

#include "primitives.hpp"
#include "transformations.hpp"


namespace irglab
{
	template<size DimensionCount>
	struct camera_internal
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
	struct camera<2> final : camera_internal<2>
	{
		constexpr explicit camera(
			const point& viewpoint = { 0.0f, 0.0f, 1.0f },
			const orthonormal_base& viewpoint_base = orthonormal_base{ 1.0f }) noexcept :
			camera_internal<2>{ viewpoint, viewpoint_base } { }
	};

	
	template<>
	struct camera<3> final : camera_internal<3>
	{
		constexpr explicit camera(
			const point& viewpoint = { 0.0f, 0.2f, -2.0f, 1.0f },
			const orthonormal_base& viewpoint_base = orthonormal_base{ 1.0f }) noexcept :
			camera_internal<3>{ viewpoint, viewpoint_base } { }


		[[nodiscard]] three_dimensional::transformation get_view_transformation()
		{
			three_dimensional::normalize(viewpoint);

			return
				transpose(viewpoint_base) *
				three_dimensional::get_translation(
				-viewpoint.x,
				-viewpoint.y,
				-viewpoint.z);
		}

		using projection = two_dimensional::cartesian_coordinates;
		
		[[nodiscard]] projection get_projection(
			const three_dimensional::cartesian_coordinates& point)
		{
			three_dimensional::normalize(viewpoint);
			const auto point_distance = 
				distance({ viewpoint.x, viewpoint.y, viewpoint.z }, point);

			return
			{
				point.x * point_distance / point.z,
				point.y * point_distance / point.z
			};
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
				three_dimensional::get_rotation(-angle, viewpoint_base[0]) * viewpoint_base;
		}

		void view_down(const angle angle)
		{
			viewpoint_base = 
				three_dimensional::get_rotation(angle, viewpoint_base[0]) * viewpoint_base;
		}

		void view_right(const angle angle)
		{
			viewpoint_base = 
				three_dimensional::get_rotation(-angle, viewpoint_base[1]) * viewpoint_base;
		}

		void view_left(const angle angle)
		{
			viewpoint_base = 
				three_dimensional::get_rotation(angle, viewpoint_base[1]) * viewpoint_base;
		}
	};
}

namespace irglab::two_dimensional
{
	using camera = irglab::camera<dimension_count>;
}

namespace irglab::three_dimensional
{
	using camera = irglab::camera<dimension_count>;
}

#endif
