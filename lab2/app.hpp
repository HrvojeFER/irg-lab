// ReSharper disable CppZeroConstantCanBeReplacedWithNullptr
#ifndef APP_HPP
#define APP_HPP

#pragma once

#include "pch.hpp"

#include "app_utils.hpp"

namespace irglab
{
	class app {
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
		
        const int window_width_ = 800;
        const int window_height_ = 600;
        const std::string window_title_ = app_name_;
        GLFWwindow* window_ = nullptr;

        VkInstance instance_ = VK_NULL_HANDLE;
        const std::vector<const char*> validation_layers_ = 
        {
			"VK_LAYER_KHRONOS_validation"
        };
#ifdef NDEBUG
        const bool enable_validation_layers_ = false;
#else
        const bool enable_validation_layers_ = true;
#endif
        VkDebugUtilsMessengerEXT debug_messenger_ = VK_NULL_HANDLE;  // NOLINT(clang-diagnostic-unused-private-field)
        VkSurfaceKHR surface_ = VK_NULL_HANDLE;
		
        VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
        VkDevice device_ = VK_NULL_HANDLE;

		VkQueue graphics_queue_ = VK_NULL_HANDLE;
        VkQueue present_queue_ = VK_NULL_HANDLE;
		
        void init_window()
        {
            glfwInit();
        	
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

            window_ = glfwCreateWindow(
                window_width_,
                window_height_,
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
        }

        void main_loop()
    	{
            while (!glfwWindowShouldClose(window_)) 
            {
                glfwPollEvents();
            }
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void cleanup()
    	{
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

		
        void create_instance()
		{
            VkApplicationInfo app_info = {};
            app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            app_info.pApplicationName = app_name_.c_str();
            app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            app_info.pEngineName = "No Engine";
            app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            app_info.apiVersion = VK_API_VERSION_1_2;

        	
            VkInstanceCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            create_info.pApplicationInfo = &app_info;

        	
            auto required_glfw_extensions = get_required_glfw_extensions();
            if (!glfw_extensions_supported(required_glfw_extensions))
            {
                throw std::runtime_error("GLFW extensions not supported");
            }
            create_info.enabledExtensionCount = static_cast<uint32_t>(required_glfw_extensions.size());
            create_info.ppEnabledExtensionNames = required_glfw_extensions.data();


            VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {};
            populate_debug_messenger_create_info(debug_create_info);
            if (enable_validation_layers_)
            {
                if (!validation_layers_supported())
                {
                    throw std::runtime_error("Validation layers not supported.");
                }

                create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers_.size());
                create_info.ppEnabledLayerNames = validation_layers_.data();

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
        	
            VkDebugUtilsMessengerCreateInfoEXT create_info = {};
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
                    break;
                }
            }

            // ReSharper disable once CppZeroConstantCanBeReplacedWithNullptr
            if (physical_device_ == VK_NULL_HANDLE) {
                throw std::runtime_error("Failed to find a suitable GPU.");
            }

            std::cout << "Physical device selected." << std::endl;
        }

        void create_logical_device() {
            auto indices = find_queue_families(physical_device_);

            std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
            std::set<uint32_t> unique_queue_families = 
            {
            	indices.graphics_family.value(),
            	indices.present_family.value()
            };

            auto queue_priority = 1.0f;
            for (auto queue_family : unique_queue_families) {
                VkDeviceQueueCreateInfo queue_create_info = {};
                queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queue_create_info.queueFamilyIndex = queue_family;
                queue_create_info.queueCount = 1;
                queue_create_info.pQueuePriorities = &queue_priority;

            	queue_create_infos.push_back(queue_create_info);
            }
        	
            VkDeviceQueueCreateInfo queue_create_info = {};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = indices.graphics_family.value();
            queue_create_info.queueCount = 1;
            queue_create_info.pQueuePriorities = &queue_priority;

            VkPhysicalDeviceFeatures device_features = {};
        	
            VkDeviceCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
            create_info.pQueueCreateInfos = queue_create_infos.data();
            create_info.pEnabledFeatures = &device_features;

            create_info.enabledExtensionCount = 0;

            if (enable_validation_layers_)
            {
                create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers_.size());
                create_info.ppEnabledLayerNames = validation_layers_.data();
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
                indices.graphics_family.value(),
                0,
                &graphics_queue_);

            vkGetDeviceQueue(
                device_,
                indices.present_family.value(),
                0, &present_queue_);

            std::cout << "Device queues created." << std::endl;
        }


        [[nodiscard]] std::vector<const char*> get_required_glfw_extensions() const
    	{
            uint32_t glfw_extension_count = 0;
            // ReSharper disable once CppUseAuto
            const char** glfw_extensions = 
                glfwGetRequiredInstanceExtensions(&glfw_extension_count);

            std::vector<const char*> extensions(
                glfw_extensions, 
                glfw_extensions + glfw_extension_count);

            if (enable_validation_layers_) {
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }

            return extensions;
        }

        // ReSharper disable once CppMemberFunctionMayBeStatic
        bool glfw_extensions_supported(
            const std::vector<const char*>& required_extensions)
        {
            uint32_t extension_count;
            if (vkEnumerateInstanceExtensionProperties(
                nullptr,
                &extension_count,
                nullptr) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to enumerate instance layers.");
            }

            std::vector<VkExtensionProperties> available_extensions(extension_count);
            if (vkEnumerateInstanceExtensionProperties(nullptr,
                &extension_count,
                available_extensions.data()) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to enumerate instance layers.");
            }

            for (const auto& glfw_extension : required_extensions)
            {
                auto qlfw_extension_found = false;
                for (const auto& extension : available_extensions)
                {
                    if (strcmp(extension.extensionName, glfw_extension) == 0)
                    {
                        qlfw_extension_found = true;
                        break;
                    }
                }

                if (!qlfw_extension_found) return false;
            }

            return  true;
        }

        [[nodiscard]] bool validation_layers_supported() const
    	{
            uint32_t layer_count;
            if (vkEnumerateInstanceLayerProperties(&layer_count, nullptr) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to enumerate instance layers.");
            }

            std::vector<VkLayerProperties> available_layers(layer_count);
            if (vkEnumerateInstanceLayerProperties(
                &layer_count, 
                available_layers.data()) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to enumerate instance layers.");
            }

        	for (const auto& layer_name : validation_layers_)
            {
                auto layer_found = false;

                for (const auto& layer_properties : available_layers)
                {
                    if (strcmp(layer_name, layer_properties.layerName) == 0)
                    {
                        layer_found = true;
                        break;
                    }
                }

                if (!layer_found) return false;
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

        // ReSharper disable once CppMemberFunctionMayBeStatic
        bool device_suitable(const VkPhysicalDevice& device)
		{
			const auto indices = find_queue_families(device);
        	
            return indices.is_complete();
        }

        struct queue_family_indices
		{
            std::optional<uint32_t> graphics_family;
            std::optional<uint32_t> present_family;

            [[nodiscard]] bool is_complete() const {
                return graphics_family.has_value() && present_family.has_value();
            }
        };

        // ReSharper disable once CppMemberFunctionMayBeStatic
        queue_family_indices find_queue_families(const VkPhysicalDevice& device)
		{
            queue_family_indices indices;

            uint32_t queue_family_count = 0;
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
                VkBool32 present_support = false;
                if (vkGetPhysicalDeviceSurfaceSupportKHR(
                    device, i,
                    surface_,
                    &present_support) != VK_SUCCESS)
                {
                    throw std::runtime_error("Failed to get surface support information.");
                }
            	
                if (present_support) {
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

		
        static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
            VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
            VkDebugUtilsMessageTypeFlagsEXT message_type,
            const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
            void* p_user_data)
		{
            std::cerr << "validation layer: " << p_callback_data->pMessage << std::endl;

            return VK_FALSE;
        }
    };
}

#endif
