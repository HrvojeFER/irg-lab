#ifndef IRGLAB_FRACTAL_APP_HPP
#define IRGLAB_FRACTAL_APP_HPP


#include "pch.hpp"

#include "app_base.hpp"
#include "primitives.hpp"


namespace irglab
{
	struct fractal_app final : app_base
	{
		explicit fractal_app() : app_base{ "Fractal" } { }

	private:
		void pre_run() override
		{
			set_screen_covering_triangle_to_draw();
		}

		void set_screen_covering_triangle_to_draw() const
		{
			static constexpr auto square_root_of_three = 1.73205080757f;
			static constexpr auto two_thirds = 2.0f / 3.0f;

			static constexpr auto x_max = two_thirds * square_root_of_three + 1;
			static constexpr auto x_mid = rational_zero;
			static constexpr auto x_min = -x_max;
			static constexpr auto y_max = -(square_root_of_three + 1);
			static constexpr auto y_min = 1.0f;
			
			artist_.set_vertices_to_draw(
				{
					{
						{ x_mid, y_max },
						{ 1.0f, 0.0f, 0.0f }
					},
					{
						{ x_min, y_min },
						{ 1.0f, 0.0f, 0.0f }
					},
					{
						{ x_max, y_min },
						{ 1.0f, 0.0f, 0.0f }
					}
				});
		}
	};
}

#endif
