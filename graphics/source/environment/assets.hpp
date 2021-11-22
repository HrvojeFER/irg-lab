#ifndef IRGLAB_ASSETS_HPP
#define IRGLAB_ASSETS_HPP


#include "external/external.hpp"


namespace il
{
    [[nodiscard, maybe_unused]] inline std::vector<char> read_shader_file(const std::string &path)
    {
#if !defined (NDEBUG)
        std::cout << "Reading shader file at: '" << path << "'." << std::endl;
#endif

        // Start at the end_owned with std::ios:ate to get the size.
        std::ifstream file{path, std::ios::ate | std::ios::binary};
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file from path '" + path + "'.");
        }

        const auto file_size = file.tellg();
        std::vector<char> buffer(static_cast<size_t>(file_size));
        // Back to start to read.
        file.seekg(0);
        file.read(buffer.data(), static_cast<std::streamsize>(file_size));

        return buffer;
    }

    [[nodiscard, maybe_unused]] static std::vector<std::string> read_object_file(const std::string &path)
    {
#if !defined (NDEBUG)
        std::cout << "Reading object file at: '" << path << "'." << std::endl;
#endif

        std::ifstream file{path};
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file from path '" + path + "'.");
        }

        std::vector<std::string> result;
        for (std::string line ; std::getline(file, line) ;)
        {
            // Check if empty or contains only whitespace
            if (!line.empty() &&
                line.find_first_not_of(' ') != std::string::npos &&
                line.find_first_not_of('\t') != std::string::npos)
            {
                result.emplace_back(line);
            }
        }

        return result;
    }
}

#endif
