#ifndef GRAPHICS_SYNCHRONIZER_HPP
#define GRAPHICS_SYNCHRONIZER_HPP


#include "pch.hpp"
#include "extensions.hpp"

#include "device.hpp"


namespace irglab
{
	struct synchronizer
	{
		struct key_hasher;

		struct key {
			key() : inner_(inner_counter_++) {}

			bool operator ==(const key& other) const
			{
				return inner_ == other.inner_;
			}

		private:
			friend struct key_hasher;

			inline static unsigned int inner_counter_ = 0;
			const unsigned int inner_;
		};

		struct key_hasher
		{
			std::size_t operator()(key const& key) const noexcept
			{
				return key.inner_;
			}
		};

		struct sync_vector_description
		{
			const key key;
			const size_t& count = 1;
		};

		using sync_container_description = std::vector<sync_vector_description>;

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

		[[nodiscard]] std::vector<std::reference_wrapper<const vk::Fence>>
			fences(const key& key) const
		{
			return dereference_handles(get_vector(fences_, key));
		}

		[[nodiscard]] std::vector<std::reference_wrapper<const vk::Semaphore>>
			semaphores(const key& key) const
		{
			return dereference_handles(get_vector(semaphores_, key));
		}
		
		[[nodiscard]] const vk::Fence& fence(const key& key, const size_t& index) const
		{
			return *get(fences_, key, index);
		}

		[[nodiscard]] const vk::Semaphore& semaphore(const key& key, const size_t& index) const
		{
			return *get(semaphores_, key, index);
		}
		
	private:
		template<typename SyncType>
		using sync_map = std::unordered_map<key, std::vector<SyncType>, key_hasher>;
		
		const sync_map<vk::UniqueFence> fences_;
		const sync_map<vk::UniqueSemaphore> semaphores_;

		template<typename SyncType>
		[[nodiscard]] sync_map<SyncType> create_sync_container(
			const sync_container_description& description,
			std::function<SyncType(const device&)> create_sync_type,
			const device& device) const
		{
			sync_map<SyncType> result;
			
			for (const auto& sync_vector_description : description)
			{
				if (sync_vector_description.count < 1)
				{
					throw std::runtime_error("Sync vector count should be higher than zero.");
				}
				
				result[sync_vector_description.key] = std::vector<SyncType>
				{
					sync_vector_description.count
				};

				for (size_t i = 0 ; i < sync_vector_description.count ; ++i)
				{
					result[sync_vector_description.key][i] = create_sync_type(device);
				}
			}

			return result;
		}

		[[nodiscard]] static vk::UniqueFence create_fence(const device& device)
		{
			return device->createFenceUnique({ vk::FenceCreateFlagBits::eSignaled });
		}

		[[nodiscard]] static vk::UniqueSemaphore create_semaphore(const device& device)
		{
			return device->createSemaphoreUnique({});
		}

		
		template<typename SyncType>
		[[nodiscard]] static const SyncType& get(
			const sync_map<SyncType>& sync_map, 
			const key& key,
			const size_t& index)
		{
			const auto& result_vector = get_vector(sync_map, key);
			if (index >= result_vector.size())
			{
				throw std::out_of_range("Index out of range.");
			}
			
			return result_vector[index];
		}

		template<typename SyncType>
		[[nodiscard]] static const std::vector<SyncType>& get_vector(
			const sync_map<SyncType>& sync_map,
			const key& key)
		{
			const auto& result_iterator = sync_map.find(key);
			if (result_iterator == sync_map.end())
			{
				throw std::range_error("Key not found.");
			}
			
			return result_iterator->second;
		}
	};
}


#endif
