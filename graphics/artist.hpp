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
			pipeline_{ device_, swapchain_, memory_manager_ },
			sync_
				{
					device_,
					{
						{
							in_flight,
							max_frames_in_flight
						}
					},
					{
						{
							image_available,
							max_frames_in_flight
						},
						{
							render_finished,
							max_frames_in_flight
						}
					}
				}
		{
			window->on_resize([&](vk::Extent2D)
				{
					window_resized_ = true;
				});
			
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
                sync_.fence(in_flight, current_frame_),
                VK_TRUE,
                UINT64_MAX); // Means there is no timeout

			unsigned int image_index;
			try
			{
				image_index = device_->acquireNextImageKHR(
					*swapchain_,
					UINT64_MAX,
					sync_.semaphore(image_available, current_frame_),
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
					image_in_flight_fence_indices_[image_index]
				; image_in_flight_index.has_value())
			{
				device_->waitForFences(
					sync_.fence(in_flight, image_in_flight_index.value()),
					VK_TRUE,
					UINT64_MAX);
			}
			
			image_in_flight_fence_indices_[image_index] = current_frame_;
			
            device_->resetFences(sync_.fence(in_flight, current_frame_));

			
            device_.graphics_queue.submit(
                {
	                {
	                    1,
	                    &sync_.semaphore(image_available, current_frame_),
	                    wait_stages.data(),
	                    1,
	                    &pipeline_.command_buffers()[image_index].get(),
	                    1,
	                    &sync_.semaphore(render_finished, current_frame_)
	                }
                },
                sync_.fence(in_flight, current_frame_));

			
			vk::Result present_result;
            try
            {
				present_result = device_.present_queue.presentKHR(
					{
						1,
						&sync_.semaphore(render_finished, current_frame_),
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

		
		using wire = std::pair<vertex, vertex>;

		void set_wires_to_draw(const std::vector<wire>& wires) const
		{
			std::vector<vertex> vertices{ wires.size() * 2 };

			for (const auto& [start, end] : wires)
				vertices.emplace_back(start),
				vertices.emplace_back(end);

			set_vertices_to_draw(std::move(vertices));
		}

		void set_vertices_to_draw(std::vector<vertex> vertices) const
		{
			memory_manager_.set_vertex_buffer(std::move(vertices));
		}
		
		
	private:
		std::weak_ptr<const window> window_;
		
		const device device_;
		memory_manager memory_manager_;
		swapchain swapchain_;
		pipeline pipeline_;

		static constexpr std::array<vk::PipelineStageFlags, 1> wait_stages
		{
			vk::PipelineStageFlagBits::eColorAttachmentOutput
		};
		
		static inline const unsigned int max_frames_in_flight = 2;

		using synchronizer = synchronizer<true>;
		
		inline static const synchronizer::key in_flight{};
		inline static const synchronizer::key image_available{};
		inline static const synchronizer::key render_finished{};
		const synchronizer sync_;

		std::vector<std::optional<size_t>> image_in_flight_fence_indices_{ max_frames_in_flight };
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
				pipeline_.reconstruct(device_, swapchain_, memory_manager_);

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
