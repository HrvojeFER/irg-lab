#ifndef APP_HPP
#define APP_HPP

#pragma once

#include "pch.hpp"

namespace irglab
{
	// ReSharper disable once CppClassNeedsConstructorBecauseOfUninitializedMember
	class app {  // NOLINT(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
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
        // ReSharper disable once CppUninitializedNonStaticDataMember
        GLFWwindow* window_;

        // ReSharper disable once CppUninitializedNonStaticDataMember
        VkInstance instance_;

        const std::vector<const char*> validation_layers_ = 
        {
			"VK_LAYER_KHRONOS_validation"
        };
#ifdef NDEBUG
        const bool enable_validation_layers_ = false;
#else
        const bool enable_validation_layers_ = true;
#endif
        VkDebugUtilsMessengerEXT debug_messenger_;

		
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
        }

        void main_loop()
    	{
            while (!glfwWindowShouldClose(window_)) 
            {
                glfwPollEvents();
            }
        }

        void cleanup()
    	{
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

        	
            if (enable_validation_layers_)
            {
                if (!validation_layers_supported())
                {
                    throw std::runtime_error("Validation layers not supported.");
                }

                create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers_.size());
                create_info.ppEnabledLayerNames = validation_layers_.data();
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
            create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            create_info.messageSeverity = 
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
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


        std::vector<const char*> get_required_glfw_extensions() const {
            uint32_t glfw_extension_count = 0;
            // ReSharper disable once CppUseAuto
            const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

            std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

            if (enable_validation_layers_) {
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }

            return extensions;
        }
		
        bool validation_layers_supported() const {
            uint32_t layer_count;
            if (vkEnumerateInstanceLayerProperties(&layer_count, nullptr) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to enumerate instance layers.");
            }

            std::vector<VkLayerProperties> available_layers(layer_count);
            if (vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data()) != VK_SUCCESS)
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

		
        static bool glfw_extensions_supported(
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
