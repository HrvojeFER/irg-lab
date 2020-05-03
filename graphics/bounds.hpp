#ifndef IRGLAB_BOUNDS_HPP
#define IRGLAB_BOUNDS_HPP


#include "pch.hpp"

#include "primitives.hpp"


namespace irglab
{
	inline const number number_max = FLT_MAX;
	inline const number number_min = -FLT_MAX;

	template<size DimensionCount>
	struct bounds { bounds() = delete; };

	template<>
	struct bounds<2>
	{
		using difference = cartesian_coordinates<2>;
		using center = cartesian_coordinates<2>;

		number x_min{ number_max };
		number x_max{ number_min };

		number y_min{ number_max };
		number y_max{ number_min };

		constexpr void operator|=(const point<2>& point) noexcept
		{
			if (point.x < this->x_min)
			{
				this->x_min = point.x;
			}
			if (point.x > this->x_max)
			{
				this->x_max = point.x;
			}

			if (point.y < this->y_min)
			{
				this->y_min = point.y;
			}
			if (point.y > this->y_max)
			{
				this->y_max = point.y;
			}
		}

		constexpr void operator|=(const bounds& bounds) noexcept
		{
			if (bounds.x_min < this->x_min)
			{
				this->x_min = bounds.x_min;
			}
			if (bounds.x_max > this->x_max)
			{
				this->x_max = bounds.x_max;
			}

			if (bounds.y_min < this->y_min)
			{
				this->y_min = bounds.y_min;
			}
			if (bounds.y_max > this->y_max)
			{
				this->y_max = bounds.y_max;
			}
		}

		[[nodiscard]] constexpr difference get_difference() const noexcept
		{
			return { this->x_max - this->x_min, this->y_max - this->y_min };
		}

		[[nodiscard]] constexpr center get_center() const noexcept
		{
			return
			{
				this->x_min + (this->x_max - this->x_min) / 2.0f,
				this->y_min + (this->y_max - this->y_min) / 2.0f
			};
		}


		friend std::ostream& operator<<(std::ostream& output_stream, const bounds& bounds)
		{
			return output_stream <<
				"Min x: " << bounds.x_min << "Max x: " << bounds.x_max << std::endl <<
				"Min y: " << bounds.y_min << "Max y: " << bounds.y_max << std::endl <<
				std::endl;
		}
	};

	template<>
	struct bounds<3>
	{
		using difference = cartesian_coordinates<3>;
		using center = cartesian_coordinates<3>;
		
		number x_min{ number_max };
		number x_max{ number_min };

		number y_min{ number_max };
		number y_max{ number_min };

		number z_min{ number_max };
		number z_max{ number_min };

		constexpr void operator|=(const point<3>& point) noexcept
		{
			if (point.x < this->x_min)
			{
				this->x_min = point.x;
			}
			if (point.x > this->x_max)
			{
				this->x_max = point.x;
			}

			if (point.y < this->y_min)
			{
				this->y_min = point.y;
			}
			if (point.y > this->y_max)
			{
				this->y_max = point.y;
			}

			if (point.z < this->z_min)
			{
				this->z_min = point.z;
			}
			if (point.z > this->z_max)
			{
				this->z_max = point.z;
			}
		}

		constexpr void operator|=(const bounds& bounds) noexcept
		{
			if (bounds.x_min < this->x_min)
			{
				this->x_min = bounds.x_min;
			}
			if (bounds.x_max > this->x_max)
			{
				this->x_max = bounds.x_max;
			}

			if (bounds.y_min < this->y_min)
			{
				this->y_min = bounds.y_min;
			}
			if (bounds.y_max > this->y_max)
			{
				this->y_max = bounds.y_max;
			}

			if (bounds.z_min < this->z_min)
			{
				this->z_min = bounds.z_min;
			}
			if (bounds.z_max > this->z_max)
			{
				this->z_max = bounds.z_max;
			}
		}

		[[nodiscard]] constexpr difference get_difference() const noexcept
		{
			return
			{
				this->x_max - this->x_min,
				this->y_max - this->y_min,
				this->z_max - this->z_min
			};
		}
		
		[[nodiscard]] constexpr center get_center() const noexcept
		{
			return
			{
				this->x_min + (this->x_max - this->x_min) / 2.0f,
				this->y_min + (this->y_max - this->y_min) / 2.0f,
				this->z_min + (this->z_max - this->z_min) / 2.0f
			};
		}

		friend std::ostream& operator<<(std::ostream& output_stream, const bounds& bounds)
		{
			return output_stream <<
				"Min x: " << bounds.x_min << "Max x: " << bounds.x_max << std::endl <<
				"Min y: " << bounds.y_min << "Max y: " << bounds.y_max << std::endl <<
				"Min z: " << bounds.z_min << "Max z: " << bounds.z_max << std::endl <<
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
