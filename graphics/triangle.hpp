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
	template<size DimensionCount, bool IsTracking, typename InheritorType>
	struct triangle_root
	{
		using vertex = point<DimensionCount>;
		using tracked_vertex = tracked_pointer<vertex, InheritorType>;
		using conditional_vertex = std::conditional_t<IsTracking, tracked_vertex, vertex>;

	protected:
		conditional_vertex first_, second_, third_;

		explicit triangle_root(
			conditional_vertex first,
			conditional_vertex second,
			conditional_vertex third) noexcept :

			first_{ std::move(first) }, second_{ std::move(second) }, third_{ std::move(third) } { }


		// Usually the condition would be IsTracking, but Visual Studio is giving me a
		// headache when I do it that way.

		[[nodiscard]] vertex& first_mutable()
		{
			if constexpr (std::is_same_v<conditional_vertex, vertex>) return first_;
			else return *first_;
		}

		[[nodiscard]] vertex& second_mutable()
		{
			if constexpr (std::is_same_v<conditional_vertex, vertex>) return second_;
			else return *second_;
		}

		[[nodiscard]] vertex& third_mutable()
		{
			if constexpr (std::is_same_v<conditional_vertex, vertex>) return third_;
			else return *third_;
		}


	public:
		[[nodiscard]] const conditional_vertex& first_conditional() const { return first_; }
		[[nodiscard]] const conditional_vertex& second_conditional() const { return second_; }
		[[nodiscard]] const conditional_vertex& third_conditional() const { return third_; }


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


		constexpr bool operator==(triangle_root& other) const
		{
			return
				first_conditional() == other.first_conditional() &&
				second_conditional() == other.second_conditional() &&
				third_conditional() == other.third_conditional();
		}


		using barycentric_coordinates = vector<DimensionCount>;

		[[nodiscard]] barycentric_coordinates get_barycentric_coordinates(
			const point<DimensionCount>& point) const
		{
			return point * inverse(
				glm::mat<3, DimensionCount + 1, number, precision>
			{
				first(), second(), third()
			});
		}


		friend bounds<DimensionCount> operator|(
			bounds<DimensionCount>& bounds, const triangle_root& triangle)
		{
			return bounds | triangle.first() | triangle.second() | triangle.third();
		}

		friend constexpr void operator|=(
			bounds<DimensionCount>& bounds, const triangle_root& triangle)
		{
			bounds |= triangle.first(), bounds |= triangle.second(), bounds |= triangle.third();
		}

		
		friend std::ostream& operator<<(std::ostream& output_stream, const triangle_root& triangle)
		{
			return output_stream <<
				"Vertices:" << std::endl <<
				glm::to_string(triangle.first()) << std::endl <<
				glm::to_string(triangle.second()) << std::endl <<
				glm::to_string(triangle.third()) << std::endl <<
				std::endl;
		}


		// Not using std::swap because it is not constexpr and noexcept.
		constexpr void operator~() noexcept
		{
			const auto temp = first_;
			first_ = third_;
			third_ = temp;
		}


		triangle_root operator*(
			const transformation<DimensionCount>& transformation) const noexcept
		{
			triangle_root new_triangle{ *this };

			new_triangle.first_mutable() = this->first() * transformation;
			new_triangle.second_mutable() = this->second() * transformation;
			new_triangle.third_mutable() = this->third() * transformation;

			return new_triangle;
		}

		
		friend void operator+=(
			owning_wireframe<DimensionCount>& wireframe,
			const triangle_root& triangle)
		{
			if constexpr (IsTracking)
			{
				using wireframe_t = tracking_wireframe<DimensionCount>;
				using wireframe_tracked_vertex = typename wireframe_t::tracked_vertex;
				using wireframe_wire = typename wireframe_t::wire;

				wireframe_tracked_vertex first_vertex{ triangle.first_.shared_inner() };
				wireframe_tracked_vertex second_vertex{ triangle.second_.shared_inner() };
				wireframe_tracked_vertex third_vertex{ triangle.third_.shared_inner() };

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
				using wire = typename owning_wireframe<DimensionCount>::wire;
				wireframe += wire{ triangle.first(), triangle.second() };
				wireframe += wire{ triangle.second(), triangle.third() };
				wireframe += wire{ triangle.third(), triangle.first() };
			}
		}

		// If not IsTracking
		
		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> and not IsTracking,
			int> = 0>
		constexpr void normalize()
		{
			irglab::normalize<DimensionCount>(this->first_mutable());
			irglab::normalize<DimensionCount>(this->second_mutable());
			irglab::normalize<DimensionCount>(this->third_mutable());
		}

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> and not IsTracking,
			int> = 0>
		constexpr void operator*=(const transformation<DimensionCount>& transformation) noexcept
		{
			this->first_mutable() = this->first() * transformation;
			this->second_mutable() = this->second() * transformation;
			this->third_mutable() = this->third() * transformation;
		}

		
		// If IsTracking
		
		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> and IsTracking,
			int> = 0>
		void normalize() const
		{
			irglab::normalize<DimensionCount>(*this->first_conditional());
			irglab::normalize<DimensionCount>(*this->second_conditional());
			irglab::normalize<DimensionCount>(*this->third_conditional());
		}

		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> and IsTracking,
			int> = 0>
		void operator*=(const transformation<DimensionCount>& transformation) const
		{
			*this->first_conditional() = this->first() * transformation;
			*this->second_conditional() = this->second() * transformation;
			*this->third_conditional() = this->third() * transformation;
		}

		
		// Two dimensional
		
		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void> and DimensionCount == 2,
			int> = 0>
		[[nodiscard]] direction get_direction() const
		{
			return two_dimensional::get_direction(
				this->first(), this->second(), this->third());
		}
		
		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void>and DimensionCount == 2,
			int> = 0>
		void operator%=(const direction direction)
		{
			if (this->get_direction() != direction) ~*this;
		}
		
		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void>and DimensionCount == 2,
			int> = 0>
		[[nodiscard]] friend bool operator<(
			const two_dimensional::point& point, const triangle_root& triangle)
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
			std::is_same_v<Dummy, void>and DimensionCount == 3,
			int> = 0>
		[[nodiscard]] three_dimensional::plane get_plane() const
		{
			return three_dimensional::get_common_plane(
				three_dimensional::to_cartesian_coordinates(this->first()),
				three_dimensional::to_cartesian_coordinates(this->second()),
				three_dimensional::to_cartesian_coordinates(this->third()));
		}
		
		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void>and DimensionCount == 3,
			int> = 0>
		[[nodiscard]] three_dimensional::plane_normal get_plane_normal() const
		{
			return three_dimensional::get_plane_normal(
				three_dimensional::to_cartesian_coordinates(this->first()),
				three_dimensional::to_cartesian_coordinates(this->second()),
				three_dimensional::to_cartesian_coordinates(this->third()));
		}
		
		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void>and DimensionCount == 3,
			int> = 0>
		[[nodiscard]] three_dimensional::cartesian_coordinates get_center() const
		{
			return three_dimensional::to_cartesian_coordinates(this->first()) +
				three_dimensional::to_cartesian_coordinates(this->second()) +
				three_dimensional::to_cartesian_coordinates(this->third()) / 3.0f;
		}
		
		template<typename Dummy = void, std::enable_if_t<
			std::is_same_v<Dummy, void>and DimensionCount == 3,
			int> = 0>
		[[nodiscard]] friend bool operator<(
			const three_dimensional::point& point, const triangle_root& triangle)
		{
			return three_dimensional::get_direction(point, triangle.get_plane());
		}
	};


	template<size DimensionCount, typename InheritorType>
	using tracking_triangle_root = triangle_root<DimensionCount, true, InheritorType>;

	template<size DimensionCount, typename InheritorType>
	using owning_triangle_root = triangle_root<DimensionCount, false, InheritorType>;



	template<size DimensionCount, bool IsTracking, typename InheritorType>
	struct triangle_internal : triangle_root<DimensionCount, IsTracking, InheritorType>
	{
		triangle_internal() = delete;
	};

	
	template<size DimensionCount, typename InheritorType>
	struct triangle_internal<DimensionCount, false, InheritorType> :
		triangle_root<DimensionCount, false, InheritorType>
	{
	private:
		using base = triangle_root<DimensionCount, false, InheritorType>;
		
	protected:
		explicit triangle_internal(
			typename base::vertex first,
			typename base::vertex second,
			typename base::vertex third) noexcept :

			base{ std::move(first), std::move(second), std::move(third) } { }

	public:
		friend void operator+=(
			owning_wireframe<DimensionCount>& wireframe,
			const triangle_internal& triangle)
		{
			using wire = typename owning_wireframe<DimensionCount>::wire;
			wireframe += wire{ triangle.first(), triangle.second() };
			wireframe += wire{ triangle.second(), triangle.third() };
			wireframe += wire{ triangle.third(), triangle.first() };
		}

		
		constexpr void normalize()
		{
			irglab::normalize<DimensionCount>(this->first_mutable());
			irglab::normalize<DimensionCount>(this->second_mutable());
			irglab::normalize<DimensionCount>(this->third_mutable());
		}

		
		constexpr void operator*=(const transformation<DimensionCount>& transformation) noexcept
		{
			this->first_mutable() = this->first() * transformation;
			this->second_mutable() = this->second() * transformation;
			this->third_mutable() = this->third() * transformation;
		}
	};

	template<size DimensionCount, typename InheritorType>
	using owning_triangle_internal = triangle_internal<DimensionCount, false, InheritorType>;


	template<size DimensionCount, typename InheritorType>
	struct triangle_internal<DimensionCount, true, InheritorType> :
		triangle_root<DimensionCount, true, InheritorType>
	{
	private:
		using base = triangle_root<DimensionCount, true, InheritorType>;

	protected:
		explicit triangle_internal(
			typename base::tracked_vertex first,
			typename base::tracked_vertex second,
			typename base::tracked_vertex third) noexcept :

			base{ std::move(first), std::move(second), std::move(third) } { }

	public:
		[[nodiscard]] InheritorType<DimensionCount, false> detach() const
		{
			return InheritorType<DimensionCount, false>
			{
				this->first(),
				this->second(),
				this->third()
			};
		}

		
		friend void operator+=(
			tracking_wireframe<DimensionCount>& wireframe, const triangle_internal& triangle)
		{
			using wireframe_t = tracking_wireframe<DimensionCount>;
			using wireframe_tracked_vertex = typename wireframe_t::tracked_vertex;
			using wireframe_wire = typename wireframe_t::wire;
			
			wireframe_tracked_vertex first_vertex { triangle.first_.shared_inner() };
			wireframe_tracked_vertex second_vertex { triangle.second_.shared_inner() };
			wireframe_tracked_vertex third_vertex { triangle.third_.shared_inner() };

			const auto first_wire = std::make_shared<wireframe_wire>(first_vertex, second_vertex);
			const auto second_wire = std::make_shared<wireframe_wire>(second_vertex, third_vertex);
			const auto third_wire = std::make_shared<wireframe_wire>(third_vertex, first_vertex);

			first_vertex += first_wire, first_vertex += second_wire;
			second_vertex += second_wire, second_vertex += third_wire;
			third_vertex += third_wire, third_vertex += first_wire;

			wireframe += first_wire, wireframe += second_wire, wireframe += third_wire;
		}

		
		void normalize() const
		{
			irglab::normalize<DimensionCount>(*this->first_conditional());
			irglab::normalize<DimensionCount>(*this->second_conditional());
			irglab::normalize<DimensionCount>(*this->third_conditional());
		}

		void operator*=(const transformation<DimensionCount>& transformation) const
		{
			*this->first_conditional() = this->first() * transformation;
			*this->second_conditional() = this->second() * transformation;
			*this->third_conditional() = this->third() * transformation;
		}
	};

	template<size DimensionCount, typename InheritorType>
	using tracking_triangle_internal = triangle_internal<DimensionCount, true, InheritorType>;
}

template<irglab::size DimensionCount, typename InheritorType>
struct std::hash<irglab::tracking_triangle_internal<DimensionCount, InheritorType>>
{
private:
	using key = irglab::triangle_internal<DimensionCount, true, InheritorType>;

	using tracked_vertex = typename key::tracked_vertex;
	static inline const std::hash<tracked_vertex> tracked_vertex_hasher{};

public:
	size_t operator()(const key& key) const noexcept
	{
		return
			tracked_vertex_hasher(key.first_conditional()) ^ 
			tracked_vertex_hasher(key.second_conditional()) ^
			tracked_vertex_hasher(key.third_conditional());
	}
};


namespace irglab
{
	template<size DimensionCount, bool IsTracking>
	struct triangle :
		triangle_internal<DimensionCount, IsTracking, triangle<DimensionCount, IsTracking>>
	{
		triangle() = delete;
	};

	template<size DimensionCount>
	using tracking_triangle = triangle<DimensionCount, true>;

	template<size DimensionCount>
	using owning_triangle = triangle<DimensionCount, false>;

	
	template<bool IsTracking>
	struct triangle<2, IsTracking> final : triangle_internal<2, IsTracking, triangle<2, IsTracking>>
	{
	private:
		using base = triangle_internal<2, IsTracking, triangle<2, IsTracking>>;

	public:
		explicit triangle(
			typename base::conditional_vertex first,
			typename base::conditional_vertex second,
			typename base::conditional_vertex third) noexcept :

			base{ std::move(first), std::move(second), std::move(third) } { }


		[[nodiscard]] direction get_direction() const
		{
			return two_dimensional::get_direction(
				this->first(), this->second(), this->third());
		}

		void operator%=(const direction direction)
		{
			if (this->get_direction() != direction) ~*this;
		}

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
	};


	template<bool IsTracking>
	struct triangle<3, IsTracking> final : triangle_internal<3, IsTracking, triangle<3, IsTracking>>
	{
	private:
		using base = triangle_internal<3, IsTracking, triangle<3, IsTracking>>;

	public:
		explicit triangle(
			typename base::conditional_vertex first,
			typename base::conditional_vertex second,
			typename base::conditional_vertex third) noexcept :

			base{ std::move(first), std::move(second), std::move(third) } { }


		[[nodiscard]] three_dimensional::plane get_plane() const
		{
			return three_dimensional::get_common_plane(
				three_dimensional::to_cartesian_coordinates(this->first()), 
				three_dimensional::to_cartesian_coordinates(this->second()), 
				three_dimensional::to_cartesian_coordinates(this->third()));
		}

		[[nodiscard]] three_dimensional::plane_normal get_plane_normal() const
		{
			return three_dimensional::get_plane_normal(
				three_dimensional::to_cartesian_coordinates(this->first()),
				three_dimensional::to_cartesian_coordinates(this->second()),
				three_dimensional::to_cartesian_coordinates(this->third()));
		}

		[[nodiscard]] three_dimensional::cartesian_coordinates get_center() const
		{
			return three_dimensional::to_cartesian_coordinates(this->first()) +
				three_dimensional::to_cartesian_coordinates(this->second()) +
				three_dimensional::to_cartesian_coordinates(this->third()) / 3.0f;
		}
		
		[[nodiscard]] friend bool operator<(
			const three_dimensional::point& point, const triangle& triangle)
		{
			return three_dimensional::get_direction(point, triangle.get_plane());
		}
	};
}

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
