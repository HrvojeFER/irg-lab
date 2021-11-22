#ifndef GRAPHICS_SWAPCHAIN_HPP
#define GRAPHICS_SWAPCHAIN_HPP


#include "../external/pch.hpp"

#include "../environment/device.hpp"
#include "env/window.hpp"


namespace il
{
	struct swapchain
	{
		struct configuration
		{
			vk::Format format;
			vk::PresentModeKHR present_mode;
			vk::Extent2D extent;
			vk::SurfaceTransformFlagBitsKHR transform_flag_bit;
			vk::ColorSpaceKHR color_space;
			unsigned int image_count;
		};

		explicit swapchain(const device& device, const window& window) :
			swapchain_configuration_(select_swapchain_configuration(device, window)),
			inner_(create_inner(device, window))
		{
#if !defined(NDEBUG)
			std::cout << std::endl << "-- Swapchain done --" << std::endl << std::endl;
#endif
		}

		[[nodiscard]] const vk::SwapchainKHR& operator *() const
		{
			return *inner_;
		}

		[[nodiscard]] configuration get_configuration() const
		{
			return swapchain_configuration_;
		}

		[[nodiscard]] const configuration& get_configuration_view() const
		{
			return swapchain_configuration_;
		}

		void reconstruct(const device& device, const window& window)
		{
			swapchain_configuration_ = select_swapchain_configuration(device, window);
			inner_ = create_inner(device, window, *this->inner_);

#if !defined(NDEBUG)
			std::cout << std::endl << "-- Swapchain reconstructed --" << std::endl << std::endl;
#endif
		}

	private:
		configuration swapchain_configuration_;

		vk::UniqueSwapchainKHR inner_;


		[[nodiscard]] static configuration select_swapchain_configuration(
			const device& device,
			const window& window)
		{
			const auto& surface_info = device._query_surface_info(window);
			const auto swap_surface = select_swap_surface_format(surface_info.formats);

			auto image_count = surface_info.capabilities.minImageCount + 1;
			// 0 means there is no maximum
			if (surface_info.capabilities.maxImageCount > 0 &&
				image_count > surface_info.capabilities.maxImageCount)
			{
				image_count = surface_info.capabilities.maxImageCount;
			}

			const configuration result
			{
				swap_surface.format,
				select_swap_present_mode(surface_info.present_modes),
				select_swap_extent(surface_info.capabilities, window),
				surface_info.capabilities.currentTransform,
				swap_surface.colorSpace,
				image_count
			};
#if !defined(NDEBUG)
			std::cout << "Swapchain configuration selected" << std::endl;
#endif

			return result;
		}

		[[nodiscard]] static vk::SurfaceFormatKHR select_swap_surface_format(
			const std::vector<vk::SurfaceFormatKHR>& available_formats)
		{
			for (const auto& available_format : available_formats)
			{
				if (available_format.format == vk::Format::eB8G8R8A8Srgb &&
					available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
				{
					return available_format;
				}
			}

#if !defined(NDEBUG)
			std::cerr
				<< "Failed to find a desirable drawing_surface format. "
				<< "The first available format will be selected instead."
				<< std::endl;
#endif

			return available_formats[0];
		}

		[[nodiscard]] static vk::PresentModeKHR select_swap_present_mode(
			const std::vector<vk::PresentModeKHR>& available_present_modes) noexcept
		{
			for (const auto& available_present_mode : available_present_modes)
			{
				if (available_present_mode == vk::PresentModeKHR::eMailbox)
				{
#if !defined(NDEBUG)
					std::cout << "Triple buffering available" << std::endl;
#endif
					return available_present_mode;
				}
			}

#if !defined(NDEBUG)
			std::cerr
				<< "Triple buffering unavailable -> "
				<< "Using standard FIFO presentation mode (Vsync)"
				<< std::endl;
#endif
			return vk::PresentModeKHR::eFifo;
		}

		[[nodiscard]] static vk::Extent2D select_swap_extent(
			const vk::SurfaceCapabilitiesKHR& capabilities,
			const window& window) noexcept
		{
			if (capabilities.currentExtent.width != UINT32_MAX)
			{
				return capabilities.currentExtent;
			}

			auto actual_extent = window.query_extent();

			// Clamps the extent to the capabilities of Vulkan.
			actual_extent.width = max(capabilities.minImageExtent.width,
			                          min(capabilities.maxImageExtent.width, actual_extent.width));
			actual_extent.height = max(capabilities.minImageExtent.height,
			                           min(capabilities.maxImageExtent.height, actual_extent.height));

			return actual_extent;
		}

		[[nodiscard]] vk::UniqueSwapchainKHR create_inner(
			const device& device,
			const window& window,
			const std::optional<vk::SwapchainKHR>& old = std::nullopt) const
		{
			vk::SwapchainCreateInfoKHR create_info =
			{
				{},
				window.drawing_surface(),
				swapchain_configuration_.image_count,
				swapchain_configuration_.format,
				swapchain_configuration_.color_space,
				swapchain_configuration_.extent,
				1,
				vk::ImageUsageFlagBits::eColorAttachment,
				vk::SharingMode::eExclusive,
				0,
				nullptr,
				swapchain_configuration_.transform_flag_bit,
				vk::CompositeAlphaFlagBitsKHR::eOpaque,
				swapchain_configuration_.present_mode,
				VK_TRUE,
				old.has_value() ? old.value() : nullptr
			};

			const std::unordered_set<unsigned int> sharing_queue_family_indices_set
			{
				device.queue_family_indices.graphics_family.value(),
				device.queue_family_indices.present_family.value()
			};

			if (sharing_queue_family_indices_set.size() > 1)
			{
				create_info.imageSharingMode = vk::SharingMode::eConcurrent;

				const std::vector<unsigned int> sharing_queue_family_indices_array
				{
					sharing_queue_family_indices_set.begin(),
					sharing_queue_family_indices_set.end()
				};
				create_info.queueFamilyIndexCount = static_cast<unsigned int>(
					sharing_queue_family_indices_array.size());
			}

			auto result =
				device->createSwapchainKHRUnique(create_info);

#if !defined(NDEBUG)
			std::cout << "Swapchain created" << std::endl;
#endif

			return result;
		}
	};
}

#endif
