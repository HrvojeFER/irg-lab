#ifndef APP_HPP
#define APP_HPP

#pragma once

#include "pch.hpp"

#include "app_utils.hpp"

namespace irglab
{
    struct compiled_shader_paths
    {
        std::string vertex;
        std::string fragment;
    } const compiled_shader_paths
	{
        "./spirv/vertex_shader.spirv",
        "./spirv/fragment_shader.spirv"
    };

	class app {
    public:
        app() :
            window_(create_window(), window_deleter_),
            instance_(create_instance()),
#if !defined(NDEBUG)
			debug_utils_messenger_(create_debug_messenger()),
#endif
			surface_(create_surface()),
			physical_device_(select_physical_device()),
			queue_family_indices_(query_queue_families(physical_device_)),
			device_(create_logical_device()),
			graphics_queue_(create_queue(queue_family_indices_.graphics_family.value())),
			present_queue_(create_queue(queue_family_indices_.present_family.value())),
            swapchain_configuration_(select_swapchain_configuration()),
			swapchain_(create_swapchain()),
			image_views_(create_image_views()),
			render_pass_(create_render_pass()),
			pipeline_layout_(create_pipeline_layout()),
			vertex_shader_module_(create_shader_module(compiled_shader_paths.vertex)),
            fragment_shader_module_(create_shader_module(compiled_shader_paths.fragment)),
			graphics_pipeline_(create_graphics_pipeline()),
			framebuffers_(create_frame_buffers()),
			command_buffers_(create_command_buffers()),
			sync_objects_(create_sync_objects()) { }

        void run()
    	{
            while (!glfwWindowShouldClose(window_.get()))
            {
                glfwPollEvents();
                draw_frame();
            }

            vkDeviceWaitIdle(device_.get());
        }
    private:
		// Draw command
        static const int max_frames_in_flight = 2;
        size_t current_frame_ = 0;
		
        void draw_frame()
        {
            if (device_->waitForFences(
                sync_objects_.in_flight_fences.size(),
                &sync_objects_.in_flight_fences.at(current_frame_).get(),
                VK_TRUE,
                UINT64_MAX) // Means there is no timeout
                != vk::Result::eSuccess)
            {
                std::cerr << "Failed waiting for in flight fence." << std::endl;
                return;
            }

            auto image_index = device_->acquireNextImageKHR(
	            swapchain_.get(),
	            UINT64_MAX,
	            sync_objects_.image_available_semaphores[current_frame_].get(),
	            nullptr).value;

            if (sync_objects_.images_in_flight[image_index].has_value()) {
                if (device_->waitForFences(
                    sync_objects_.in_flight_fences.size(),
                    &sync_objects_.images_in_flight[image_index].value().get(),
                    VK_TRUE,
                    UINT64_MAX) != vk::Result::eSuccess)
                {
                    std::cerr << "Failed waiting for in flight image fence." << std::endl;
                    return;
                }
            }
            sync_objects_.images_in_flight[image_index].value().get() = 
                sync_objects_.in_flight_fences[current_frame_].get();

        	
            std::array<vk::PipelineStageFlags, 1> wait_stages
            {
                vk::PipelineStageFlagBits::eColorAttachmentOutput
            };
            vk::SubmitInfo submit_info
        	{
        		1,
                &sync_objects_.image_available_semaphores[current_frame_].get(),
        		wait_stages.data(),
        		1,
                &command_buffers_[image_index].get(),
        		1,
                &sync_objects_.render_finished_semaphores[current_frame_].get()
        	};

            if (device_->resetFences(1, &sync_objects_.in_flight_fences[current_frame_].get())
                != vk::Result::eSuccess)
            {
                std::cerr << "Failed to reset fences." << std::endl;
                return;
            }

            if (graphics_queue_.submit(
                1,
                &submit_info,
                sync_objects_.in_flight_fences[current_frame_].get()) != vk::Result::eSuccess)
            {
                throw std::runtime_error("Failed to submit draw command buffer.");
            }

            present_queue_.presentKHR(
            {
            	1,
            	&sync_objects_.render_finished_semaphores[current_frame_].get(),
            	1,
            	&swapchain_.get(),
            	&image_index,
            	nullptr
            });
        	
            vkQueueWaitIdle(present_queue_);
            current_frame_ = (current_frame_ + 1) % max_frames_in_flight;
        }


		// Start of objects and handles
        const char* app_name_ = "IrgLab";


		// Window
        const unsigned int window_width_ = 800;
        const unsigned int window_height_ = 600;
        const std::string window_title_ = app_name_;
        using window_deleter_type = std::function<void(GLFWwindow*)>;
        using window_type = std::unique_ptr<GLFWwindow, window_deleter_type>;
		const window_type window_;

        [[nodiscard]] GLFWwindow* create_window() const
        {
            glfwInit();

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

            const auto result = glfwCreateWindow(
                static_cast<int>(window_width_),
                static_cast<int>(window_height_),
                window_title_.c_str(),
                nullptr,
                nullptr
			);

            std::cout << "Window created." << std::endl;

            return result;
        }

        const window_deleter_type window_deleter_ = [&](GLFWwindow* window)
        {
            glfwDestroyWindow(window);
            glfwTerminate();
        };


		// Vulkan instance
#if !defined(NDEBUG)
        const std::vector<std::string> validation_layer_names_
        {
            "VK_LAYER_KHRONOS_validation"
        };
#endif

		const vk::UniqueInstance instance_;

        [[nodiscard]] vk::UniqueInstance create_instance() const
        {
            const vk::ApplicationInfo app_info
            {
                app_name_,
                1,
                "No Engine",
                1,
                VK_API_VERSION_1_2
            };

            std::vector<char*> extension_names{};
            std::vector<char*> layer_names{};

            auto required_glfw_extension_names = get_required_glfw_extension_names();
            extension_names.insert(
                extension_names.end(),
                required_glfw_extension_names.begin(),
                required_glfw_extension_names.end());

#if !defined(NDEBUG)
            layer_names.insert(
                layer_names.end(),
                validation_layer_names_.begin(),
                validation_layer_names_.end());

            extension_names.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

            if (!extensions_supported(extension_names))
            {
                throw std::runtime_error("Extensions not supported.");
            }
        	
            if (!layers_supported(layer_names))
            {
                throw std::runtime_error("Layers not supported.");
            }

            auto result = vk::createInstanceUnique(
            {
                {},
                &app_info,
                static_cast<unsigned int>(layer_names.size()),
                layer_names.data(),
                static_cast<unsigned int>(extension_names.size()),
                extension_names.data()
            });

            std::cout << "Vulkan instance created." << std::endl;

            return result;
        }

        [[nodiscard]] static std::vector<char*> get_required_glfw_extension_names()
        {
            unsigned int glfw_extension_count = 0;
            // ReSharper disable once CppUseAuto
            const char** glfw_extensions =
                glfwGetRequiredInstanceExtensions(&glfw_extension_count);

            std::vector<char*> extension_names
        	{
        		glfw_extensions,
                glfw_extensions + glfw_extension_count
        	};

            return extension_names;
        }

        [[nodiscard]] static bool extensions_supported(
            const std::vector<char*>& required_extension_names)
        {
            return irglab::is_subset<char*, vk::LayerProperties>(
                required_extension_names,
                vk::enumerateInstanceExtensionProperties(),
                [](const char*& required_layer_name, vk::LayerProperties& available)
                {
                    return strcmp(required_layer_name, available.layerName) == 0;
                });
        }

        [[nodiscard]] static bool layers_supported(
            const std::vector<char*>& required_layer_names)
        {
            return irglab::is_subset<char*, vk::LayerProperties>(
                required_layer_names,
                vk::enumerateInstanceLayerProperties(),
                [](const char*& required_layer_name, const vk::LayerProperties& available)
                {
                    return strcmp(required_layer_name, available.layerName) == 0;
                });
        }


		// Debug messenger
#if !defined(NDEBUG)
        const vk::UniqueDebugUtilsMessengerEXT debug_utils_messenger_;

        [[nodiscard]] vk::UniqueDebugUtilsMessengerEXT create_debug_messenger() const
        {
            if (!reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                instance_->getProcAddr("vkCreateDebugUtilsMessengerEXT")))
            {
                throw std::runtime_error("Failed to find debug messenger creator.");
            }

            if (!reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                instance_->getProcAddr("vkDestroyDebugUtilsMessengerEXT")))
            {
                throw std::runtime_error("Failed to find debug messenger deleter.");
            }

            auto result = instance_->createDebugUtilsMessengerEXTUnique(
            {
                {},
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
                | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
                // For now, performance messages are disabled.
                // | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
                ,
                &debug_callback,
                {}
            });

            std::cout << "Debug messenger created." << std::endl;

            return result;
        }
		
        static vk::Bool32 debug_callback(
            VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
            // ReSharper disable once CppParameterMayBeConst
            VkDebugUtilsMessageTypeFlagsEXT message_types,
            const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
            void*  /* p_user_data */)
        {
            std::ostringstream message;

            message <<
                vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(message_severity)) <<
                ": " << vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(message_types)) <<
                ":\n";


            message << "\t" << "messageIDName   = <" << p_callback_data->pMessageIdName << ">\n";
            message << "\t" << "messageIdNumber = " << p_callback_data->messageIdNumber << "\n";
            message << "\t" << "message         = <" << p_callback_data->pMessage << ">\n";

            if (p_callback_data->queueLabelCount > 0)
            {
                message << "\t" << "Queue Labels:\n";
                for (uint8_t i = 0; i < p_callback_data->queueLabelCount; i++)
                {
                    message <<
                        "\t\t" <<
                        "labelName = <" <<
                        p_callback_data->pQueueLabels[i].pLabelName <<
                        ">\n";
                }
            }

            if (p_callback_data->cmdBufLabelCount > 0)
            {
                message << "\t" << "CommandBuffer Labels:\n";
                for (unsigned int i = 0; i < p_callback_data->cmdBufLabelCount; i++)
                {
                    message <<
                        "\t\t" <<
                        "labelName = <" <<
                        p_callback_data->pCmdBufLabels[i].pLabelName <<
                        ">\n";
                }
            }

            if (p_callback_data->objectCount > 0)
            {
                message << "\t" << "Objects:\n";
                for (unsigned int i = 0; i < p_callback_data->objectCount; i++)
                {
                    message << "\t\t" << "Object " << i << "\n";

                    message <<
                        "\t\t\t" <<
                        "objectType   = " <<
                        vk::to_string(static_cast<vk::ObjectType>(p_callback_data->pObjects[i].objectType)) <<
                        "\n";

                    message << "\t\t\t" << "objectHandle = " <<
                        p_callback_data->pObjects[i].objectHandle <<
                        "\n";

                    if (p_callback_data->pObjects[i].pObjectName)
                    {
                        message <<
                            "\t\t\t" <<
                            "objectName   = <" <<
                            p_callback_data->pObjects[i].pObjectName <<
                            ">\n";
                    }
                }
            }


            std::cerr << message.str() << std::endl;

            return VK_FALSE;
        }
#endif


		// Surface
		const vk::UniqueSurfaceKHR surface_;

        [[nodiscard]] vk::UniqueSurfaceKHR create_surface() const
        {
            VkSurfaceKHR inner_surface = {};
            if (glfwCreateWindowSurface(
                instance_.get(),
                window_.get(),
                nullptr,
                &inner_surface) != VK_SUCCESS) 
			{
                throw std::runtime_error("Failed to create a window surface.");
            }

            std::cout << "Surface created." << std::endl;

            return vk::UniqueSurfaceKHR { inner_surface };
        }


		// Physical device
        const vk::PhysicalDevice physical_device_;

		struct queue_family_indices
        {
            std::optional<unsigned int> graphics_family;
            std::optional<unsigned int> present_family;

            [[nodiscard]] bool is_complete() const noexcept
			{
                return graphics_family.has_value() && present_family.has_value();
            }

			[[nodiscard]] std::vector<unsigned int> to_vector() const
            {
                return std::vector<unsigned int>
                {
                    graphics_family.value(),
                	present_family.value()
                };
            }

			[[nodiscard]] bool are_all_unique() const
            {
                return to_vector().size() == to_unordered_set().size();
            }

		private:
            [[nodiscard]] std::unordered_set<unsigned int> to_unordered_set() const
            {
                return std::unordered_set<unsigned int>
                {
                    graphics_family.value(),
                    present_family.value()
                };
            }
        } const queue_family_indices_;

		const std::vector<std::string> device_extension_names_
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        [[nodiscard]] vk::PhysicalDevice select_physical_device() const
        {
            auto devices = instance_->enumeratePhysicalDevices();

            for (const auto& device : devices)
            {
                if (device_suitable(device))
                {
                    std::cout << "Physical device selected." << std::endl;

                    return device;
                }
            }

            throw std::runtime_error("Failed to find a suitable GPU.");
        }

        [[nodiscard]] bool device_suitable(const vk::PhysicalDevice& device) const
        {
            if (!device_extensions_supported(device)) return false;

            const auto swap_chain_support_info = query_swapchain_support_info(device);

            return query_queue_families(device).is_complete()
                && !swap_chain_support_info.formats.empty()
                && !swap_chain_support_info.present_modes.empty();
        }

        [[nodiscard]] bool device_extensions_supported(const vk::PhysicalDevice& device) const
        {
            return irglab::is_subset<char*, vk::ExtensionProperties>(
                device_extension_names_,
                device.enumerateDeviceExtensionProperties(),
                [](
                    const char*& required_extension_name,
                    const vk::ExtensionProperties& available)
                {
                    return strcmp(required_extension_name, available.extensionName) == 0;
                });
        }

        [[nodiscard]] queue_family_indices query_queue_families(const vk::PhysicalDevice& device) const
        {
            queue_family_indices indices;
        	
            auto queue_family_index = 0;
            for (const auto& queue_family : device.getQueueFamilyProperties())
            {
                if (device.getSurfaceSupportKHR(queue_family_index, surface_.get()) == VK_TRUE) 
                {
                    indices.present_family = queue_family_index;
                }

                if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics)
                {
                    indices.graphics_family = queue_family_index;
                }

                if (indices.is_complete())
                {
                    break;
                }

                queue_family_index++;
            }

            return indices;
        }

        struct swapchain_support_info
        {
            vk::SurfaceCapabilitiesKHR capabilities;
            std::vector<vk::SurfaceFormatKHR> formats;
            std::vector<vk::PresentModeKHR> present_modes;
        };

        [[nodiscard]] swapchain_support_info query_swapchain_support_info(
            const vk::PhysicalDevice& device) const
        {
            return
        	{
                device.getSurfaceCapabilitiesKHR(surface_.get()),
                device.getSurfaceFormatsKHR(surface_.get()),
        		device.getSurfacePresentModesKHR(surface_.get())
        	};
        }


		// Logical device
        const vk::UniqueDevice device_;
		
        [[nodiscard]] vk::UniqueDevice create_logical_device() const
		{
            const auto queue_family_indices = queue_family_indices_.to_vector();
            const std::vector<vk::DeviceQueueCreateInfo> queues_create_info(
                static_cast<unsigned int>(queue_family_indices.size()));

            const auto queue_priority = 1.0f;
            std::transform(
                queue_family_indices.begin(),
                queue_family_indices.end(),
                queues_create_info.begin(),
                [queue_priority](const unsigned int& queue_family_index)
                {
                    return vk::DeviceQueueCreateInfo
                    {
                        {},
                        queue_family_index,
                        1,
                        &queue_priority
                    };
                }
            );
        	
            std::vector<char*> extension_names{};
            std::vector<char*> layer_names{};
            vk::PhysicalDeviceFeatures features{};
        	
#if !defined(NDEBUG)
            layer_names.insert(
                layer_names.end(),
                validation_layer_names_.begin(),
                validation_layer_names_.end());
#endif
        	
            auto result = physical_device_.createDeviceUnique(
            {
                {},
                static_cast<unsigned int>(queues_create_info.size()),
                queues_create_info.data(),
            	static_cast<unsigned int>(layer_names.size()),
            	layer_names.data(),
            	static_cast<unsigned int>(extension_names.size()),
            	extension_names.data(),
            	&features
            });

            std::cout << "Logical device created." << std::endl;

            return result;
        }

		
		// Queues
        vk::Queue graphics_queue_;
        vk::Queue present_queue_;

		[[nodiscard]] vk::Queue create_queue(const unsigned int& queue_family_index) const
        {
	        const auto result = device_->getQueue(queue_family_index, 0);

            std::cout << "Queue created." << std::endl;

            return result;
        }


		// Swapchain configuration
		const struct swapchain_configuration
		{
            vk::Format format{};
            vk::PresentModeKHR present_mode{};
            vk::Extent2D extent;
            vk::SurfaceTransformFlagBitsKHR transform_flag_bit{};
            vk::ColorSpaceKHR color_space{};
            unsigned int image_count{};
		} swapchain_configuration_;

		[[nodiscard]] swapchain_configuration select_swapchain_configuration() const
		{
            const auto support_info = query_swapchain_support_info(physical_device_);
            const auto surface = select_swap_surface_format(support_info.formats);

            auto image_count = support_info.capabilities.minImageCount + 1;
            // 0 means there is no maximum
            if (support_info.capabilities.maxImageCount > 0 &&
                image_count > support_info.capabilities.maxImageCount)
            {
                image_count = support_info.capabilities.maxImageCount;
            }

            const swapchain_configuration result
			{
                surface.format,
				select_swap_present_mode(support_info.present_modes),
                select_swap_extent(support_info.capabilities),
            	support_info.capabilities.currentTransform,
            	surface.colorSpace,
            	image_count
            };

            std::cout << "Swapchain configuration selected." << std::endl;

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

            std::cerr
				<< "Failed to find a desirable surface format. "
				<< "The first available format will be selected instead."
				<< std::endl;

            return available_formats[0];
        }

        [[nodiscard]] static vk::PresentModeKHR select_swap_present_mode(
            const std::vector<vk::PresentModeKHR>& available_present_modes) noexcept
        {
            for (const auto& available_present_mode : available_present_modes)
            {
                if (available_present_mode == vk::PresentModeKHR::eMailbox)
                {
                    std::cout << "Triple buffering available." << std::endl;
                    return available_present_mode;
                }
            }

            std::cerr
				<< "Triple buffering unavailable. "
				<< "Using standard FIFO presentation mode (Vsync)."
				<< std::endl;
            return vk::PresentModeKHR::eFifo;
        }

        [[nodiscard]] vk::Extent2D select_swap_extent(
            const vk::SurfaceCapabilitiesKHR& capabilities) const noexcept
        {
            if (capabilities.currentExtent.width != UINT32_MAX)
            {
                return capabilities.currentExtent;
            }

            VkExtent2D actual_extent = { window_width_, window_height_ };

            // Clamps the extent to the capabilities of Vulkan.
            actual_extent.width = max(capabilities.minImageExtent.width,
                min(capabilities.maxImageExtent.width, actual_extent.width));
            actual_extent.height = max(capabilities.minImageExtent.height,
                min(capabilities.maxImageExtent.height, actual_extent.height));

            return actual_extent;
        }


		// Swapchain
        vk::UniqueSwapchainKHR swapchain_;

        [[nodiscard]] vk::UniqueSwapchainKHR create_swapchain() const
        {
            auto image_sharing_mode = vk::SharingMode::eExclusive;
            std::vector<unsigned int> queue_family_indices{};

        	if (queue_family_indices_.are_all_unique())
        	{
                image_sharing_mode = vk::SharingMode::eConcurrent;
                queue_family_indices = queue_family_indices_.to_vector();
        	}
        	
            auto result = device_->createSwapchainKHRUnique(
            {
                {},
                surface_.get(),
                swapchain_configuration_.image_count,
                swapchain_configuration_.format,
                swapchain_configuration_.color_space,
                swapchain_configuration_.extent,
                0,
                vk::ImageUsageFlagBits::eColorAttachment,
                image_sharing_mode,
                static_cast<unsigned int>(queue_family_indices.size()),
                queue_family_indices.data(),
                swapchain_configuration_.transform_flag_bit,
                vk::CompositeAlphaFlagBitsKHR::eOpaque,
                swapchain_configuration_.present_mode,
                VK_TRUE,
                nullptr
            });

            std::cout << "Swapchain created." << std::endl;

            return result;
        }


		// Image views
        const std::vector<vk::UniqueImageView> image_views_;

        [[nodiscard]] std::vector<vk::UniqueImageView> create_image_views() const
        {
            std::vector<vk::UniqueImageView> image_views{};
        	
            for (const auto image : device_.get().getSwapchainImagesKHR(swapchain_.get()))
            {
                image_views.emplace_back(device_->createImageViewUnique(
                {
                    {},
                    image,
                	vk::ImageViewType::e2D,
                	swapchain_configuration_.format,
                    {
                        vk::ComponentSwizzle::eIdentity,
                        vk::ComponentSwizzle::eIdentity,
                    	vk::ComponentSwizzle::eIdentity,
                        vk::ComponentSwizzle::eIdentity
                    },
                    {
                    	vk::ImageAspectFlagBits::eColor,
                    	0,
                    	1,
                    	0,
                    	1
                    }
                }));
            }

            std::cout << "Image views created." << std::endl;

            return image_views;
        }


		// Render pass
        vk::UniqueRenderPass render_pass_;

        [[nodiscard]] vk::UniqueRenderPass create_render_pass() const
        {
            std::vector<vk::AttachmentDescription> color_attachment_descriptions
        	{
        		vk::AttachmentDescription
				{
                    {},
					swapchain_configuration_.format,
					vk::SampleCountFlagBits::e1,
					vk::AttachmentLoadOp::eClear,
					vk::AttachmentStoreOp::eStore,
					vk::AttachmentLoadOp::eDontCare,
					vk::AttachmentStoreOp::eDontCare,
					{},
					vk::ImageLayout::ePresentSrcKHR
				}
        	};
        	
            std::vector<vk::AttachmentReference> color_attachment_references
            {
            	vk::AttachmentReference
                {
					0,
                	vk::ImageLayout::eColorAttachmentOptimal
                }
            };
        	
            std::vector<vk::SubpassDescription> subpass_descriptions
        	{
                vk::SubpassDescription
        		{
                    {},
	                vk::PipelineBindPoint::eGraphics,
                	0,
                	nullptr,
                    static_cast<unsigned int>(color_attachment_references.size()),
                	color_attachment_references.data(),
        			nullptr,
        			nullptr,
        			0,
        			nullptr
                }
        	};

            std::vector<vk::SubpassDependency> subpass_dependencies
            {
                vk::SubpassDependency
                {
                    VK_SUBPASS_EXTERNAL,
                	0,
                	vk::PipelineStageFlagBits::eColorAttachmentOutput,
                	vk::PipelineStageFlagBits::eColorAttachmentOutput,
					{},
                	vk::AccessFlagBits::eColorAttachmentRead
                		| vk::AccessFlagBits::eColorAttachmentWrite,
					{}
				}
            };
        	
            auto result = device_->createRenderPassUnique(
            {
                {},
                static_cast<unsigned int>(color_attachment_descriptions.size()),
                color_attachment_descriptions.data(),
                static_cast<unsigned int>(subpass_descriptions.size()),
            	subpass_descriptions.data(),
            	static_cast<unsigned int>(subpass_dependencies.size()),
            	subpass_dependencies.data()
            });

            std::cout << "Render pass created." << std::endl;

            return result;
        }


		// Pipeline layout
		vk::UniquePipelineLayout pipeline_layout_;

        [[nodiscard]] vk::UniquePipelineLayout create_pipeline_layout() const
        {
            std::vector<vk::DescriptorSetLayout> descriptor_set_layouts{};
            std::vector<vk::PushConstantRange> push_constant_ranges{};
        	
            auto result = device_->createPipelineLayoutUnique(
            {
                {},
                static_cast<unsigned int>(descriptor_set_layouts.size()),
            	descriptor_set_layouts.data(),
                static_cast<unsigned int>(push_constant_ranges.size()),
            	push_constant_ranges.data()
            });

            std::cout << "Pipeline layout created." << std::endl;

            return result;
        }


		// Shader modules
        vk::UniqueShaderModule vertex_shader_module_;
        vk::UniqueShaderModule fragment_shader_module_;
        const char* shader_main_function_name_ = "main";
        [[nodiscard]] vk::UniqueShaderModule create_shader_module(const std::string& path) const
        {
            const auto code = irglab::read_shader_file(path);

            auto result = device_->createShaderModuleUnique(
                {
                    {},
                    code.size(),
                    reinterpret_cast<const unsigned int*>(code.data())
                });

            std::cout << "Shader modules created." << std::endl;

            return result;
        }


		// Graphics pipeline
        vk::UniquePipeline graphics_pipeline_;

        [[nodiscard]] vk::UniquePipeline create_graphics_pipeline() const
        {
            std::vector<vk::PipelineShaderStageCreateInfo> shader_stages_create_info
            {
            	vk::PipelineShaderStageCreateInfo
				{
                    {},
					vk::ShaderStageFlagBits::eVertex,
					vertex_shader_module_.get(),
					shader_main_function_name_,
					{}
				},
            	vk::PipelineShaderStageCreateInfo
				{
                    {},
					vk::ShaderStageFlagBits::eFragment,
					fragment_shader_module_.get(),
					shader_main_function_name_,
					{}
				}
            };

            std::vector<vk::VertexInputBindingDescription> vertex_input_binding_descriptions{};
            std::vector<vk::VertexInputAttributeDescription> vertex_input_attribute_descriptions{};
        	
            vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info
            {
                {},
                static_cast<unsigned int>(vertex_input_binding_descriptions.size()),
            	vertex_input_binding_descriptions.data(),
                static_cast<unsigned int>(vertex_input_attribute_descriptions.size()),
            	vertex_input_attribute_descriptions.data()
            };

            vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_create_info
            {
                {},
            	vk::PrimitiveTopology::eTriangleList,
            	VK_FALSE
            };

            std::vector<vk::Viewport> viewports
        	{
        		vk::Viewport
				{
	                0.0f,
	                0.0f,
	                static_cast<float>(swapchain_configuration_.extent.width),
	                static_cast<float>(swapchain_configuration_.extent.height),
	                0.0f,
	                1.0f
				}
        	};

            std::vector<vk::Rect2D> scissors
        	{
            	vk::Rect2D
	            {
	                { 0, 0 },
	                swapchain_configuration_.extent
	            }
            };

            vk::PipelineViewportStateCreateInfo viewport_state_create_info
            {
                {},
                static_cast<unsigned int>(viewports.size()),
            	viewports.data(),
                static_cast<unsigned int>(scissors.size()),
            	scissors.data()
            };

            vk::PipelineRasterizationStateCreateInfo rasterization_state_create_info
            {
                {},
                VK_FALSE,
                VK_FALSE,
                vk::PolygonMode::eFill,
                vk::CullModeFlagBits::eBack,
            	vk::FrontFace::eClockwise,
            	VK_FALSE,
            	0.0f,
            	0.0f,
            	0.0f,
            	1.0f
            };

            vk::PipelineMultisampleStateCreateInfo multisample_state_create_info
            {
                {},
            	vk::SampleCountFlagBits::e1,
            	VK_FALSE,
            	1.0f,
            	nullptr,
            	VK_FALSE,
            	VK_FALSE
            };

            std::vector<vk::PipelineColorBlendAttachmentState> color_blend_attachment_states
            {
            	vk::PipelineColorBlendAttachmentState
	            {
	                VK_FALSE,
	                vk::BlendFactor::eOne,
	                vk::BlendFactor::eZero,
	                vk::BlendOp::eAdd,
	                vk::BlendFactor::eOne,
	                vk::BlendFactor::eZero,
	                vk::BlendOp::eAdd,
	                vk::ColorComponentFlagBits::eR
	                | vk::ColorComponentFlagBits::eG
	                | vk::ColorComponentFlagBits::eB
	                | vk::ColorComponentFlagBits::eA
	            }
            };

            vk::PipelineColorBlendStateCreateInfo color_blend_state_create_info
            {
            	{},
            	VK_FALSE,
            	vk::LogicOp::eCopy,
                static_cast<unsigned int>(color_blend_attachment_states.size()),
            	color_blend_attachment_states.data(),
            	std::array<float, 4>
            	{
            		0.0f,
            		0.0f,
            		0.0f,
            		0.0f
            	}
            };

            auto result = device_->createGraphicsPipelineUnique(nullptr,
            {
                {},
                static_cast<unsigned int>(shader_stages_create_info.size()),
            	shader_stages_create_info.data(),
            	&vertex_input_state_create_info,
            	&input_assembly_state_create_info,
                nullptr,
            	&viewport_state_create_info,
            	&rasterization_state_create_info,
            	&multisample_state_create_info,
            	nullptr,
            	&color_blend_state_create_info,
            	nullptr,
            	pipeline_layout_.get(),
            	render_pass_.get(),
            	0,
            	{},
                {}
            });

            std::cout << "Graphics pipeline created." << std::endl;

            return result;
        }


		// Framebuffers
        std::vector<vk::UniqueFramebuffer> framebuffers_;

        [[nodiscard]] std::vector<vk::UniqueFramebuffer> create_frame_buffers() const
        {
            std::vector<vk::UniqueFramebuffer> framebuffers;

            std::transform(
                image_views_.begin(), 
                image_views_.end(), 
                framebuffers.begin(),
                [this](vk::UniqueImageView image_view)
                {
                    std::vector<vk::ImageView> attachments{ image_view.get() };

                    return device_->createFramebufferUnique(
                    {
                        {},
                        render_pass_.get(),
                        static_cast<unsigned int>(attachments.size()),
                        attachments.data(),
                        swapchain_configuration_.extent.width,
                        swapchain_configuration_.extent.height,
                        1
                    });
                });

            std::cout << "Framebuffers created." << std::endl;

            return framebuffers;
        }


		// Command pool
        vk::UniqueCommandPool command_pool_;

        [[nodiscard]] vk::UniqueCommandPool create_command_pool() const
        {
            auto result = device_->createCommandPoolUnique(
            {
                {},
            	queue_family_indices_.graphics_family.value()
            });

            std::cout << "Command pool created." << std::endl;

            return result;
        }


		// Command buffers
        std::vector<vk::UniqueCommandBuffer> command_buffers_{};

        [[nodiscard]] std::vector<vk::UniqueCommandBuffer> create_command_buffers() const
        {
            const std::vector<vk::UniqueCommandBuffer> command_buffers(framebuffers_.size());
        	
            device_->allocateCommandBuffersUnique(
            {
                command_pool_.get(),
                vk::CommandBufferLevel::ePrimary,
                static_cast<unsigned int>(command_buffers.size())
            });

            for (size_t i = 0 ; i < command_buffers.size() ; ++i)
            {
            	vk::CommandBufferBeginInfo begin_info
                {
                    {},
                	nullptr
                };

                if (command_buffers[i]->begin(&begin_info) != vk::Result::eSuccess)
                {
                    throw std::runtime_error(
                        "Failed to begin command buffer " + std::to_string(i) + " .");
                }

                std::vector<vk::ClearValue> clear_values
                {
                	vk::ClearValue
	                {
	                    vk::ClearColorValue
	                    {
	                        std::array<float, 4>
	                        {
	                            0.0f,
	                            0.0f,
	                            0.0f,
	                            1.0f
	                        }
	                    }
	                }
                };

                vk::RenderPassBeginInfo render_pass_begin_info
                {
                    render_pass_.get(),
                    framebuffers_[i].get(),
                    vk::Rect2D
                    {
                        { 0, 0 },
                        swapchain_configuration_.extent
                    },
                    static_cast<unsigned int>(clear_values.size()),
                	clear_values.data()
                };

                command_buffers[i]->beginRenderPass(&render_pass_begin_info, vk::SubpassContents::eInline);
                command_buffers[i]->bindPipeline(
                    vk::PipelineBindPoint::eGraphics,
                    graphics_pipeline_.get());
            	// Draw a triangle
                command_buffers[i]->draw(3, 1, 0, 0);
                command_buffers[i]->endRenderPass();
                command_buffers[i]->end();
            }

            std::cout << "Command buffers created." << std::endl;

            return command_buffers;
        }

		
		// Sync objects
        struct sync_objects {
            std::array<vk::UniqueSemaphore, max_frames_in_flight> image_available_semaphores{};
            std::array<vk::UniqueSemaphore, max_frames_in_flight> render_finished_semaphores{};
            std::array<vk::UniqueFence, max_frames_in_flight> in_flight_fences{};
            std::vector<std::optional<vk::UniqueFence>> images_in_flight{};
        } sync_objects_;

        [[nodiscard]] sync_objects create_sync_objects() const
		{
            sync_objects sync_objects;
        	
			const vk::SemaphoreCreateInfo semaphore_create_info{};
            const vk::FenceCreateInfo fence_create_info
            {
            	vk::FenceCreateFlagBits::eSignaled
            };
        	
            sync_objects.images_in_flight.resize(image_views_.size());

        	for (size_t i = 0 ; i < max_frames_in_flight ; ++i)
        	{
                sync_objects.image_available_semaphores[i] = 
                    device_->createSemaphoreUnique(semaphore_create_info);
                sync_objects.render_finished_semaphores[i] = 
                    device_->createSemaphoreUnique(semaphore_create_info);
                sync_objects.in_flight_fences[i] = device_->createFenceUnique(fence_create_info);
        	}

            std::cout << "Sync objects created." << std::endl;

            return sync_objects;
        }
    };
}

#endif
