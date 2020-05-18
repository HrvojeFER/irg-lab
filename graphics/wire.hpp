#ifndef IRGLAB_WIRE_HPP
#define IRGLAB_WIRE_HPP


#include "pch.hpp"
#include "extensions.hpp"

#include "primitives.hpp"
#include "bounds.hpp"
#include "transformations.hpp"


namespace irglab
{
	template<
	small_natural_number DimensionCount, bool IsTracking>
	struct is_wire_description_supported : 
		are_primitive_operations_supported<DimensionCount> {};

	template<small_natural_number DimensionCount, bool IsTracking>
	inline constexpr bool is_wire_description_supported_v =
		is_wire_description_supported<DimensionCount, IsTracking>::value;

	
	template<
	small_natural_number DimensionCount, bool IsTracking, std::enable_if_t<
		is_wire_description_supported_v<DimensionCount, IsTracking>,
	int> = 0>
	struct wire
	{
		static constexpr small_natural_number dimension_count = DimensionCount;
		static constexpr bool is_tracking = IsTracking;

		static constexpr small_natural_number vertex_count = 2;
		
		using point = point<dimension_count>;
		using tracked_point = tracked_pointer<point, wire<dimension_count, is_tracking>>;

	private:
		using conditional_point = std::conditional_t<is_tracking, tracked_point, point>;
		conditional_point begin_, end_;



		// Mutable accessors
		
		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && !is_tracking,
		int> = 0>
			[[nodiscard]] point& begin_mutable()
		{
			return begin_;
		}

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && !is_tracking,
		int> = 0>
			[[nodiscard]] point& end_mutable()
		{
			return end_;
		}


		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && is_tracking,
		int> = 0>
		[[nodiscard]] point& begin_mutable() const
		{
			return *begin_;
		}

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && is_tracking,
		int> = 0>
		[[nodiscard]] point& end_mutable() const
		{
			return *end_;
		}

		
	public:
		// Immutable accessors
		
		[[nodiscard]] const point& begin() const
		{
			if constexpr (std::is_same_v<conditional_point, point>) return begin_;
			else return *begin_;
		}

		[[nodiscard]] const point& end() const
		{
			if constexpr (std::is_same_v<conditional_point, point>) return end_;
			else return *end_;
		}


		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && is_tracking,
		int> = 0>
		[[nodiscard]] const tracked_point& begin_tracked() const
		{
			return begin_;
		}

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && is_tracking,
		int> = 0>
		[[nodiscard]] const tracked_point& end_tracked() const
		{
			return end_;
		}


		
		explicit wire(conditional_point begin, conditional_point end) noexcept :
			begin_{ std::move(begin) }, end_{ std::move(end) } { }


		
		[[nodiscard]] constexpr bool operator==(const wire& other) const
		{
			return
				(this->begin_ == other.begin_ &&
					this->end_ == other.end_) ||
				(this->begin_ == other.end_ &&
					this->end_ == other.begin_);
		}

		
		friend bounds<DimensionCount>& operator|(
			const bounds<DimensionCount>& old_bounds, const wire& wire)
		{
			return old_bounds | wire.begin() | wire.end();
		}
		
		friend constexpr void operator|=(
			bounds<DimensionCount>& bounds, const wire& wire)
		{
			bounds |= wire.begin(), bounds |= wire.end();
		}


		friend std::ostream& operator<<(std::ostream& output_stream, const wire& wire)
		{
			return output_stream <<
				glm::to_string(wire.begin()) << ", " <<
				glm::to_string(wire.end()) << std::endl;
		}


		// Not using std::swap because it is not constexpr and noexcept.
		constexpr void operator~()
		{
			const auto temp = this->begin_;
			this->begin_ = this->end_;
			this->end_ = temp;
		}

		
		wire operator*(const transformation<dimension_count>& transformation) const noexcept
		{
			wire new_wire{ *this };

			new_wire.begin_mutable() = this->begin() * transformation;
			new_wire.end_mutable() = this->end() * transformation;

			return new_wire;
		}

		
		// If not IsTracking

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && !is_tracking,
			int> = 0>
		constexpr void normalize()
		{
			irglab::normalize<dimension_count>(this->begin_mutable()),
			irglab::normalize<dimension_count>(this->end_mutable());
		}

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && !is_tracking,
			int> = 0>
		constexpr void operator*=(const transformation<DimensionCount>& transformation) noexcept
		{
			this->begin_mutable() = this->begin() * transformation;
			this->end_mutable() = this->end() * transformation;
		}


		// If IsTracking

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && is_tracking,
			int> = 0>
		[[nodiscard]] wire<dimension_count, false> detach() const
		{
			return wire<dimension_count, false>
			{
				this->begin(),
				this->end()
			};
		}

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && is_tracking,
			int> = 0>
		constexpr void normalize() const
		{
			irglab::normalize<dimension_count>(this->begin_mutable()),
			irglab::normalize<dimension_count>(this->end_mutable());
		}

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && is_tracking,
			int> = 0>
		constexpr void operator*=(
			const transformation<dimension_count>& transformation) const noexcept
		{
			this->begin_mutable() = this->begin() * transformation;
			this->end_mutable() = this->end() * transformation;
		}
	};


	template<
	natural_number DimensionCount, std::enable_if_t<
		is_wire_description_supported_v<DimensionCount, true>,
	int> = 0>
	using tracking_wire = wire<DimensionCount, true>;

	template<natural_number DimensionCount, std::enable_if_t<
		is_wire_description_supported_v<DimensionCount, false>,
	int> = 0>
	using owning_wire = wire<DimensionCount, false>;
}


template<
irglab::natural_number DimensionCount>
struct std::hash<irglab::tracking_wire<DimensionCount>>
{
	using key = irglab::tracking_wire<DimensionCount>;
	static constexpr irglab::small_natural_number dimension_count = key::dimension_count;

private:
	using tracked_vertex = typename key::tracked_point;
	static inline const std::hash<tracked_vertex> tracked_vertex_hasher{};

public:
	size_t operator()(const key& key) const noexcept
	{
		return
			tracked_vertex_hasher(key.begin_tracked()) ^
			tracked_vertex_hasher(key.end_tracked());
	}
};


namespace irglab::two_dimensional
{
	template<bool IsTracking>
	using wire = irglab::wire<dimension_count, IsTracking>;
	
	using owning_wire = irglab::owning_wire<dimension_count>;
	using tracking_wire = irglab::tracking_wire<dimension_count>;
}

namespace irglab::three_dimensional
{
	template<bool IsTracking>
	using wire = irglab::wire<dimension_count, IsTracking>;

	using owning_wire = irglab::owning_wire<dimension_count>;
	using tracking_wire = irglab::tracking_wire<dimension_count>;
}

#endif
