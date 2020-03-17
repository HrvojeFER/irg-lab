// ReSharper disable CppZeroConstantCanBeReplacedWithNullptr
#pragma once

#include "pch.hpp"

#include "assets.hpp"

namespace irglab
{
	class app_unsafe {
    public:
        void run()
    	{
            init_window();
            init_vulkan();
            main_loop();
            cleanup();
        }

    private:
        const std::string app_name_ = "IrgLab";
		
        const unsigned int window_width_ = 800;
        const unsigned int window_height_ = 600;
        const std::string window_title_ = app_name_;
        GLFWwindow* window_ = nullptr;

        VkInstance instance_ = VK_NULL_HANDLE;
#ifdef NDEBUG
        const bool enable_validation_layers_ = false;
#else
        const bool enable_validation_layers_ = true;
#endif
        const std::array<const char*, 1> validation_layers_names_
        {
            "VK_LAYER_KHRONOS_validation"
        };
        VkDebugUtilsMessengerEXT debug_messenger_ = VK_NULL_HANDLE;
		
		VkSurfaceKHR surface_ = VK_NULL_HANDLE;
		
        VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
		struct queue_family_indices
        {
            std::optional<unsigned int> graphics_family;
            std::optional<unsigned int> present_family;

            [[nodiscard]] bool is_complete() const {
                return graphics_family.has_value() && present_family.has_value();
            }
        } queue_family_indices_{};

		
        VkDevice device_ = VK_NULL_HANDLE;
        const std::vector<const char*> device_extension_names_
        { 
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        VkQueue graphics_queue_ = VK_NULL_HANDLE;
        VkQueue present_queue_ = VK_NULL_HANDLE;

        VkSwapchainKHR swap_chain_ = VK_NULL_HANDLE;
        VkFormat swap_chain_image_format_ = VK_FORMAT_UNDEFINED;
        VkExtent2D swap_chain_extent_{};
        std::vector<VkImage> swap_chain_images_{};
        std::vector<VkImageView> swap_chain_image_views_{};
        std::vector<VkFramebuffer> swap_chain_framebuffers_{};

        VkRenderPass render_pass_ = VK_NULL_HANDLE;

		VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;
        VkPipeline graphics_pipeline_ = VK_NULL_HANDLE;

        VkCommandPool command_pool_ = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> command_buffers_{};

        static const int max_frames_in_flight = 2;
        size_t current_frame_ = 0;
		std::array<VkSemaphore, max_frames_in_flight> image_available_semaphores_{};
        std::array<VkSemaphore, max_frames_in_flight> render_finished_semaphores_{};
        std::array<VkFence, max_frames_in_flight> in_flight_fences_{};
		std::vector<VkFence> images_in_flight_{};
		
		
        void init_window()
        {
            glfwInit();
        	
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

            window_ = glfwCreateWindow(
                static_cast<int>(window_width_),
                static_cast<int>(window_height_),
                window_title_.c_str(),
                nullptr,
                nullptr
            );
        }
    	
        void init_vulkan()
    	{
            create_instance();
            setup_debug_messenger();
            create_surface();
            select_physical_device();
            create_logical_device();
            create_swap_chain();
            create_image_views();
            create_render_pass();
            create_graphics_pipeline();
            create_frame_buffers();
            create_command_pool();
            create_command_buffers();
            create_sync_objects();
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void main_loop()
    	{
            while (!glfwWindowShouldClose(window_)) 
            {
                glfwPollEvents();
                draw_frame();
            }

            vkDeviceWaitIdle(device_);
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void cleanup()
    	{
        	for (size_t i = 0 ; i < max_frames_in_flight ; ++i)
        	{
                vkDestroySemaphore(device_, render_finished_semaphores_[i], nullptr);
                vkDestroySemaphore(device_, image_available_semaphores_[i], nullptr);
                vkDestroyFence(device_, in_flight_fences_[i], nullptr);
        	}
        	
            vkDestroyCommandPool(device_, command_pool_, nullptr);
        	
            for (auto framebuffer : swap_chain_framebuffers_) {
                vkDestroyFramebuffer(device_, framebuffer, nullptr);
            }
        	
            vkDestroyPipeline(device_, graphics_pipeline_, nullptr);
        	
            vkDestroyPipelineLayout(device_, pipeline_layout_, nullptr);

        	vkDestroyRenderPass(device_, render_pass_, nullptr);
        	
            for (auto image_view : swap_chain_image_views_)
            {
                vkDestroyImageView(device_, image_view, nullptr);
            }
        	
            vkDestroySwapchainKHR(device_, swap_chain_, nullptr);
        	
            vkDestroyDevice(device_, nullptr);
        	
            if (enable_validation_layers_) 
            {
                destroy_debug_utils_messenger_ext(instance_, debug_messenger_, nullptr);
            }

            vkDestroySurfaceKHR(instance_, surface_, nullptr);
        	
            vkDestroyInstance(instance_, nullptr);
        	
            glfwDestroyWindow(window_);

            glfwTerminate();
        }


		void draw_frame()
        {
            vkWaitForFences(
                device_,
                1,
                &in_flight_fences_[current_frame_],
                VK_TRUE,
                UINT64_MAX); // Means there is no timeout
        	
            unsigned int image_index;
        	// Add error handling
            vkAcquireNextImageKHR(
                device_,
                swap_chain_,
                UINT64_MAX, // Means there is no timeout
                image_available_semaphores_[current_frame_],
                VK_NULL_HANDLE,
                &image_index);

            if (images_in_flight_[image_index] != VK_NULL_HANDLE) {
                vkWaitForFences(
                    device_,
                    1,
                    &images_in_flight_[image_index],
                    VK_TRUE,
                    UINT64_MAX);
            }
            images_in_flight_[image_index] = in_flight_fences_[current_frame_];
        	
            VkSubmitInfo submit_info{};
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            std::array<VkSemaphore, 1> image_available_semaphores
        	{
        		image_available_semaphores_[current_frame_]
        	};
            std::array<VkPipelineStageFlags, 1> wait_stages
        	{
        		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        	};
            submit_info.waitSemaphoreCount = 1;
            submit_info.pWaitSemaphores = image_available_semaphores.data();
            submit_info.pWaitDstStageMask = wait_stages.data();
        	
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &command_buffers_[image_index];

            std::array<VkSemaphore, 1> render_finished_semaphores
        	{
        		render_finished_semaphores_[current_frame_]
        	};
            submit_info.signalSemaphoreCount = 1;
			submit_info.pSignalSemaphores = render_finished_semaphores.data();

        	vkResetFences(
                device_,
                1,
                &in_flight_fences_[current_frame_]);
            if (vkQueueSubmit(
                graphics_queue_,
                1, &submit_info,
                in_flight_fences_[current_frame_]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to submit draw command buffer.");
            }

            VkPresentInfoKHR present_info = {};
            present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

            present_info.waitSemaphoreCount = 1;
            present_info.pWaitSemaphores = render_finished_semaphores.data();

        	std::array<VkSwapchainKHR, 1> swap_chains { swap_chain_ };
            present_info.swapchainCount = 1;
            present_info.pSwapchains = swap_chains.data();
            present_info.pImageIndices = &image_index;

            present_info.pResults = nullptr; // Optional
        	// Add error handling
            vkQueuePresentKHR(present_queue_, &present_info);

            vkQueueWaitIdle(present_queue_);
            current_frame_ = (current_frame_ + 1) % max_frames_in_flight;
        }
		
		
        void create_instance()
		{
            VkApplicationInfo app_info{};
            app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            app_info.pApplicationName = app_name_.c_str();
            app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            app_info.pEngineName = "No Engine";
            app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            app_info.apiVersion = VK_API_VERSION_1_2;

            VkInstanceCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            create_info.pApplicationInfo = &app_info;

            auto required_glfw_extensions = get_required_glfw_extension_names();
            if (!glfw_extensions_supported(required_glfw_extensions))
            {
                throw std::runtime_error("GLFW extensions not supported");
            }
            create_info.enabledExtensionCount = static_cast<uint32_t>(required_glfw_extensions.size());
            create_info.ppEnabledExtensionNames = required_glfw_extensions.data();

            VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
            populate_debug_messenger_create_info(debug_create_info);
            if (enable_validation_layers_)
            {
                if (!validation_layers_supported())
                {
                    throw std::runtime_error("Validation layers not supported.");
                }

                create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers_names_.size());
                create_info.ppEnabledLayerNames = validation_layers_names_.data();

                populate_debug_messenger_create_info(debug_create_info);
                create_info.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debug_create_info);
            }
            else
            {
                create_info.enabledLayerCount = 0;
            }
        	
        	if (vkCreateInstance(&create_info, nullptr, &instance_) != VK_SUCCESS)
        	{
                throw std::runtime_error("Failed to create Vulkan instance.");
        	}

            std::cout << "Vulkan instance created." << std::endl;
        }

        void setup_debug_messenger()
		{
            if (!enable_validation_layers_) return;
        	
            VkDebugUtilsMessengerCreateInfoEXT create_info{};
            populate_debug_messenger_create_info(create_info);

            if (create_debug_utils_messenger_ext(
                instance_,
                &create_info,
                nullptr,
                &debug_messenger_) != VK_SUCCESS) 
            {
                throw std::runtime_error("failed to set up debug messenger!");
            }

            std::cout << "Debug messenger created." << std::endl;
        }

        void create_surface()
        {
            if (glfwCreateWindowSurface(
                instance_,
                window_,
                nullptr,
                &surface_) != VK_SUCCESS) {
                throw std::runtime_error("failed to create window surface!");
            }

            std::cout << "Window surface created." << std::endl;
        }
		
        void select_physical_device()
        {
            uint32_t device_count = 0;
            if (vkEnumeratePhysicalDevices(
                instance_,
                &device_count,
                nullptr) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to enumerate physical devices.");
            }
            if (device_count == 0)
            {
                throw std::runtime_error("Failed to find any GPUs with Vulkan support!");
            }

            std::vector<VkPhysicalDevice> devices(device_count);
            if (vkEnumeratePhysicalDevices(
                instance_,
                &device_count,
                devices.data()) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to enumerate physical devices.");
            }

            for (const auto& device : devices) {
                if (device_suitable(device)) {
                    physical_device_ = device;
                    queue_family_indices_ = query_queue_families(physical_device_);
                    break;
                }
            }

            if (physical_device_ == VK_NULL_HANDLE) {
                throw std::runtime_error("Failed to find a suitable GPU.");
            }

            std::cout << "Physical device selected." << std::endl;
        }

        void create_logical_device() {
            std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
            std::set<unsigned int> unique_queue_families = 
            {
            	queue_family_indices_.graphics_family.value(),
            	queue_family_indices_.present_family.value()
            };

            auto queue_priority = 1.0f;
            for (auto queue_family : unique_queue_families) {
                VkDeviceQueueCreateInfo queue_create_info{};
                queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queue_create_info.queueFamilyIndex = queue_family;
                queue_create_info.queueCount = 1;
                queue_create_info.pQueuePriorities = &queue_priority;

            	queue_create_infos.push_back(queue_create_info);
            }
        	
            VkDeviceQueueCreateInfo queue_create_info{};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = queue_family_indices_.graphics_family.value();
            queue_create_info.queueCount = 1;
            queue_create_info.pQueuePriorities = &queue_priority;

            VkPhysicalDeviceFeatures device_features{};
        	
            VkDeviceCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
            create_info.pQueueCreateInfos = queue_create_infos.data();
            create_info.pEnabledFeatures = &device_features;

            create_info.enabledExtensionCount = static_cast<uint32_t>(device_extension_names_.size());
            create_info.ppEnabledExtensionNames = device_extension_names_.data();

            if (enable_validation_layers_)
            {
                create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers_names_.size());
                create_info.ppEnabledLayerNames = validation_layers_names_.data();
            }
            else
            {
                create_info.enabledLayerCount = 0;
            }

            if (vkCreateDevice(
                physical_device_,
                &create_info,
                nullptr,
                &device_) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create logical device.");
            }

            std::cout << "Logical device created." << std::endl;

            vkGetDeviceQueue(
                device_,
                queue_family_indices_.graphics_family.value(),
                0,
                &graphics_queue_);

            vkGetDeviceQueue(
                device_,
                queue_family_indices_.present_family.value(),
                0, &present_queue_);

            std::cout << "Device queues created." << std::endl;
        }

        void create_swap_chain()
        {
	        const auto swap_chain_support = query_swap_chain_support_info(physical_device_);

	        const auto surface_format = select_swap_surface_format(swap_chain_support.formats);
            const auto present_mode = select_swap_present_mode(swap_chain_support.present_modes);
	        const auto extent = select_swap_extent(swap_chain_support.capabilities);

	        auto image_count = swap_chain_support.capabilities.minImageCount + 1;
        	// 0 means there is no maximum
        	if (swap_chain_support.capabilities.maxImageCount > 0 &&
                image_count > swap_chain_support.capabilities.maxImageCount)
        	{
                image_count = swap_chain_support.capabilities.maxImageCount;
        	}

            VkSwapchainCreateInfoKHR create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            create_info.surface = surface_;

            create_info.minImageCount = image_count;
            create_info.imageFormat = surface_format.format;
            create_info.imageColorSpace = surface_format.colorSpace;
            create_info.imageExtent = extent;
            create_info.imageArrayLayers = 1;
            create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            const std::array<unsigned int, 2> queue_family_indices
        	{
            	queue_family_indices_.graphics_family.value(),
            	queue_family_indices_.present_family.value()
            };
            if (queue_family_indices_.graphics_family != queue_family_indices_.present_family) 
            {
                create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                create_info.queueFamilyIndexCount = 2;
                create_info.pQueueFamilyIndices = queue_family_indices.data();
            }
            else 
            {
                create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                create_info.queueFamilyIndexCount = 0; // Optional
                create_info.pQueueFamilyIndices = nullptr; // Optional
            }

            create_info.preTransform = swap_chain_support.capabilities.currentTransform;
        	
            create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

            create_info.presentMode = present_mode;
            create_info.clipped = VK_TRUE;

            create_info.oldSwapchain = VK_NULL_HANDLE;

            if (vkCreateSwapchainKHR(device_, &create_info, nullptr, &swap_chain_) != VK_SUCCESS) 
            {
                throw std::runtime_error("Failed to create a swap chain.");
            }
            swap_chain_image_format_ = surface_format.format;
            swap_chain_extent_ = extent;

            std::cout << "Swap chain created." << std::endl;

        	if (vkGetSwapchainImagesKHR(
                device_,
                swap_chain_,
                &image_count,
                nullptr) != VK_SUCCESS)
        	{
                throw std::runtime_error("Failed to get swap chain images.");
        	}
            swap_chain_images_.resize(image_count);
        	if (vkGetSwapchainImagesKHR(
                device_,
                swap_chain_,
                &image_count,
                swap_chain_images_.data()) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to get swap chain images.");
            }

            std::cout << "Swap chain images retrieved." << std::endl;
        }

		void create_image_views()
        {
            swap_chain_image_views_.resize(swap_chain_images_.size());

        	for(size_t i = 0 ; i < swap_chain_images_.size() ; ++i)
        	{
                VkImageViewCreateInfo create_info{};
                create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                create_info.image = swap_chain_images_[i];

                create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
                create_info.format = swap_chain_image_format_;

                create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
                create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
                create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
                create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

                create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                create_info.subresourceRange.baseMipLevel = 0;
                create_info.subresourceRange.levelCount = 1;
                create_info.subresourceRange.baseArrayLayer = 0;
                create_info.subresourceRange.layerCount = 1;

                if (vkCreateImageView(
                    device_,
                    &create_info,
                    nullptr,
                    &swap_chain_image_views_[i]) != VK_SUCCESS) 
                {
                    throw std::runtime_error("Failed to create image view.");
                }
        	}

            std::cout << "Image views created" << std::endl;
        }

        void create_graphics_pipeline()
        {
            const auto vertex_shader_code =
                read_file(compiled_shader_paths.vertex);
            const auto fragment_shader_code =
                read_file(compiled_shader_paths.fragment);

            const auto vertex_shader_module = create_shader_module(vertex_shader_code);
            const auto fragment_shader_module = create_shader_module(fragment_shader_code);

            VkPipelineShaderStageCreateInfo vertex_shader_stage_info{};
            vertex_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertex_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vertex_shader_stage_info.module = vertex_shader_module;
            vertex_shader_stage_info.pName = "main";

            VkPipelineShaderStageCreateInfo fragment_shader_stage_info{};
            fragment_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragment_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragment_shader_stage_info.module = fragment_shader_module;
            fragment_shader_stage_info.pName = "main";

            std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages
            {
                vertex_shader_stage_info,
                fragment_shader_stage_info
            };

            std::cout << "Shader modules created." << std::endl;

            VkPipelineVertexInputStateCreateInfo vertex_input_info{};
            vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex_input_info.vertexBindingDescriptionCount = 0;
            vertex_input_info.pVertexBindingDescriptions = nullptr; // Optional
            vertex_input_info.vertexAttributeDescriptionCount = 0;
            vertex_input_info.pVertexAttributeDescriptions = nullptr; // Optional

            VkPipelineInputAssemblyStateCreateInfo input_assembly{};
            input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            input_assembly.primitiveRestartEnable = VK_FALSE;

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(swap_chain_extent_.width);
            viewport.height = static_cast<float>(swap_chain_extent_.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRect2D scissor{};
            scissor.offset = { 0, 0 };
            scissor.extent = swap_chain_extent_;

            VkPipelineViewportStateCreateInfo viewport_state{};
            viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewport_state.viewportCount = 1;
            viewport_state.pViewports = &viewport;
            viewport_state.scissorCount = 1;
            viewport_state.pScissors = &scissor;

            VkPipelineRasterizationStateCreateInfo rasterizer{};
            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.depthClampEnable = VK_FALSE;
            rasterizer.rasterizerDiscardEnable = VK_FALSE;
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.lineWidth = 1.0f;
            rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
            rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
            rasterizer.depthBiasEnable = VK_FALSE;
            rasterizer.depthBiasConstantFactor = 0.0f; // Optional
            rasterizer.depthBiasClamp = 0.0f; // Optional
            rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

            VkPipelineMultisampleStateCreateInfo multisampling{};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.sampleShadingEnable = VK_FALSE;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            multisampling.minSampleShading = 1.0f; // Optional
            multisampling.pSampleMask = nullptr; // Optional
            multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
            multisampling.alphaToOneEnable = VK_FALSE; // Optional

            VkPipelineColorBlendAttachmentState color_blend_attachment{};
            color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
                | VK_COLOR_COMPONENT_G_BIT
                | VK_COLOR_COMPONENT_B_BIT
                | VK_COLOR_COMPONENT_A_BIT;
            color_blend_attachment.blendEnable = VK_FALSE;
            color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
            color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
            color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
            color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
            color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
            color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

            VkPipelineColorBlendStateCreateInfo color_blending = {};
            color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            color_blending.logicOpEnable = VK_FALSE;
            color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
            color_blending.attachmentCount = 1;
            color_blending.pAttachments = &color_blend_attachment;
            color_blending.blendConstants[0] = 0.0f; // Optional
            color_blending.blendConstants[1] = 0.0f; // Optional
            color_blending.blendConstants[2] = 0.0f; // Optional
            color_blending.blendConstants[3] = 0.0f; // Optional

            VkPipelineLayoutCreateInfo pipeline_layout_info = {};
            pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_info.setLayoutCount = 0; // Optional
            pipeline_layout_info.pSetLayouts = nullptr; // Optional
            pipeline_layout_info.pushConstantRangeCount = 0; // Optional
            pipeline_layout_info.pPushConstantRanges = nullptr; // Optional

            if (vkCreatePipelineLayout(
                device_,
                &pipeline_layout_info,
                nullptr,
                &pipeline_layout_) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create pipeline layout.");
            }

            std::cout << "Pipeline layout created." << std::endl;

            VkGraphicsPipelineCreateInfo pipeline_info{};
            pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipeline_info.stageCount = 2;
            pipeline_info.pStages = shader_stages.data();
            pipeline_info.pVertexInputState = &vertex_input_info;
            pipeline_info.pInputAssemblyState = &input_assembly;
            pipeline_info.pViewportState = &viewport_state;
            pipeline_info.pRasterizationState = &rasterizer;
            pipeline_info.pMultisampleState = &multisampling;
            pipeline_info.pDepthStencilState = nullptr; // Optional
            pipeline_info.pColorBlendState = &color_blending;
            pipeline_info.pDynamicState = nullptr; // Optional
            pipeline_info.layout = pipeline_layout_;
            pipeline_info.renderPass = render_pass_;
            pipeline_info.subpass = 0;
            pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
            pipeline_info.basePipelineIndex = -1; // Optional

            if (vkCreateGraphicsPipelines(
                device_,
                VK_NULL_HANDLE, 1,
                &pipeline_info,
                nullptr,
                &graphics_pipeline_) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create graphics pipeline!");
            }

            std::cout << "Graphics pipeline created." << std::endl;

            vkDestroyShaderModule(device_, fragment_shader_module, nullptr);
            vkDestroyShaderModule(device_, vertex_shader_module, nullptr);
        }

        void create_render_pass()
        {
            VkAttachmentDescription color_attachment_description{};
            color_attachment_description.format = swap_chain_image_format_;
            color_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
            color_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            color_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            color_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            color_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            // Added because validation layers said to add something.
            color_attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            VkAttachmentReference color_attachment_ref;
            color_attachment_ref.attachment = 0;
            color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass_description{};
            subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass_description.colorAttachmentCount = 1;
            subpass_description.pColorAttachments = &color_attachment_ref;

        	// Implicit dependencies
            VkSubpassDependency subpass_dependency = {};
            subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            subpass_dependency.dstSubpass = 0;
            subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            subpass_dependency.srcAccessMask = 0;
            subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
                | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            VkRenderPassCreateInfo render_pass_create_info{};
            render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            render_pass_create_info.attachmentCount = 1;
            render_pass_create_info.pAttachments = &color_attachment_description;
            render_pass_create_info.subpassCount = 1;
            render_pass_create_info.pSubpasses = &subpass_description;
            render_pass_create_info.dependencyCount = 1;
            render_pass_create_info.pDependencies = &subpass_dependency;

            if (vkCreateRenderPass(
                device_,
                &render_pass_create_info,
                nullptr,
                &render_pass_) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create render pass.");
            }

            std::cout << "Render pass created." << std::endl;
        }

        void create_frame_buffers()
        {
            swap_chain_framebuffers_.resize(swap_chain_image_views_.size());

            for (size_t i = 0; i < swap_chain_image_views_.size(); i++) {
                VkImageView attachments[] { swap_chain_image_views_[i] };

                VkFramebufferCreateInfo framebuffer_create_info{};
                framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebuffer_create_info.renderPass = render_pass_;
                framebuffer_create_info.attachmentCount = 1;
                framebuffer_create_info.pAttachments = attachments;
                framebuffer_create_info.width = swap_chain_extent_.width;
                framebuffer_create_info.height = swap_chain_extent_.height;
                framebuffer_create_info.layers = 1;

                if (vkCreateFramebuffer(
                    device_,
                    &framebuffer_create_info,
                    nullptr,
                    &swap_chain_framebuffers_[i]) != VK_SUCCESS) 
                {
                    throw std::runtime_error("Failed to create framebuffer.");
                }
            }

            std::cout << "Frame buffers created." << std::endl;
        }

		void create_command_pool()
        {
            VkCommandPoolCreateInfo command_pool_create_info = {};
            command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            command_pool_create_info.queueFamilyIndex = queue_family_indices_.graphics_family.value();
            command_pool_create_info.flags = 0; // Optional

            if (vkCreateCommandPool(
                device_,
                &command_pool_create_info,
                nullptr,
                &command_pool_) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create command pool.");
            }

            std::cout << "Command pool created." << std::endl;
        }

        void create_command_buffers()
        {
            command_buffers_.resize(swap_chain_framebuffers_.size());
            VkCommandBufferAllocateInfo allocate_info{};
            allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocate_info.commandPool = command_pool_;
            allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocate_info.commandBufferCount = static_cast<uint32_t>(command_buffers_.size());

            if (vkAllocateCommandBuffers(
                device_,
                &allocate_info,
                command_buffers_.data()) != VK_SUCCESS) 
            {
                throw std::runtime_error("Failed to allocate command buffers.");
            }

            std::cout << "Command buffers allocated." << std::endl;

            for (size_t i = 0 ; i < command_buffers_.size() ; ++i)
            {
                VkCommandBufferBeginInfo begin_info{};
                begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                begin_info.flags = 0; // Optional
                begin_info.pInheritanceInfo = nullptr; // Optional

                if (vkBeginCommandBuffer(command_buffers_[i], &begin_info) != VK_SUCCESS)
                {
                    throw std::runtime_error("Failed to begin recording command buffer.");
                }
            	
                VkRenderPassBeginInfo render_pass_begin_info{};
                render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                render_pass_begin_info.renderPass = render_pass_;
                render_pass_begin_info.framebuffer = swap_chain_framebuffers_[i];
                render_pass_begin_info.renderArea.offset = { 0, 0 };
                render_pass_begin_info.renderArea.extent = swap_chain_extent_;

                VkClearValue clear_color
				{
				{
					{
							0.0f,
                			0.0f,
                			0.0f,
                			1.0f
						}
					}
				};
                render_pass_begin_info.clearValueCount = 1;
                render_pass_begin_info.pClearValues = &clear_color;

                vkCmdBeginRenderPass(
                    command_buffers_[i],
                    &render_pass_begin_info,
                    VK_SUBPASS_CONTENTS_INLINE);

                vkCmdBindPipeline(
                    command_buffers_[i], 
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    graphics_pipeline_);

            	// Add vertex info here
                vkCmdDraw(
                    command_buffers_[i],
                    3,
                    1,
                    0,
                    0);

                vkCmdEndRenderPass(command_buffers_[i]);

                if (vkEndCommandBuffer(command_buffers_[i]) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to record command buffers.");
                }

                std::cout << "Command buffer " << i << " recorded." << std::endl;
            }
        }

        void create_sync_objects()
		{
            VkSemaphoreCreateInfo semaphore_create_info{};
            semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            VkFenceCreateInfo fence_info = {};
            fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            images_in_flight_.resize(swap_chain_images_.size(), VK_NULL_HANDLE);

        	for (size_t i = 0 ; i < max_frames_in_flight ; ++i)
        	{
                if (vkCreateSemaphore(
						device_,
						&semaphore_create_info,
						nullptr,
						&image_available_semaphores_[i]) != VK_SUCCESS ||
                    vkCreateSemaphore(
                        device_,
                        &semaphore_create_info,
                        nullptr,
                        &render_finished_semaphores_[i]) != VK_SUCCESS ||
                    vkCreateFence(
                        device_,
                        &fence_info,
                        nullptr,
                        &in_flight_fences_[i]) != VK_SUCCESS)
                {
                    throw std::runtime_error("Failed to create semaphores.");
                }
        	}

            std::cout << "Sync objects created." << std::endl;
        }
		

        [[nodiscard]] std::vector<const char*> get_required_glfw_extension_names() const
    	{
            unsigned int glfw_extension_count = 0;
            // ReSharper disable once CppUseAuto
            const char** glfw_extensions = 
                glfwGetRequiredInstanceExtensions(&glfw_extension_count);

            std::vector<const char*> extension_names(glfw_extensions, 
                glfw_extensions + glfw_extension_count);

            if (enable_validation_layers_)
            {
                extension_names.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }

            return extension_names;
        }

        // ReSharper disable once CppMemberFunctionMayBeStatic
        [[nodiscard]] bool glfw_extensions_supported(
            const std::vector<const char*>& required_extension_names) const
        {
            unsigned int available_extension_count;
            if (vkEnumerateInstanceExtensionProperties(
                nullptr,
                &available_extension_count,
                nullptr) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to enumerate instance layers.");
            }

            std::vector<VkExtensionProperties> available_extensions(available_extension_count);
            if (vkEnumerateInstanceExtensionProperties(nullptr,
                &available_extension_count,
                available_extensions.data()) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to enumerate instance layers.");
            }

            for (const auto& required_extension_name : required_extension_names)
            {
                auto required_extension_found = false;
                for (const auto& available : available_extensions)
                {
                	// Check if they are equal
                    if (strcmp(required_extension_name, available.extensionName) == 0)
                    {
                        required_extension_found = true;
                        break;
                    }
                }

                if (!required_extension_found) return false;
            }

            return  true;
        }

        [[nodiscard]] bool validation_layers_supported() const
    	{
            unsigned int available_layer_count;
            if (vkEnumerateInstanceLayerProperties(
                &available_layer_count,
                nullptr) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to enumerate instance layers.");
            }

            std::vector<VkLayerProperties> available_layers(available_layer_count);
            if (vkEnumerateInstanceLayerProperties(
                &available_layer_count, 
                available_layers.data()) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to enumerate instance layers.");
            }

        	for (const auto& required_layer_name : validation_layers_names_)
            {
                auto required_layer_found = false;

                for (const auto& available : available_layers)
                {
                	// Check if they are equal
                    if (strcmp(required_layer_name, available.layerName) == 0)
                    {
                        required_layer_found = true;
                        break;
                    }
                }

                if (!required_layer_found) return false;
            }

            return true;
        }

        // ReSharper disable once CppMemberFunctionMayBeStatic
        void populate_debug_messenger_create_info(
            VkDebugUtilsMessengerCreateInfoEXT& create_info)
        {
            create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            create_info.messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            create_info.messageType =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                // For now, performance messages are disabled.
                // | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
                ;
            create_info.pfnUserCallback = debug_callback;
            create_info.pUserData = nullptr; // Optional
        }

        [[nodiscard]] bool device_suitable(const VkPhysicalDevice& device) const
		{
            const auto extensions_supported = device_extensions_supported(device);

            const auto queue_family_indices = query_queue_families(device);
            const auto queue_families_supported = queue_family_indices.is_complete();
        	
            auto swap_chain_adequate = false;
            if (extensions_supported) {
	            const auto swap_chain_support_info = query_swap_chain_support_info(device);
                swap_chain_adequate = 
                    !swap_chain_support_info.formats.empty() && 
                    !swap_chain_support_info.present_modes.empty();
            }
        	
            return queue_families_supported && extensions_supported && swap_chain_adequate;
        }

        [[nodiscard]] queue_family_indices query_queue_families(const VkPhysicalDevice& device) const
		{
            queue_family_indices indices;

            unsigned int queue_family_count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(
                device,
                &queue_family_count,
                nullptr);

            std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
            vkGetPhysicalDeviceQueueFamilyProperties(
                device,
                &queue_family_count,
                queue_families.data());

            auto i = 0;
            for (const auto& queue_family : queue_families) {
                VkBool32 present_supported = false;
                if (vkGetPhysicalDeviceSurfaceSupportKHR(
                    device, i,
                    surface_,
                    &present_supported) != VK_SUCCESS)
                {
                    throw std::runtime_error("Failed to get surface support information.");
                }
            	
                if (present_supported) {
                    indices.present_family = i;
                }
            	
                if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    indices.graphics_family = i;
                }

                if (indices.is_complete()) {
                    break;
                }

                i++;
            }
        	
            return indices;
        }

		[[nodiscard]] bool device_extensions_supported(const VkPhysicalDevice& device) const
        {
            unsigned int available_extension_count;
            if (vkEnumerateDeviceExtensionProperties(
                device,
                nullptr,
                &available_extension_count,
                nullptr) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to enumerate device extensions.");
            }

            std::vector<VkExtensionProperties> available_extension_names(available_extension_count);
            if (vkEnumerateDeviceExtensionProperties(
                device,
                nullptr,
                &available_extension_count,
                available_extension_names.data()) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to enumerate device extensions.");
            }

            std::set<std::string> required_extension_names(
                device_extension_names_.begin(),
                device_extension_names_.end());

            for (const auto& available : available_extension_names) {
                required_extension_names.erase(available.extensionName);
            }

            return required_extension_names.empty();
        }

        struct swap_chain_support_info {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> present_modes;
        };

		[[nodiscard]] swap_chain_support_info query_swap_chain_support_info(
			const VkPhysicalDevice& device) const
		{
            swap_chain_support_info support_info = {};

            if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                device,
                surface_,
                &support_info.capabilities) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to get physical device surface capabilities.");
            }

            unsigned int format_count;
            if (vkGetPhysicalDeviceSurfaceFormatsKHR(
                device,
                surface_,
                &format_count,
                nullptr) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to get physical device surface formats.");
            }

            if (format_count > 0) 
            {
                support_info.formats.resize(format_count);
            	if (vkGetPhysicalDeviceSurfaceFormatsKHR(
                    device,
                    surface_,
                    &format_count,
                    support_info.formats.data()) != VK_SUCCESS)
                {
                    throw std::runtime_error("Failed to get physical device surface formats.");
                }
            }

            unsigned int present_mode_count;
			if (vkGetPhysicalDeviceSurfacePresentModesKHR(
                device,
                surface_,
                &present_mode_count,
                nullptr) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to get physical device surface present modes.");
            }
			
            if (present_mode_count > 0) {
                support_info.present_modes.resize(present_mode_count);
            	if (vkGetPhysicalDeviceSurfacePresentModesKHR(
                    device,
                    surface_,
                    &present_mode_count,
                    support_info.present_modes.data()) != VK_SUCCESS)
                {
                    throw std::runtime_error(
                        "Failed to get physical device surface present modes.");
                }
            }
			
            return support_info;
		}

        // ReSharper disable once CppMemberFunctionMayBeStatic
        [[nodiscard]] VkSurfaceFormatKHR select_swap_surface_format(
            const std::vector<VkSurfaceFormatKHR>& available_formats) const
		{
            for (const auto& available_format : available_formats)
            {
                if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
                    available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                {
                    return available_format;
                }
            }

            std::cerr << "Failed to find a desirable surface format. \
The first available format will be selected instead." << std::endl;

            return available_formats[0];
		}

        // ReSharper disable once CppMemberFunctionMayBeStatic
        [[nodiscard]] VkPresentModeKHR select_swap_present_mode(
            const std::vector<VkPresentModeKHR>& available_present_modes) const
		{
            for (const auto& available_present_mode : available_present_modes) {
                if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                    std::cout << "Triple buffering available." << std::endl;
                    return available_present_mode;
                }
            }

            std::cout << "Triple buffering unavailable. \
Using standard FIFO presentation mode (Vsync)." << std::endl;
            return VK_PRESENT_MODE_FIFO_KHR;
        }

        [[nodiscard]] VkExtent2D select_swap_extent(
            const VkSurfaceCapabilitiesKHR capabilities) const
		{
            if (capabilities.currentExtent.width != UINT32_MAX) {
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

        [[nodiscard]] VkShaderModule create_shader_module(const std::vector<char>& code) const
		{
            VkShaderModuleCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            create_info.codeSize = code.size();
            create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

            VkShaderModule shader_module;
            if (vkCreateShaderModule(
                device_,
                &create_info,
                nullptr,
                &shader_module) != VK_SUCCESS) 
            {
                throw std::runtime_error("failed to create shader module!");
            }

            return shader_module;
        }
		
        static VkResult create_debug_utils_messenger_ext(
            // ReSharper disable once CppParameterMayBeConst
            VkInstance instance,
            const VkDebugUtilsMessengerCreateInfoEXT* p_create_info,
            const VkAllocationCallbacks* p_allocator,
            VkDebugUtilsMessengerEXT* p_debug_messenger)
        {
            const auto func = PFN_vkCreateDebugUtilsMessengerEXT(
                vkGetInstanceProcAddr(
                    instance,
                    "vkCreateDebugUtilsMessengerEXT")
            );

            if (func != nullptr)
            {
                return func(instance, p_create_info, p_allocator, p_debug_messenger);
            }
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        static void destroy_debug_utils_messenger_ext(
            // ReSharper disable once CppParameterMayBeConst
            VkInstance instance,
            // ReSharper disable once CppParameterMayBeConst
            VkDebugUtilsMessengerEXT debug_messenger,
            const VkAllocationCallbacks* p_allocator)
        {
            const auto func = PFN_vkDestroyDebugUtilsMessengerEXT(
                vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

            if (func != nullptr) {
                func(instance, debug_messenger, p_allocator);
            }
        }

        static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
            VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
            VkDebugUtilsMessageTypeFlagsEXT message_type,
            const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
            void* p_user_data)
		{
            std::cerr << "Validation layer: " << std::endl << p_callback_data->pMessage << std::endl;

            return VK_FALSE;
        }
    };
}