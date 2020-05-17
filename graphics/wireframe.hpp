#ifndef IRGLAB_WIREFRAME_HPP
#define IRGLAB_WIREFRAME_HPP


#include "pch.hpp"

#include "primitives.hpp"
#include "bounds.hpp"
#include "transformations.hpp"

#include "wire.hpp"


namespace irglab
{
	template<size DimensionCount, bool IsTracking>
	struct wireframe;

	
	template<size DimensionCount>
	struct wireframe<DimensionCount, false>
	{
		using wire = irglab::owning_wire<DimensionCount>;
		using vertex = typename wire::point;

	private:
		std::vector<wire> wires_;

	public:
		void prune()
		{
			wires_.erase(remove_duplicates(wires_.begin(), wires_.end()), wires_.end());
		}

		
		[[nodiscard]] const std::vector<wire>& wires() const
		{
			return wires_;
		}
		
		[[nodiscard]] std::vector<vertex> vertices() const
		{
			std::vector<typename wire::point> points{ };

			for (const auto& wire : wires_)
				points.emplace_back(wire.begin()),
				points.emplace_back(wire.end());

			return points;
		}


		friend constexpr  void operator|=(
			bounds<DimensionCount>& bounds, const wireframe& wireframe) noexcept
		{
			for (const auto& wire : wireframe.wires_) bounds |= wire;
		}


		friend std::ostream& operator<<(std::ostream& output_stream, const wireframe& wireframe)
		{
			output_stream << "Wires:" << std::endl;

			for (const auto& wire : wireframe.wires_) output_stream << wire;

			return output_stream;
		}


		constexpr void normalize()
		{
			for (auto& wire : wires_) wire.normalize();
		}

		constexpr void operator*=(const transformation<DimensionCount>& transformation) noexcept
		{
			for (auto& wire : wires_) wire *= transformation;
		}

		
		void operator+=(const wire& wire)
		{
			wires_.emplace_back(wire);
		}

		void operator+=(const wireframe& other)
		{
			this->wires_.insert(this->wires_.end(), other.wires_.begin(), other.wires_.end());
		}
	};

	template<size DimensionCount>
	using owning_wireframe = wireframe<DimensionCount, false>;

	
	template<size DimensionCount>
	struct wireframe<DimensionCount, true>
	{
		using wire = tracking_wire<DimensionCount>;
		using shared_wire = std::shared_ptr<wire>;
		using vertex = typename wire::point;
		using tracked_vertex = typename wire::tracked_point;

	private:
		std::unordered_set<shared_wire> wires_{};
		std::unordered_set<tracked_vertex> vertices_{};
		
	public:
		void prune() const
		{
			for (const auto& vertex : vertices_) vertex.prune();
		}
		
		[[nodiscard]] const std::unordered_set<shared_wire>& wires() const
		{
			return wires_;
		}

		[[nodiscard]] const std::unordered_set<tracked_vertex>& vertices() const
		{
			return vertices_;
		}

		
		friend constexpr  void operator|=(
			bounds<DimensionCount>& bounds, const wireframe& wireframe) noexcept
		{
			for (const auto& vertex : wireframe.vertices_) bounds |= *vertex;
		}


		friend std::ostream& operator<<(std::ostream& output_stream, const wireframe& wireframe)
		{
			output_stream << "Wires:" << std::endl;

			for (const auto& wire : wireframe.wires_) output_stream << wire;

			return output_stream;
		}


		constexpr void normalize() const
		{
			for (auto& wire : wires_) wire->normalize();
		}

		constexpr void operator*=(const transformation<DimensionCount>& transformation) const noexcept
		{
			for (auto& vertex : vertices_) *vertex *= transformation;
		}


		void operator+=(const shared_wire& wire)
		{
			const auto wire_begin_iterator = vertices_.find(wire->begin_tracked());
			const auto wire_end_iterator = vertices_.find(wire->end_tracked());

			if (wire_begin_iterator != vertices_.end())
			{
				if (wire_end_iterator == vertices_.end())
				{
					*wire_begin_iterator += wire->begin_tracked();
					vertices_.insert(wire->end_tracked());
				}
			}
			else if (wire_end_iterator != vertices_.end())
			{
				*wire_end_iterator += wire->begin_tracked();
				vertices_.insert(wire->end_tracked());
			}
			else
			{
				vertices_.insert(wire->begin_tracked());
				vertices_.insert(wire->end_tracked());
			}
			
			wires_.insert(wire);
		}

		void operator+=(const wireframe& other)
		{
			this->wires_.insert(other.wires_.begin(), other.wires_.end());
			this->vertices_.insert(other.vertices_.begin(), other.vertices_.end());
		}
	};

	template<size DimensionCount>
	using tracking_wireframe = wireframe<DimensionCount, true>;
}

namespace irglab::two_dimensional
{
	template<bool IsTracking>
	using wireframe = irglab::wireframe<dimension_count, IsTracking>;
	
	using tracking_wireframe = irglab::tracking_wireframe<dimension_count>;
	using owning_wireframe = irglab::owning_wireframe<dimension_count>;
}

namespace irglab::three_dimensional
{
	template<bool IsTracking>
	using wireframe = irglab::wireframe<dimension_count, IsTracking>;

	using tracking_wireframe = irglab::tracking_wireframe<dimension_count>;
	using owning_wireframe = irglab::owning_wireframe<dimension_count>;
}


#endif
