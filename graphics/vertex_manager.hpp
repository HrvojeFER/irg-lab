#ifndef GRAPHICS_VERTEX_MANAGER_HPP
#define GRAPHICS_VERTEX_MANAGER_HPP


#include "pch.hpp"

#include "device.hpp"


namespace irglab
{
	struct vertex {
		glm::vec2 position{ 0.0f, 0.0f };
		glm::vec3 color{ 0.0f, 0.0f, 0.0f };

		[[nodiscard]] static std::vector<vk::VertexInputBindingDescription>
			get_binding_descriptions()
		{
			return
			{
				{
					0,
					sizeof(vertex),
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
					offsetof(vertex, position)
				},
				{
					1,
					0,
					vk::Format::eR32G32B32Sfloat,
					offsetof(vertex, color)
				}
			};
		}
	};
	
	struct vertex_manager
	{
		// sizeof(float) = 4
		// sizeof(vertex) = 2 * 4 + 3 * 4 = 20 - position + color
		// buffer size = 20 * 3276 = 65520 < 65536 = 2^16
		static inline const unsigned int vertex_count = 3276;
		static inline const vk::DeviceSize buffer_size = sizeof(vertex) * vertex_count;
		const vk::DeviceSize buffer_offset = 0;
		
		explicit vertex_manager(const device& device) :
			device_(device),
			vertex_buffer_
			{
				create_buffer(
					vk::BufferUsageFlagBits::eVertexBuffer
					| vk::BufferUsageFlagBits::eTransferDst)
			},
			vertex_buffer_memory_{ allocate_buffer_memory(*vertex_buffer_) },
			transfer_command_pool_{ create_transfer_command_pool() }
		{
			device->bindBufferMemory(*vertex_buffer_, *vertex_buffer_memory_, 0);

#if !defined(NDEBUG)
			std::cout << "Memory bound to vertex buffer" << std::endl;
#endif


#if !defined(NDEBUG)
			std::cout << std::endl << "-- Vertex buffer done --" << std::endl << std::endl;
#endif
		}

		[[nodiscard]] const vk::Buffer& buffer() const
		{
			return *vertex_buffer_;
		}

		void set_buffer(const std::vector<vertex>& vertices) const
		{
			auto staging_buffer{ create_buffer(vk::BufferUsageFlagBits::eTransferSrc) };
			auto staging_buffer_memory{ allocate_buffer_memory(*staging_buffer) };
			device_->bindBufferMemory(*staging_buffer, *staging_buffer_memory, 0);

#if !defined(NDEBUG)
			std::cout << "Memory bound to staging buffer" << std::endl;
#endif

			const auto to_write = new vertex[vertex_count] { {} };

			std::memcpy(to_write,
				vertices.data(),
				min(sizeof(vertex) * vertices.size(), buffer_size));

			std::memcpy(
				device_->mapMemory(*staging_buffer_memory, buffer_offset, buffer_size, {}),
				to_write,
				buffer_size);
			device_->unmapMemory(*staging_buffer_memory);

			delete[] to_write;
			
#if !defined(NDEBUG)
			std::cout << "Vertices copied to staging buffer" << std::endl;
#endif
			
			copy_buffer(*vertex_buffer_, *staging_buffer);

#if !defined(NDEBUG)
			std::cout << "Staging buffer copied to vertex buffer" << std::endl;
			std::cout << std::endl << "-- Vertex copy operation complete --" << std::endl << std::endl;
#endif

		}
		
	private:
		const device& device_;

		const vk::UniqueBuffer vertex_buffer_;
		const vk::UniqueDeviceMemory vertex_buffer_memory_;

		const vk::UniqueCommandPool transfer_command_pool_;

		[[nodiscard]] vk::UniqueBuffer create_buffer(const vk::BufferUsageFlags& usage) const
		{
			const std::unordered_set<unsigned int> sharing_queue_family_indices_set
			{
				device_.queue_family_indices.graphics_family.value(),
				device_.queue_family_indices.transfer_family.value()
			};

			vk::UniqueBuffer result;
			if (sharing_queue_family_indices_set.size() > 1)
			{
				const std::vector<unsigned int> sharing_queue_family_indices_array
				{
					sharing_queue_family_indices_set.begin(),
					sharing_queue_family_indices_set.end()
				};
				
				result = device_->createBufferUnique(
					{
						{},
						buffer_size,
						usage,
						vk::SharingMode::eConcurrent,
						static_cast<unsigned int>(sharing_queue_family_indices_array.size()),
						sharing_queue_family_indices_array.data()
					});
			}
			else
			{
				result = device_->createBufferUnique(
					{
						{},
						buffer_size,
						usage,
						vk::SharingMode::eExclusive
					});
			}
#if !defined(NDEBUG)
			std::cout  << "Vertex buffer created"  << std::endl;
#endif

			return result;
		}

		[[nodiscard]] vk::UniqueDeviceMemory allocate_buffer_memory(const vk::Buffer& buffer) const
		{
			const auto& memory_requirements = device_->getBufferMemoryRequirements(*vertex_buffer_);
			
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

			if (device_->waitForFences(
					1,
					&*transfer_command_fence, 
					VK_TRUE,
					UINT64_MAX) != vk::Result::eSuccess)
			{
				device_.transfer_queue.waitIdle();
			}
		}
	};
}

#endif
