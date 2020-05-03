#ifndef GRAPHICS_BODY_APP
#define GRAPHICS_BODY_APP


#include "pch.hpp"

#include "app_base.hpp"
#include "assets.hpp"

#include "wireframe.hpp"
#include "convex_body.hpp"
#include "camera.hpp"


namespace irglab
{
	struct body_app final : app_base
	{
		explicit body_app(
			const std::string& path_to_body_file = "./objects/tetrahedron.obj",
			const std::string& path_to_reference_plane_file = "./objects/reference_plane.obj") :
			app_base("Body")
		{
			const auto body_parsing_report =
				three_dimensional::convex_body::parse(read_object_file(path_to_body_file));
			std::cout << "Body parsing report:" << std::endl <<
				body_parsing_report;

			auto body = body_parsing_report.body;

			const auto bound_fit_report = body &= vulkan_friendly_limit;
			std::cout << "Body fit report: " << std::endl <<
				bound_fit_report;
			body.normalize();
			std::cout << "Body: " << std::endl <<
				body;

			wireframe_ += body;
			wireframe_.remove_duplicate_wires();
			std::cout << "Wireframe:" << std::endl << 
				wireframe_ << std::endl;


			const auto reference_plane_parsing_report =
				three_dimensional::convex_body::parse(
					read_object_file(path_to_reference_plane_file));
			reference_frame_ += reference_plane_parsing_report.body;
		}

		
	private:
		static inline const auto vulkan_friendly_limit = 0.5f;

		static inline const auto step_size = 0.2f;
		static inline const auto angle_step = 0.1f;

		
		three_dimensional::camera camera_{};
		three_dimensional::wireframe wireframe_{};
		three_dimensional::wireframe reference_frame_{};

		
		void pre_run() override
		{
			// Move inward
			window_.on_key(GLFW_KEY_W, GLFW_PRESS,
				[&]()
				{
					camera_.move_inward(step_size);
					std::cout << "Current position: " <<
						to_string(three_dimensional::to_cartesian_coordinates(camera_.viewpoint)) <<
						std::endl;
					set_scene_for_drawing();
				});

			// Move left
			window_.on_key(GLFW_KEY_A, GLFW_PRESS,
				[&]()
				{
					camera_.move_left(step_size);
					std::cout << "Current position: " <<
						to_string(three_dimensional::to_cartesian_coordinates(camera_.viewpoint)) <<
						std::endl;
					set_scene_for_drawing();
				});
			
			// Move outward
			window_.on_key(GLFW_KEY_S, GLFW_PRESS,
				[&]()
				{
					camera_.move_outward(step_size);
					std::cout << "Current position: " <<
						to_string(three_dimensional::to_cartesian_coordinates(camera_.viewpoint)) <<
						std::endl;
					set_scene_for_drawing();
				});

			// Move right
			window_.on_key(GLFW_KEY_D, GLFW_PRESS,
				[&]()
				{
					camera_.move_right(step_size);
					std::cout << "Current position: " <<
						to_string(three_dimensional::to_cartesian_coordinates(camera_.viewpoint)) <<
						std::endl;
					set_scene_for_drawing();
				});

			
			// View up
			window_.on_key(GLFW_KEY_I, GLFW_PRESS,
				[&]()
				{
					camera_.view_up(angle_step);
					std::cout << "Current orientation: " << to_string(camera_.viewpoint_base) <<
						std::endl;
					set_scene_for_drawing();
				});

			// View left
			window_.on_key(GLFW_KEY_J, GLFW_PRESS,
				[&]()
				{
					camera_.view_left(angle_step);
					std::cout << "Current orientation: " << to_string(camera_.viewpoint_base) <<
						std::endl;
					set_scene_for_drawing();
				});

			// View down
			window_.on_key(GLFW_KEY_K, GLFW_PRESS,
				[&]()
				{
					camera_.view_down(angle_step);
					std::cout << "Current orientation: " << to_string(camera_.viewpoint_base) <<
						std::endl;
					set_scene_for_drawing();
				});

			// View right
			window_.on_key(GLFW_KEY_L, GLFW_PRESS,
				[&]()
				{
					camera_.view_right(angle_step);
					std::cout << "Current orientation: " << to_string(camera_.viewpoint_base) <<
						std::endl;
					set_scene_for_drawing();
				});

			
			set_scene_for_drawing();
			window_.show();
		}

		
		void set_scene_for_drawing()
		{
			const auto view_transformation = camera_.get_view_transformation();
			
			const auto points = wireframe_.get_points();
			const auto reference_frame_points = 
				reference_frame_.get_points();
			std::vector<vertex> vertices{  };

			
			for (const auto& wire : wireframe_.wires)
			{
				auto begin = view_transformation * wire.begin;
				auto end = view_transformation * wire.end;

				if (begin.z / begin.w > 0 || end.z / end.w > 0)
				{
					three_dimensional::normalize(begin);
					three_dimensional::normalize(end);

					vertices.emplace_back(
						vertex
						{
							camera_.get_projection(
								three_dimensional::to_cartesian_coordinates(begin)),
							{0.0f, 0.6f, 1.0f}
						});

					vertices.emplace_back(
						vertex
						{
							camera_.get_projection(
								three_dimensional::to_cartesian_coordinates(end)),
							{1.0f, 0.6f, 0.0f}
						});
				}
			}
			
			for (const auto& wire : reference_frame_.wires)
			{
				auto begin = view_transformation * wire.begin;
				auto end = view_transformation * wire.end;

				if (begin.z / begin.w > 0 || end.z / end.w > 0)
				{
					three_dimensional::normalize(begin);
					three_dimensional::normalize(end);

					vertices.emplace_back(
						vertex
						{
							camera_.get_projection(
								three_dimensional::to_cartesian_coordinates(begin)),
							{0.0f, 0.6f, 1.0f}
						});

					vertices.emplace_back(
						vertex
						{
							camera_.get_projection(
								three_dimensional::to_cartesian_coordinates(end)),
							{1.0f, 0.6f, 0.0f}
						});
				}
			}

			
			artist_.set_vertices_to_draw(vertices);
		}
	};
}

#endif
