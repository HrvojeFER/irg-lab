#ifndef GRAPHICS_SYNCHRONIZER_HPP
#define GRAPHICS_SYNCHRONIZER_HPP


#include "pch.hpp"
#include "extensions.hpp"

#include "device.hpp"


namespace irglab
{
	template<bool UseMap = true>
	struct synchronizer
	{
		using key = semantic_key;
		
		struct sync_vector_description
		{
			const key key;
			const size_t& count = 1;
		};

		using sync_container_description = std::initializer_list<sync_vector_description>;

		synchronizer(
			const device& device,
			const sync_container_description& fence_container_description,
			const sync_container_description& semaphore_container_description) :

			fences_{ create_sync_container<vk::UniqueFence>(
				fence_container_description, create_fence, device) },
			semaphores_{ create_sync_container<vk::UniqueSemaphore>(
				semaphore_container_description, create_semaphore, device) }
		{
#if !defined(NDEBUG)
			std::cout << "Synchronizer created" << std::endl;
#endif
		}

		[[nodiscard, maybe_unused]] std::vector<std::reference_wrapper<const vk::Fence>>
			fences(const key& key) const
		{
			return dereference_vulkan_handles<vk::Fence>(
				get_vector<vk::UniqueFence>(fences_, key));
		}

		[[nodiscard, maybe_unused]] std::vector<std::reference_wrapper<const vk::Semaphore>>
			semaphores(const key& key) const
		{
			return dereference_vulkan_handles<vk::Semaphore>(
				get_vector<vk::UniqueSemaphore>(semaphores_, key));
		}
		
		[[nodiscard, maybe_unused]] const vk::Fence& fence(const key& key, const size_t& index) const
		{
			return *get<vk::UniqueFence>(fences_, key, index);
		}

		[[nodiscard, maybe_unused]] const vk::Semaphore& semaphore(
			const key& key, const size_t& index) const
		{
			return *get<vk::UniqueSemaphore>(semaphores_, key, index);
		}
		
	private:
		template<typename SyncType>
		using sync_vector = std::vector<SyncType>;
		
		template<typename SyncType>
		using sync_container = typename std::conditional<UseMap,
		                                                 semantic_map<sync_vector<SyncType>>,
		                                                 semantic_vector<sync_vector<SyncType>>>::type;
		
		[[maybe_unused]] const sync_container<vk::UniqueFence> fences_;
		[[maybe_unused]] const sync_container<vk::UniqueSemaphore> semaphores_;

		template<typename SyncType>
		[[nodiscard]] sync_container<SyncType> create_sync_container(
			const sync_container_description& description,
			std::function<SyncType(const device&)> create_sync_type,
			const device& device) const
		{
			sync_container<SyncType> result;

			if constexpr(!UseMap)
			{
				result.resize(get_highest_key_index(description));
			}
			
			for (const auto& sync_vector_description : description)
			{
				if (sync_vector_description.count < 1)
				{
					throw std::runtime_error("Sync vector count should be higher than zero.");
				}

				result[sync_vector_description.key] = sync_vector<SyncType>
				{
					sync_vector_description.count
				};
				
				for (size_t i = 0; i < sync_vector_description.count; ++i)
				{
					result[sync_vector_description.key][i] = create_sync_type(device);
				}
			}

			return result;
		}

		[[nodiscard, maybe_unused]] static size_t get_highest_key_index(
			const sync_container_description& container_description)
		{
			size_t result{ 0 };

			for (const auto& vector_description : container_description)
				if (result < vector_description.key)
					result = vector_description.key;

			return result;
		}

		[[nodiscard, maybe_unused]] static vk::UniqueFence create_fence(const device& device)
		{
			return device->createFenceUnique({ vk::FenceCreateFlagBits::eSignaled });
		}

		[[nodiscard, maybe_unused]] static vk::UniqueSemaphore create_semaphore(const device& device)
		{
			return device->createSemaphoreUnique({});
		}

		
		template<typename SyncType>
		[[nodiscard]] static const SyncType& get(
			const sync_container<SyncType>& sync_container, 
			const key& key,
			const size_t& index)
		{
			const auto& result_vector = get_vector<SyncType>(sync_container, key);
			if (index >= result_vector.size())
			{
				throw std::out_of_range{ "Index out of range." };
			}
			
			return result_vector[index];
		}

		template<typename SyncType>
		[[nodiscard]] static const std::vector<SyncType>& get_vector(
			const sync_container<SyncType>& sync_container,
			const key& key)
		{
			if constexpr (UseMap)
			{
				const auto& find_pair = sync_container.find(key);
				if (find_pair == sync_container.end())
				{
					throw std::range_error{ "Key not found." };
				}

				return find_pair->second;
			}
			else 
			{
				if (key >= sync_container.size())
				{
					throw std::range_error{ "Key out of range." };
				}

				return sync_container[size_t(key)];
			}
		}
	};
}


#endif
