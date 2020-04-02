#ifndef GRAPHICS_VERTEX_MANAGER_HPP
#define GRAPHICS_VERTEX_MANAGER_HPP


#include "pch.hpp"

#include "device.hpp"


namespace irglab
{
	struct vertex {
		glm::vec2 pos;
		glm::vec3 color;

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
					offsetof(vertex, pos)
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
		static inline const unsigned int vertex_count = 10;
		static inline const vk::DeviceSize buffer_size = sizeof(vertex) * vertex_count;
		const vk::DeviceSize buffer_offset = 0;
		
		explicit vertex_manager(const device& device) :
			device_(device),
			buffer_{ create_buffer() },
			buffer_memory_{ allocate_buffer_memory() }
		{
			device->bindBufferMemory(*buffer_, *buffer_memory_, 0);

#if !defined(NDEBUG)
			std::cout << "Memory bound to vertex buffer" << std::endl;
#endif


#if !defined(NDEBUG)
			std::cout << "-Vertex buffer done-" << std::endl;
#endif

			// TODO: add some real vertices
			fill_buffer(
				{
					{
						{
							{0.0f, -0.5f},
							{0.3f, 0.0f, 1.0f}
						},
						{
							{0.5f, 0.5f},
							{1.0f, 0.6f, 0.0f}
						},
						{
							{-0.5f, 0.5f},
							{0.7f, 0.0f, 1.0f}
						}
					}
				});
		}

		[[nodiscard]] const vk::Buffer& buffer() const
		{
			return *buffer_;
		}

	private:
		const device& device_;

		const vk::UniqueBuffer buffer_;
		const vk::UniqueDeviceMemory buffer_memory_;

		[[nodiscard]] vk::UniqueBuffer create_buffer() const
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
						vk::BufferUsageFlagBits::eVertexBuffer,
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
						vk::BufferUsageFlagBits::eVertexBuffer,
						vk::SharingMode::eExclusive
					});
			}
#if !defined(NDEBUG)
			std::cout  << "Vertex buffer created"  << std::endl;
#endif

			return result;
		}

		[[nodiscard]] vk::UniqueDeviceMemory allocate_buffer_memory() const
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

		void fill_buffer(std::array<vertex, buffer_size> vertices) const
		{
			memcpy(
				device_->mapMemory(*buffer_memory_, buffer_offset, buffer_size, {}),
				vertices.data(),
				buffer_size);
			device_->unmapMemory(*buffer_memory_);
		}
	};
}

#endif
