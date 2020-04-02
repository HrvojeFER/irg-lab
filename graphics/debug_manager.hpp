#ifndef GRAPHICS_DEBUG_MESSENGER_HPP
#define GRAPHICS_DEBUG_MESSENGER_HPP


#include "pch.hpp"


namespace irglab
{
	class debug_manager
	{
        static vk::Bool32 vulkan_callback(
            VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
            // ReSharper disable once CppParameterMayBeConst
            VkDebugUtilsMessageTypeFlagsEXT message_types,
            const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
            void*  /* p_user_data */);
		
	public:
        static inline const std::array<const char*, 1> validation_layer_names
        {
            "VK_LAYER_KHRONOS_validation"
        };

        static inline const vk::DebugUtilsMessengerCreateInfoEXT vulkan_debug_messenger_create_info
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
            &vulkan_callback,
            nullptr
        };

        // ReSharper disable once CppParameterMayBeConst
        static void glfw_callback(int code, const char* description)
        {
            std::cerr << "GLFW error:" << std::endl;

            std::cerr << "\t" << "Code: " << code << std::endl;

            std::cerr << "\t" << "Description: " << description << std::endl;
        }
	};
	
    inline vk::Bool32 debug_manager::vulkan_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        // ReSharper disable once CppParameterMayBeConst
        VkDebugUtilsMessageTypeFlagsEXT message_types,
        const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
        void*  /* p_user_data */)
    {
        std::ostringstream message;

        message <<
            to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(message_severity)) <<
            ": " << to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(message_types)) <<
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
}


#endif // !GRAPHICS_DEBUG_MESSENGER_HPP
