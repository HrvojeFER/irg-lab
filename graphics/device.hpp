#ifndef GRAPHICS_DEVICE_HPP
#define GRAPHICS_DEVICE_HPP


#include "pch.hpp"

#include "debug_manager.hpp"
#include "environment.hpp"
#include "window.hpp"


namespace irglab
{
    struct queue_family_indices
    {
        std::optional<unsigned int> graphics_family;
        std::optional<unsigned int> present_family;
        std::optional<unsigned int> transfer_family;

        [[nodiscard]] bool is_complete() const noexcept
        {
            return graphics_family.has_value()
        		&& present_family.has_value()
        		&& transfer_family.has_value();
        }

        [[nodiscard]] std::vector<unsigned int> to_vector() const
        {
            return std::vector<unsigned int>
            {
                graphics_family.value(),
                present_family.value(),
            	transfer_family.value()
            };
        }

        [[nodiscard]] bool are_all_unique() const
        {
            return to_unordered_set().size() == 3;
        }

        [[nodiscard]] std::unordered_set<unsigned int> to_unordered_set() const
        {
            return
            {
                graphics_family.value(),
                present_family.value(),
            	transfer_family.value()
            };
        }
    };

    struct device_surface_info
    {
        const vk::SurfaceCapabilitiesKHR capabilities;
        const std::vector<vk::SurfaceFormatKHR> formats;
        const std::vector<vk::PresentModeKHR> present_modes;
    };

	class device
	{
        const std::array<std::string, 1> required_device_extension_names_
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        const vk::PhysicalDevice physical_;

	public:
        const queue_family_indices queue_family_indices;

    private:
        const vk::UniqueDevice inner_;
		
	public:
        const vk::Queue graphics_queue;
        const vk::Queue present_queue;
        const vk::Queue transfer_queue;

		
		explicit device(
            const environment& environment,
            const window& window) :

			physical_(select_physical_device(environment, window)),
			queue_family_indices(query_queue_families(physical_, window)),
			inner_(create_inner(environment, debug_manager::validation_layer_names)),
			graphics_queue(create_queue(queue_family_indices.graphics_family.value())),
			present_queue(create_queue(queue_family_indices.present_family.value())),
			transfer_queue(create_queue(queue_family_indices.transfer_family.value()))
		{
#if !defined(NDEBUG)
            std::cout << "Queues created" << std::endl;
            std::cout << std::endl << "-- Device done --" << std::endl << std::endl;
#endif
		}

		
        [[nodiscard]] const vk::Device& operator *() const
		{
            return *inner_;
		}

        [[nodiscard]] const vk::Device* operator ->() const
		{
            return &*inner_;
		}

		[[nodiscard]] const vk::PhysicalDevice& physical() const
		{
            return physical_;
		}

        [[nodiscard]] device_surface_info query_surface_info(const window& window) const
		{
            return query_surface_info(physical_, window);
		}

		
	private:
        [[nodiscard]] vk::PhysicalDevice select_physical_device(
            const environment& environment,
            const window& window) const
        {
            auto devices = environment.vulkan_instance().enumeratePhysicalDevices();

            for (const auto& device : devices)
            {
                if (device_suitable(device, window))
                {
#if !defined(NDEBUG)
                    std::cout << "Physical device selected" << std::endl;
#endif

                    return device;
                }
            }

            throw std::runtime_error("Failed to find a suitable GPU.");
        }

        [[nodiscard]] bool device_suitable(
            const vk::PhysicalDevice& device,
            const window& surface) const
        {
            if (!device_extensions_supported(device)) return false;

            const auto swap_chain_support_info = query_surface_info(device, surface);

            return query_queue_families(device, surface).is_complete()
                && !swap_chain_support_info.formats.empty()
                && !swap_chain_support_info.present_modes.empty();
        }

        [[nodiscard]] bool device_extensions_supported(const vk::PhysicalDevice& device) const
        {
            auto available_extension_properties =
                device.enumerateDeviceExtensionProperties();

            for (const auto& required_extension_name : required_device_extension_names_)
            {
                auto found = false;
                for (auto available : available_extension_properties)
                {
                    if (strcmp(required_extension_name.c_str(), available.extensionName) == 0)
                    {
                        found = true;
                        break;
                    }
                }

                if (!found) return false;
            }

            return true;
        }

        [[nodiscard]] static irglab::queue_family_indices query_queue_families(
            const vk::PhysicalDevice& device,
            const window& window)
        {
            irglab::queue_family_indices indices;

            auto queue_family_index = 0;
            for (const auto& queue_family : device.getQueueFamilyProperties())
            {
                if (device.getSurfaceSupportKHR(queue_family_index, window.drawing_surface()) == VK_TRUE)
                {
                    indices.present_family = queue_family_index;
                }

                if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics)
                {
                    indices.graphics_family = queue_family_index;
                }

            	if (queue_family.queueFlags & vk::QueueFlagBits::eTransfer)
            	{
                    indices.transfer_family = queue_family_index;
            	}

                if (indices.is_complete())
                {
                    break;
                }

                queue_family_index++;
            }

            return indices;
        }

        [[nodiscard]] static device_surface_info query_surface_info(
            const vk::PhysicalDevice& device,
			const window& window)
        {
            return
            {
                device.getSurfaceCapabilitiesKHR(window.drawing_surface()),
                device.getSurfaceFormatsKHR(window.drawing_surface()),
                device.getSurfacePresentModesKHR(window.drawing_surface())
            };
        }

        [[nodiscard]] vk::UniqueDevice create_inner(
            const environment& environment,
            const std::array<const char*, 1>& validation_layer_names) const
        {
            std::vector<vk::DeviceQueueCreateInfo> queues_create_info{};

            const auto queue_priority = 1.0f;
            for (auto unique_queue_family_index : queue_family_indices.to_unordered_set())
            {
                queues_create_info.push_back(
                    vk::DeviceQueueCreateInfo
                    {
                        {},
                        unique_queue_family_index,
                        1,
                        &queue_priority
                    });
            }

            std::vector<char*> extension_names{};
            std::vector<char*> layer_names{};
            vk::PhysicalDeviceFeatures features{};

            for (const auto& device_extension_name : required_device_extension_names_)
            {
                extension_names.push_back(const_cast<char*>(device_extension_name.c_str()));
            }

#if !defined(NDEBUG)
            for (const auto& validation_layer_name : validation_layer_names)
            {
                layer_names.push_back(const_cast<char*>(validation_layer_name));
            }
#endif

            auto result = physical_.createDeviceUnique(
                {
                    {},
                    static_cast<unsigned int>(queues_create_info.size()),
                    queues_create_info.data(),
                    static_cast<unsigned int>(layer_names.size()),
                    layer_names.data(),
                    static_cast<unsigned int>(extension_names.size()),
                    extension_names.data(),
                    & features
                });
#if !defined(NDEBUG)
            std::cout << "Logical device created" << std::endl;
#endif

            environment.update_dynamic_loader(*result);
#if !defined(NDEBUG)
            std::cout << "Dynamic loader updated" << std::endl;
#endif

            return result;
        }

        [[nodiscard]] vk::Queue create_queue(const unsigned int& queue_family_index) const
        {
            const auto result = inner_->getQueue(queue_family_index, 0);

            return result;
        }
	};
}

#endif
