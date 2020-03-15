#ifndef APP_UTILS_HPP
#define APP_UTILS_HPP

#pragma once

#include "pch.hpp"

namespace irglab
{
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
