#pragma once

#include "pch.hpp"

namespace irglab
{
    struct compiled_shader_paths
    {
        std::string vertex;
        std::string fragment;
    } const compiled_shader_paths
    {
        "./shaders/vertex_shader.spirv",
        "./shaders/fragment_shader.spirv"
    };

    inline std::vector<char> read_file(const std::string& path)
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
