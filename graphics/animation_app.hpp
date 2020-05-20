#ifndef IRGLAB_ANIMATION_APP
#define IRGLAB_ANIMATION_APP



#include "pch.hpp"


#include "app_base.hpp"
#include "assets.hpp"


#include "primitives.hpp"

#include "curve.hpp"

#include "wireframe.hpp"
#include "body.hpp"

#include "camera.hpp"
#include "light_source.hpp"



namespace irglab
{
	struct [[maybe_unused]] animation_app final : app_base
	{
		explicit animation_app(
			const std::string& path_to_body_file = "./objects/cube.obj"
#if !defined(NDEBUG)
			, const std::string& path_to_reference_plane_file = "./objects/reference_plane.obj"
#endif
		) :
			app_base{ "Body" },
			body_{ three_dimensional::convex_tracking_body::parse(
				read_object_file(path_to_body_file)) }
		{
			body_.prune();
			
			body_ &= vulkan_friendly_limit;
			
#if !defined(NDEBUG)
			reference_frame_ += three_dimensional::convex_tracking_body::parse(
				read_object_file(path_to_reference_plane_file));
#endif
		}


	private:
		static inline const rational_number vulkan_friendly_limit = 0.5f;

		static inline const rational_number step_size = 0.2f;
		static inline const rational_number angle_step = 0.1f;

		static inline const rational_number frame_rate = 60.0f;
		static inline const rational_number frame_time = 1 / static_cast<float>(frame_rate);


		three_dimensional::camera camera_
		{
			{ -0.1f, 0.1f, -2.0f, 1.0f },
			three_dimensional::camera::rotation{ 1.0f },
			1.0f
		};

		three_dimensional::light_source light_source_
		{
			{ -0.1f, 0.1f, -2.0f, 1.0f }
		};
		
#if !defined(NDEBUG)
		three_dimensional::tracking_wireframe reference_frame_{};
#endif
		three_dimensional::convex_tracking_body body_;


		three_dimensional::curve curve_
		{
			{ -2.0f, 0.0f, 0.0f },
			{ -1.0f, 4.0f, -2.0f },
			{ 0.0f, -7.0f, -4.0f },
			{ 1.0f, 4.0f, -2.0f },
			{ 2.0f, 0.0f, 0.0f }
		};

		// Animations per second.
		rational_number animation_speed_ = 0.01f;
		bool did_exit_animation_ = false;
		std::chrono::time_point<std::chrono::system_clock> animation_start_{};


		void pre_run() override
		{
			animation_start_ = std::chrono::system_clock::now();
			window_->on_key_oneshot(GLFW_KEY_ESCAPE, GLFW_PRESS,
				[&]()
				{
					did_exit_animation_ = true;
					setup_movement();
				});

			set_scene_for_drawing();
		}

		void loop() override
		{
			if (!did_exit_animation_)
			{
				const auto current_animation_start =
					std::chrono::system_clock::now();

				auto curve_parameter =
					animation_speed_ *
					std::chrono::duration_cast<std::chrono::microseconds>(
						current_animation_start - animation_start_).count() / 1000000.0f;

				if (curve_parameter > 1.0f)
				{
					animation_start_ = current_animation_start;
					curve_parameter -= 1;
				}

				camera_.set_viewpoint(
					three_dimensional::to_homogeneous_coordinates(curve_(curve_parameter)));

				camera_.point_to(three_dimensional::camera::origin, { 0.0f, 1.0f, 0.0f });

				set_scene_for_drawing();

				app_base::loop();
			}
			else
			{
				app_base::loop();
			}
		}


		void setup_movement()
		{
			// Move inward
			window_->on_key(GLFW_KEY_W, GLFW_PRESS,
				[&]()
				{
					camera_.move_inward(step_size);
					if (camera_.viewpoint() < body_)
					{
#if !defined(NDEBUG)
						std::cout << "Step away from the body, you wretched beast!" << std::endl;
#endif
						camera_.move_outward(step_size);
					}
#if !defined(NDEBUG)
					else
					{
						std::cout << "You are outside of the body, as you should be." << std::endl;
					}
#endif

					set_scene_for_drawing();
				});

			// Move left
			window_->on_key(GLFW_KEY_A, GLFW_PRESS,
				[&]()
				{
					camera_.move_left(step_size);
					if (camera_.viewpoint() < body_)
					{
#if !defined(NDEBUG)
						std::cout << "Step away from the body, you wretched beast!" << std::endl;
#endif
						camera_.move_right(step_size);
					}
#if !defined(NDEBUG)
					else
					{
						std::cout << "You are outside of the body, as you should be." << std::endl;
					}
#endif

					set_scene_for_drawing();
				});

			// Move outward
			window_->on_key(GLFW_KEY_S, GLFW_PRESS,
				[&]()
				{
					camera_.move_outward(step_size);
					if (camera_.viewpoint() < body_)
					{
#if !defined(NDEBUG)
						std::cout << "Step away from the body, you wretched beast!" << std::endl;
#endif
						camera_.move_inward(step_size);
					}
#if !defined(NDEBUG)
					else
					{
						std::cout << "You are outside of the body, as you should be." << std::endl;
					}
#endif

					set_scene_for_drawing();
				});

			// Move right
			window_->on_key(GLFW_KEY_D, GLFW_PRESS,
				[&]()
				{
					camera_.move_right(step_size);
					if (camera_.viewpoint() < body_)
					{
#if !defined(NDEBUG)
						std::cout << "Step away from the body, you wretched beast!" << std::endl;
#endif
						camera_.move_left(step_size);
					}
#if !defined(NDEBUG)
					else
					{
						std::cout << "You are outside of the body, as you should be." << std::endl;
					}
#endif

					set_scene_for_drawing();
				});


			// View up
			window_->on_key(GLFW_KEY_I, GLFW_PRESS,
				[&]()
				{
					camera_.view_up(angle_step);

					set_scene_for_drawing();
				});

			// View left
			window_->on_key(GLFW_KEY_J, GLFW_PRESS,
				[&]()
				{
					camera_.view_left(angle_step);
					set_scene_for_drawing();
				});

			// View down
			window_->on_key(GLFW_KEY_K, GLFW_PRESS,
				[&]()
				{
					camera_.view_down(angle_step);
					set_scene_for_drawing();
				});

			// View right
			window_->on_key(GLFW_KEY_L, GLFW_PRESS,
				[&]()
				{
					camera_.view_right(angle_step);
					set_scene_for_drawing();
				});


			window_->on_resize(
				[&](vk::Extent2D)
				{
					set_scene_for_drawing();
				});
		}


		void set_scene_for_drawing()
		{
			three_dimensional::tracking_wireframe visible{};
#if !defined(NDEBUG)
			three_dimensional::tracking_wireframe invisible{};
#endif

			const auto view_transformation = camera_.get_view_transformation();
			const auto viewpoint_cartesian =
				three_dimensional::to_cartesian_coordinates(camera_.viewpoint());

			std::vector<graphics_vertex> triangle_vertices{  };

			for (const auto& shared_triangle : body_.triangles())
			{
				const auto triangle_first_cartesian =
					three_dimensional::to_cartesian_coordinates(shared_triangle->first());
				const auto triangle_second_cartesian =
					three_dimensional::to_cartesian_coordinates(shared_triangle->second());
				const auto triangle_third_cartesian =
					three_dimensional::to_cartesian_coordinates(shared_triangle->third());

				if (dot(
					three_dimensional::get_plane_normal(
						triangle_first_cartesian,
						triangle_second_cartesian,
						triangle_third_cartesian),

					viewpoint_cartesian -
					(triangle_first_cartesian +
						triangle_second_cartesian +
						triangle_third_cartesian) / 3.0f) > 0)
				{
					visible += *shared_triangle;

					vector<three_dimensional::dimension_count> first_normal{ 0.0f };
					vector<three_dimensional::dimension_count> second_normal{ 0.0f };
					vector<three_dimensional::dimension_count> third_normal{ 0.0f };

					auto first_normal_count = small_zero;
					auto second_normal_count = small_zero;
					auto third_normal_count = small_zero;

					for (const auto& triangle :
						*shared_triangle->first_tracked().trackers())
					{
						if (!triangle.expired())
						{
							first_normal += triangle.lock()->get_plane_normal();
							++first_normal_count;
						}
					}

					for (const auto& triangle :
						*shared_triangle->second_tracked().trackers())
					{
						if (!triangle.expired())
						{
							second_normal += triangle.lock()->get_plane_normal();
							++second_normal_count;
						}
					}

					for (const auto& triangle :
						*shared_triangle->third_tracked().trackers())
					{
						if (!triangle.expired())
						{
							third_normal += triangle.lock()->get_plane_normal();
							++third_normal_count;
						}
					}

					first_normal /= first_normal_count;
					second_normal /= second_normal_count;
					third_normal /= third_normal_count;

					const auto first_lighting =
						light_source_.get_lighting(
							camera_.viewpoint(), shared_triangle->first(), first_normal);
					const auto second_lighting =
						light_source_.get_lighting(
							camera_.viewpoint(), shared_triangle->second(), second_normal);
					const auto third_lighting =
						light_source_.get_lighting(
							camera_.viewpoint(), shared_triangle->third(), third_normal);

					auto triangle = shared_triangle->get_detached();
					triangle *= view_transformation;

					const auto first_cartesian =
						three_dimensional::to_cartesian_coordinates(triangle.first());
					const auto second_cartesian =
						three_dimensional::to_cartesian_coordinates(triangle.second());
					const auto third_cartesian =
						three_dimensional::to_cartesian_coordinates(triangle.third());

					if (first_cartesian.z > 0 && second_cartesian.z > 0 && third_cartesian.z > 0)
					{
						triangle_vertices.emplace_back(
							graphics_vertex
							{
								camera_.get_projection(first_cartesian),
								graphics_vertex::color_vector{ 0.6f, 0.0f, 1.0f } *
									first_lighting
							});

						triangle_vertices.emplace_back(
							graphics_vertex
							{
								camera_.get_projection(second_cartesian),
								graphics_vertex::color_vector{ 0.6f, 0.0f, 1.0f } *
									second_lighting
							});

						triangle_vertices.emplace_back(
							graphics_vertex
							{
								camera_.get_projection(third_cartesian),
								graphics_vertex::color_vector{ 0.6f, 0.0f, 1.0f } * 
									third_lighting
							});
					}
				}

#if !defined(NDEBUG)
				else invisible += *shared_triangle;
#endif

			}

			visible.prune();
#if !defined(NDEBUG)
			invisible.prune();
#endif


			std::vector<graphics_vertex> line_vertices{  };

#if !defined(NDEBUG)
			for (const auto& shared_wire : invisible.wires())
			{
				auto wire = shared_wire->detach();
				wire *= view_transformation;

				const auto wire_begin_cartesian =
					three_dimensional::to_cartesian_coordinates(wire.begin());
				const auto wire_end_cartesian =
					three_dimensional::to_cartesian_coordinates(wire.end());

				if (wire_begin_cartesian.z > 0 && wire_end_cartesian.z > 0)
				{
					line_vertices.emplace_back(
						graphics_vertex
						{
							camera_.get_projection(wire_begin_cartesian),
							{0.0f, 0.3f, 0.2f}
						});

					line_vertices.emplace_back(
						graphics_vertex
						{
							camera_.get_projection(wire_end_cartesian),
							{0.2f, 0.0f, 0.4f}
						});
				}
			}
#endif

			for (const auto& shared_wire : visible.wires())
			{
				auto wire = shared_wire->detach();
				wire *= view_transformation;

				const auto wire_begin_cartesian =
					three_dimensional::to_cartesian_coordinates(wire.begin());
				const auto wire_end_cartesian =
					three_dimensional::to_cartesian_coordinates(wire.end());

				const auto begin_body = body_.vertices().find(
					three_dimensional::convex_tracking_body::tracked_vertex
					{
						shared_wire->begin_tracked().inner()
					});

				const auto end_body = body_.vertices().find(
					three_dimensional::convex_tracking_body::tracked_vertex
					{
						shared_wire->end_tracked().inner()
					});

				vector<three_dimensional::dimension_count> begin_normal{ 0.0f };
				vector<three_dimensional::dimension_count> end_normal{ 0.0f };

				auto begin_normal_count = small_zero;
				auto end_normal_count = small_zero;
				
				if (begin_body != body_.vertices().end())
				{
					for (const auto& triangle : *begin_body->trackers())
					{
						if (!triangle.expired())
						{
							begin_normal += triangle.lock()->get_plane_normal();
							++begin_normal_count;
						}
					}
				}

				if (end_body != body_.vertices().end())
				{
					for (const auto& triangle : *end_body->trackers())
					{
						if (!triangle.expired())
						{
							end_normal += triangle.lock()->get_plane_normal();
							++end_normal_count;
						}
					}
				}

				begin_normal /= begin_normal_count;
				end_normal /= end_normal_count;

				const auto begin_lighting =
					light_source_.get_lighting(
						camera_.viewpoint(), wire.begin(), begin_normal);
				const auto end_lighting =
					light_source_.get_lighting(
						camera_.viewpoint(), wire.end(), end_normal);
				
				if (wire_begin_cartesian.z > 0 && wire_end_cartesian.z > 0)
				{
					line_vertices.emplace_back(
						graphics_vertex
						{
							camera_.get_projection(wire_begin_cartesian),
							begin_lighting * graphics_vertex::color_vector{1.0f, 0.6f, 0.0f}
						});

					line_vertices.emplace_back(
						graphics_vertex
						{
							camera_.get_projection(wire_end_cartesian),
							end_lighting * graphics_vertex::color_vector{1.0f, 0.6f, 0.0f}
						});
				}
			}
			
#if !defined(NDEBUG)
			for (const auto& shared_wire : reference_frame_.wires())
			{
				auto wire = shared_wire->detach();
				wire *= view_transformation;

				const auto wire_begin_cartesian =
					three_dimensional::to_cartesian_coordinates(wire.begin());
				const auto wire_end_cartesian =
					three_dimensional::to_cartesian_coordinates(wire.end());

				if (wire_begin_cartesian.z > 0 && wire_end_cartesian.z > 0)
				{
					line_vertices.emplace_back(
						graphics_vertex
						{
							camera_.get_projection(wire_begin_cartesian),
							{0.0f, 0.3f, 0.2f}
						});

					line_vertices.emplace_back(
						graphics_vertex
						{
							camera_.get_projection(wire_end_cartesian),
							{0.0f, 0.2f, 0.3f}
						});
				}
			}
#endif
			
			const auto window_extent = window_->query_extent();
			const auto aspect_ratio = window_extent.width /
				static_cast<float>(window_extent.height);

			for (auto& vertex : line_vertices) vertex.position.x /= aspect_ratio;
			for (auto& vertex : triangle_vertices) vertex.position.x /= aspect_ratio;

			artist_.set_vertices_to_draw(triangle_vertices);
			// artist_.set_vertices_to_draw(line_vertices);
		}
	};
}

#endif

