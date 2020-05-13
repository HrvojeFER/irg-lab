#ifndef IRGLAB_SHADER_MANAGER_HPP
#define IRGLAB_SHADER_MANAGER_HPP


#include "pch.hpp"

#include "assets.hpp"

#include "device.hpp"


namespace irglab
{
	struct shader_manager
	{
		struct request
		{
            vk::ShaderStageFlagBits shader_stage_flag;
			
            std::string path;
		};

        using configuration = std::initializer_list<request>;

		explicit shader_manager(const configuration& configuration, const device& device) :
			shader_module_indices_{ configuration.size() },
			shader_modules_{ configuration.size() },
			shader_stages_create_info_{ configuration.size() }
		{
            size_t i = 0;
			
			for (const auto& request : configuration)
			{
                shader_modules_[i] = create_shader_module(
#if !defined(NDEBUG)
                    request.shader_stage_flag,
#endif
                    read_shader_file(request.path),
                    device
                );

                shader_module_indices_[i] = shader_index
                    {
                    	request.shader_stage_flag,
                    	i
                    };

                shader_stages_create_info_[i] = 
                    create_shader_stage_create_info(shader_module_indices_[i]);
				
                ++i;
			}
		}

        [[nodiscard]] const std::vector<vk::PipelineShaderStageCreateInfo>& 
            shader_stages_create_info() const
		{
            return shader_stages_create_info_;
		}
		
		
	private:
		struct shader_index
		{
            vk::ShaderStageFlagBits shader_stage_flag;
            size_t index;
		};
        std::vector<shader_index> shader_module_indices_;
        std::vector<vk::UniqueShaderModule> shader_modules_;

        static inline const char* shader_main_function_name_ = "main";
        std::vector<vk::PipelineShaderStageCreateInfo> shader_stages_create_info_;

		
        [[nodiscard]] static vk::UniqueShaderModule create_shader_module(
#if !defined(NDEBUG)
            const vk::ShaderStageFlagBits shader_stage_flag,
#endif
            const std::vector<char>& code,
            const device& device)
        {
            auto result = device->createShaderModuleUnique(
                {
                    {},
                    code.size(),
                    reinterpret_cast<const unsigned int*>(code.data())
                });

#if !defined(NDEBUG)
            std::cout << to_string(shader_stage_flag) << " shader module created." << std::endl;
#endif

            return result;
        }

        [[nodiscard]] vk::PipelineShaderStageCreateInfo create_shader_stage_create_info(
            const shader_index& shader_index)
        {
            return vk::PipelineShaderStageCreateInfo
            {
                {},
                shader_index.shader_stage_flag,
                *shader_modules_[shader_index.index],
                shader_main_function_name_,
                {}
            };
        }
	};
}


#endif
