#ifndef IRGLAB_VERTEX_HPP
#define IRGLAB_VERTEX_HPP


#include "external/external.hpp"

#include "primitive/primitive.hpp"


// TODO: Integrate with other geometry types

namespace il
{
    // Type traits

    enum class [[maybe_unused]] vertex_access_type : small_natural_number
    {
        owned [[maybe_unused]],
        shared [[maybe_unused]],
        tracked [[maybe_unused]]
    };

    template<small_natural_number DimensionCount, vertex_access_type AccessType, typename TrackerType = void>
    [[maybe_unused]] constexpr bool is_vertex_description_supported =
            ((AccessType != vertex_access_type::tracked) ||
             (AccessType == vertex_access_type::tracked && !std::is_same_v<TrackerType, void>)) &&
            are_primitive_operations_supported(DimensionCount);


    // Declaration

    template<
            small_natural_number DimensionCount, vertex_access_type AccessType, typename TrackerType = void,
            ENABLE_IF((is_vertex_description_supported<DimensionCount, AccessType, TrackerType>))>
    using vertex [[maybe_unused]] =
    std::conditional_t<
            AccessType == vertex_access_type::tracked, tracked_pointer < point < DimensionCount>, TrackerType>,
    std::conditional_t<
            AccessType == vertex_access_type::shared, std::shared_ptr<point < DimensionCount>>,
    point <DimensionCount>>>;


    // Iterator and range traits

    template<
            typename BeginIterator, typename EndIterator,
            small_natural_number DimensionCount, vertex_access_type AccessType, typename TrackerType = void>
    [[maybe_unused]] constexpr bool are_vertex_iterators =
            is_iterator < BeginIterator > && is_iterator < EndIterator > &&
            have_value_type<BeginIterator, EndIterator, vertex<DimensionCount, AccessType, TrackerType>>;

    template<
            typename Range,
            small_natural_number DimensionCount, vertex_access_type AccessType, typename TrackerType = void>
    [[maybe_unused]] constexpr bool is_vertex_range =
            is_range < Range > &&
            std::is_same_v<range_value_type < Range>, vertex<DimensionCount, AccessType, TrackerType>>

    ;


    // Aliases

    template<small_natural_number DimensionCount>
    using owned_vertex [[maybe_unused]] = vertex<DimensionCount, vertex_access_type::owned>;

    template<small_natural_number DimensionCount>
    using shared_vertex [[maybe_unused]] = vertex<DimensionCount, vertex_access_type::shared>;

    template<small_natural_number DimensionCount, typename TrackerType>
    using tracked_vertex [[maybe_unused]] = vertex<DimensionCount, vertex_access_type::tracked, TrackerType>;


    template<small_natural_number DimensionCount, typename TrackerType>
    using virtual_vertex [[maybe_unused]] = std::variant<
            shared_vertex<DimensionCount>, tracked_vertex<DimensionCount, TrackerType>>;
}


// Dimensional aliases

namespace il::d2
{
    template<vertex_access_type AccessType, typename TrackerType = void>
    using vertex [[maybe_unused]] = il::vertex<dimension_count, AccessType, TrackerType>;


    using owned_vertex [[maybe_unused]] = il::shared_vertex<dimension_count>;

    using shared_vertex [[maybe_unused]] = il::shared_vertex<dimension_count>;

    template<typename TrackerType>
    using tracked_vertex [[maybe_unused]] = il::tracked_vertex<dimension_count, TrackerType>;


    template<typename TrackerType>
    using virtual_vertex [[maybe_unused]] = il::virtual_vertex<dimension_count, TrackerType>;
}

namespace il::d3
{
    template<vertex_access_type AccessType, typename TrackerType = void>
    using vertex [[maybe_unused]] = il::vertex<dimension_count, AccessType, TrackerType>;


    using owned_vertex [[maybe_unused]] = il::shared_vertex<dimension_count>;

    using shared_vertex [[maybe_unused]] = il::shared_vertex<dimension_count>;

    template<typename TrackerType>
    using tracked_vertex [[maybe_unused]] = il::tracked_vertex<dimension_count, TrackerType>;


    template<typename TrackerType>
    using virtual_vertex [[maybe_unused]] = il::virtual_vertex<dimension_count, TrackerType>;
}

#endif //IRGLAB_VERTEX_HPP
