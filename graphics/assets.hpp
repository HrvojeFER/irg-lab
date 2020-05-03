#ifndef GRAPHICS_ASSETS_HPP
#define GRAPHICS_ASSETS_HPP


#include "pch.hpp"


namespace irglab
{
    struct compiled_shader_paths
    {
        std::string vertex;
        std::string fragment;
    } const compiled_shader_paths
    {
#if !defined(NDEBUG)
        "./shaders/compiled/vertex_shader.spirv",
        "./shaders/compiled/fragment_shader.spirv"
#else
        "./shaders/vertex_shader.spirv",
        "./shaders/fragment_shader.spirv"
#endif
    };

    [[nodiscard]] inline std::vector<char> read_shader_file(const std::string& path)
    {
#if !defined (NDEBUG)
        std::cout << "Reading shader file at: '" << path << "'." << std::endl;
#endif
    	
    	// start at the end with std::ios:ate to get the size
        std::ifstream file{ path, std::ios::ate | std::ios::binary };
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file from path '" + path + "'.");
        }

        const auto file_size = file.tellg();
        std::vector<char> buffer(static_cast<size_t>(file_size));
    	// back to start to read
        file.seekg(0);
        file.read(buffer.data(), static_cast<std::streamsize>(file_size));

        return buffer;
    }

	[[nodiscard]] static std::vector<std::string> read_object_file(const std::string& path)
	{
        std::cout << "Reading object file at: '" << path << "'." << std::endl;
    	
        std::ifstream file{ path };
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file from path '" + path + "'.");
        }

        std::vector<std::string> result;
        for (std::string line; std::getline(file, line); )
		{
        	// Check if empty or contains only whitespace
        	if (!line.empty() && 
                !(line.find_first_not_of(' ') == std::string::npos) && 
                !(line.find_first_not_of('\t') == std::string::npos))
        	{
                result.emplace_back(line);
        	}
		}

		return result;
	}
}

#endif
