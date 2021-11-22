#ifndef IRGLAB_DEVICE_HPP
#define IRGLAB_DEVICE_HPP


#include "external/external.hpp"

#if !defined(NDEBUG)
#include "debug_printer.hpp"
#endif

#include "environment.hpp"
#include "window.hpp"


namespace il
{
    struct [[maybe_unused]] queue_family_indices
    {
        std::optional<unsigned int> graphics_family;
        std::optional<unsigned int> present_family;
        std::optional<unsigned int> transfer_family;

        [[nodiscard, maybe_unused]] bool is_complete() const noexcept
        {
            return graphics_family.has_value()
                   && present_family.has_value()
                   && transfer_family.has_value();
        }

        [[nodiscard, maybe_unused]] std::vector<unsigned int> to_vector() const
        {
            return std::vector<unsigned int>
                    {
                            graphics_family.value(),
                            present_family.value(),
                            transfer_family.value()
                    };
        }

        [[nodiscard, maybe_unused]] bool are_all_unique() const
        {
            return to_unordered_set().size() == 3;
        }

        [[nodiscard, maybe_unused]] std::unordered_set<unsigned int> to_unordered_set() const
        {
            return
                    {
                            graphics_family.value(),
                            present_family.value(),
                            transfer_family.value()
                    };
        }
    };


    struct [[maybe_unused]] device_surface_info
    {
        const vk::SurfaceCapabilitiesKHR capabilities;
        const std::vector<vk::SurfaceFormatKHR> formats;
        const std::vector<vk::PresentModeKHR> present_modes;
    };


    class [[maybe_unused]] device
    {
        const std::array<std::string, 1> _required_device_extension_names
                {
                        VK_KHR_SWAPCHAIN_EXTENSION_NAME
                };

        const vk::PhysicalDevice _physical;

    public:
        const queue_family_indices queue_family_indices;

    private:
        const vk::UniqueDevice _inner;

    public:
        const vk::Queue graphics_queue;
        const vk::Queue present_queue;
        const vk::Queue transfer_queue;


       [[nodiscard, maybe_unused]] explicit device(
                const environment &environment,
                const window &window) :

                _physical(_select_physical_device(environment, window)),
                queue_family_indices(_query_queue_families(_physical, window)),
                _inner(_create_inner(environment, debug_printer::validation_layer_names)),
                graphics_queue(_create_queue(queue_family_indices.graphics_family.value())),
                present_queue(_create_queue(queue_family_indices.present_family.value())),
                transfer_queue(_create_queue(queue_family_indices.transfer_family.value()))
        {
#if !defined(NDEBUG)
            std::cout << "Queues created" << std::endl;
            std::cout << std::endl << "-- device done --" << std::endl << std::endl;
#endif
        }


        [[nodiscard, maybe_unused]] const vk::Device &operator*() const
        {
            return *_inner;
        }

        [[nodiscard, maybe_unused]] const vk::Device *operator->() const
        {
            return &*_inner;
        }

        [[nodiscard, maybe_unused]] const vk::PhysicalDevice &physical() const
        {
            return _physical;
        }

        [[nodiscard, maybe_unused]] device_surface_info query_surface_info(const window &window) const
        {
            return _query_surface_info(_physical, window);
        }


    private:
        [[nodiscard]] vk::PhysicalDevice _select_physical_device(
                const environment &environment,
                const window &window) const
        {
            auto devices = environment.vulkan_instance().enumeratePhysicalDevices();

            for (const auto &device : devices)
            {
                if (_device_suitable(device, window))
                {
#if !defined(NDEBUG)
                    std::cout << "Physical device selected" << std::endl;
#endif

                    return device;
                }
            }

            throw std::runtime_error("Failed to find a suitable GPU.");
        }

        [[nodiscard]] bool _device_suitable(
                const vk::PhysicalDevice &device,
                const window &surface) const
        {
            if (!_device_extensions_supported(device)) return false;

            const auto swap_chain_support_info = _query_surface_info(device, surface);

            return _query_queue_families(device, surface).is_complete()
                   && !swap_chain_support_info.formats.empty()
                   && !swap_chain_support_info.present_modes.empty();
        }

        [[nodiscard]] bool _device_extensions_supported(const vk::PhysicalDevice &device) const
        {
            auto available_extension_properties =
                    device.enumerateDeviceExtensionProperties();

            for (const auto &required_extension_name : _required_device_extension_names)
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

        [[nodiscard]] static il::queue_family_indices _query_queue_families(
                const vk::PhysicalDevice &device,
                const window &window)
        {
            il::queue_family_indices indices;

            auto queue_family_index = 0;
            for (const auto &queue_family : device.getQueueFamilyProperties())
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

        [[nodiscard]] static device_surface_info _query_surface_info(
                const vk::PhysicalDevice &device,
                const window &window)
        {
            return
                    {
                            device.getSurfaceCapabilitiesKHR(window.drawing_surface()),
                            device.getSurfaceFormatsKHR(window.drawing_surface()),
                            device.getSurfacePresentModesKHR(window.drawing_surface())
                    };
        }

        [[nodiscard]] vk::UniqueDevice _create_inner(
                const environment &environment,
                const std::array<const char *, 1> &validation_layer_names) const
        {
            std::vector<vk::DeviceQueueCreateInfo> queues_create_info{ };

            const auto queue_priority = 1.0f;
            for (auto unique_queue_family_index : queue_family_indices.to_unordered_set())
            {
                queues_create_info.push_back(
                        vk::DeviceQueueCreateInfo
                                {
                                        { },
                                        unique_queue_family_index,
                                        1,
                                        &queue_priority
                                });
            }

            std::vector<char *> extension_names{ };
            std::vector<char *> layer_names{ };
            vk::PhysicalDeviceFeatures features{ };

            for (const auto &device_extension_name : _required_device_extension_names)
            {
                extension_names.push_back(const_cast<char *>(device_extension_name.c_str()));
            }

#if !defined(NDEBUG)
            for (const auto &validation_layer_name : validation_layer_names)
            {
                layer_names.push_back(const_cast<char *>(validation_layer_name));
            }
#endif

            auto result = _physical.createDeviceUnique(
                    {
                            { },
                            static_cast<unsigned int>(queues_create_info.size()),
                            queues_create_info.data(),
                            static_cast<unsigned int>(layer_names.size()),
                            layer_names.data(),
                            static_cast<unsigned int>(extension_names.size()),
                            extension_names.data(),
                            &features
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

        [[nodiscard]] vk::Queue _create_queue(const unsigned int &queue_family_index) const
        {
            const auto result = _inner->getQueue(queue_family_index, 0);

            return result;
        }
    };
}

#endif
