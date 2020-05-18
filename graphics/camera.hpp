#ifndef IRGLAB_CAMERA_HPP
#define IRGLAB_CAMERA_HPP


#include "pch.hpp"

#include "primitives.hpp"
#include "transformations.hpp"


namespace irglab
{
	template<small_natural_number DimensionCount>
	struct [[maybe_unused]] is_camera_description_supported :
		are_primitive_operations_supported<DimensionCount> {};

	template<small_natural_number DimensionCount>
	[[maybe_unused]] inline constexpr bool is_camera_description_supported_v =
		is_camera_description_supported<DimensionCount>::value;

	
	template<small_natural_number DimensionCount, std::enable_if_t<
		is_camera_description_supported_v<DimensionCount>,
	int> = 0>
	struct [[maybe_unused]] camera final
	{
		static constexpr small_natural_number dimension_count = DimensionCount;
		
		using point = point<dimension_count>;
		using rotation = orthonormal_base<dimension_count>;

		static inline const point origin{ 0.0f, 0.0f, 0.0f, 1.0f };

	private:
		point viewpoint_;
		rotation rotation_;
		rational_number projection_plane_distance_;

	public:
		[[nodiscard]] const point& viewpoint() const
		{
			return viewpoint_;
		}

		void set_viewpoint(point new_viewpoint)
		{
			viewpoint_ = std::move(new_viewpoint);
		}

		
		constexpr explicit camera(
			point viewpoint,
			rotation viewpoint_base,
			const rational_number projection_plane_distance) noexcept :

			viewpoint_{ std::move(viewpoint) },
			rotation_{ std::move(viewpoint_base) },
			projection_plane_distance_{ projection_plane_distance } { }


		
		// Three dimensional

		// Transformations
		
		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && dimension_count == three_dimensional::dimension_count,
			int> = 0>
		[[nodiscard]] three_dimensional::transformation get_view_transformation()
		{
			three_dimensional::normalize(viewpoint_);

			return
				three_dimensional::get_translation(
				-viewpoint_.x,
				-viewpoint_.y,
				-viewpoint_.z) *
				rotation_;
		}

		using projection = two_dimensional::cartesian_coordinates;

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && dimension_count == three_dimensional::dimension_count,
			int> = 0>
		[[nodiscard]] projection get_projection(
			const three_dimensional::cartesian_coordinates& point) const
		{
			return
			{
				point.x * projection_plane_distance_ / point.z,
				point.y * projection_plane_distance_ / point.z
			};
		}


		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void>&& dimension_count == three_dimensional::dimension_count,
			int> = 0>
		void point_to(const point& point)
		{
			this->point_to(point, three_dimensional::axis
				{ rotation_[1].x, rotation_[1].y, rotation_[1].z });
		}
		
		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void>&& dimension_count == three_dimensional::dimension_count,
			int> = 0>
		void point_to(const point& point, const three_dimensional::axis& view_down)
		{
			three_dimensional::normalize(viewpoint_);


			const auto z_norm = glm::normalize(
				three_dimensional::to_cartesian_coordinates(point) -  
				three_dimensional::cartesian_coordinates
					{ viewpoint_.x, viewpoint_.y, viewpoint_.z });

			const auto x_norm = glm::normalize(cross(view_down, z_norm));

			const auto y_norm = cross(z_norm, x_norm);

			
			rotation_ =
				transpose(rotation
					{
						x_norm.x, y_norm.x, z_norm.x, 0.0f,
						x_norm.y, y_norm.y, z_norm.y, 0.0f,
						x_norm.z, y_norm.z, z_norm.z, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f
					});
		}


		// Movement
		
		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void>&& dimension_count == three_dimensional::dimension_count,
			int> = 0>
		void move_inward(const rational_number step_size)
		{
			three_dimensional::normalize(viewpoint_);
			viewpoint_ = viewpoint_ + rotation_[2] * step_size;
		}

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void>&& dimension_count == three_dimensional::dimension_count,
			int> = 0>
		void move_outward(const rational_number step_size)
		{
			three_dimensional::normalize(viewpoint_);
			viewpoint_ = viewpoint_ - rotation_[2] * step_size;
		}

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void>&& dimension_count == three_dimensional::dimension_count,
			int> = 0>
		void move_right(const rational_number step_size)
		{
			three_dimensional::normalize(viewpoint_);
			viewpoint_ = viewpoint_ + rotation_[0] * step_size;
		}

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void>&& dimension_count == three_dimensional::dimension_count,
			int> = 0>
		void move_left(const rational_number step_size)
		{
			three_dimensional::normalize(viewpoint_);
			viewpoint_ = viewpoint_ - rotation_[0] * step_size;
		}


		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void>&& dimension_count == three_dimensional::dimension_count,
			int> = 0>
		void view_up(const angle angle)
		{
			rotation_ = 
				three_dimensional::get_rotation(angle, rotation_[0]) * rotation_;
		}

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void>&& dimension_count == three_dimensional::dimension_count,
			int> = 0>
		void view_down(const angle angle)
		{
			rotation_ = 
				three_dimensional::get_rotation(-angle, rotation_[0]) * rotation_;
		}

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void>&& dimension_count == three_dimensional::dimension_count,
			int> = 0>
		void view_right(const angle angle)
		{
			rotation_ = 
				three_dimensional::get_rotation(angle, rotation_[1]) * rotation_;
		}
		
		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void>&& dimension_count == three_dimensional::dimension_count,
			int> = 0>
		void view_left(const angle angle)
		{
			rotation_ = 
				three_dimensional::get_rotation(-angle, rotation_[1]) * rotation_;
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
