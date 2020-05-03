#ifndef IRGLAB_SCENE_HPP
#define IRGLAB_SCENE_HPP


#include "pch.hpp"

#include "primitives.hpp"
#include "transformations.hpp"


namespace irglab
{
	template<size DimensionCount>
	struct scene_internal
	{
		using point = point<DimensionCount>;
		point viewpoint;

	protected:
		constexpr explicit scene_internal(const point& viewpoint) noexcept :
			viewpoint{ viewpoint } { }
	};

	
	template<size DimensionCount>
	struct scene final : scene_internal<DimensionCount> { scene() = delete; };


	template<>
	struct scene<2> final : scene_internal<2>
	{
		constexpr explicit scene(const point& viewpoint = { 0.0f, 0.0f, 1.0f }) noexcept :
			scene_internal<2>{ viewpoint } { }
	};

	
	template<>
	struct scene<3> final : scene_internal<3>
	{
		using vector = vector<3>;

		number aspect_ratio;

		using transformation = transformation<3>;
		

		constexpr explicit scene(
			const point& viewpoint = { 0.0f, 0.0f, -1.0f, 1.0f },
			const number aspect_ratio) noexcept :
			scene_internal<3>{ viewpoint },
			aspect_ratio{ aspect_ratio }{ }


		[[nodiscard]] constexpr transformation get_view_transformation() const noexcept
		{
			return
				three_dimensional::get_translation(
					-viewpoint.x,
					-viewpoint.y,
					-viewpoint.z) *
				transformation
				{
					view_right_.x, view_down_.x, view_in_.x, 0.0f,
					view_right_.y, view_down_.y, view_in_.y, 0.0f,
					view_right_.z, view_down_.z, view_in_.z, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f
				};
		}


		[[nodiscard]] constexpr transformation get_perspective_projection(
			const point& point) const noexcept
		{
			return
			{
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1 / distance(viewpoint, point),
				0.0f, 0.0f, 0.0f, 0.0f
			};
		}
		
		
	private:
		vector
			view_right_{ 1.0f, 0.0f, 0.0f },
			view_down_{ 0.0f, 1.0f, 0.0f },
			view_in_{ 0.0f, 0.0f, 1.0f };

		number min_depth_{ 0.0f }, max_depth_{ 10.0f };
		angle view_angle_ { 1.0f };
	};
}

namespace irglab::two_dimensional
{
	using scene = irglab::scene<dimension_count>;
}

namespace irglab::three_dimensional
{
	using scene = irglab::scene<dimension_count>;
}

#endif
