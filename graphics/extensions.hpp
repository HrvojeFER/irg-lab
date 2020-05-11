#ifndef GRAPHICS_EXTENSIONS_HPP
#define GRAPHICS_EXTENSIONS_HPP


#include "pch.hpp"


namespace irglab
{
	// Semantic key that works with any collection that supports the [] operator.
	
	struct [[maybe_unused]] semantic_key
	{
		semantic_key() noexcept : inner_(inner_counter_++) {}

		bool operator ==(const semantic_key& other) const noexcept
		{
			return inner_ == other.inner_;
		}

		// ReSharper disable once CppNonExplicitConversionOperator
		operator size_t() const noexcept
		{
			return inner_;
		}
		
		struct [[maybe_unused]] hasher
		{
			size_t operator()(semantic_key const& key) const noexcept
			{
				return key.inner_;
			}
		};

	private:
		inline static unsigned int inner_counter_ = 0;
		const size_t inner_;
	};

	template<typename ValueType>
	using semantic_map [[maybe_unused]] = 
		std::unordered_map<semantic_key, ValueType, semantic_key::hasher>;

	template<typename ValueType>
	using semantic_vector [[maybe_unused]] = std::vector<ValueType>;

	template<typename ValueType, size_t Size>
	using semantic_array [[maybe_unused]] = std::array<ValueType, Size>;


	// Not implemented error for not implemented functions.
	
	class [[maybe_unused]] not_implemented_error final : public std::logic_error
	{
	public:
		explicit not_implemented_error(
			const std::optional<std::string_view> & function_name = std::nullopt,
			const std::optional<std::string_view> & reason = std::nullopt) :

			std::logic_error{create_message(function_name, reason)} {}

	private:
		[[nodiscard]] static std::string create_message(
			std::optional<std::string_view> function_name,
			std::optional<std::string_view> reason)
		{
			std::stringstream string_stream;
			string_stream << "Function is not implemented yet.";

			if (function_name.has_value())
			{
				string_stream << "Name: " << function_name.value();
			}

			if (reason.has_value())
			{
				string_stream << std::endl << "Reason: " << reason.value() << std::endl;
			}

			return string_stream.str();
		}
	};


	// String trimming
	
	[[maybe_unused]] inline void left_trim(std::string& s) {
		s.erase(
			s.begin(),
			std::find_if(s.begin(), s.end(),
				[](const int character) {
					return !std::isspace(character);
				}));
	}

	[[maybe_unused]] inline void right_trim(std::string& string) {
		string.erase(
			std::find_if(string.rbegin(), string.rend(),
				[](const int character) {
					return !std::isspace(character);
				}).base(),
					string.end());
	}

	[[maybe_unused]] inline void trim(std::string& s) {
		left_trim(s);
		right_trim(s);
	}


	// These need optimization

	[[maybe_unused]] inline size_t factorial(const size_t n)
	{
		if (n == 0) return 1;

		return n * factorial(n - 1);
	}

	[[maybe_unused]] inline size_t number_of_combinations(const size_t n, const size_t r)
	{
		return factorial(n) / (factorial(r) * factorial(n - r));
	}


	template<typename ScalarType>
	[[maybe_unused]] size_t highest_bit_position_scalar(ScalarType scalar)
	{
		size_t result = -1;

		while (scalar >>= 1)
		{
			result++;
		}

		return result;
	}

	template<typename BitmaskType>
	[[maybe_unused]] size_t highest_bit_position(BitmaskType bitmask)
	{
		if constexpr (std::is_enum<BitmaskType>::value)
		{
			using underlying_type = typename std::underlying_type<BitmaskType>::type;
			auto underlying_bitmask = static_cast<underlying_type>(bitmask);
			return highest_bit_position_scalar<underlying_type>(underlying_bitmask);
		}

		return highest_bit_position_scalar<BitmaskType>(bitmask);
	}


	
	// Vulkan specific
	
	template<typename InnerType>
	[[nodiscard, maybe_unused]] static std::vector<std::reference_wrapper<const InnerType>>
		dereference_vulkan_handles(
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
