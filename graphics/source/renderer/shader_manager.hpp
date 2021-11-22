#ifndef IRGLAB_SHADER_MANAGER_HPP
#define IRGLAB_SHADER_MANAGER_HPP


#include "external/external.hpp"

#include "environment/assets.hpp"

#include "environment/Device.hpp"


namespace il
{
    struct [[maybe_unused]] shader_manager
    {
        struct [[maybe_unused]] request
        {
            vk::ShaderStageFlagBits shader_stage_flag;

            std::string path;
        };

        using configuration = std::initializer_list<request>;

        [[nodiscard, maybe_unused]] explicit shader_manager(const configuration &configuration, const device &device) :
                _shader_module_indices{configuration.size()},
                _shader_modules{configuration.size()},
                _shader_stages_create_info{configuration.size()}
        {
            size_t i = 0;

            for (const auto &request : configuration)
            {
                _shader_modules[i] = _create_shader_module(
#if !defined(NDEBUG)
                        request.shader_stage_flag,
#endif
                        read_shader_file(request.path),
                        device
                );

                _shader_module_indices[i] = shader_index
                        {
                                request.shader_stage_flag,
                                i
                        };

                _shader_stages_create_info[i] =
                        _create_shader_stage_create_info(_shader_module_indices[i]);

                ++i;
            }
        }

        [[nodiscard, maybe_unused]] const std::vector<vk::PipelineShaderStageCreateInfo>&
                shader_stages_create_info() const
        {
            // WTF?!
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
            return _shader_stages_create_info;
#pragma clang diagnostic pop
        }


    private:
        struct shader_index
        {
            vk::ShaderStageFlagBits shader_stage_flag;
            size_t index;
        };
        std::vector<shader_index> _shader_module_indices;
        std::vector<vk::UniqueShaderModule> _shader_modules;

        static inline const char *_shader_main_function_name = "main";
        std::vector<vk::PipelineShaderStageCreateInfo> _shader_stages_create_info;


        [[nodiscard]] static vk::UniqueShaderModule _create_shader_module(
#if !defined(NDEBUG)
                const vk::ShaderStageFlagBits shader_stage_flag,
#endif
                const std::vector<char> &code,
                const device &device)
        {
            auto result = device->createShaderModuleUnique(
                    {
                            { },
                            code.size(),
                            reinterpret_cast<const unsigned int *>(code.data())
                    });

#if !defined(NDEBUG)
            std::cout << to_string(shader_stage_flag) << " shader module created." << std::endl;
#endif

            return result;
        }

        [[nodiscard]] vk::PipelineShaderStageCreateInfo _create_shader_stage_create_info(
                const shader_index &shader_index)
        {
            return vk::PipelineShaderStageCreateInfo
                    {
                            { },
                            shader_index.shader_stage_flag,
                            *_shader_modules[shader_index.index],
                            _shader_main_function_name,
                            { }
                    };
        }
    };
}


#endif
