#ifndef IRGLAB_WIRE_HPP
#define IRGLAB_WIRE_HPP


#include "pch.hpp"
#include "extensions.hpp"

#include "primitives.hpp"
#include "bounds.hpp"
#include "transformations.hpp"


namespace irglab
{
	template<size DimensionCount, bool IsTracking, typename InheritorType>
	struct wire_internal
	{
		using point = point<DimensionCount>;
		using tracked_point = tracked_pointer<point, InheritorType>;
		using conditional_point = std::conditional_t<IsTracking, tracked_point, point>;

	protected:
		conditional_point begin_, end_;

		explicit wire_internal(conditional_point begin, conditional_point end) noexcept :
			begin_{ std::move(begin) }, end_{ std::move(end) } { }


		[[nodiscard]] point& begin_mutable()
		{
			if constexpr (std::is_same_v<conditional_point, point>) return begin_;
			else return *begin_;
		}

		[[nodiscard]] point& end_mutable()
		{
			if constexpr (std::is_same_v<conditional_point, point>) return end_;
			else return *end_;
		}

	public:
		[[nodiscard]] const conditional_point& begin_conditional() const
		{
			return begin_;
		}

		[[nodiscard]] const conditional_point& end_conditional() const
		{
			return end_;
		}


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


		[[nodiscard]] constexpr bool operator==(const wire_internal& other) const
		{
			return
				(this->begin_conditional() == other.begin_conditional() &&
					this->end_conditional() == other.end_conditional()) ||
				(this->begin_conditional() == other.end_conditional() &&
					this->end_conditional() == other.begin_conditional());
		}

		
		friend bounds<DimensionCount>& operator|(
			const bounds<DimensionCount>& old_bounds, const wire_internal& wire)
		{
			return old_bounds | wire.begin() | wire.end();
		}
		
		friend constexpr void operator|=(
			bounds<DimensionCount>& bounds, const wire_internal& wire)
		{
			bounds |= wire.begin(), bounds |= wire.end();
		}


		friend std::ostream& operator<<(std::ostream& output_stream, const wire_internal& wire)
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

		
		wire_internal operator*(const transformation<DimensionCount>& transformation) const noexcept
		{
			wire_internal new_wire{ *this };

			new_wire.begin_mutable() = this->begin() * transformation;
			new_wire.end_mutable() = this->end() * transformation;

			return new_wire;
		}
	};


	template<size DimensionCount, typename InheritorType>
	using tracking_wire_internal = wire_internal<DimensionCount, true, InheritorType>;

	template<size DimensionCount, typename InheritorType>
	using owning_wire_internal = wire_internal<DimensionCount, false, InheritorType>;



	template<size DimensionCount, bool IsTracking>
	struct wire : wire_internal<DimensionCount, IsTracking, wire<DimensionCount, IsTracking>>
	{
		wire() = delete;
	};

	
	template<size DimensionCount>
	struct wire<DimensionCount, false> final :
		wire_internal<DimensionCount, false, wire<DimensionCount, false>>
	{
	private:
		using base = wire_internal <DimensionCount, false, wire<DimensionCount, false>>;

	public:
		explicit wire(
			const typename base::point& begin, 
			const typename base::point& end) noexcept :

			base{ begin, end } { }

		
		constexpr void normalize()
		{
			irglab::normalize<DimensionCount>(this->begin_mutable()),
			irglab::normalize<DimensionCount>(this->end_mutable());
		}

		constexpr void operator*=(const transformation<DimensionCount>& transformation) noexcept
		{
			this->begin_mutable() = this->begin() * transformation;
			this->end_mutable() = this->end() * transformation;
		}
	};

	template<size DimensionCount>
	using owning_wire = wire<DimensionCount, false>;

	
	template<size DimensionCount>
	struct wire<DimensionCount, true> final :
		wire_internal<DimensionCount, true, wire<DimensionCount, true>>
	{
	private:
		using base = wire_internal<DimensionCount, true, wire<DimensionCount, true>>;

	public:
		explicit wire(
			const typename base::tracked_point& begin, 
			const typename base::tracked_point& end) noexcept :

			base{ begin, end } { }

		
		[[nodiscard]] wire<DimensionCount, false> detach() const
		{
			return wire<DimensionCount, false>
			{
				this->begin(),
				this->end()
			};
		}

		
		constexpr void normalize() const
		{
			irglab::normalize<DimensionCount>(*this->begin_conditional()),
			irglab::normalize<DimensionCount>(*this->end_conditional());
		}

		constexpr void operator*=(const transformation<DimensionCount>& transformation) const noexcept
		{
			*this->begin_conditional() = this->begin() * transformation;
			*this->end_conditional() = this->end() * transformation;
		}
	};

	template<size DimensionCount>
	using tracking_wire = wire<DimensionCount, true>;
}

template<irglab::size DimensionCount>
struct std::hash<irglab::tracking_wire<DimensionCount>>
{
private:
	using key = irglab::wire<DimensionCount, true>;
	
	using tracked_vertex = typename key::tracked_point;
	static inline const std::hash<tracked_vertex> tracked_vertex_hasher{};

public:
	size_t operator()(const key& key) const noexcept
	{
		return
			tracked_vertex_hasher(key.begin_conditional()) ^
			tracked_vertex_hasher(key.end_conditional());
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
