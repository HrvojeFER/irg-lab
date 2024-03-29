#ifndef GRAPHICS_ARTIST_HPP
#define GRAPHICS_ARTIST_HPP


#include "../external/pch.hpp"

#include "env/Environment.hpp"
#include "env/window.hpp"

#include "../environment/device.hpp"
#include "swapchain.hpp"
#include "MemoryManager.hpp"
#include "pipeline.hpp"
#include "synchronizer.hpp"


namespace il
{
	struct artist
	{
		artist(const environment& environment, const std::shared_ptr<window>& window) :
			window_{ window },
			device_{ std::make_shared<il::device>(environment, *window) },

			swapchain_{ device(), *window },
			memory_manager_{ device_, swapchain_ },
			pipeline_{ device(), swapchain_, memory_manager_ },

			sync_
			{
				device(),
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

			image_in_flight_fence_indices_.resize(swapchain_.get_configuration_view().image_count);

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
			device()->waitForFences(
				sync_.fence(in_flight, current_frame_),
				VK_TRUE,
				UINT64_MAX); // Means there is no timeout

			unsigned int image_index;
			try
			{
				image_index = device()->acquireNextImageKHR(
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
				device()->waitForFences(
					sync_.fence(in_flight, image_in_flight_index.value()),
					VK_TRUE,
					UINT64_MAX);
			}

			image_in_flight_fence_indices_[image_index] = current_frame_;

			device()->resetFences(sync_.fence(in_flight, current_frame_));


			device().graphics_queue.submit(
				{
					{
						1,
						&sync_.semaphore(image_available, current_frame_),
						wait_stages.data(),
						1,
						&pipeline_.command_buffer(image_index),
						1,
						&sync_.semaphore(render_finished, current_frame_)
					}
				},
				sync_.fence(in_flight, current_frame_));


			vk::Result present_result;
			try
			{
				present_result = device().present_queue.presentKHR(
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
				std::cerr << "window resized." << std::endl;
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
			device()->waitIdle();
		}


		using wire = std::pair<GraphicsVertex, GraphicsVertex>;

		void set_wires_to_draw(const std::vector<wire>& wires) const
		{
			std::vector<GraphicsVertex> vertices{wires.size() * 2 };

			for (const auto& [start, end] : wires)
				vertices.emplace_back(start),
				vertices.emplace_back(end);

			set_vertices_to_draw(std::move(vertices));
		}

		void set_vertices_to_draw(std::vector<GraphicsVertex> vertices) const
		{
			memory_manager_.set_vertex_buffer(std::move(vertices));
		}


	private:
		std::weak_ptr<const window> window_;

		const std::shared_ptr<const device> device_;

		
		swapchain swapchain_;
		MemoryManager memory_manager_;
		pipeline pipeline_;

		
		static constexpr std::array<vk::PipelineStageFlags, 1> wait_stages
		{
			vk::PipelineStageFlagBits::eColorAttachmentOutput
		};

		static inline const size_t max_frames_in_flight = 2;
		size_t current_frame_ = 0;

		inline static const synchronizer<>::key in_flight{};
		std::vector<std::optional<size_t>> image_in_flight_fence_indices_{};
		inline static const synchronizer<>::key image_available{};
		inline static const synchronizer<>::key render_finished{};
		const synchronizer<> sync_;


		bool window_resized_ = false;


		
		[[nodiscard]] const device& device() const
		{
			return *device_;
		}

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

				swapchain_.reconstruct(*device_, *shared_window);
				memory_manager_.reconstruct(swapchain_);
				pipeline_.reconstruct(*device_, swapchain_, memory_manager_);

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
