#ifndef GRAPHICS_SWAPCHAIN_HPP
#define GRAPHICS_SWAPCHAIN_HPP


#include "pch.hpp"

#include "device.hpp"
#include "window.hpp"


namespace irglab
{
    struct swapchain_configuration
    {
        vk::Format format;
        vk::PresentModeKHR present_mode;
        vk::Extent2D extent;
        vk::SurfaceTransformFlagBitsKHR transform_flag_bit;
        vk::ColorSpaceKHR color_space;
        unsigned int image_count;
    };
	
	struct swapchain
	{
        explicit swapchain(const device& device, const window& window) :
			swapchain_configuration_(select_swapchain_configuration(device, window)),
			swapchain_(create_swapchain(device, window, nullptr))
        {
#if !defined(NDEBUG)
            std::cout << std::endl << "-- Swapchain done --" << std::endl << std::endl;
#endif
        }

        [[nodiscard]] const vk::SwapchainKHR& operator *() const
        {
            return *swapchain_;
        }

        [[nodiscard]] swapchain_configuration get_configuration() const
        {
            return swapchain_configuration_;
        }

        void recreate(const device& device, const window& window)
        {
            swapchain_configuration_ = select_swapchain_configuration(device, window);
            swapchain_ = create_swapchain(device, window, *this->swapchain_);

#if !defined(NDEBUG)
            std::cout << std::endl << "-- Swapchain recreated --" << std::endl << std::endl;
#endif
        }
		
	private:
		swapchain_configuration swapchain_configuration_;

		vk::UniqueSwapchainKHR swapchain_;


        [[nodiscard]] static swapchain_configuration select_swapchain_configuration(
			const device& device,
            const window& window)
        {
            const auto& surface_info = device.query_surface_info(window);
            const auto swap_surface = select_swap_surface_format(surface_info.formats);

            auto image_count = surface_info.capabilities.minImageCount + 1;
            // 0 means there is no maximum
            if (surface_info.capabilities.maxImageCount > 0 &&
                image_count > surface_info.capabilities.maxImageCount)
            {
                image_count = surface_info.capabilities.maxImageCount;
            }

            const swapchain_configuration result
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

        [[nodiscard]] vk::UniqueSwapchainKHR create_swapchain(
            const device& device,
            const window& window,
            const vk::SwapchainKHR& old) const
        {
            auto result = device->createSwapchainKHRUnique(
                {
                    {},
                    window.drawing_surface(),
                    swapchain_configuration_.image_count,
                    swapchain_configuration_.format,
                    swapchain_configuration_.color_space,
                    swapchain_configuration_.extent,
                    1,
                    vk::ImageUsageFlagBits::eColorAttachment,
                    device.queue_family_indices.are_all_unique() ?
                        vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
                    device.queue_family_indices.are_all_unique() ?
                        static_cast<unsigned int>(device.queue_family_indices.to_vector().size()) : 0,
                    device.queue_family_indices.are_all_unique() ?
                        device.queue_family_indices.to_vector().data() : nullptr,
                    swapchain_configuration_.transform_flag_bit,
                    vk::CompositeAlphaFlagBitsKHR::eOpaque,
                    swapchain_configuration_.present_mode,
                    VK_TRUE,
                    old
                });

#if !defined(NDEBUG)
            std::cout << "Swapchain created" << std::endl;
#endif

            return result;
        }
	};
}

#endif
