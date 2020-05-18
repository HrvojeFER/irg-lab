#ifndef IRGLAB_BOUNDS_HPP
#define IRGLAB_BOUNDS_HPP


#include "pch.hpp"

#include "primitives.hpp"


namespace irglab
{
	template<small_natural_number DimensionCount>
	struct bounds { bounds() = delete; };

	template<>
	struct bounds<2>
	{
	private:
		rational_number x_min_;
		rational_number x_max_;

		rational_number y_min_;
		rational_number y_max_;


	public:
		explicit bounds (
			rational_number x_min = rational_number_max, rational_number x_max = rational_number_min, 
			rational_number y_min = rational_number_max, rational_number y_max = rational_number_min) :
			x_min_{ std::move(x_min) }, x_max_{ std::move(x_max) },
			y_min_{ std::move(y_min) }, y_max_{ std::move(y_max) } { }
		
		bounds operator|(const point<2>& point) const noexcept
		{
			auto new_bounds{ *this };
			
			if (point.x < this->x_min_) new_bounds.x_min_ = point.x;
			if (point.x > this->x_max_) new_bounds.x_max_ = point.x;

			if (point.y < this->y_min_) new_bounds.y_min_ = point.y;
			if (point.y > this->y_max_) new_bounds.y_max_ = point.y;

			return new_bounds;
		}
		
		constexpr void operator|=(const point<2>& point) noexcept
		{
			if (point.x < this->x_min_) this->x_min_ = point.x;
			if (point.x > this->x_max_) this->x_max_ = point.x;

			if (point.y < this->y_min_) this->y_min_ = point.y;
			if (point.y > this->y_max_) this->y_max_ = point.y;
		}

		
		bounds operator|(const bounds& old_bounds) const noexcept
		{
			auto new_bounds{ *this };

			if (old_bounds.x_min_ < this->x_min_) new_bounds.x_min_ = old_bounds.x_min_;
			if (old_bounds.x_max_ > this->x_max_) new_bounds.x_max_ = old_bounds.x_max_; 

			if (old_bounds.y_min_ < this->y_min_) new_bounds.y_min_ = old_bounds.y_min_;
			if (old_bounds.y_max_ > this->y_max_) new_bounds.y_max_ = old_bounds.y_max_;

			return new_bounds;
		}

		constexpr void operator|=(const bounds& bounds) noexcept
		{
			if (bounds.x_min_ < this->x_min_) this->x_min_ = bounds.x_min_;
			if (bounds.x_max_ > this->x_max_) this->x_max_ = bounds.x_max_;

			if (bounds.y_min_ < this->y_min_) this->y_min_ = bounds.y_min_;
			if (bounds.y_max_ > this->y_max_) this->y_max_ = bounds.y_max_;
		}

		
		using difference = cartesian_coordinates<2>;
		[[nodiscard]] difference get_difference() const noexcept
		{
			return { this->x_max_ - this->x_min_, this->y_max_ - this->y_min_ };
		}

		using center = cartesian_coordinates<2>;
		[[nodiscard]] center get_center() const noexcept
		{
			return
			{
				this->x_min_ + (this->x_max_ - this->x_min_) / 2.0f,
				this->y_min_ + (this->y_max_ - this->y_min_) / 2.0f
			};
		}


		friend std::ostream& operator<<(std::ostream& output_stream, const bounds& bounds)
		{
			return output_stream <<
				"Min x: " << bounds.x_min_ << "Max x: " << bounds.x_max_ << std::endl <<
				"Min y: " << bounds.y_min_ << "Max y: " << bounds.y_max_ << std::endl <<
				std::endl;
		}
	};

	template<>
	struct bounds<3>
	{
	private:
		rational_number x_min_;
		rational_number x_max_;

		rational_number y_min_;
		rational_number y_max_;

		rational_number z_min_;
		rational_number z_max_;

	public:
		explicit bounds(
			rational_number x_min = rational_number_max, rational_number x_max = rational_number_min,
			rational_number y_min = rational_number_max, rational_number y_max = rational_number_min,
			rational_number z_min = rational_number_max, rational_number z_max = rational_number_min) :

			x_min_{ std::move(x_min) }, x_max_{ std::move(x_max) },
			y_min_{ std::move(y_min) }, y_max_{ std::move(y_max) },
			z_min_{ std::move(z_min) }, z_max_{ std::move(z_max) } { }

		bounds operator|(const point<3>& point) const noexcept
		{
			auto new_bounds{ *this };
			
			if (point.x < this->x_min_) new_bounds.x_min_ = point.x;
			if (point.x > this->x_max_) new_bounds.x_max_ = point.x;

			if (point.y < this->y_min_) new_bounds.y_min_ = point.y;
			if (point.y > this->y_max_) new_bounds.y_max_ = point.y;

			if (point.z < this->z_min_) new_bounds.z_min_ = point.z;
			if (point.z > this->z_max_) new_bounds.z_max_ = point.z;

			return new_bounds;
		}

		constexpr void operator|=(const point<3>& point) noexcept
		{
			if (point.x < this->x_min_) this->x_min_ = point.x;
			if (point.x > this->x_max_) this->x_max_ = point.x;

			if (point.y < this->y_min_) this->y_min_ = point.y;
			if (point.y > this->y_max_) this->y_max_ = point.y;

			if (point.z < this->z_min_) this->z_min_ = point.z;
			if (point.z > this->z_max_) this->z_max_ = point.z;
		}


		bounds operator|(const bounds& old_bounds) const noexcept
		{
			auto new_bounds{ *this };
			
			if (old_bounds.x_min_ < this->x_min_) new_bounds.x_min_ = old_bounds.x_min_;
			if (old_bounds.x_max_ > this->x_max_) new_bounds.x_max_ = old_bounds.x_max_;

			if (old_bounds.y_min_ < this->y_min_) new_bounds.y_min_ = old_bounds.y_min_;
			if (old_bounds.y_max_ > this->y_max_) new_bounds.y_max_ = old_bounds.y_max_;

			if (old_bounds.z_min_ < this->z_min_) new_bounds.z_min_ = old_bounds.z_min_;
			if (old_bounds.z_max_ > this->z_max_) new_bounds.z_max_ = old_bounds.z_max_;

			return new_bounds;
		}

		constexpr void operator|=(const bounds& bounds) noexcept
		{
			if (bounds.x_min_ < this->x_min_) this->x_min_ = bounds.x_min_;
			if (bounds.x_max_ > this->x_max_) this->x_max_ = bounds.x_max_;

			if (bounds.y_min_ < this->y_min_) this->y_min_ = bounds.y_min_;
			if (bounds.y_max_ > this->y_max_) this->y_max_ = bounds.y_max_;

			if (bounds.z_min_ < this->z_min_) this->z_min_ = bounds.z_min_;
			if (bounds.z_max_ > this->z_max_) this->z_max_ = bounds.z_max_;
		}

		using difference = cartesian_coordinates<3>;
		[[nodiscard]] difference get_difference() const noexcept
		{
			return
			{
				this->x_max_ - this->x_min_,
				this->y_max_ - this->y_min_,
				this->z_max_ - this->z_min_
			};
		}

		using center = cartesian_coordinates<3>;
		[[nodiscard]] center get_center() const noexcept
		{
			return
			{
				this->x_min_ + (this->x_max_ - this->x_min_) / 2.0f,
				this->y_min_ + (this->y_max_ - this->y_min_) / 2.0f,
				this->z_min_ + (this->z_max_ - this->z_min_) / 2.0f
			};
		}

		friend std::ostream& operator<<(std::ostream& output_stream, const bounds& bounds)
		{
			return output_stream <<
				"Min x: " << bounds.x_min_ << "Max x: " << bounds.x_max_ << std::endl <<
				"Min y: " << bounds.y_min_ << "Max y: " << bounds.y_max_ << std::endl <<
				"Min z: " << bounds.z_min_ << "Max z: " << bounds.z_max_ << std::endl <<
				std::endl;
		}
	};
}

namespace irglab::two_dimensional
{
	using bounds = irglab::bounds<dimension_count>;
}

namespace irglab::three_dimensional
{
	using bounds = irglab::bounds<dimension_count>;
}

#endif
