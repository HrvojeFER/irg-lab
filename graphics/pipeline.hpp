#ifndef GRAPHICS_PIPELINE_HPP
#define GRAPHICS_PIPELINE_HPP


#include "pch.hpp"

#include "assets.hpp"
#include "swapchain.hpp"
#include "synchronizer.hpp"


namespace irglab
{
	struct pipeline
	{
		explicit pipeline(const device& device, const swapchain& swapchain) :
            render_pass_{ create_render_pass(device, swapchain) },
            pipeline_layout_{ create_pipeline_layout(device) },
            vertex_shader_module_{ create_shader_module(compiled_shader_paths.vertex, device) },
            fragment_shader_module_{ create_shader_module(compiled_shader_paths.fragment, device) },
            inner_{ create_graphics_pipeline(device, swapchain) },
            image_views_{ create_image_views(device, swapchain) },
            framebuffers_{ create_frame_buffers(device, swapchain) },
            command_pool_{ create_command_pool(device) },
            command_buffers_{ create_command_buffers(device, swapchain) }
		{
#if !defined(NDEBUG)
            std::cout << std::endl << "-- Pipeline done --" << std::endl << std::endl;
#endif
		}

        [[nodiscard]] const vk::Pipeline& operator *() const
		{
            return *inner_;
		}

		[[nodiscard]] std::vector<std::reference_wrapper<const vk::CommandBuffer>> command_buffers()
			const
		{
            return dereference_handles(command_buffers_);
		}

        void recreate(const device& device, const swapchain& swapchain)
		{
            render_pass_ = create_render_pass(device, swapchain);
            pipeline_layout_ = create_pipeline_layout(device);
            inner_ = create_graphics_pipeline(device, swapchain);
            image_views_ = create_image_views(device, swapchain);
            framebuffers_ = create_frame_buffers(device, swapchain);
            command_buffers_ = create_command_buffers(device, swapchain);
#if !defined(NDEBUG)
            std::cout << std::endl << "-- Pipeline recreated --" << std::endl << std::endl;
#endif
		}
		
	private:
        vk::UniqueRenderPass render_pass_;
		
        vk::UniquePipelineLayout pipeline_layout_;

        const char* shader_main_function_name_ = "main";
        const vk::UniqueShaderModule vertex_shader_module_;
        const vk::UniqueShaderModule fragment_shader_module_;

        vk::UniquePipeline inner_;

        std::vector<vk::UniqueImageView> image_views_;
        std::vector<vk::UniqueFramebuffer> framebuffers_;
        
        const vk::UniqueCommandPool command_pool_;
        std::vector<vk::UniqueCommandBuffer> command_buffers_;


        [[nodiscard]] static vk::UniqueRenderPass create_render_pass(
            const device& device,
            const swapchain& swapchain)
        {
            std::vector<vk::AttachmentDescription> color_attachment_descriptions
            {
                vk::AttachmentDescription
                {
                    {},
                    swapchain.get_configuration().format,
                    vk::SampleCountFlagBits::e1,
                    vk::AttachmentLoadOp::eClear,
                    vk::AttachmentStoreOp::eStore,
                    vk::AttachmentLoadOp::eDontCare,
                    vk::AttachmentStoreOp::eDontCare,
                    {},
                    vk::ImageLayout::ePresentSrcKHR
                }
            };

            std::vector<vk::AttachmentReference> color_attachment_references
            {
                vk::AttachmentReference
                {
                    0,
                    vk::ImageLayout::eColorAttachmentOptimal
                }
            };

            std::vector<vk::SubpassDescription> subpass_descriptions
            {
                vk::SubpassDescription
                {
                    {},
                    vk::PipelineBindPoint::eGraphics,
                    0,
                    nullptr,
                    static_cast<unsigned int>(color_attachment_references.size()),
                    color_attachment_references.data(),
                    nullptr,
                    nullptr,
                    0,
                    nullptr
                }
            };

            std::vector<vk::SubpassDependency> subpass_dependencies
            {
                vk::SubpassDependency
                {
                    VK_SUBPASS_EXTERNAL,
                    0,
                    vk::PipelineStageFlagBits::eColorAttachmentOutput,
                    vk::PipelineStageFlagBits::eColorAttachmentOutput,
                    {},
                    vk::AccessFlagBits::eColorAttachmentRead
                        | vk::AccessFlagBits::eColorAttachmentWrite,
                    {}
                }
            };

            auto result = device->createRenderPassUnique(
                {
                    {},
                    static_cast<unsigned int>(color_attachment_descriptions.size()),
                    color_attachment_descriptions.data(),
                    static_cast<unsigned int>(subpass_descriptions.size()),
                    subpass_descriptions.data(),
                    static_cast<unsigned int>(subpass_dependencies.size()),
                    subpass_dependencies.data()
                });

#if !defined(NDEBUG)
            std::cout << "Render pass created" << std::endl;
#endif

            return result;
        }

        [[nodiscard]] static vk::UniquePipelineLayout create_pipeline_layout(const device& device)
        {
            std::vector<vk::DescriptorSetLayout> descriptor_set_layouts{};
            std::vector<vk::PushConstantRange> push_constant_ranges{};

            auto result = device->createPipelineLayoutUnique(
                {
                    {},
                    static_cast<unsigned int>(descriptor_set_layouts.size()),
                    descriptor_set_layouts.data(),
                    static_cast<unsigned int>(push_constant_ranges.size()),
                    push_constant_ranges.data()
                });

#if !defined(NDEBUG)
            std::cout << "Pipeline layout created" << std::endl;
#endif

            return result;
        }

        [[nodiscard]] static vk::UniqueShaderModule create_shader_module(
            const std::string& path,
            const device& device)
        {
            const auto code = irglab::read_file(path);

            auto result = device->createShaderModuleUnique(
                {
                    {},
                    code.size(),
                    reinterpret_cast<const unsigned int*>(code.data())
                });

#if !defined(NDEBUG)
            std::cout << "Shader module at \"" << path << "\" created" << std::endl;
#endif

            return result;
        }

        [[nodiscard]] vk::UniquePipeline create_graphics_pipeline(
            const device& device,
            const swapchain& swapchain) const
        {
            std::vector<vk::PipelineShaderStageCreateInfo> shader_stages_create_info
            {
                vk::PipelineShaderStageCreateInfo
                {
                    {},
                    vk::ShaderStageFlagBits::eVertex,
                    *vertex_shader_module_,
                    shader_main_function_name_,
                    {}
                },
                vk::PipelineShaderStageCreateInfo
                {
                    {},
                    vk::ShaderStageFlagBits::eFragment,
                    *fragment_shader_module_,
                    shader_main_function_name_,
                    {}
                }
            };

            std::vector<vk::VertexInputBindingDescription> vertex_input_binding_descriptions{};
            std::vector<vk::VertexInputAttributeDescription> vertex_input_attribute_descriptions{};

            vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info
            {
                {},
                static_cast<unsigned int>(vertex_input_binding_descriptions.size()),
                vertex_input_binding_descriptions.data(),
                static_cast<unsigned int>(vertex_input_attribute_descriptions.size()),
                vertex_input_attribute_descriptions.data()
            };

            vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_create_info
            {
                {},
                vk::PrimitiveTopology::eTriangleList,
                VK_FALSE
            };

            std::vector<vk::Viewport> viewports
            {
                vk::Viewport
                {
                    0.0f,
                    0.0f,
                    static_cast<float>(swapchain.get_configuration().extent.width),
                    static_cast<float>(swapchain.get_configuration().extent.height),
                    0.0f,
                    1.0f
                }
            };

            std::vector<vk::Rect2D> scissors
            {
                vk::Rect2D
                {
                    { 0, 0 },
                    swapchain.get_configuration().extent
                }
            };

            vk::PipelineViewportStateCreateInfo viewport_state_create_info
            {
                {},
                static_cast<unsigned int>(viewports.size()),
                viewports.data(),
                static_cast<unsigned int>(scissors.size()),
                scissors.data()
            };

            vk::PipelineRasterizationStateCreateInfo rasterization_state_create_info
            {
                {},
                VK_FALSE,
                VK_FALSE,
                vk::PolygonMode::eFill,
                vk::CullModeFlagBits::eBack,
                vk::FrontFace::eClockwise,
                VK_FALSE,
                0.0f,
                0.0f,
                0.0f,
                1.0f
            };

            vk::PipelineMultisampleStateCreateInfo multisample_state_create_info
            {
                {},
                vk::SampleCountFlagBits::e1,
                VK_FALSE,
                1.0f,
                nullptr,
                VK_FALSE,
                VK_FALSE
            };

            std::vector<vk::PipelineColorBlendAttachmentState> color_blend_attachment_states
            {
                vk::PipelineColorBlendAttachmentState
                {
                    VK_FALSE,
                    vk::BlendFactor::eOne,
                    vk::BlendFactor::eZero,
                    vk::BlendOp::eAdd,
                    vk::BlendFactor::eOne,
                    vk::BlendFactor::eZero,
                    vk::BlendOp::eAdd,
                    vk::ColorComponentFlagBits::eR
                    | vk::ColorComponentFlagBits::eG
                    | vk::ColorComponentFlagBits::eB
                    | vk::ColorComponentFlagBits::eA
                }
            };

            vk::PipelineColorBlendStateCreateInfo color_blend_state_create_info
            {
                {},
                VK_FALSE,
                vk::LogicOp::eCopy,
                static_cast<unsigned int>(color_blend_attachment_states.size()),
                color_blend_attachment_states.data(),
                std::array<float, 4>
                {
                    0.0f,
                    0.0f,
                    0.0f,
                    0.0f
                }
            };

            auto result = device->createGraphicsPipelineUnique(nullptr,
                {
                    {},
                    static_cast<unsigned int>(shader_stages_create_info.size()),
                    shader_stages_create_info.data(),
                    & vertex_input_state_create_info,
                    & input_assembly_state_create_info,
                    nullptr,
                    & viewport_state_create_info,
                    & rasterization_state_create_info,
                    & multisample_state_create_info,
                    nullptr,
                    & color_blend_state_create_info,
                    nullptr,
                    *pipeline_layout_,
                    *render_pass_,
                    0,
                    {},
                    {}
                });

#if !defined(NDEBUG)
            std::cout << "Graphics pipeline created" << std::endl;
#endif

            return result;
        }

        [[nodiscard]] static std::vector<vk::UniqueImageView> create_image_views(
            const device& device,
			const swapchain& swapchain)
        {
            std::vector<vk::UniqueImageView> image_views{};

            for (const auto& image : device->getSwapchainImagesKHR(*swapchain))
            {
                image_views.push_back(device->createImageViewUnique(
                    {
                        {},
                        image,
                        vk::ImageViewType::e2D,
                        swapchain.get_configuration().format,
                        {
                            vk::ComponentSwizzle::eIdentity,
                            vk::ComponentSwizzle::eIdentity,
                            vk::ComponentSwizzle::eIdentity,
                            vk::ComponentSwizzle::eIdentity
                        },
                        {
                            vk::ImageAspectFlagBits::eColor,
                            0,
                            1,
                            0,
                            1
                        }
                    }));
            }

#if !defined(NDEBUG)
            std::cout << "Image views created" << std::endl;
#endif

            return image_views;
        }
		
        [[nodiscard]] std::vector<vk::UniqueFramebuffer> create_frame_buffers(
            const device& device,
            const swapchain& swapchain) const
        {
            std::vector<vk::UniqueFramebuffer> framebuffers;

            for (const auto& image_view : image_views_)
            {
                const std::vector<vk::ImageView> attachments{ *image_view };

                framebuffers.push_back(device->createFramebufferUnique(
                    {
                        {},
                        *render_pass_,
                        static_cast<unsigned int>(attachments.size()),
                        attachments.data(),
                        swapchain.get_configuration().extent.width,
                        swapchain.get_configuration().extent.height,
                        1
                    }));
            }

#if !defined(NDEBUG)
            std::cout << "Framebuffers created" << std::endl;
#endif

            return framebuffers;
        }

        [[nodiscard]] static vk::UniqueCommandPool create_command_pool(
			const device& device)
        {
            auto result = device->createCommandPoolUnique(
                {
                    {},
                    device.queue_family_indices.graphics_family.value()
                });

#if !defined(NDEBUG)
            std::cout << "Command pool created" << std::endl;
#endif

            return result;
        }

        [[nodiscard]] std::vector<vk::UniqueCommandBuffer> create_command_buffers(
			const device& device,
            const swapchain& swapchain) const
        {
            auto command_buffers
            {
                device->allocateCommandBuffersUnique(
                    vk::CommandBufferAllocateInfo
                    {
                        *command_pool_,
                        vk::CommandBufferLevel::ePrimary,
                        static_cast<unsigned int>(framebuffers_.size())
                    })
            };

            for (size_t i = 0; i < command_buffers.size(); ++i)
            {
                command_buffers[i]->begin(
                {
                    {},
                    nullptr
                });

                std::vector<vk::ClearValue> clear_values
                {
                    vk::ClearValue
                    {
                        vk::ClearColorValue
                        {
                            std::array<float, 4>
                            {
                                0.0f,
                                0.0f,
                                0.0f,
                                1.0f
                            }
                        }
                    }
                };

                command_buffers[i]->beginRenderPass(
		            {
		                *render_pass_,
		                *framebuffers_[i],
		                vk::Rect2D
		                {
		                    { 0, 0 },
		                    swapchain.get_configuration().extent
		                },
		                static_cast<unsigned int>(clear_values.size()),
		                clear_values.data()
		            }, 
                    vk::SubpassContents::eInline);

            	command_buffers[i]->bindPipeline(vk::PipelineBindPoint::eGraphics, *inner_);

            	command_buffers[i]->draw(3, 1, 0, 0);

            	command_buffers[i]->endRenderPass();

            	command_buffers[i]->end();
            }

#if !defined(NDEBUG)
            std::cout << "Command buffers created" << std::endl;
#endif

            return command_buffers;
        }

	};
}


#endif