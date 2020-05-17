#ifndef IRGLAB_EXTENSIONS_HPP
#define IRGLAB_EXTENSIONS_HPP


#include "pch.hpp"


namespace irglab
{
	// Semantic key that works with any collection that supports the [] operator.
	// It would be great if I could make a compile time counter so that it is constexpr.

	struct [[maybe_unused]] semantic_key
	{
		semantic_key() : inner_(inner_counter_++) {}

		bool operator ==(const semantic_key& other) const
		{
			return inner_ == other.inner_;
		}

		// ReSharper disable once CppNonExplicitConversionOperator
		operator size_t() const
		{
			return inner_;
		}

		friend std::hash<semantic_key>;

	private:
		static inline size_t inner_counter_{ 0 };
		const size_t inner_;
	};
}

template<>
struct std::hash<irglab::semantic_key>
{
private:
	using key = irglab::semantic_key;

public:
	size_t operator()(const key& key) const noexcept
	{
		return key.inner_;
	}
};

namespace irglab
{
	template<typename ValueType>
	using semantic_map [[maybe_unused]] =
		std::unordered_map<semantic_key, ValueType>;

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

			// ReSharper disable once CppFunctionIsNotImplemented
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


	// Check Variant type

	template<typename CheckingType, typename Type>
	struct [[maybe_unused]] is_one_of : std::is_same<CheckingType, Type> {};

	template<typename CheckingType, typename... Variants>
	struct [[maybe_unused]] is_one_of<CheckingType, std::variant<Variants...>>
		: std::disjunction<std::is_same<CheckingType, Variants>...> {};

	template<typename CheckingType, typename Type>
	[[maybe_unused]] constexpr bool is_one_of_v = is_one_of<CheckingType, Type>::value;


	// Check Iterator type

	template<typename T, typename = void>
	struct [[maybe_unused]] is_iterator : std::false_type{};

	template<typename T>
	struct [[maybe_unused]] is_iterator<T, std::enable_if_t<
		!std::is_same_v<typename std::iterator_traits<T>::value_type, void>,
		void>> : std::true_type{};

	template<typename T>
	[[maybe_unused]] constexpr bool is_iterator_v = is_iterator<T>::value;


	// Have operator check (use std::equal_to<> and such to check)

	template<typename Operator, typename First, typename Second = First>
	struct have_operator
	{
	private:
		template<typename StaticOperator, typename StaticFirst, typename StaticSecond = StaticFirst>
		// ReSharper disable once CppFunctionIsNotImplemented
		static auto test(int) ->
			decltype(std::declval<StaticOperator>()(
				std::declval<StaticFirst>(), std::declval<StaticSecond>()),
				void(), std::true_type{});

		template<typename StaticOperator, typename StaticFirst, typename StaticSecond = StaticFirst>
		// ReSharper disable once CppFunctionIsNotImplemented
		static auto test(...)->std::false_type;

	public:
		using type = decltype(test<First, Second, Operator>(0));
	};

	template<typename Operator, typename First, typename Second = First>
	using have_operator_t = typename have_operator<First, Second, Operator>::type;

	template<typename Operator, typename First, typename Second = First>
	constexpr bool have_operator_v = have_operator_t<First, Second, Operator>::value;


	// Tracked pointer that keeps a list of its trackers.
	// It would be great if the owner type didn't matter and
	// if I could delete the owners automatically.

	template<typename InnerType, typename TrackerType>
	struct [[maybe_unused]] tracked_pointer;
}

template<typename InnerType, typename TrackerType>
// ReSharper disable once CppEmptyDeclaration
struct std::hash<irglab::tracked_pointer<InnerType, TrackerType>>;

namespace irglab
{
	template<typename InnerType, typename TrackerType>
	struct [[maybe_unused]] tracked_pointer
	{
		using inner_type = InnerType;
		using tracker_type = TrackerType;
		
		explicit tracked_pointer(std::shared_ptr<InnerType> shared) :
			inner_{ std::move(shared) },
			trackers_{ std::make_shared<std::vector<std::weak_ptr<TrackerType>>>(0) } { }

		
		[[nodiscard]] InnerType& operator*() const noexcept
		{
			return *inner_;
		}

		[[nodiscard]] InnerType* operator->() const noexcept
		{
			return &*inner_;
		}

		[[nodiscard]] std::shared_ptr<InnerType> shared_inner() const noexcept
		{
			return inner_;
		}

		
		[[nodiscard]] std::shared_ptr<const std::vector<std::weak_ptr<TrackerType>>> trackers() const
		{
			return std::const_pointer_cast<const std::vector<std::weak_ptr<TrackerType>>>(trackers_);
		}

		void prune() const
		{
			trackers_->erase(std::remove_if(trackers_->begin(), trackers_->end(),
				[](const std::weak_ptr<TrackerType>& tracker) -> bool
				{
					return tracker.expired();
				}), trackers_->end());
		}


		[[nodiscard]] bool operator==(const tracked_pointer& other) const
		{
			return this->inner_ == other.inner_;
		}

		
		void operator+=(const std::shared_ptr<TrackerType>& owner) const
		{
			trackers_->emplace_back(std::weak_ptr<TrackerType>(owner));
		}

		void operator+=(const tracked_pointer& other) const
		{
			if (*this == other && !(this->trackers_ == other.trackers_))
			{
				other.trackers_->erase(std::remove_if(other.trackers_->begin(), other.trackers_->end(),
					[this](const std::weak_ptr<TrackerType>& tracker) -> bool
					{
						if (!tracker.expired())
						{
							const auto tracker_lock = tracker.lock();
							return std::find_if(this->trackers_->begin(), this->trackers_->end(),
								[&tracker_lock](const std::weak_ptr<TrackerType>& this_tracker) -> bool
								{
									return !this_tracker.expired() and
										this_tracker.lock() == tracker_lock;
								}) != this->trackers_->end();
						}
						return false;
					}), other.trackers_->end());
				
				this->trackers_->insert(this->trackers_->begin(),
					other.trackers_->begin(), other.trackers_->end());
			}
		}

		[[nodiscard]] bool operator&&(const std::shared_ptr<TrackerType> tracker) const
		{
			return *this && tracker.get();
		}

		[[nodiscard]] bool operator&&(const TrackerType* tracker) const
		{
			return std::find_if(trackers_->begin(), trackers_->end(),
				[&tracker](const std::weak_ptr<TrackerType>& this_tracker)
				{
					return !this_tracker.expired() and this_tracker.lock().get() == tracker;
				}) != trackers_->end();
		}

		
	private:
		template<typename InnerInType, typename TrackerInType>
		friend struct tracked_pointer;

		using allowed_inner_conversion_types =  std::conditional_t<std::is_const_v<InnerType>,
				const InnerType, std::variant<const InnerType, InnerType>>;

		explicit tracked_pointer(
			std::shared_ptr<InnerType> shared, 
			std::shared_ptr<std::vector<std::weak_ptr<TrackerType>>> trackers) :

			inner_{ std::move(shared) },
			trackers_{ std::move(trackers) } { }

	public:
		template<typename InnerOutType, typename TrackerOutType, std::enable_if_t<
			is_one_of_v<InnerOutType, allowed_inner_conversion_types> &&
			have_operator_v<std::equal_to<>, TrackerOutType>,
			int> = 0>
		[[nodiscard]] explicit operator tracked_pointer<InnerOutType, TrackerOutType>()
		{
			if constexpr(std::is_same_v<TrackerType, TrackerOutType>)
			{
				return tracked_pointer<InnerOutType, TrackerOutType>(
					std::const_pointer_cast<InnerOutType>(inner_), trackers_);
			}
			else
			{
				return tracked_pointer<InnerOutType, TrackerOutType>(
					std::const_pointer_cast<InnerOutType>(inner_));
			}
		}

		friend std::hash<tracked_pointer>;
		
	private:
		std::shared_ptr<InnerType> inner_;
		std::shared_ptr<std::vector<std::weak_ptr<TrackerType>>> trackers_;
	};
}


template<typename InnerType, typename TrackerType>
struct std::hash<irglab::tracked_pointer<InnerType, TrackerType>>
{
private:
	static inline const std::hash<std::shared_ptr<InnerType>> inner_hasher{};

public:
	size_t operator()(const irglab::tracked_pointer<InnerType, TrackerType>& key) const noexcept
	{
		return inner_hasher(key.inner_);
	}
};


namespace irglab
{
	
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


	template<typename ScalarType, std::enable_if_t<std::is_arithmetic_v<ScalarType>, int> = 0>
	[[maybe_unused]] size_t highest_bit_position(ScalarType scalar)
	{
		size_t result = -1;

		while (scalar >>= 1)
		{
			result++;
		}

		return result;
	}

	template<typename BitmaskType, std::enable_if_t<
		std::is_enum_v<BitmaskType> and std::is_arithmetic_v<std::underlying_type_t<BitmaskType>>,
		int> = 0>
	[[maybe_unused]] size_t highest_bit_position(BitmaskType bitmask)
	{
		using underlying_type = std::underlying_type_t<BitmaskType>;
		auto underlying_bitmask = static_cast<underlying_type>(bitmask);
		return highest_bit_position<underlying_type>(underlying_bitmask);
	}

	template <typename ForwardIterator, std::enable_if_t<
		is_iterator_v<ForwardIterator> && 
		std::is_convertible_v<typename std::iterator_traits<ForwardIterator>::iterator_category,
			std::forward_iterator_tag> &&
		have_operator_v<std::equal_to<>, typename  std::iterator_traits<ForwardIterator>::value_type>,
	int> = 0>
	ForwardIterator remove_duplicates(ForwardIterator first, ForwardIterator last)
	{
		auto new_last = first;

		for (auto current = first; current != last; ++current)
		{
			if (std::find(first, new_last, *current) == new_last)
			{
				if (new_last != current) *new_last = *current;
				++new_last;
			}
		}

		return new_last;
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
			result.emplace_back(std::cref(*handle));
		}

		return result;
	}
}


#endif
