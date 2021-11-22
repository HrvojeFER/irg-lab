#ifndef IRGLAB_MEMORYMANAGER_HPP
#define IRGLAB_MEMORYMANAGER_HPP


#include "../external/pch.hpp"

#include "../environment/device.hpp"
#include "swapchain.hpp"


namespace il
{
    struct GraphicsVertex
    {
        using PositionVector = glm::vec2;
        using ColorVector = glm::vec3;


        PositionVector position{0.0f, 0.0f};
        ColorVector color{0.0f, 0.0f, 0.0f};


        [[nodiscard]] static std::vector<vk::VertexInputBindingDescription>
        get_binding_descriptions()
        {
            return
                    {
                            {
                                    0,
                                    sizeof(GraphicsVertex),
                                    vk::VertexInputRate::eVertex
                            }
                    };
        }

        [[nodiscard]] static std::vector<vk::VertexInputAttributeDescription>
        get_attribute_descriptions()
        {
            return
                    {
                            {
                                    0,
                                    0,
                                    vk::Format::eR32G32Sfloat,
                                    offsetof(GraphicsVertex, position)
                            },
                            {
                                    1,
                                    0,
                                    vk::Format::eR32G32B32Sfloat,
                                    offsetof(GraphicsVertex, color)
                            }
                    };
        }
    };


    struct MemoryManager
    {
        static constexpr size_t vertex_count = 50000;
        static constexpr vk::DeviceSize vertex_buffer_offset = 0;

        static constexpr vk::DeviceSize buffer_size = sizeof(GraphicsVertex) * vertex_count;


        [[maybe_unused]] explicit MemoryManager(
                const std::shared_ptr<const device> &device,
                const swapchain &swapchain) :

                _device(device),

                _vertex_buffer
                        {
                                _create_buffer(
                                        vk::BufferUsageFlagBits::eVertexBuffer
                                        | vk::BufferUsageFlagBits::eTransferDst, *device)
                        },
                _vertex_buffer_memory{_allocate_buffer_memory(*_vertex_buffer, *device)},

                _uniform_buffers{_create_uniform_buffers(swapchain, *device)},
                _uniform_buffers_memory{_allocate_uniform_buffers(swapchain, *device)},

                _transfer_command_pool{_create_transfer_command_pool(*device)}
        {
#if !defined(NDEBUG)
            std::cout << "Vertex vertex_buffer created" << std::endl;
            std::cout << "Memory bound to owned_vertex vertex_buffer" << std::endl;
            std::cout << "Uniform buffers created" << std::endl;
            std::cout << "Memory bound to uniform buffers" << std::endl;
            std::cout << std::endl << "-- Memory manager done --" << std::endl << std::endl;
#endif
        }


        [[nodiscard]] const vk::Buffer &vertex_buffer() const
        {
            return *_vertex_buffer;
        }

        [[maybe_unused]] void switch_device(const std::shared_ptr<device> &new_device)
        {
            _device = new_device;
        }

        void reconstruct(const swapchain &swapchain)
        {
            const auto shared_device = _get_shared_device();
            const auto &device = *shared_device;

            _uniform_buffers = _create_uniform_buffers(swapchain, device);
            _uniform_buffers_memory = _allocate_uniform_buffers(swapchain, device);

#if !defined(NDEBUG)
            std::cout << "Uniform buffers created" << std::endl;
            std::cout << "Memory bound to uniform buffers" << std::endl;
            std::cout << std::endl << "-- Memory manager reconstructed --" << std::endl << std::endl;
#endif
        }


        void set_vertex_buffer(std::vector<GraphicsVertex> vertices) const
        {
            const auto shared_device = _get_shared_device();
            const auto &device = *shared_device;

            auto staging_buffer{
                    _create_buffer(vk::BufferUsageFlagBits::eTransferSrc, device)};
            auto staging_buffer_memory{
                    _allocate_buffer_memory(*staging_buffer, device)};
            device->bindBufferMemory(*staging_buffer, *staging_buffer_memory, 0);

            vertices.resize(vertex_count);

            std::memcpy(
                    device->mapMemory(*staging_buffer_memory, vertex_buffer_offset, buffer_size, { }),
                    vertices.data(),
                    buffer_size);
            device->unmapMemory(*staging_buffer_memory);

            _copy_buffer(*_vertex_buffer, *staging_buffer);
        }


    private:
        void _copy_buffer(const vk::Buffer &destination, const vk::Buffer &source) const
        {
            const auto shared_device = _get_shared_device();
            const auto &device = *shared_device;

            auto transfer_command_buffer
                    {
                            std::move(
                                    device->allocateCommandBuffersUnique(
                                            {
                                                    *_transfer_command_pool,
                                                    vk::CommandBufferLevel::ePrimary,
                                                    1
                                            })[0])
                    };

            transfer_command_buffer->begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

            transfer_command_buffer->copyBuffer(
                    source, destination,
                    {
                            {
                                    0,
                                    0,
                                    buffer_size
                            }
                    });

            transfer_command_buffer->end();

            const auto transfer_command_fence =
                    device->createFenceUnique({ });

            device.transfer_queue.submit(
                    {
                            {
                                    0,
                                    nullptr,
                                    nullptr,
                                    1,
                                    &*transfer_command_buffer,
                                    0,
                                    nullptr
                            }
                    },
                    *transfer_command_fence);

            auto wait_result = vk::Result::eSuccess;
            try
            {
                wait_result = device->waitForFences(
                        1,
                        &*transfer_command_fence,
                        VK_TRUE,
                        UINT64_MAX);
            }
            catch (const vk::SystemError &)
            {
                // Do something meaningful.
                device.transfer_queue.waitIdle();
            }
            if (wait_result == vk::Result::eTimeout)
            {
                device.transfer_queue.waitIdle();
            }
        }


        std::weak_ptr<const device> _device;

        const vk::UniqueBuffer _vertex_buffer;
        const vk::UniqueDeviceMemory _vertex_buffer_memory;

        std::vector<vk::UniqueBuffer> _uniform_buffers;
        std::vector<vk::UniqueDeviceMemory> _uniform_buffers_memory;

        const vk::UniqueCommandPool _transfer_command_pool;


        [[nodiscard]] static vk::UniqueBuffer _create_buffer(
                const vk::BufferUsageFlags &usage,
                const device &device)
        {
            vk::BufferCreateInfo create_info =
                    {
                            { },
                            buffer_size,
                            usage,
                            vk::SharingMode::eExclusive
                    };

            if (!(device.graphics_queue == device.transfer_queue))
            {
                const std::vector<unsigned int> sharing_queue_family_indices_array
                        {
                                device.queue_family_indices.transfer_family.value(),
                                device.queue_family_indices.graphics_family.value()
                        };

                create_info.setSharingMode(vk::SharingMode::eConcurrent)
                        .setPQueueFamilyIndices(sharing_queue_family_indices_array.data())
                        .setQueueFamilyIndexCount(
                                static_cast<unsigned int>(sharing_queue_family_indices_array.size()));
            }

            auto result = device->createBufferUnique(create_info);

            return result;
        }

        [[nodiscard]] static std::vector<vk::UniqueBuffer> _create_uniform_buffers(
                const swapchain &swapchain,
                const device &device)
        {
            std::vector<vk::UniqueBuffer> result{0};
            for (unsigned int i = 0 ; i < swapchain.get_configuration_view().image_count ; ++i)
                result.emplace_back(_create_buffer(vk::BufferUsageFlagBits::eUniformBuffer, device));

            return result;
        }

        [[nodiscard]] static vk::UniqueDeviceMemory _allocate_buffer_memory(
                const vk::Buffer &buffer,
                const device &device)
        {
            const auto &memory_requirements = device->getBufferMemoryRequirements(buffer);

            auto result = device->allocateMemoryUnique(
                    {
                            memory_requirements.size,
                            _select_memory_type_index(
                                    memory_requirements,
                                    vk::MemoryPropertyFlagBits::eHostVisible |
                                    vk::MemoryPropertyFlagBits::eHostCoherent,
                                    device)
                    });

            device->bindBufferMemory(buffer, *result, 0);

            return result;
        }

        [[nodiscard]] std::vector<vk::UniqueDeviceMemory> _allocate_uniform_buffers(
                const swapchain &swapchain,
                const device &device) const
        {
            std::vector<vk::UniqueDeviceMemory> result{0};
            for (unsigned int i = 0 ; i < swapchain.get_configuration_view().image_count ; ++i)
                result.emplace_back(_allocate_buffer_memory(*_uniform_buffers[i], device));

            return result;
        }

        [[nodiscard]] static unsigned int _select_memory_type_index(
                const vk::MemoryRequirements &memory_requirements,
                const vk::MemoryPropertyFlags &properties,
                const device &device)
        {
            const auto physical_memory_properties = device.physical().getMemoryProperties();

            for (unsigned int i = 0 ; i < physical_memory_properties.memoryTypeCount ; ++i)
            {
                if (memory_requirements.memoryTypeBits & 1 << i &&
                    (physical_memory_properties.memoryTypes[i].propertyFlags & properties) ==
                    properties)
                {
                    return i;
                }
            }

            throw std::runtime_error("Failed to find suitable memory type");
        }

        [[nodiscard]] static vk::UniqueCommandPool _create_transfer_command_pool(
                const device &device)
        {
            return device->createCommandPoolUnique(
                    {
                            vk::CommandPoolCreateFlagBits::eTransient,
                            device.queue_family_indices.transfer_family.value()
                    });
        }


        [[nodiscard]] std::shared_ptr<const device> _get_shared_device() const
        {
            if (_device.expired())
            {
                throw std::runtime_error("device expired.");
            }

            return _device.lock();
        }
    };
}

#endif
