#ifndef GRAPHICS_ARTIST_HPP
#define GRAPHICS_ARTIST_HPP


#include "pch.hpp"

#include "environment.hpp"
#include "window.hpp"

#include "device.hpp"
#include "swapchain.hpp"
#include "pipeline.hpp"


namespace irglab
{
	struct artist
	{
		artist(const environment& environment, window& window) :
			window_{ window },
			device_{ environment, window_ },
			swapchain_{ device_, window_ },
			pipeline_{ device_, swapchain_ },
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
			window_.on_resize([&](vk::Extent2D)
				{
					window_resized_ = true;
				});
			
#if !defined(NDEBUG)
			std::cout << std::endl << "---- Artist done ----" << std::endl << std::endl << std::endl;
#endif
		}


        void draw_frame()
        {
            if (device_->waitForFences(
                sync_.fence(in_flight, current_frame_),
                VK_TRUE,
                UINT64_MAX) // Means there is no timeout
                != vk::Result::eSuccess)
            {
                std::cerr << "Failed waiting for in flight fence." << std::endl;
                return;
            }

			unsigned int image_index;
			
			try
			{
				image_index = device_->acquireNextImageKHR(
					*swapchain_,
					UINT64_MAX,
					sync_.semaphore(image_available, current_frame_),
					nullptr).value;
			}
			catch (const vk::OutOfDateKHRError& out_of_date)
			{
#if !defined(NDEBUG)
				std::cerr << out_of_date.what() << std::endl;
#endif
				
				adapt();
				return;
			}

			
			if (const auto image_in_flight_pair = 
					image_in_flight_fence_indices_.find(image_index); 
				image_in_flight_pair != image_in_flight_fence_indices_.end())
			{
				if (device_->waitForFences(
					sync_.fence(in_flight, image_in_flight_pair->second),
					VK_TRUE,
					UINT64_MAX) != vk::Result::eSuccess)
				{
					std::cerr << "Failed waiting for in flight image fence." << std::endl;
					return;
				}
			}
			
			image_in_flight_fence_indices_[image_index] = current_frame_;
			
            device_->resetFences(sync_.fence(in_flight, current_frame_));

			std::array<vk::PipelineStageFlags, 1> wait_stages
			{
				vk::PipelineStageFlagBits::eColorAttachmentOutput
			};
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
			if (present_result == vk::Result::eErrorOutOfDateKHR)
			{
#if !defined(NDEBUG)
				std::cerr << "Swapchain is outdated." << std::endl;
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
			if	(present_result == vk::Result::eSuboptimalKHR)
			{
#if !defined(NDEBUG)
				std::cerr << "Swapchain is suboptimal." << std::endl;
#endif

				adapt();
				return;
			}
			if (present_result != vk::Result::eSuccess)
			{
				throw std::runtime_error("Failed to present image.");
			}
			
			device_.present_queue.waitIdle();
            current_frame_ = (current_frame_ + 1) % max_frames_in_flight;
        }

		void wait_idle() const
		{
			device_->waitIdle();
		}

		
	private:
		window& window_;
		const device device_;
		swapchain swapchain_;
		pipeline pipeline_;

		
		static const unsigned int max_frames_in_flight = 2;
		inline static const synchronizer::key in_flight{};
		inline static const synchronizer::key image_available{};
		inline static const synchronizer::key render_finished{};
		const synchronizer sync_;

		std::unordered_map<size_t, size_t> image_in_flight_fence_indices_{};
		bool window_resized_ = false;
        size_t current_frame_ = 0;

		void adapt()
		{
			auto window_extent = window_.query_extent();
			while (window_extent.width == 0 || window_extent.height == 0)
			{
				window_extent = window_.query_extent();
				window_.wait_events();
			}
			
			wait_idle();

			swapchain_.recreate(device_, window_);
			pipeline_.recreate(device_, swapchain_);
			
#if !defined(NDEBUG)
			std::cout << std::endl << "---- Artist adapted ----" << std::endl << std::endl << std::endl;
#endif
		}
	};
}


#endif
