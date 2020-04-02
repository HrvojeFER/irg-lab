#ifndef GRAPHICS_ENVIRONMENT_HPP
#define GRAPHICS_ENVIRONMENT_HPP


#include "pch.hpp"
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#include "debug_manager.hpp"


namespace irglab
{
	struct environment
	{
        static inline const std::string_view default_name = "IrgLab";
        const std::string_view name;

        explicit environment(const std::string_view name = default_name) :
			name(name),
            instance_{ create_instance() },
#if !defined(NDEBUG)
            debug_messenger_
				{
					instance_->createDebugUtilsMessengerEXTUnique(
                        debug_manager::vulkan_debug_messenger_create_info)
				}
#endif
        {
#if !defined(NDEBUG)
            std::cout << "Vulkan debug messenger established" << std::endl;
        	
            std::cout << std::endl << "---- Environment done ----" << std::endl << std::endl << std::endl;
#endif
        }

		~environment()
        {
            glfwTerminate();
        }
        environment(environment&) = delete;
        environment(environment&&) = delete;
        environment& operator =(environment&) = delete;
        environment& operator =(environment&&) = delete;


        [[nodiscard]] const vk::Instance& vulkan_instance() const
        {
            return *instance_;
        }

		void update_dynamic_loader(const vk::Device& device) const
        {
        	VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance_, device);
        }
		

    private:
        const vk::UniqueInstance instance_;
#if !defined(NDEBUG)
        const vk::UniqueDebugUtilsMessengerEXT debug_messenger_;
#endif

        [[nodiscard]] vk::UniqueInstance create_instance() const
        {
            initialize_glfw();

            initialize_dynamic_loader();

            const vk::ApplicationInfo app_info
            {
                name.data(),
                1,
                "No Engine",
                1,
                VK_API_VERSION_1_2
            };

            std::vector<const char*> extension_names{};
            std::vector<const char*> layer_names{};

            auto required_glfw_extension_names = get_required_glfw_extension_names();
            extension_names.insert(
                extension_names.end(),
                required_glfw_extension_names.begin(),
                required_glfw_extension_names.end());

#if !defined(NDEBUG)
            for (const auto& validation_layer_name : debug_manager::validation_layer_names)
            {
                layer_names.push_back(validation_layer_name);
            }

            extension_names.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

            if (!extensions_supported(extension_names))
            {
                throw std::runtime_error("Extensions not supported.");
            }

            if (!layers_supported(layer_names))
            {
                throw std::runtime_error("Layers not supported.");
            }

            vk::InstanceCreateInfo create_info
            {
                {},
                &app_info,
                static_cast<unsigned int>(layer_names.size()),
                layer_names.data(),
                static_cast<unsigned int>(extension_names.size()),
                extension_names.data()
            };

#if !defined(NDEBUG)
            auto p_next = vk::StructureChain<vk::DebugUtilsMessengerCreateInfoEXT>
            {
                debug_manager::vulkan_debug_messenger_create_info
            };
            create_info.setPNext(&p_next);
#endif
            auto result = createInstanceUnique(create_info);
#if !defined(NDEBUG)
            std::cout << "Vulkan instance created" << std::endl;
#endif

        	VULKAN_HPP_DEFAULT_DISPATCHER.init(*result);
#if !defined(NDEBUG)
            std::cout << "Dynamic loader updated" << std::endl;
#endif
        	
            return result;
        }

		static void initialize_glfw()
        {
#if !defined(NDEBUG)
            glfwSetErrorCallback(debug_manager::glfw_callback);
        	
            std::cout << "GLFW debug messenger established" << std::endl;
#endif
        	
            glfwInit();
        	
#if !defined(NDEBUG)
            std::cout << "GLFW initialized" << std::endl;
#endif

        }

        static void initialize_dynamic_loader()
        {
            const vk::DynamicLoader dynamic_loader;
            const auto vk_get_instance_proc_address =
                // ReSharper disable once StringLiteralTypo
                dynamic_loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
            VULKAN_HPP_DEFAULT_DISPATCHER.init(vk_get_instance_proc_address);

#if !defined(NDEBUG)
            std::cout << "Dynamic loader initialized" << std::endl;
#endif
        }

        [[nodiscard]] static std::vector<const char*> get_required_glfw_extension_names()
        {
            unsigned int count;
            const auto name_char_array = glfwGetRequiredInstanceExtensions(&count);

            std::vector<const char*> result{ name_char_array, name_char_array + count };

            return result;
        }

        [[nodiscard]] static bool extensions_supported(
            const std::vector<const char*>& required_extension_names)
        {
            auto available_extension_properties =
                vk::enumerateInstanceExtensionProperties();

            for (auto required_extension_name : required_extension_names)
            {
                auto found = false;
                for (auto available : available_extension_properties)
                {
                    if (strcmp(required_extension_name, available.extensionName) == 0)
                    {
                        found = true;
                        break;
                    }
                }

                if (!found) return false;
            }

            return true;
        }

        [[nodiscard]] static bool layers_supported(
            const std::vector<const char*>& required_layer_names)
        {
            auto available_layer_properties =
                vk::enumerateInstanceLayerProperties();

            for (auto required_layer_name : required_layer_names)
            {
                auto found = false;
                for (auto available : available_layer_properties)
                {
                    if (strcmp(required_layer_name, available.layerName) == 0)
                    {
                        found = true;
                        break;
                    }
                }

                if (!found) return false;
            }

            return true;
        }
    };
}


#endif
