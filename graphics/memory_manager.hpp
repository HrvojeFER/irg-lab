#ifndef GRAPHICS_VERTEX_MANAGER_HPP
#define GRAPHICS_VERTEX_MANAGER_HPP


#include "pch.hpp"

#include "device.hpp"
#include "swapchain.hpp"


namespace irglab
{
	struct graphics_vertex {
		using position_vector = glm::vec2;
		using color_vector = glm::vec3;


		position_vector position{ 0.0f, 0.0f };
		color_vector color{ 0.0f, 0.0f, 0.0f };


		[[nodiscard]] static std::vector<vk::VertexInputBindingDescription>
			get_binding_descriptions()
		{
			return
			{
				{
					0,
					sizeof(graphics_vertex),
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
					offsetof(graphics_vertex, position)
				},
				{
					1,
					0,
					vk::Format::eR32G32B32Sfloat,
					offsetof(graphics_vertex, color)
				}
			};
		}
	};

	struct uniform
	{
		float time_in_seconds_since_start;
	};

	struct memory_manager
	{
		static constexpr size_t vertex_count = 50000;
		static constexpr vk::DeviceSize vertex_buffer_offset = 0;

		static constexpr vk::DeviceSize buffer_size = sizeof(graphics_vertex) * vertex_count;

		
		explicit memory_manager(
			const std::shared_ptr<const device>& device, 
			const swapchain& swapchain) :

			device_(device),

			vertex_buffer_
			{
				create_buffer(
					vk::BufferUsageFlagBits::eVertexBuffer
					| vk::BufferUsageFlagBits::eTransferDst, *device)
			},
			vertex_buffer_memory_{ allocate_buffer_memory(*vertex_buffer_, *device) },

			uniform_buffers_{ create_uniform_buffers(swapchain, *device) },
			uniform_buffers_memory_{ allocate_uniform_buffers(swapchain, *device) },

			transfer_command_pool_{ create_transfer_command_pool(*device) }
		{
#if !defined(NDEBUG)
			std::cout << "Vertex vertex_buffer created" << std::endl;
			std::cout << "Memory bound to vertex vertex_buffer" << std::endl;
			std::cout << "Uniform buffers created" << std::endl;
			std::cout << "Memory bound to uniform buffers" << std::endl;
			std::cout << std::endl << "-- Memory manager done --" << std::endl << std::endl;
#endif
		}


		[[nodiscard]] const vk::Buffer& vertex_buffer() const
		{
			return *vertex_buffer_;
		}

		[[nodiscard]] void switch_device(const std::shared_ptr<device>& new_device)
		{
			device_ = new_device;
		}

		void reconstruct(const swapchain& swapchain)
		{
			const auto shared_device = get_shared_device();
			const auto& device = *shared_device;

			uniform_buffers_ = create_uniform_buffers(swapchain, device);
			uniform_buffers_memory_ = allocate_uniform_buffers(swapchain, device);

#if !defined(NDEBUG)
			std::cout << "Uniform buffers created" << std::endl;
			std::cout << "Memory bound to uniform buffers" << std::endl;
			std::cout << std::endl << "-- Memory manager reconstructed --" << std::endl << std::endl;
#endif
		}


		void set_vertex_buffer(std::vector<graphics_vertex> vertices) const
		{
			const auto shared_device = get_shared_device();
			const auto& device = *shared_device;

			auto staging_buffer {
				create_buffer(vk::BufferUsageFlagBits::eTransferSrc, device) };
			auto staging_buffer_memory{
				allocate_buffer_memory(*staging_buffer, device) };
			device->bindBufferMemory(*staging_buffer, *staging_buffer_memory, 0);

			vertices.resize(vertex_count);

			std::memcpy(
				device->mapMemory(*staging_buffer_memory, vertex_buffer_offset, buffer_size, {}),
				vertices.data(),
				buffer_size);
			device->unmapMemory(*staging_buffer_memory);

			copy_buffer(*vertex_buffer_, *staging_buffer);
		}


	private:
		void copy_buffer(const vk::Buffer& destination, const vk::Buffer& source) const
		{
			const auto shared_device = get_shared_device();
			const auto& device = *shared_device;

			auto transfer_command_buffer
			{
				std::move(
					device->allocateCommandBuffersUnique(
						{
							*transfer_command_pool_,
							vk::CommandBufferLevel::ePrimary,
							1
						})[0])
			};

			transfer_command_buffer->begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

			transfer_command_buffer->copyBuffer(source, destination,
				{
					{
						0,
						0,
						buffer_size
					}
				});

			transfer_command_buffer->end();

			const auto transfer_command_fence =
				device->createFenceUnique({});

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
			catch (const vk::SystemError&)
			{
				// Do something meaningful.
				device.transfer_queue.waitIdle();
			}
			if (wait_result == vk::Result::eTimeout)
			{
				device.transfer_queue.waitIdle();
			}
		}


		std::weak_ptr<const device> device_;

		const vk::UniqueBuffer vertex_buffer_;
		const vk::UniqueDeviceMemory vertex_buffer_memory_;

		std::vector<vk::UniqueBuffer> uniform_buffers_;
		std::vector<vk::UniqueDeviceMemory> uniform_buffers_memory_;

		const vk::UniqueCommandPool transfer_command_pool_;


		[[nodiscard]] static vk::UniqueBuffer create_buffer(
			const vk::BufferUsageFlags& usage,
			const device& device)
		{
			vk::BufferCreateInfo create_info =
			{
				{},
				buffer_size,
				usage,
				vk::SharingMode::eExclusive
			};

			if (!device.queue_family_indices.has_same_graphics_and_transfer_family())
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

		[[nodiscard]] static std::vector<vk::UniqueBuffer> create_uniform_buffers(
			const swapchain& swapchain,
			const device& device)
		{
			std::vector<vk::UniqueBuffer> result{ 0 };
			for (unsigned int i = 0; i < swapchain.get_configuration_view().image_count; ++i)
				result.emplace_back(create_buffer(vk::BufferUsageFlagBits::eUniformBuffer, device));

			return result;
		}

		[[nodiscard]] static vk::UniqueDeviceMemory allocate_buffer_memory(
			const vk::Buffer& buffer,
			const device& device)
		{
			const auto& memory_requirements = device->getBufferMemoryRequirements(buffer);

			auto result = device->allocateMemoryUnique(
				{
					memory_requirements.size,
					select_memory_type_index(
						memory_requirements,
						vk::MemoryPropertyFlagBits::eHostVisible |
							vk::MemoryPropertyFlagBits::eHostCoherent,
						device)
				});

			device->bindBufferMemory(buffer, *result, 0);

			return result;
		}

		[[nodiscard]] std::vector<vk::UniqueDeviceMemory> allocate_uniform_buffers(
			const swapchain& swapchain,
			const device& device) const
		{
			std::vector<vk::UniqueDeviceMemory> result{ 0 };
			for (unsigned int i = 0; i < swapchain.get_configuration_view().image_count; ++i)
				result.emplace_back(allocate_buffer_memory(*uniform_buffers_[i], device));

			return result;
		}

		[[nodiscard]] static unsigned int select_memory_type_index(
			const vk::MemoryRequirements& memory_requirements,
			const vk::MemoryPropertyFlags& properties,
			const device& device)
		{
			const auto physical_memory_properties = device.physical().getMemoryProperties();

			for (unsigned int i = 0; i < physical_memory_properties.memoryTypeCount; ++i)
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

		[[nodiscard]] static vk::UniqueCommandPool create_transfer_command_pool(
			const device& device)
		{
			return device->createCommandPoolUnique(
				{
					vk::CommandPoolCreateFlagBits::eTransient,
					device.queue_family_indices.transfer_family.value()
				});
		}


		[[nodiscard]] std::shared_ptr<const device> get_shared_device() const
		{
			if (device_.expired())
			{
				throw std::runtime_error("Device expired.");
			}

			return device_.lock();
		}
	};
}

#endif
