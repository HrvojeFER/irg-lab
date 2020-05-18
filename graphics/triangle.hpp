#ifndef IRGLAB_TRIANGLE_HPP
#define IRGLAB_TRIANGLE_HPP


#include "pch.hpp"
#include "extensions.hpp"

#include "primitives.hpp"
#include "transformations.hpp"
#include "direction.hpp"
#include "bounds.hpp"

#include "wire.hpp"
#include "wireframe.hpp"


namespace irglab
{
	template<
	small_natural_number DimensionCount, bool IsTracking>
	struct is_triangle_description_supported : 
		are_primitive_operations_supported<DimensionCount> {};

	template<small_natural_number DimensionCount, bool IsTracking>
	inline constexpr bool is_triangle_description_supported_v =
		is_triangle_description_supported<DimensionCount, IsTracking>::value;

	
	template<
	small_natural_number DimensionCount, bool IsTracking = false, std::enable_if_t<
		is_triangle_description_supported_v<DimensionCount, IsTracking>,
	int> = 0>
	struct triangle
	{
		static constexpr small_natural_number dimension_count = DimensionCount;
		static constexpr small_natural_number is_tracking = IsTracking;

		static constexpr small_natural_number vertex_count = 3;
		
		using vertex = point<DimensionCount>;
		using tracked_vertex = tracked_pointer<vertex, triangle>;

	private:
		using conditional_vertex = std::conditional_t<IsTracking, tracked_vertex, vertex>;
		conditional_vertex first_, second_, third_;

	public:
		explicit triangle(
			conditional_vertex first,
			conditional_vertex second,
			conditional_vertex third) noexcept :

			first_{ std::move(first) }, second_{ std::move(second) }, third_{ std::move(third) } { }

		

	private:
		// Mutable accessors

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && !is_tracking,
		int> = 0>
		[[nodiscard]] vertex& first_mutable()
		{
			return first_;
		}

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && !is_tracking,
		int> = 0>
		[[nodiscard]] vertex& second_mutable()
		{
			return second_;
		}

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && !is_tracking,
		int> = 0>
		[[nodiscard]] vertex& third_mutable()
		{
			return third_;
		}

		
		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && is_tracking,
		int> = 0>
		[[nodiscard]] vertex& first_mutable() const
		{
			return *first_;
		}

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && is_tracking,
		int> = 0>
		[[nodiscard]] vertex& second_mutable() const
		{
			return *second_;
		}

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && is_tracking,
		int> = 0>
		[[nodiscard]] vertex& third_mutable() const
		{
			return *third_;
		}


		
	public:
		// Immutable accessors
		
		[[nodiscard]] const vertex& first() const
		{
			if constexpr (std::is_same_v<conditional_vertex, vertex>) return first_;
			else return *first_;
		}

		[[nodiscard]] const vertex& second() const
		{
			if constexpr (std::is_same_v<conditional_vertex, vertex>) return second_;
			else return *second_;
		}

		[[nodiscard]] const vertex& third() const
		{
			if constexpr (std::is_same_v<conditional_vertex, vertex>) return third_;
			else return *third_;
		}

		
		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && is_tracking,
			int> = 0>
		[[nodiscard]] const tracked_vertex& first_tracked() const
		{
			return first_;
		}

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && is_tracking,
			int> = 0>
			[[nodiscard]] const tracked_vertex& second_tracked() const
		{
			return first_;
		}

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && is_tracking,
			int> = 0>
			[[nodiscard]] const tracked_vertex& third_tracked() const
		{
			return first_;
		}


		
		constexpr bool operator==(triangle& other) const
		{
			return
				first_ == other.first_ &&
				second_ == other.second_ &&
				third_ == other.third_;
		}


		using barycentric_coordinates = vector<dimension_count>;

		[[nodiscard]] barycentric_coordinates get_barycentric_coordinates(
			const point<dimension_count>& point) const
		{
			return point * glm::inverse(
				matrix<dimension_count + small_one, vertex_count>
			{
				first(), second(), third()
			});
		}


		friend bounds<dimension_count> operator|(
			bounds<dimension_count>& bounds, const triangle& triangle)
		{
			return bounds | triangle.first() | triangle.second() | triangle.third();
		}

		friend constexpr void operator|=(
			bounds<dimension_count>& bounds, const triangle& triangle)
		{
			bounds |= triangle.first(), bounds |= triangle.second(), bounds |= triangle.third();
		}

		
		friend std::ostream& operator<<(std::ostream& output_stream, const triangle& triangle)
		{
			return output_stream <<
				"Vertices:" << std::endl <<
				glm::to_string(triangle.first()) << std::endl <<
				glm::to_string(triangle.second()) << std::endl <<
				glm::to_string(triangle.third()) << std::endl <<
				std::endl;
		}


		void operator~()
		{
			std::swap(first_, third_);
		}


		triangle operator*(
			const transformation<dimension_count>& transformation) const noexcept
		{
			triangle new_triangle{ *this };

			new_triangle.first_mutable() = this->first() * transformation;
			new_triangle.second_mutable() = this->second() * transformation;
			new_triangle.third_mutable() = this->third() * transformation;

			return new_triangle;
		}

		
		// It would be great if this madness was shorter.
		
		friend void operator+=(
			wireframe<dimension_count, IsTracking>& wireframe,
			const triangle& triangle)
		{
			if constexpr (IsTracking)
			{
				using wireframe_t = tracking_wireframe<dimension_count>;
				using wireframe_tracked_vertex = typename wireframe_t::tracked_vertex;
				using wireframe_wire = typename wireframe_t::wire;

				wireframe_tracked_vertex first_vertex{ triangle.first_.inner() };
				wireframe_tracked_vertex second_vertex{ triangle.second_.inner() };
				wireframe_tracked_vertex third_vertex{ triangle.third_.inner() };

				const auto first_wire = std::make_shared<wireframe_wire>(first_vertex, second_vertex);
				const auto second_wire = std::make_shared<wireframe_wire>(second_vertex, third_vertex);
				const auto third_wire = std::make_shared<wireframe_wire>(third_vertex, first_vertex);

				first_vertex += first_wire, first_vertex += second_wire;
				second_vertex += second_wire, second_vertex += third_wire;
				third_vertex += third_wire, third_vertex += first_wire;

				wireframe += first_wire, wireframe += second_wire, wireframe += third_wire;
			}
			else
			{
				using wire = typename owning_wireframe<dimension_count>::wire;
				wireframe += wire{ triangle.first(), triangle.second() };
				wireframe += wire{ triangle.second(), triangle.third() };
				wireframe += wire{ triangle.third(), triangle.first() };
			}
		}


		
		// If not IsTracking
		
		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && !is_tracking,
			int> = 0>
		constexpr void normalize()
		{
			irglab::normalize<dimension_count>(this->first_mutable());
			irglab::normalize<dimension_count>(this->second_mutable());
			irglab::normalize<dimension_count>(this->third_mutable());
		}

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && !is_tracking,
			int> = 0>
		constexpr void operator*=(const transformation<dimension_count>& transformation) noexcept
		{
			this->first_mutable() = this->first() * transformation;
			this->second_mutable() = this->second() * transformation;
			this->third_mutable() = this->third() * transformation;
		}

		
		// If IsTracking

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void>&& is_tracking,
			int> = 0>
		[[nodiscard]] triangle<dimension_count, false> get_detached() const
		{
			return triangle<dimension_count, false>
			{
				this->first(),
				this->second(),
				this->third()
			};
		}
		
		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && is_tracking,
			int> = 0>
		void normalize() const
		{
			irglab::normalize<dimension_count>(this->first_mutable());
			irglab::normalize<dimension_count>(this->second_mutable());
			irglab::normalize<dimension_count>(this->third_mutable());
		}

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && is_tracking,
			int> = 0>
		void operator*=(const transformation<dimension_count>& transformation) const
		{
			this->first_mutable() = this->first() * transformation;
			this->second_mutable() = this->second() * transformation;
			this->third_mutable() = this->third() * transformation;
		}


		
		// Two dimensional
		
		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && 
			DimensionCount == two_dimensional::dimension_count,
			int> = 0>
		[[nodiscard]] direction get_direction() const
		{
			return two_dimensional::get_direction(
				this->first(), this->second(), this->third());
		}
		
		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> &&
			DimensionCount == two_dimensional::dimension_count,
			int> = 0>
		void operator%=(const direction direction)
		{
			if (this->get_direction() != direction) ~*this;
		}
		
		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> &&
			DimensionCount == two_dimensional::dimension_count,
			int> = 0>
		[[nodiscard]] friend bool operator<(
			const two_dimensional::point& point, const triangle& triangle)
		{
			const auto triangle_direction = triangle.get_direction();

			return
				two_dimensional::get_direction(
					triangle.first(), triangle.second(), point
				) == triangle_direction &&

				two_dimensional::get_direction(
					triangle.second(), triangle.third(), point
				) == triangle_direction &&

				two_dimensional::get_direction(
					triangle.third(), triangle.first(), point
				) == triangle_direction;
		}


		// Three dimensional
		
		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> && 
			dimension_count == three_dimensional::dimension_count,
		int> = 0>
		[[nodiscard]] three_dimensional::plane get_plane() const
		{
			return three_dimensional::get_common_plane(
				three_dimensional::to_cartesian_coordinates(this->first()),
				three_dimensional::to_cartesian_coordinates(this->second()),
				three_dimensional::to_cartesian_coordinates(this->third()));
		}
		
		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> &&
			dimension_count == three_dimensional::dimension_count,
		int> = 0>
		[[nodiscard]] three_dimensional::plane_normal get_plane_normal() const
		{
			return three_dimensional::get_plane_normal(
				three_dimensional::to_cartesian_coordinates(this->first()),
				three_dimensional::to_cartesian_coordinates(this->second()),
				three_dimensional::to_cartesian_coordinates(this->third()));
		}
		
		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> &&
			dimension_count == three_dimensional::dimension_count,
		int> = 0>
		[[nodiscard]] three_dimensional::cartesian_coordinates get_center() const
		{
			return
				three_dimensional::to_cartesian_coordinates(this->first()) +
				three_dimensional::to_cartesian_coordinates(this->second()) +
				three_dimensional::to_cartesian_coordinates(this->third()) / 3.0f;
		}
		
		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> &&
			dimension_count == three_dimensional::dimension_count,
		int> = 0>
		[[nodiscard]] friend bool operator<(
			const three_dimensional::point& point, const triangle& triangle)
		{
			return three_dimensional::get_direction(triangle.get_plane(), point);
		}
	};

	
	template<natural_number DimensionCount, std::enable_if_t<
		is_triangle_description_supported_v<DimensionCount, false>,
	int> = 0>
	using owning_triangle = triangle<DimensionCount, false>;
	
	template<natural_number DimensionCount, std::enable_if_t<
		is_triangle_description_supported_v<DimensionCount, true>,
	int> = 0>
	using tracking_triangle = triangle<DimensionCount, true>;
}


template<irglab::natural_number DimensionCount>
struct std::hash<irglab::tracking_triangle<DimensionCount>>
{
	using key = irglab::tracking_triangle<DimensionCount>;
	static constexpr irglab::small_natural_number dimension_count = key::dimension_count;

private:
	using tracked_vertex = typename key::tracked_vertex;
	static inline const std::hash<tracked_vertex> tracked_vertex_hasher{};

public:
	size_t operator()(const key& key) const noexcept
	{
		return
			tracked_vertex_hasher(key.first_tracked()) ^ 
			tracked_vertex_hasher(key.second_tracked()) ^
			tracked_vertex_hasher(key.third_tracked());
	}
};


namespace irglab::two_dimensional
{
	template<bool IsTracking>
	using triangle = irglab::triangle<dimension_count, IsTracking>;
	
	using owning_triangle = irglab::owning_triangle<dimension_count>;
	using tracking_triangle = irglab::tracking_triangle<dimension_count>;
}


namespace irglab::three_dimensional
{
	template<bool IsTracking>
	using triangle = irglab::triangle<dimension_count, IsTracking>;

	using owning_triangle = irglab::owning_triangle<dimension_count>;
	using tracking_triangle = irglab::tracking_triangle<dimension_count>;
}

#endif
