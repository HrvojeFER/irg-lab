#ifndef GRAPHICS_VERTEX_MANAGER_HPP
#define GRAPHICS_VERTEX_MANAGER_HPP


#include "pch.hpp"

#include "device.hpp"


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

	struct memory_manager
	{
		// sizeof(float) = 4
		// sizeof(vertex) = 2 * 4 + 3 * 4 = 20 - position_vector + color
		// buffer size = 20 * 3276 = 65520 < 65536 = 2^16
		static constexpr size_t vertex_count = 50000;
		static constexpr vk::DeviceSize vertex_buffer_offset = 0;

		static constexpr vk::DeviceSize buffer_size = sizeof(graphics_vertex) * vertex_count;

		explicit memory_manager(const device& device) :
			device_(device),
			buffer_
		{
			create_buffer(
				vk::BufferUsageFlagBits::eVertexBuffer
				| vk::BufferUsageFlagBits::eTransferDst)
		},
			vertex_buffer_memory_{ allocate_buffer_memory(*buffer_) },
			transfer_command_pool_{ create_transfer_command_pool() }
		{
			device->bindBufferMemory(*buffer_, *vertex_buffer_memory_, 0);

#if !defined(NDEBUG)
			std::cout << "Memory bound to vertex buffer" << std::endl;
#endif


#if !defined(NDEBUG)
			std::cout << std::endl << "-- Vertex buffer done --" << std::endl << std::endl;
#endif
		}


		[[nodiscard]] const vk::Buffer& buffer() const
		{
			return *buffer_;
		}


		void set_vertex_buffer(std::vector<graphics_vertex> vertices) const
		{
			auto staging_buffer{ create_buffer(vk::BufferUsageFlagBits::eTransferSrc) };
			auto staging_buffer_memory{ allocate_buffer_memory(*staging_buffer) };
			device_->bindBufferMemory(*staging_buffer, *staging_buffer_memory, 0);

#if !defined(NDEBUG)
			std::cout << "Memory bound to staging buffer" << std::endl;
#endif

			vertices.resize(vertex_count);

			std::memcpy(
				device_->mapMemory(*staging_buffer_memory, vertex_buffer_offset, buffer_size, {}),
				vertices.data(),
				buffer_size);
			device_->unmapMemory(*staging_buffer_memory);

#if !defined(NDEBUG)
			std::cout << "Vertices copied to staging buffer" << std::endl;
#endif

			copy_buffer(*buffer_, *staging_buffer);

#if !defined(NDEBUG)
			std::cout << "Staging buffer copied to vertex buffer" << std::endl;
			std::cout << std::endl << "-- Vertex copy operation complete --" << std::endl << std::endl;
#endif
		}


	private:
		void copy_buffer(const vk::Buffer& destination, const vk::Buffer& source) const
		{
			auto transfer_command_buffer
			{
				std::move(
					device_->allocateCommandBuffersUnique(
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
				device_->createFenceUnique({});

			device_.transfer_queue.submit(
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
				wait_result = device_->waitForFences(
					1,
					&*transfer_command_fence,
					VK_TRUE,
					UINT64_MAX);
			}
			catch (const vk::SystemError&)
			{
				// Do something meaningful.
				device_.transfer_queue.waitIdle();
			}
			if (wait_result == vk::Result::eTimeout)
			{
				device_.transfer_queue.waitIdle();
			}
		}


		const device& device_;

		const vk::UniqueBuffer buffer_;
		const vk::UniqueDeviceMemory vertex_buffer_memory_;

		const vk::UniqueCommandPool transfer_command_pool_;


		[[nodiscard]] vk::UniqueBuffer create_buffer(const vk::BufferUsageFlags& usage) const
		{
			vk::BufferCreateInfo create_info =
			{
				{},
				buffer_size,
				usage,
				vk::SharingMode::eExclusive
			};

			const std::unordered_set<unsigned int> sharing_queue_family_indices_set
			{
				device_.queue_family_indices.graphics_family.value(),
				device_.queue_family_indices.transfer_family.value()
			};

			if (sharing_queue_family_indices_set.size() > 1)
			{
				const std::vector<unsigned int> sharing_queue_family_indices_array
				{
					sharing_queue_family_indices_set.begin(),
					sharing_queue_family_indices_set.end()
				};

				create_info.setSharingMode(vk::SharingMode::eConcurrent)
					.setPQueueFamilyIndices(sharing_queue_family_indices_array.data())
					.setQueueFamilyIndexCount(
						static_cast<unsigned int>(sharing_queue_family_indices_array.size()));
			}

			auto result = device_->createBufferUnique(create_info);
#if !defined(NDEBUG)
			std::cout << "Vertex buffer created" << std::endl;
#endif

			return result;
		}

		[[nodiscard]] vk::UniqueDeviceMemory allocate_buffer_memory(const vk::Buffer& buffer) const
		{
			const auto& memory_requirements = device_->getBufferMemoryRequirements(*buffer_);

			auto result = device_->allocateMemoryUnique(
				{
					memory_requirements.size,
					select_memory_type_index(
						memory_requirements,
						vk::MemoryPropertyFlagBits::eHostVisible |
							vk::MemoryPropertyFlagBits::eHostCoherent)
				});

#if !defined(NDEBUG)
			std::cout << "Device memory allocated" << std::endl;
#endif

			return result;
		}

		[[nodiscard]] unsigned int select_memory_type_index(
			const vk::MemoryRequirements& memory_requirements,
			const vk::MemoryPropertyFlags& properties) const
		{
			const auto physical_memory_properties = device_.physical().getMemoryProperties();

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

		[[nodiscard]] vk::UniqueCommandPool create_transfer_command_pool() const
		{
			return device_->createCommandPoolUnique(
				{
					vk::CommandPoolCreateFlagBits::eTransient,
					device_.queue_family_indices.transfer_family.value()
				});
		}
	};
}

#endif
