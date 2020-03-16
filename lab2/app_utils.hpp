#ifndef APP_UTILS_HPP
#define APP_UTILS_HPP

#pragma once

#include "pch.hpp"

namespace irglab
{
    inline VkResult create_debug_utils_messenger_ext(
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

    inline void destroy_debug_utils_messenger_ext(
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

    inline std::vector<char> read_shader_file(const std::string& path)
    {
        std::ifstream file(path, std::ios::ate | std::ios::binary);

    	if (!file.is_open())
    	{
            throw std::runtime_error("Failed to open file from path '" + path + "'.");
    	}

        const auto file_size = file.tellg();
        std::vector<char> buffer(static_cast<size_t>(file_size));
        file.seekg(0);
        file.read(buffer.data(), static_cast<std::streamsize>(file_size));

        return buffer;
    }
}

#endif
