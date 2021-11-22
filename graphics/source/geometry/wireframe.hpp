#ifndef IRGLAB_WIREFRAME_HPP
#define IRGLAB_WIREFRAME_HPP


#include "external/external.hpp"


#include "primitive/primitive.hpp"

#include "vertex.hpp"
#include "wire.hpp"


namespace il
{
    // Type traits

    [[nodiscard, maybe_unused]] constexpr bool is_wireframe_description_supported(
            small_natural_number dimension_count, [[maybe_unused]] vertex_access_type access_type)
    {
        return are_primitive_operations_supported(dimension_count);
    }


    // Declaration

    template<small_natural_number DimensionCount, vertex_access_type AccessType>
    class [[maybe_unused]] wireframe;


    // Alias

    template<small_natural_number DimensionCount>
    using owning_wireframe [[maybe_unused]] = wireframe<DimensionCount, vertex_access_type::owned>;

    template<small_natural_number DimensionCount>
    using sharing_wireframe [[maybe_unused]] = wireframe<DimensionCount, vertex_access_type::shared>;

    template<small_natural_number DimensionCount>
    using tracking_wireframe [[maybe_unused]] = wireframe<DimensionCount, vertex_access_type::tracked>;


    // Dimensional aliases

    namespace d2
    {
        template<vertex_access_type AccessType>
        using wireframe [[maybe_unused]] = il::wireframe<dimension_count, AccessType>;

        using owning_wireframe [[maybe_unused]] = il::owning_wireframe<dimension_count>;
        using sharing_wireframe [[maybe_unused]] = il::sharing_wireframe<dimension_count>;
        using tracking_wireframe [[maybe_unused]] = il::tracking_wireframe<dimension_count>;
    }

    namespace d3
    {
        template<vertex_access_type AccessType>
        using wireframe [[maybe_unused]] = il::wireframe<dimension_count, AccessType>;

        using owning_wireframe [[maybe_unused]] = il::owning_wireframe<dimension_count>;
        using sharing_wireframe [[maybe_unused]] = il::sharing_wireframe<dimension_count>;
        using tracking_wireframe [[maybe_unused]] = il::tracking_wireframe<dimension_count>;
    }



    // Owning implementation

    template<small_natural_number DimensionCount>
    class [[maybe_unused]] wireframe<DimensionCount, vertex_access_type::owned>
    {
        // Traits and types

    public:
        [[maybe_unused]] static constexpr small_natural_number dimension_count = DimensionCount;
        [[maybe_unused]] static constexpr vertex_access_type access_type = vertex_access_type::owned;

        [[maybe_unused]] static constexpr bool is_tracking = false;
        [[maybe_unused]] static constexpr bool is_owning = true;
        [[maybe_unused]] static constexpr bool is_virtual = false;

        using wire [[maybe_unused]] = il::wire<dimension_count, access_type>;
        using vertex [[maybe_unused]] = typename wire::vertex;


        // Constructors and related methods

        ENABLE_IF_TEMPLATE(is_wireframe_description_supported(dimension_count, access_type))
        [[nodiscard, maybe_unused]] wireframe(const std::initializer_list<wire>& wires) : _wires{wires}
        { }

        template<
                typename BeginIterator, typename EndIterator, ENABLE_IF(
                        (are_wire_iterators<BeginIterator, EndIterator, dimension_count, is_tracking> &&
                         is_wireframe_description_supported(DimensionCount, access_type)))>
        [[nodiscard, maybe_unused]] explicit wireframe(const BeginIterator& begin, const EndIterator& end) :
                _wires{begin, end}
        { }

        template<
                typename Range, ENABLE_IF(
                        (is_wire_range<Range, dimension_count, access_type> &&
                         is_wireframe_description_supported(DimensionCount, access_type)))>
        [[nodiscard, maybe_unused]] explicit wireframe(const Range& wires) : _wires{wires.begin(), wires.end()}
        { }

        [[maybe_unused]] void prune()
        {
            _wires.erase(remove_duplicates(_wires.begin(), _wires.end()), _wires.end());
        }


        // Accessors

        [[nodiscard]] const std::vector<wire>& wires() const
        {
            return _wires;
        }


        // Non-modifiers

        [[nodiscard, maybe_unused]] friend bounds<dimension_count>& operator|(
                const bounds<dimension_count>& old_bounds, const wireframe& wireframe)
        {
            bounds<dimension_count> new_bounds{old_bounds};
            new_bounds |= wireframe;
            return new_bounds;
        }

        [[maybe_unused]] friend constexpr void operator|=(
                bounds<dimension_count>& bounds, const wireframe& wireframe) noexcept
        {
            for (const auto& wire : wireframe.wires_) bounds |= wire;
        }


#ifndef NDEBUG

        [[maybe_unused]] friend std::ostream& operator<<(std::ostream& output_stream, const wireframe& wireframe)
        {
            output_stream << "Wires:" << std::endl;

            for (const auto& wire : wireframe.wires_) output_stream << wire;

            return output_stream;
        }

#endif


        // Modifiers

        [[maybe_unused]] constexpr void normalize()
        {
            for (auto& wire : _wires) wire.normalize();
        }

        [[maybe_unused]] constexpr void operator*=(const transformation<dimension_count>& transformation) noexcept
        {
            for (auto& wire : _wires) wire *= transformation;
        }


        [[maybe_unused]] void operator+=(const wire& wire)
        {
            _wires.emplace_back(wire);
        }

        [[maybe_unused]] void operator+=(const wireframe& other)
        {
            this->_wires.insert(this->_wires.end(), other.wires_.begin(), other.wires_.end());
        }


        // Data

    private:
        std::vector<wire> _wires;
    };


    // Virtual implementation

    template<small_natural_number DimensionCount, vertex_access_type AccessType>
    class [[maybe_unused]] wireframe
    {
        // Traits and types

    public:
        [[maybe_unused]] static constexpr small_natural_number dimension_count = DimensionCount;
        [[maybe_unused]] static constexpr vertex_access_type access_type = AccessType;

        [[maybe_unused]] static constexpr bool is_tracking = access_type == vertex_access_type::tracked;
        [[maybe_unused]] static constexpr bool is_owning = false;
        [[maybe_unused]] static constexpr bool is_virtual = true;

        using wire [[maybe_unused]] = il::wire<dimension_count, access_type>;
        using vertex [[maybe_unused]] = typename wire::vertex;


        // Constructors and related methods

        ENABLE_IF_TEMPLATE(is_wireframe_description_supported(dimension_count, access_type))
        [[nodiscard, maybe_unused]] wireframe(
                const std::initializer_list<vertex>& vertices, const std::initializer_list<wire>& wires) :
                _vertices{vertices}, _wires{wires}
        { }

        template<
                typename VertexBeginIterator, typename VertexEndIterator,
                typename WireBeginIterator, typename WireEndIterator, ENABLE_IF(
                        (are_vertex_iterators<
                                VertexBeginIterator, VertexEndIterator, dimension_count, access_type> &&
                         are_wire_iterators<WireBeginIterator, WireEndIterator, dimension_count, access_type> &&
                         is_wireframe_description_supported(DimensionCount, access_type)))>
        [[nodiscard, maybe_unused]] explicit wireframe(
                const VertexBeginIterator& vertex_begin, const VertexEndIterator& vertex_end,
                const WireBeginIterator& wires_begin, const WireEndIterator& wires_end) :
                _vertices{vertex_begin, vertex_end}, _wires{wires_begin, wires_end}
        { }

        template<
                typename VertexRange, typename WireRange, typename, ENABLE_IF(
                        (is_vertex_range<VertexRange, dimension_count, access_type> &&
                         is_wire_range<WireRange, dimension_count, access_type> &&
                         is_wireframe_description_supported(DimensionCount, access_type)))>
        [[nodiscard, maybe_unused]] explicit wireframe(const VertexRange& vertices, const WireRange& wires) :
                _vertices{vertices.begin(), vertices.end()},
                _wires{wires.begin(), wires.end()}
        { }

        ENABLE_IF_TEMPLATE(is_tracking)
        [[maybe_unused]] void prune() const
        {
            for (const auto& vertex : _vertices) vertex.prune();
        }


        // Accessors

        [[nodiscard, maybe_unused]] const std::unordered_set<wire>& wires() const
        {
            return _wires;
        }

        [[nodiscard, maybe_unused]] const std::unordered_set<vertex>& vertices() const
        {
            return _vertices;
        }


        // Non-modifiers

        [[maybe_unused]] friend constexpr void operator|(
                bounds <dimension_count>& old_bounds, const wireframe& wireframe) noexcept
        {
            bounds < dimension_count > new_bounds{old_bounds};
            new_bounds |= wireframe;
            return new_bounds;
        }

        [[maybe_unused]] friend constexpr void operator|=(
                bounds <dimension_count>& bounds, const wireframe& wireframe) noexcept
        {
            for (const auto& vertex : wireframe.vertices_) bounds |= *vertex;
        }


#ifndef NDEBUG

        [[nodiscard, maybe_unused]] friend std::ostream& operator<<(
                std::ostream& output_stream, const wireframe& wireframe)
        {
            output_stream << "Wires:" << std::endl;

            for (const auto& wire : wireframe.wires_) output_stream << wire;

            return output_stream;
        }

#endif


        [[maybe_unused]] constexpr void normalize() const
        {
            for (const auto& vertex : _vertices) il::normalize<dimension_count>(*vertex);
        }

        [[maybe_unused]] constexpr void operator*=(
                const transformation <DimensionCount>& transformation) const noexcept
        {
            for (const auto& vertex : _vertices) *vertex = *vertex * transformation;
        }


        // Modifiers

        [[maybe_unused]] void operator+=(const wire& wire)
        {
            const auto wire_begin_iterator = _vertices.find(wire->begin_tracked());
            if (wire_begin_iterator != _vertices.end()) *wire_begin_iterator += wire->begin_tracked();
            else _vertices.insert(wire->begin_tracked());

            const auto wire_end_iterator = _vertices.find(wire->end_tracked());
            if (wire_end_iterator != _vertices.end()) *wire_end_iterator += wire->end_tracked();
            else _vertices.insert(wire->end_tracked());

            _wires.insert(wire);
        }

        [[maybe_unused]] void operator+=(const wireframe& other)
        {
            this->_wires.insert(other.wires_.begin(), other.wires_.end());
            this->_vertices.insert(other.vertices_.begin(), other.vertices_.end());
        }


        // Data

    private:
        std::unordered_set<wire> _wires;
        std::unordered_set<vertex> _vertices;
    };
}

#endif
