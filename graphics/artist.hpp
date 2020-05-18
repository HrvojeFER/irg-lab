#ifndef GRAPHICS_ARTIST_HPP
#define GRAPHICS_ARTIST_HPP


#include "pch.hpp"
#include "extensions.hpp"

#include "environment.hpp"
#include "window.hpp"

#include "device.hpp"
#include "swapchain.hpp"
#include "memory_manager.hpp"
#include "pipeline.hpp"


namespace irglab
{
	struct artist
	{
		artist(const environment& environment, const std::shared_ptr<window>& window) : 
			window_{ window },

			device_{ environment, *window },
			memory_manager_{ device_ },
			swapchain_{ device_, *window },
			line_pipeline_
			{
				{
					vk::PrimitiveTopology::eLineList
				},
				device_,
				swapchain_,
				memory_manager_
			},
			triangle_pipeline_
			{
				{
					vk::PrimitiveTopology::eTriangleList
				},
				device_,
				swapchain_,
				memory_manager_
			},
			

			sync_
				{
					device_,
					{
						{
							in_flight_lines,
							max_frames_in_flight
						},
						{
							in_flight_triangles,
							max_frames_in_flight
						}
					},
					{
						{
							line_image_available,
							max_frames_in_flight
						},
						{
							line_render_finished,
							max_frames_in_flight
						},
						{
							triangle_image_available,
							max_frames_in_flight
						},
						{
							triangle_render_finished,
							max_frames_in_flight
						}
					}
				}
		{
			window->on_resize([&](vk::Extent2D)
				{
					window_resized_ = true;
				});

			lines_in_flight_fence_indices_.resize(swapchain_.get_configuration_view().image_count);
			
#if !defined(NDEBUG)
			std::cout << std::endl << "---- Artist done ----" << std::endl << std::endl << std::endl;
#endif
		}

		void switch_window(const std::shared_ptr<window>& window)
		{
			window_ = window;
			register_new_window(*window);
			adapt();
		}
		

		// ReSharper disable CppExpressionWithoutSideEffects
        void draw_frame()
        {
	        device_->waitForFences(
                sync_.fence(in_flight_lines, current_frame_),
                VK_TRUE,
                UINT64_MAX); // Means there is no timeout

			device_->waitForFences(
				sync_.fence(in_flight_triangles, current_frame_),
				VK_TRUE,
				UINT64_MAX);
			
			unsigned int image_index;
			try
			{
				image_index = device_->acquireNextImageKHR(
					*swapchain_,
					UINT64_MAX,
					sync_.semaphore(line_image_available, current_frame_),
					nullptr).value;
			}
#if !defined(NDEBUG)
			catch (const vk::OutOfDateKHRError& out_of_date)
#else
			catch (const vk::OutOfDateKHRError&)
#endif
			{
#if !defined(NDEBUG)
				std::cerr << out_of_date.what() << std::endl;
#endif
				
				adapt();
				return;
			}

			
			if (const auto image_in_flight_index = 
					lines_in_flight_fence_indices_[image_index]
				; image_in_flight_index.has_value())
			{
				device_->waitForFences(
					sync_.fence(in_flight_lines, image_in_flight_index.value()),
					VK_TRUE,
					UINT64_MAX);
			}
			
			if (const auto image_in_flight_index =
					triangles_in_flight_fence_indices_[image_index]
				; image_in_flight_index.has_value())
			{
				device_->waitForFences(
					sync_.fence(in_flight_lines, image_in_flight_index.value()),
					VK_TRUE,
					UINT64_MAX);
			}
			
			lines_in_flight_fence_indices_[image_index] = current_frame_;
			triangles_in_flight_fence_indices_[image_index] = current_frame_;
			
            device_->resetFences(sync_.fence(in_flight_lines, current_frame_));
			device_->resetFences(sync_.fence(in_flight_triangles, current_frame_));

			
            device_.graphics_queue.submit(
                {
	                {
	                    2,
						std::array<vk::Semaphore, 2>
	                	{
							sync_.semaphore(line_image_available, current_frame_),
	                		sync_.semaphore(triangle_image_available, current_frame_)
	                	}.data(),
	                    wait_stages.data(),
	                    1,
	                    &line_pipeline_.command_buffers()[image_index].get(),
	                    1,
	                    &sync_.semaphore(line_render_finished, current_frame_)
	                }
                },
                sync_.fence(in_flight_lines, current_frame_));

			device_.graphics_queue.submit(
				{
					{
						2,
						std::array<vk::Semaphore, 2>
						{
							sync_.semaphore(line_image_available, current_frame_),
							sync_.semaphore(triangle_image_available, current_frame_)
						}.data(),
						wait_stages.data(),
						1,
						&triangle_pipeline_.command_buffers()[image_index].get(),
						1,
						&sync_.semaphore(triangle_render_finished, current_frame_)
					}
				},
				sync_.fence(in_flight_triangles, current_frame_));
			
			vk::Result present_result;
            try
            {
				present_result = device_.present_queue.presentKHR(
					{
						2,
						std::array<vk::Semaphore, 2>
						{
							sync_.semaphore(line_render_finished, current_frame_),
							sync_.semaphore(triangle_render_finished, current_frame_)
						}.data(),
						1,
						&*swapchain_,
						&image_index,
						nullptr
					});
            }
            catch (vk::OutOfDateKHRError&)
            {
#if !defined(NDEBUG)
				std::cerr << "Swapchain is outdated." << std::endl;
#endif

				adapt();
				return;
            }
			if (present_result == vk::Result::eSuboptimalKHR)
			{
#if !defined(NDEBUG)
				std::cerr << "Swapchain is suboptimal." << std::endl;
#endif

				adapt();
				return;
			}
			
			if (window_resized_)
			{
#if !defined(NDEBUG)
				std::cerr << "Window resized." << std::endl;
#endif
				window_resized_ = false;

				adapt();
				return;
			}

			
            current_frame_ = (current_frame_ + 1) % max_frames_in_flight;
        }
		// ReSharper enable CppExpressionWithoutSideEffects

		void wait_idle() const
		{
			device_->waitIdle();
		}

		
		using wire = std::pair<graphics_vertex, graphics_vertex>;

		void set_wires_to_draw(const std::vector<wire>& wires) const
		{
			std::vector<graphics_vertex> vertices{ wires.size() * 2 };

			for (const auto& [start, end] : wires)
				vertices.emplace_back(start),
				vertices.emplace_back(end);


			set_line_vertices_to_draw(vertices);
		}

		void set_line_vertices_to_draw(const std::vector<graphics_vertex>& vertices) const
		{
			memory_manager_.set_line_buffer(vertices);
		}

		
		using triangle = std::tuple<graphics_vertex, graphics_vertex, graphics_vertex>;

		void set_triangles_to_draw(const std::vector<triangle>& triangles) const
		{
			std::vector<graphics_vertex> vertices{ triangles.size() * 3 };

			for (const auto& [first, second, third] : triangles)
				vertices.emplace_back(first),
				vertices.emplace_back(second),
				vertices.emplace_back(third);


			set_triangle_vertices_to_draw(vertices);
		}

		void set_triangle_vertices_to_draw(const std::vector<graphics_vertex>& vertices) const
		{
			memory_manager_.set_triangle_buffer(vertices);
		}
		
	private:
		std::weak_ptr<const window> window_;
		
		const device device_;
		memory_manager memory_manager_;
		swapchain swapchain_;
		pipeline line_pipeline_;
		pipeline triangle_pipeline_;

		static constexpr std::array<vk::PipelineStageFlags, 1> wait_stages
		{
			vk::PipelineStageFlagBits::eColorAttachmentOutput
		};
		
		static inline const size_t max_frames_in_flight = 2;

		inline static const synchronizer<>::key in_flight_lines{};
		inline static const synchronizer<>::key in_flight_triangles{};
		inline static const synchronizer<>::key line_image_available{};
		inline static const synchronizer<>::key triangle_image_available{};
		inline static const synchronizer<>::key line_render_finished{};
		inline static const synchronizer<>::key triangle_render_finished{};
		const synchronizer<> sync_;

		std::vector<std::optional<size_t>> lines_in_flight_fence_indices_{};
		std::vector<std::optional<size_t>> triangles_in_flight_fence_indices_{};
		bool window_resized_ = false;
        size_t current_frame_ = 0;

		
		void register_new_window(window& window)
		{
			window.on_resize([&](vk::Extent2D)
				{
					window_resized_ = true;
				});
		}
		
		void adapt()
		{
			if (!window_.expired())
			{
				const auto shared_window = window_.lock();
				
				auto window_extent = shared_window->query_extent();
				while (window_extent.width == 0 || window_extent.height == 0)
				{
					window_extent = shared_window->query_extent();
					shared_window->wait_events();
				}

				wait_idle();

				swapchain_.reconstruct(device_, *shared_window);
				line_pipeline_.reconstruct(device_, swapchain_, memory_manager_);
				triangle_pipeline_.reconstruct(device_, swapchain_, memory_manager_);

#if !defined(NDEBUG)
				std::cout << std::endl << "---- Artist adapted ----" << std::endl << 
					std::endl << std::endl;
#endif
			}
			else
			{
				throw std::runtime_error("Artist couldn't adapt because the window expired.");
			}
		}
	};
}


#endif
