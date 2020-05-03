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
	[[nodiscard]] static std::vector<std::reference_wrapper<const InnerType>> dereference_handles(
		const std::vector<vk::UniqueHandle<InnerType, vk::DispatchLoaderDynamic>>& handles)
	{
		std::vector<std::reference_wrapper<const InnerType>> result{};

		for (const auto& handle : handles)
		{
			result.push_back(std::cref(*handle));
		}

		return result;
	}

	inline void left_trim(std::string& s) {
		s.erase(
			s.begin(), 
			std::find_if(s.begin(), s.end(), 
				[](const int character) {
					return !std::isspace(character);
				}));
	}

	inline void right_trim(std::string& string) {
		string.erase(
			std::find_if(string.rbegin(), string.rend(), 
				[](const int character) {
					return !std::isspace(character);
				}).base(), 
			string.end());
	}

	static void trim(std::string& s) {
		left_trim(s);
		right_trim(s);
	}
}


#endif
