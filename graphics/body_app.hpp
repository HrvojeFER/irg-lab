#ifndef GRAPHICS_BODY_APP
#define GRAPHICS_BODY_APP


#include "pch.hpp"

#include "app_base.hpp"
#include "assets.hpp"

#include "wireframe.hpp"

#include "convex_body.hpp"


namespace irglab
{
	struct body_app final : app_base
	{
		explicit body_app(const std::string& path_to_object_file = "./objects/tetrahedron.obj") :
			app_base("Body"),
			body_{ get_body(path_to_object_file) } { }

	private:
		static inline const auto vulkan_friendly_limit = 1.0f;

		[[nodiscard]] static three_dimensional::convex_body get_body(
			const std::string& path_to_object_file)
		{
			const auto body_parsing_report =
				three_dimensional::convex_body::parse(read_object_file(path_to_object_file));
			std::cout << "Body parsing report:" << std::endl << 
				body_parsing_report;
			auto body = body_parsing_report.body;
			
			const auto bound_fit_report = body &= vulkan_friendly_limit;
			std::cout << "Body fit report: " << std::endl << 
				bound_fit_report;

			body.normalize();
			return body;
		}

		three_dimensional::convex_body body_;

		void pre_run() override
		{
			std::cout << "Welcome to the Body App (TM) where we draw a convex_body and " <<
				"poke holes in it to see if a point is inside it. :)" << std::endl;
			std::cout << "Just kidding, haha. We don't poke holes into bodies. " <<
				"We take the normal of each plane of each polytope on the surface of the convex_body and " <<
				"determine whether or not a point is inside it using the magic of linear algebra." <<
				std::endl;
			std::cout << "You got that? It is EXTREMELY important to get what I said " <<
				"in order to proceed. Please press any button to confirm..." << std::endl <<
				std::endl;

			std::cin.get();
			
			set_body_for_drawing();
			window_.show();
			artist_.draw_frame();
			
			std::cout << "The last part wasn't actually important to understand at all, haha." << 
				std::endl;
			std::cout << "It was constructed in an overly complicated manner " <<
				"so you wouldn't get it, haha." << std::endl;
			std::cout << "Even I don't know if I truly understand it, haha." << std::endl;
			std::cout << "Ok, moving on..." << std::endl;
			std::cout << "There should be another window with a convex_body drawn on it, so " <<
				"all you need to do is write down the cartesian coordinates of a single point" <<
				"in three dimensions using floating point values in the range of <-1, 1> " << 
				"separated by spaces, so that I can calculate " << 
				"whether or not that point is in the convex_body." <<
				std::endl;
			std::cout << "Simple, right?" << std::endl;
			std::cout << "Ok, enter the coordinates below and press enter." << std::endl << std::endl;


			three_dimensional::point point;

			std::cin >> point.x >> point.y >> point.z;

			if (point < body_)
			{
				std::cout << "The point is inside the body." << std::endl;
			}
			else
			{
				std::cout << "The point is not inside the body." << std::endl;
			}

			std::cout << std::endl << "Thank you for your cooperation. :) ";
		}
		
		void set_body_for_drawing() const
		{
			three_dimensional::wireframe wireframe{};
			wireframe += body_;
			wireframe.remove_duplicate_wires();

			std::cout << wireframe << std::endl;
			
			const auto points = wireframe.get_points();
			std::vector<vertex> vertices{ points.size() };
			std::transform(points.begin(), points.end(), vertices.begin(),
				[] (const three_dimensional::point& point) -> vertex
				{
					return
					{
						two_dimensional::to_cartesian_coordinates(point),
						{1.0f, 0.6f, 0.0f}
					};
				});

			artist_.set_vertices_to_draw(vertices);
		}
	};
}

#endif
