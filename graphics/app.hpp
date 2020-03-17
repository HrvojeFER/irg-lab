#ifndef GRAPHICS_APP_HPP
#define GRAPHICS_APP_HPP


#include "pch.hpp"


// TODO: add window somehow
#include "window.hpp"


namespace irglab
{
	class app
	{
	public:
        const std::string name;

        explicit app(std::string name) :
            name(std::move(name)),
			instance_(create_instance()),
			debug_messenger_(create_debug_messenger(), debug_messenger_deleter_) {}
		
	private:
		
		
        // Vulkan instance
#if !defined(NDEBUG)
        const std::array<std::string, 1> validation_layer_names_
        {
            "VK_LAYER_KHRONOS_validation"
        };
#endif

        const vk::UniqueInstance instance_;

        [[nodiscard]] vk::UniqueInstance create_instance() const
        {
            const vk::ApplicationInfo app_info
            {
                name.c_str(),
                1,
                "No Engine",
                1,
                VK_API_VERSION_1_2
            };

            std::vector<char*> extension_names{};
            std::vector<char*> layer_names{};

            auto required_glfw_extension_names = std::vector<std::string>{};
            extension_names.insert(
                extension_names.end(),
                required_glfw_extension_names.begin(),
                required_glfw_extension_names.end());

#if !defined(NDEBUG)
            for (const auto& validation_layer_name : validation_layer_names_)
            {
                layer_names.push_back(const_cast<char*>(validation_layer_name.c_str()));
            }

            extension_names.push_back(const_cast<char*>(VK_EXT_DEBUG_UTILS_EXTENSION_NAME));
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

        [[nodiscard]] static bool extensions_supported(
            const std::vector<char*>& required_extension_names)
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
            const std::vector<char*>& required_layer_names)
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

		
        // Debug messenger
#if !defined(NDEBUG)
        using debug_messenger_deleter_type = std::function<void(VkDebugUtilsMessengerEXT*)>;
        using debug_messenger_type = std::unique_ptr<
            VkDebugUtilsMessengerEXT,
            debug_messenger_deleter_type>;
        const debug_messenger_type debug_messenger_;

        [[nodiscard]] VkDebugUtilsMessengerEXT* create_debug_messenger() const
        {
            VkDebugUtilsMessengerCreateInfoEXT create_info
            {
                VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                nullptr,
                {},
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                // For now, performance messages are disabled.
                // | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
                ,
                &debug_callback,
                nullptr
            };

            const auto create_function = PFN_vkCreateDebugUtilsMessengerEXT(
                instance_->getProcAddr("vkCreateDebugUtilsMessengerEXT")
            );

            if (create_function != nullptr)
            {
                VkDebugUtilsMessengerEXT debug_messenger;
                if (create_function(
                    instance_.get(),
                    &create_info,
                    nullptr,
                    &debug_messenger) == VK_SUCCESS)
                {
                    return &debug_messenger;
                }
            }

            throw std::runtime_error("Failed initializing debug messenger.");
        }

        std::function<void(VkDebugUtilsMessengerEXT*)> debug_messenger_deleter_ =
            [this](VkDebugUtilsMessengerEXT* debug_messenger)
        {
            const auto destroy_function = PFN_vkDestroyDebugUtilsMessengerEXT(
                instance_->getProcAddr("vkDestroyDebugUtilsMessengerEXT"));

            if (destroy_function != nullptr) {
                destroy_function(instance_.get(), *debug_messenger, nullptr);
            }
        };

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
	};
}

#endif
