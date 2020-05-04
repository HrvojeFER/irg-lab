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
			const std::string& path_to_body_file = "./objects/cube.obj",
			const std::string& path_to_reference_plane_file = "./objects/reference_plane.obj") :
			app_base{ "Body" },
			body_{ three_dimensional::convex_body::parse(
				read_object_file(path_to_body_file)).body }
		{
			body_ &= vulkan_friendly_limit;
			reference_frame_ += three_dimensional::convex_body::parse(
				read_object_file(path_to_reference_plane_file)).body;
		}

		
	private:
		static inline const auto vulkan_friendly_limit = 0.5f;

		static inline const auto step_size = 0.2f;
		static inline const auto angle_step = 0.1f;

		
		three_dimensional::camera camera_{};
		three_dimensional::wireframe reference_frame_{};
		three_dimensional::convex_body body_;

		
		void pre_run() override
		{
			// Move inward
			window_.on_key(GLFW_KEY_W, GLFW_PRESS,
				[&]()
				{
					camera_.move_inward(step_size);
					if (camera_.viewpoint < body_) camera_.move_outward(step_size);
				
					set_scene_for_drawing();
				});

			// Move left
			window_.on_key(GLFW_KEY_A, GLFW_PRESS,
				[&]()
				{
					camera_.move_left(step_size);
					if (camera_.viewpoint < body_) camera_.move_right(step_size);
				
					set_scene_for_drawing();
				});
			
			// Move outward
			window_.on_key(GLFW_KEY_S, GLFW_PRESS,
				[&]()
				{
					camera_.move_outward(step_size);
					if (camera_.viewpoint < body_) camera_.move_inward(step_size);
				
					set_scene_for_drawing();
				});

			// Move right
			window_.on_key(GLFW_KEY_D, GLFW_PRESS,
				[&]()
				{
					camera_.move_right(step_size);
					if (camera_.viewpoint < body_) camera_.move_left(step_size);
				
					set_scene_for_drawing();
				});

			
			// View up
			window_.on_key(GLFW_KEY_I, GLFW_PRESS,
				[&]()
				{
					camera_.view_up(angle_step);
				
					set_scene_for_drawing();
				});

			// View left
			window_.on_key(GLFW_KEY_J, GLFW_PRESS,
				[&]()
				{
					camera_.view_left(angle_step);
					set_scene_for_drawing();
				});

			// View down
			window_.on_key(GLFW_KEY_K, GLFW_PRESS,
				[&]()
				{
					camera_.view_down(angle_step);
					set_scene_for_drawing();
				});

			// View right
			window_.on_key(GLFW_KEY_L, GLFW_PRESS,
				[&]()
				{
					camera_.view_right(angle_step);
					set_scene_for_drawing();
				});

			
			set_scene_for_drawing();
			window_.show();
		}

		
		void set_scene_for_drawing()
		{
			three_dimensional::wireframe visible{};
			three_dimensional::wireframe invisible{};

			const auto viewpoint_cartesian =
				three_dimensional::to_cartesian_coordinates(camera_.viewpoint);
			
			for (const auto& triangle : body_.triangles)
			{
				const auto triangle_first_cartesian =
					three_dimensional::to_cartesian_coordinates(triangle.first);
				const auto triangle_second_cartesian =
					three_dimensional::to_cartesian_coordinates(triangle.second);
				const auto triangle_third_cartesian =
					three_dimensional::to_cartesian_coordinates(triangle.third);
				
				if (dot(
					three_dimensional::get_plane_normal(
						triangle_first_cartesian,
						triangle_second_cartesian,
						triangle_third_cartesian),

					viewpoint_cartesian -
					(triangle_first_cartesian +
						triangle_second_cartesian +
						triangle_third_cartesian) / 3.0f) > 0) visible += triangle;

				else invisible += triangle;

			}

			visible.remove_duplicate_wires();
			invisible.remove_duplicate_wires();

			
			const auto view_transformation = camera_.get_view_transformation();
			std::vector<vertex> vertices{  };
			
			for (const auto& wire : invisible.wires)
			{
				auto begin = view_transformation * wire.begin;
				auto end = view_transformation * wire.end;

				if (begin.z / begin.w > 0 && end.z / end.w > 0)
				{
					vertices.emplace_back(
						vertex
						{
							camera_.get_projection(
								three_dimensional::to_cartesian_coordinates(begin)),
							{0.0f, 0.3f, 0.2f}
						});

					vertices.emplace_back(
						vertex
						{
							camera_.get_projection(
								three_dimensional::to_cartesian_coordinates(end)),
							{0.2f, 0.0f, 0.4f}
						});
				}
			}

			for (const auto& wire : visible.wires)
			{
				auto begin = view_transformation * wire.begin;
				auto end = view_transformation * wire.end;

				if (begin.z / begin.w > 0 && end.z / end.w > 0)
				{
					vertices.emplace_back(
						vertex
						{
							camera_.get_projection(
								three_dimensional::to_cartesian_coordinates(begin)),
							{1.0f, 0.6f, 0.0f}
						});

					vertices.emplace_back(
						vertex
						{
							camera_.get_projection(
								three_dimensional::to_cartesian_coordinates(end)),
							{0.8f, 1.0f, 0.3f}
						});
				}
			}

			for (const auto& wire : reference_frame_.wires)
			{
				auto begin = view_transformation * wire.begin;
				auto end = view_transformation * wire.end;

				if (begin.z / begin.w > 0 && end.z / end.w > 0)
				{
					vertices.emplace_back(
						vertex
						{
							camera_.get_projection(
								three_dimensional::to_cartesian_coordinates(begin)),
							{0.0f, 0.6f, 0.4f}
						});

					vertices.emplace_back(
						vertex
						{
							camera_.get_projection(
								three_dimensional::to_cartesian_coordinates(end)),
							{0.0f, 0.4f, 0.6f}
						});
				}
			}

			
			artist_.set_vertices_to_draw(vertices);
		}
	};
}

#endif
