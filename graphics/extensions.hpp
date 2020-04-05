#ifndef GRAPHICS_EXTENSIONS_HPP
#define GRAPHICS_EXTENSIONS_HPP


#include "pch.hpp"


namespace irglab
{
	struct semantic_key_hasher;

	struct semantic_key {
		semantic_key() : inner_(inner_counter_++) {}

		bool operator ==(const semantic_key& other) const
		{
			return inner_ == other.inner_;
		}

		// ReSharper disable once CppNonExplicitConversionOperator
		operator unsigned int() const
		{
			return inner_;
		}

	private:
		friend struct semantic_key_hasher;

		inline static unsigned int inner_counter_ = 0;
		const unsigned int inner_;
	};

	struct semantic_key_hasher
	{
		std::size_t operator()(semantic_key const& key) const noexcept
		{
			return key.inner_;
		}
	};

	template<typename ValueType>
	using semantic_map = std::unordered_map<semantic_key, ValueType, semantic_key_hasher>;

	template<typename InnerType>
	[[nodiscard]] std::vector<std::reference_wrapper<const InnerType>> dereference_handles(
		const std::vector<vk::UniqueHandle<InnerType, vk::DispatchLoaderDynamic>>& handles)
	{
		std::vector<std::reference_wrapper<const InnerType>> result{};

		for (const auto& handle : handles)
		{
			result.push_back(std::cref(*handle));
		}

		return result;
	}
}


#endif
