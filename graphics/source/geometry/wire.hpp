#ifndef IRGLAB_WIRE_HPP
#define IRGLAB_WIRE_HPP


#include "external/external.hpp"


#include "primitive/primitive.hpp"

#include "vertex.hpp"


namespace il
{
    // Type traits

    [[nodiscard, maybe_unused]] constexpr bool is_wire_description_supported(
            small_natural_number dimension_count, [[maybe_unused]] vertex_access_type vertex_access)
    {
        return are_primitive_operations_supported(dimension_count);
    }


    // Declaration

    template<small_natural_number DimensionCount, vertex_access_type AccessType>
    class [[maybe_unused]] wire;


    // Iterator and range traits

    template<
            typename BeginIterator, typename EndIterator,
            small_natural_number DimensionCount, vertex_access_type AccessType>
    [[maybe_unused]] constexpr bool are_wire_iterators =
            is_iterator<BeginIterator> && is_iterator<EndIterator> &&
            have_value_type<BeginIterator, EndIterator, wire<DimensionCount, AccessType>>;

    template<
            typename Range,
            small_natural_number DimensionCount, vertex_access_type AccessType>
    [[maybe_unused]] constexpr bool is_wire_range =
            is_range<Range> &&
            std::is_same_v<range_value_type<Range>, wire<DimensionCount, AccessType>>;


    // Aliases

    template<small_natural_number DimensionCount>
    using owning_wire [[maybe_unused]] = wire<DimensionCount, vertex_access_type::owned>;

    template<small_natural_number DimensionCount>
    using sharing_wire [[maybe_unused]] = wire<DimensionCount, vertex_access_type::shared>;

    template<small_natural_number DimensionCount>
    using tracking_wire [[maybe_unused]] = wire<DimensionCount, vertex_access_type::tracked>;


    template<small_natural_number DimensionCount>
    using virtual_wire [[maybe_unused]] = std::variant<
            sharing_wire<DimensionCount>, tracking_wire<DimensionCount>>;


    // Dimensional aliases

    namespace d2
    {
        template<vertex_access_type AccessType>
        using wire [[maybe_unused]] = il::wire<dimension_count, AccessType>;

        using owning_wire [[maybe_unused]] = il::owning_wire<dimension_count>;
        using sharing_wire [[maybe_unused]] = il::sharing_wire<dimension_count>;
        using tracking_wire [[maybe_unused]] = il::tracking_wire<dimension_count>;

        using virtual_wire [[maybe_unused]] = il::virtual_wire<dimension_count>;
    }

    namespace d3
    {
        template<vertex_access_type AccessType>
        using wire [[maybe_unused]] = il::wire<dimension_count, AccessType>;

        using owning_wire [[maybe_unused]] = il::owning_wire<dimension_count>;
        using sharing_wire [[maybe_unused]] = il::sharing_wire<dimension_count>;
        using tracking_wire [[maybe_unused]] = il::tracking_wire<dimension_count>;

        using virtual_wire [[maybe_unused]] = il::virtual_wire<dimension_count>;
    }



    // Implementation

    template<small_natural_number DimensionCount, vertex_access_type AccessType>
    class [[maybe_unused]] wire
    {
        // Traits and types

    public:
        [[maybe_unused]] static constexpr small_natural_number dimension_count = DimensionCount;


        [[maybe_unused]] static constexpr vertex_access_type access_type = AccessType;

        [[maybe_unused]] static constexpr bool is_tracking = access_type == vertex_access_type::tracked;
        [[maybe_unused]] static constexpr bool is_owning = access_type == vertex_access_type::owned;
        [[maybe_unused]] static constexpr bool is_virtual = !is_owning;


        [[maybe_unused]] static constexpr small_natural_number vertex_count = 2;


        using owned_vertex [[maybe_unused]] = il::owned_vertex<dimension_count>;
        using shared_vertex [[maybe_unused]] = il::shared_vertex<dimension_count>;
        using tracked_vertex [[maybe_unused]] = il::tracked_vertex<dimension_count, wire>;

        using virtual_vertex [[maybe_unused]] = il::virtual_vertex<dimension_count, wire>;

        using vertex [[maybe_unused]] = il::vertex<dimension_count, access_type, wire>;



        // Constructors and related methods

        ENABLE_IF_TEMPLATE(is_wire_description_supported(dimension_count, access_type))
        [[nodiscard, maybe_unused]] explicit wire(vertex begin, vertex end) :
                _begin{begin}, _end{end}
        { }


        ENABLE_IF_TEMPLATE(is_tracking)
        [[maybe_unused]] void prune() const
        {
            begin_virtual().prune();
            end_virtual().prune();
        }


        // Immutable accessors

    public:
        [[nodiscard, maybe_unused]] const owned_vertex& begin() const
        {
            if constexpr (access_type == vertex_access_type::owned) return _begin;
            else return *_begin;
        }

        [[nodiscard, maybe_unused]] const owned_vertex& end() const
        {
            if constexpr (access_type == vertex_access_type::owned) return _end;
            else return *_end;
        }


        ENABLE_IF_TEMPLATE(is_virtual)
        [[nodiscard, maybe_unused]] const vertex& begin_virtual() const
        { return _begin; }

        ENABLE_IF_TEMPLATE(is_virtual)
        [[nodiscard, maybe_unused]] const vertex& end_virtual() const
        { return _end; }


        // Non-modifiers

        [[nodiscard, maybe_unused]] constexpr bool operator==(const wire& other) const
        {
            return
                    (this->_begin == other._begin && this->_end == other._end) ||
                    (this->_begin == other._end && this->_end == other._begin);
        }


        [[nodiscard, maybe_unused]] friend bounds<DimensionCount>& operator|(
                const bounds<DimensionCount>& old_bounds, const wire& wire)
        {
            return old_bounds | wire.begin_owned() | wire.end_owned();
        }

        [[maybe_unused]] friend constexpr void operator|=(
                bounds<DimensionCount>& bounds, const wire& wire)
        {
            bounds |= wire.begin_owned(), bounds |= wire.end_owned();
        }


#ifndef NDEBUG

        [[nodiscard, maybe_unused]] friend std::ostream& operator<<(
                std::ostream& output_stream, const wire& wire)
        {
            return output_stream <<
                                 glm::to_string(wire.begin_owned()) << ", " <<
                                 glm::to_string(wire.end_owned()) << std::endl;
        }

#endif


        [[nodiscard, maybe_unused]] wire operator*(
                const transformation<dimension_count>& transformation) const noexcept
        {
            wire new_wire{*this};

            new_wire._begin_mutable() = this->begin_owned() * transformation;
            new_wire._end_mutable() = this->end_owned() * transformation;

            return new_wire;
        }


        // Modifiers

        // Not using std::swap because it is not constexpr and noexcept.
        [[maybe_unused]] constexpr void operator~() noexcept
        {
            const auto temp = this->_begin;
            this->_begin = this->_end;
            this->_end = temp;
        }



        // Owning

        // Modifiers

        ENABLE_IF_TEMPLATE(is_owning)
        [[maybe_unused]] constexpr void normalize()
        {
            il::normalize<dimension_count>(this->_begin_mutable());
            il::normalize<dimension_count>(this->_end_mutable());
        }

        ENABLE_IF_TEMPLATE(is_owning)
        [[maybe_unused]] constexpr void operator*=(
                const transformation <DimensionCount>& transformation) noexcept
        {
            this->_begin_mutable() = this->begin_owned() * transformation;
            this->_end_mutable() = this->end_owned() * transformation;
        }



        // Virtual

        // Non-modifiers

        ENABLE_IF_TEMPLATE(is_virtual)
        [[nodiscard, maybe_unused]] wire<dimension_count, vertex_access_type::owned> get_detached() const
        {
            return wire<dimension_count, vertex_access_type::owned>
                    {
                            this->begin_owned(),
                            this->end_owned()
                    };
        }

        ENABLE_IF_TEMPLATE(is_virtual)
        [[maybe_unused]] constexpr void normalize() const
        {
            il::normalize<dimension_count>(this->_begin_mutable());
            il::normalize<dimension_count>(this->_end_mutable());
        }

        ENABLE_IF_TEMPLATE(is_virtual)
        [[maybe_unused]] constexpr void operator*=(
                const transformation <dimension_count>& transformation) const noexcept
        {
            this->_begin_mutable() = this->begin_owned() * transformation;
            this->_end_mutable() = this->end_owned() * transformation;
        }



        // Mutable accessors

    private:
        // Owning

        ENABLE_IF_TEMPLATE(is_owning)
        [[nodiscard, maybe_unused]] owned_vertex& _begin_mutable()
        { return _begin; }

        ENABLE_IF_TEMPLATE(is_owning)
        [[nodiscard, maybe_unused]] owned_vertex& _end_mutable()
        { return _end; }


        // Virtual

        ENABLE_IF_TEMPLATE(is_virtual)
        [[nodiscard, maybe_unused]] owned_vertex& _begin_mutable() const
        { return *_begin; }

        ENABLE_IF_TEMPLATE(is_virtual)
        [[nodiscard, maybe_unused]] owned_vertex& _end_mutable() const
        { return *_end; }



        // Data

        vertex _begin, _end;
    };
}



// Hash

template<il::small_natural_number DimensionCount, il::vertex_access_type AccessType>
struct [[maybe_unused]] std::hash<il::wire<DimensionCount, AccessType>>
{
    using key [[maybe_unused]] = il::wire<DimensionCount, AccessType>;

    ENABLE_IF_TEMPLATE(AccessType != il::vertex_access_type::owned)
    hash()
    { };

    ENABLE_IF_TEMPLATE(AccessType == il::vertex_access_type::owned)
    hash() = delete;

private:
    static inline const std::hash<typename key::vertex> _vertex_hasher{ };

public:
    [[nodiscard, maybe_unused]] size_t operator()(const key& to_hash) const noexcept
    {
        return
                _vertex_hasher(to_hash.begin_virtual()) ^
                _vertex_hasher(to_hash.end_virtual());
    }
};


#endif
