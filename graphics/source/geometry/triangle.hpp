#ifndef IRGLAB_TRIANGLE_HPP
#define IRGLAB_TRIANGLE_HPP


#include "external/external.hpp"


#include "primitive/primitive.hpp"

#include "vertex.hpp"

#include "wire.hpp"
#include "wireframe.hpp"


namespace il
{
    // Type traits

    [[nodiscard, maybe_unused]] constexpr bool is_triangle_description_supported(
            small_natural_number dimension_count, [[maybe_unused]] vertex_access_type access_type)
    {
        return are_primitive_operations_supported(dimension_count);
    }


    // Declaration

    template<small_natural_number DimensionCount, vertex_access_type AccessType>
    class [[maybe_unused]] triangle;


    // Iterator and range traits

    template<
            typename BeginIterator, typename EndIterator,
            small_natural_number DimensionCount, vertex_access_type AccessType>
    [[maybe_unused]] constexpr bool are_triangle_iterators =
            is_iterator < BeginIterator > && is_iterator < EndIterator > &&
            have_value_type<BeginIterator, EndIterator, triangle<DimensionCount, AccessType>>;

    template<
            typename Range,
            small_natural_number DimensionCount, vertex_access_type AccessType>
    [[maybe_unused]] constexpr bool is_triangle_range =
            is_range < Range > &&
            std::is_same_v<range_value_type < Range>, triangle<DimensionCount, AccessType>>

    ;


    // Aliases

    template<small_natural_number DimensionCount>
    using owning_triangle [[maybe_unused]] = triangle<DimensionCount, vertex_access_type::owned>;

    template<small_natural_number DimensionCount>
    using sharing_triangle [[maybe_unused]] = triangle<DimensionCount, vertex_access_type::shared>;

    template<small_natural_number DimensionCount>
    using tracking_triangle [[maybe_unused]] = triangle<DimensionCount, vertex_access_type::tracked>;


    template<small_natural_number DimensionCount>
    using virtual_triangle [[maybe_unused]] = std::variant<
            sharing_triangle<DimensionCount>, tracking_triangle<DimensionCount>>;


    // Dimensional aliases

    namespace d2
    {
        template<vertex_access_type AccessType>
        using triangle [[maybe_unused]] = il::triangle<dimension_count, AccessType>;

        using owning_triangle [[maybe_unused]] = il::owning_triangle<dimension_count>;
        using sharing_triangle [[maybe_unused]] = il::sharing_triangle<dimension_count>;
        using tracking_triangle [[maybe_unused]] = il::tracking_triangle<dimension_count>;

        using virtual_triangle [[maybe_unused]] = il::virtual_triangle<dimension_count>;
    }


    namespace d3
    {
        template<vertex_access_type AccessType>
        using triangle [[maybe_unused]] = il::triangle<dimension_count, AccessType>;

        using owning_triangle [[maybe_unused]] = il::owning_triangle<dimension_count>;
        using sharing_triangle [[maybe_unused]] = il::sharing_triangle<dimension_count>;
        using tracking_triangle [[maybe_unused]] = il::tracking_triangle<dimension_count>;

        using virtual_triangle [[maybe_unused]] = il::virtual_triangle<dimension_count>;
    }



    // Implementation

    template<small_natural_number DimensionCount, vertex_access_type AccessType>
    class [[maybe_unused]] triangle
    {
        // Traits and types

    public:
        [[maybe_unused]] static constexpr small_natural_number dimension_count = DimensionCount;
        [[maybe_unused]] static constexpr vertex_access_type access_type = AccessType;

        [[maybe_unused]] static constexpr bool is_tracking = access_type == vertex_access_type::tracked;
        [[maybe_unused]] static constexpr bool is_owning = access_type == vertex_access_type::owned;
        [[maybe_unused]] static constexpr bool is_virtual = !is_owning;

        [[maybe_unused]] static constexpr small_natural_number vertex_count = 3;

        using owned_vertex [[maybe_unused]] = il::owned_vertex<dimension_count>;
        using shared_vertex [[maybe_unused]] = il::shared_vertex<dimension_count>;
        using tracked_vertex [[maybe_unused]] = il::tracked_vertex<dimension_count, triangle>;

        using virtual_vertex [[maybe_unused]] = il::virtual_vertex<dimension_count, triangle>;

        using vertex [[maybe_unused]] = il::vertex<dimension_count, access_type, triangle>;


        // Constructors and related methods

        ENABLE_IF_TEMPLATE(is_triangle_description_supported(dimension_count, access_type))
        [[nodiscard, maybe_unused]] explicit triangle(vertex first, vertex second, vertex third) noexcept:
                _first{std::move(first)}, _second{std::move(second)}, _third{std::move(third)}
        { }


        ENABLE_IF_TEMPLATE(is_tracking)
        [[maybe_unused]] void prune() const
        {
            first_virtual().prune();
            second_virtual().prune();
            third_tracked().prune();
        }


        // Immutable accessors

        [[nodiscard, maybe_unused]] const owned_vertex& first() const
        {
            if constexpr (access_type == vertex_access_type::owned) return _first;
            else return *_first;
        }

        [[nodiscard, maybe_unused]] const owned_vertex& second() const
        {
            if constexpr (access_type == vertex_access_type::owned) return _second;
            else return *_second;
        }

        [[nodiscard, maybe_unused]] const owned_vertex& third() const
        {
            if constexpr (access_type == vertex_access_type::owned) return _third;
            else return *_third;
        }


        ENABLE_IF_TEMPLATE(is_virtual)
        [[nodiscard, maybe_unused]] const vertex& first_virtual() const
        {
            return _first;
        }

        ENABLE_IF_TEMPLATE(is_virtual)
        [[nodiscard, maybe_unused]] const vertex& second_virtual() const
        {
            return _first;
        }

        ENABLE_IF_TEMPLATE(is_virtual)
        [[nodiscard, maybe_unused]] const vertex& third_tracked() const
        {
            return _first;
        }


        // Non-modifiers

        [[nodiscard, maybe_unused]] constexpr bool operator==(triangle& other) const
        {
            return
                    _first == other._first &&
                    _second == other._second &&
                    _third == other._third;
        }


        using barycentric_coordinates [[maybe_unused]] = vector<dimension_count>;

        [[nodiscard, maybe_unused]] barycentric_coordinates get_barycentric_coordinates(
                const point <dimension_count>& point) const
        {
            return point * glm::inverse(
                    matrix<dimension_count + small_one, vertex_count>
                            {
                                    first(), second(), third()
                            });
        }


        [[nodiscard, maybe_unused]] friend bounds <dimension_count> operator|(
                bounds <dimension_count>& bounds, const triangle& triangle)
        {
            return bounds | triangle.first() | triangle.second() | triangle.third();
        }

        [[maybe_unused]] friend constexpr void operator|=(
                bounds <dimension_count>& bounds, const triangle& triangle)
        {
            bounds |= triangle.first(), bounds |= triangle.second(), bounds |= triangle.third();
        }


#ifndef NDEBUG

        [[nodiscard, maybe_unused]] friend std::ostream& operator<<(
                std::ostream& output_stream, const triangle& triangle)
        {
            return output_stream <<
                                 "Vertices:" << std::endl <<
                                 glm::to_string(triangle.first()) << std::endl <<
                                 glm::to_string(triangle.second()) << std::endl <<
                                 glm::to_string(triangle.third()) << std::endl <<
                                 std::endl;
        }

#endif


        [[nodiscard, maybe_unused]] triangle operator*(
                const transformation <dimension_count>& transformation) const noexcept
        {
            triangle new_triangle{*this};

            new_triangle._first_mutable() = this->first() * transformation;
            new_triangle._second_mutable() = this->second() * transformation;
            new_triangle._third_mutable() = this->third() * transformation;

            return new_triangle;
        }


        // It would be great if this madness was shorter.
        [[maybe_unused]] friend void operator+=(
                wireframe <dimension_count, access_type>& wireframe_to_expand, const triangle& triangle_to_add)
        {
            if constexpr (is_tracking)
            {
                using wireframe_t = tracking_wireframe<dimension_count>;
                using wireframe_tracked_vertex = typename wireframe_t::tracked_vertex;
                using wireframe_wire = typename wireframe_t::wire;

                auto first_vertex = static_cast<wireframe_tracked_vertex>(triangle_to_add._first);
                auto second_vertex = static_cast<wireframe_tracked_vertex>(triangle_to_add._second);
                auto third_vertex = static_cast<wireframe_tracked_vertex>(triangle_to_add._third);

                const auto first_wire = std::make_shared<wireframe_wire>(first_vertex, second_vertex);
                const auto second_wire = std::make_shared<wireframe_wire>(second_vertex, third_vertex);
                const auto third_wire = std::make_shared<wireframe_wire>(third_vertex, first_vertex);

                first_vertex += first_wire;
                first_vertex += second_wire;

                second_vertex += second_wire;
                second_vertex += third_wire;

                third_vertex += third_wire;
                third_vertex += first_wire;

                wireframe_to_expand += first_wire;
                wireframe_to_expand += second_wire;
                wireframe_to_expand += third_wire;
            } else
            {
                using wire = typename wireframe<dimension_count, access_type>::wire;
                wireframe_to_expand += wire{triangle_to_add.first(), triangle_to_add.second()};
                wireframe_to_expand += wire{triangle_to_add.second(), triangle_to_add.third()};
                wireframe_to_expand += wire{triangle_to_add.third(), triangle_to_add.first()};
            }
        }


        // Modifiers

        [[maybe_unused]] void operator~()
        {
            std::swap(_first, _third);
        }



        // Owning

        // Modifiers

        ENABLE_IF_TEMPLATE(is_owning)
        [[maybe_unused]] constexpr void normalize()
        {
            il::normalize<dimension_count>(this->_first_mutable());
            il::normalize<dimension_count>(this->_second_mutable());
            il::normalize<dimension_count>(this->_third_mutable());
        }

        ENABLE_IF_TEMPLATE(is_owning)
        [[maybe_unused]] constexpr void operator*=(
                const transformation <dimension_count>& transformation) noexcept
        {
            this->_first_mutable() = this->first() * transformation;
            this->_second_mutable() = this->second() * transformation;
            this->_third_mutable() = this->third() * transformation;
        }



        // Virtual

        // Non-modifiers

        ENABLE_IF_TEMPLATE(is_virtual)
        [[nodiscard, maybe_unused]] triangle<dimension_count, vertex_access_type::owned> get_detached() const
        {
            return triangle<dimension_count, vertex_access_type::owned>
                    {
                            this->first(),
                            this->second(),
                            this->third()
                    };
        }

        ENABLE_IF_TEMPLATE(is_virtual)
        [[maybe_unused]] void normalize() const
        {
            il::normalize<dimension_count>(this->_first_mutable());
            il::normalize<dimension_count>(this->_second_mutable());
            il::normalize<dimension_count>(this->_third_mutable());
        }

        ENABLE_IF_TEMPLATE(is_virtual)
        [[maybe_unused]] void operator*=(const transformation <dimension_count>& transformation) const
        {
            this->_first_mutable() = this->first() * transformation;
            this->_second_mutable() = this->second() * transformation;
            this->_third_mutable() = this->third() * transformation;
        }



        // 2D

        // Non-modifiers

        ENABLE_IF_TEMPLATE(dimension_count == d2::dimension_count)
        [[nodiscard]] direction_inner get_direction() const
        {
            return d2::get_direction(this->first(), this->second(), this->third());
        }

        ENABLE_IF_TEMPLATE(dimension_count == d2::dimension_count)
        [[nodiscard, maybe_unused]] friend bool operator<(
                const d2::point& point, const triangle& triangle)
        {
            const auto triangle_direction = triangle.get_direction();

            return
                    d2::get_direction(triangle.first(), triangle.second(), point) == triangle_direction &&
                    d2::get_direction(triangle.second(), triangle.third(), point) == triangle_direction &&
                    d2::get_direction(triangle.third(), triangle.first(), point) == triangle_direction;
        }


        // Modifiers

        ENABLE_IF_TEMPLATE(dimension_count == d2::dimension_count)
        [[maybe_unused]]  void operator%=(const direction_inner direction)
        {
            if (this->get_direction() != direction) ~*this;
        }



        // 3D

        // Non-modifiers

        ENABLE_IF_TEMPLATE(dimension_count == d3::dimension_count)
        [[nodiscard, maybe_unused]] d3::plane get_plane() const
        {
            return d3::get_common_plane(
                    d3::to_cartesian_coordinates(this->first()),
                    d3::to_cartesian_coordinates(this->second()),
                    d3::to_cartesian_coordinates(this->third()));
        }

        ENABLE_IF_TEMPLATE(dimension_count == d3::dimension_count)
        [[nodiscard, maybe_unused]] d3::plane_normal get_plane_normal() const
        {
            return d3::get_plane_normal(
                    d3::to_cartesian_coordinates(this->first()),
                    d3::to_cartesian_coordinates(this->second()),
                    d3::to_cartesian_coordinates(this->third()));
        }

        ENABLE_IF_TEMPLATE(dimension_count == d3::dimension_count)
        [[nodiscard, maybe_unused]] d3::cartesian_coordinates get_center() const
        {
            return
                    d3::to_cartesian_coordinates(this->first()) +
                    d3::to_cartesian_coordinates(this->second()) +
                    d3::to_cartesian_coordinates(this->third()) / 3.0f;
        }

        ENABLE_IF_TEMPLATE(dimension_count == d3::dimension_count)
        [[nodiscard, maybe_unused]] friend bool operator<(
                const d3::point& point, const triangle& triangle)
        {
            return d3::get_direction(triangle.get_plane(), point);
        }



        // Mutable accessors

    private:
        // Owning

        ENABLE_IF_TEMPLATE(is_owning)
        [[nodiscard, maybe_unused]] owned_vertex& _first_mutable()
        { return _first; }

        ENABLE_IF_TEMPLATE(is_owning)
        [[nodiscard, maybe_unused]] owned_vertex& _second_mutable()
        { return _second; }

        ENABLE_IF_TEMPLATE(is_owning)
        [[nodiscard, maybe_unused]] owned_vertex& _third_mutable()
        { return _third; }


        // Virtual

        ENABLE_IF_TEMPLATE(is_virtual)
        [[nodiscard, maybe_unused]] owned_vertex& _first_mutable() const
        { return *_first; }

        ENABLE_IF_TEMPLATE(is_virtual)
        [[nodiscard, maybe_unused]] owned_vertex& _second_mutable() const
        { return *_second; }

        ENABLE_IF_TEMPLATE(is_owning)
        [[nodiscard, maybe_unused]] owned_vertex& _third_mutable() const
        { return *_third; }


        // Data

        vertex _first, _second, _third;
    };
}



// Hash

template<il::small_natural_number DimensionCount, il::vertex_access_type AccessType>
struct [[maybe_unused]] std::hash<il::triangle<DimensionCount, AccessType>>
{
    using key [[maybe_unused]] = il::tracking_triangle<DimensionCount>;

    ENABLE_IF_TEMPLATE(AccessType != il::vertex_access_type::owned)
    hash()
    { };

    ENABLE_IF_TEMPLATE(AccessType == il::vertex_access_type::owned)
    hash() = delete;

private:
    static inline const std::hash<typename key::vertex> _tracked_vertex_hasher{ };

public:
    [[nodiscard, maybe_unused]] size_t operator()(const key& key) const noexcept
    {
        return
                _tracked_vertex_hasher(key.first_virtual()) ^
                _tracked_vertex_hasher(key.second_virtual()) ^
                _tracked_vertex_hasher(key.third_tracked());
    }
};


#endif
