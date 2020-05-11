#ifndef IRGLAB_RENDER_PASS_HPP
#define IRGLAB_RENDER_PASS_HPP


#include "pch.hpp"

#include "device.hpp"
#include "swapchain.hpp"


namespace irglab
{
	struct render_pass
	{
        explicit render_pass(const device& device, const swapchain& swapchain) :
            inner_{ create_inner(device, swapchain) } { }

        [[nodiscard]] const vk::RenderPass& operator*() const
        {
            return *inner_;
        }

        void reconstruct(const device& device, const swapchain& swapchain)
        {
            inner_ = create_inner(device, swapchain);
        }
		
	private:
        vk::UniqueRenderPass inner_;
		
        [[nodiscard]] static vk::UniqueRenderPass create_inner(
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
	};
}

#endif
