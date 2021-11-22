#ifndef IRGLAB_DIRECTION_HPP
#define IRGLAB_DIRECTION_HPP


#include "external/external.hpp"

#include "primitives.hpp"



// Constexpr definitions

namespace il
{
    using direction_inner = bool;


    [[maybe_unused]] constexpr direction_inner counterclockwise = true;

    [[maybe_unused]] constexpr direction_inner clockwise = false;


    [[maybe_unused]] constexpr direction_inner above = true;

    [[maybe_unused]] constexpr direction_inner below = false;


    [[maybe_unused]] constexpr direction_inner right = true;

    [[maybe_unused]] constexpr direction_inner left = false;


    [[maybe_unused]] constexpr direction_inner bottom = true;

    [[maybe_unused]] constexpr direction_inner top = false;


    [[maybe_unused]] constexpr direction_inner direction_far = true;

    [[maybe_unused]] constexpr direction_inner direction_near = false;
}


// Constexpr 2D implementations

namespace il::d2
{
    [[nodiscard, maybe_unused]] inline direction_inner get_direction(
            const line& line,
            const point& point)
    {
        if (const auto relation = dot(line, point); relation != 0) return relation < 0;

        throw std::invalid_argument("Point is on the line.");
    }

    [[nodiscard, maybe_unused]] inline direction_inner get_direction(
            const point& first,
            const point& second,
            const point& third)
    {
        if (const auto relation = dot(third, get_connecting_line(first, second));
                relation != 0)
        {
            return relation < 0;
        }

        throw std::invalid_argument("Points are collinear.");
    }
}


// Constexpr 3D implementations

namespace il::d3
{
    [[nodiscard, maybe_unused]] inline direction_inner get_direction(const plane& plane, const point& point)
    {
        if (const auto relation = dot(plane, point); relation != 0) return relation < 0;

        throw std::invalid_argument("Point is on the plane.");
    }

    [[nodiscard, maybe_unused]] inline direction_inner get_direction(
            const point& first,
            const point& second,
            const point& third,
            const point& fourth)
    {
        if (const auto relation = dot(fourth, get_common_plane(first, second, third));
                relation != 0)
        {
            return relation < 0;
        }

        throw std::invalid_argument("Points are coplanar.");
    }
}



// Strict definition

namespace il
{
    enum struct [[maybe_unused]] degree_of_freedom : unsigned short int
    {
        circular [[maybe_unused]],
        height [[maybe_unused]],
        width [[maybe_unused]],
        depth [[maybe_unused]]
    };


    template<
            degree_of_freedom DegreeOfFreedom,
            small_natural_number dimension_count = (DegreeOfFreedom == degree_of_freedom::depth ? 3 : 2)>
    class [[maybe_unused]] direction
    {
        // Operations
    public:
        [[nodiscard, maybe_unused]] constexpr direction operator~() const
        { return direction{!_inner}; }

        [[nodiscard, maybe_unused]] constexpr bool operator==(direction other) const
        { return _inner == other._inner; }

        [[nodiscard, maybe_unused]] constexpr bool operator!=(direction other) const
        { return _inner == other._inner; }



        // All possible directions

        ENABLE_IF_TEMPLATE(DegreeOfFreedom == degree_of_freedom::circular)
        [[maybe_unused]] static inline const direction counterclockwise{true};

        ENABLE_IF_TEMPLATE(DegreeOfFreedom == degree_of_freedom::circular)
        [[maybe_unused]] static inline const direction clockwise{false};


        ENABLE_IF_TEMPLATE(DegreeOfFreedom == degree_of_freedom::height)
        [[maybe_unused]] static inline const direction above{false};

        ENABLE_IF_TEMPLATE(DegreeOfFreedom == degree_of_freedom::height)
        [[maybe_unused]] static inline const direction below{true};


        ENABLE_IF_TEMPLATE(DegreeOfFreedom == degree_of_freedom::width)
        [[maybe_unused]] static inline const direction right{true};

        ENABLE_IF_TEMPLATE(DegreeOfFreedom == degree_of_freedom::width)
        [[maybe_unused]] static inline const direction left{false};


        ENABLE_IF_TEMPLATE(DegreeOfFreedom == degree_of_freedom::height)
        [[maybe_unused]] static inline const direction bottom{true};

        ENABLE_IF_TEMPLATE(DegreeOfFreedom == degree_of_freedom::height)
        [[maybe_unused]] static inline const direction top{false};


        ENABLE_IF_TEMPLATE(DegreeOfFreedom == degree_of_freedom::depth && dimension_count == 3)
        [[maybe_unused]] static inline const direction far{true};

        ENABLE_IF_TEMPLATE(DegreeOfFreedom == degree_of_freedom::depth && dimension_count == 3)
        [[maybe_unused]] static inline const direction near{false};



        // From primitives

        ENABLE_IF_TEMPLATE(DegreeOfFreedom == degree_of_freedom::height && dimension_count == 2)
        [[nodiscard, maybe_unused]] static constexpr direction get(
                const d2::line& line,
                const d2::point& point)
        {
            return direction{d2::get_direction(line, point)};
        }

        ENABLE_IF_TEMPLATE(DegreeOfFreedom == degree_of_freedom::circular && dimension_count == 2)
        [[nodiscard, maybe_unused]] static constexpr direction get(
                const d2::point& first,
                const d2::point& second,
                const d2::point& third)
        {
            return direction{d2::get_direction(first, second, third)};
        }


        ENABLE_IF_TEMPLATE(DegreeOfFreedom == degree_of_freedom::height && dimension_count == 3)
        [[nodiscard, maybe_unused]] static constexpr direction get(
                const d3::plane& plane,
                const d3::point& point)
        {
            return direction{d3::get_direction(plane, point)};
        }

        ENABLE_IF_TEMPLATE(DegreeOfFreedom == degree_of_freedom::height && dimension_count == 3)
        [[nodiscard, maybe_unused]] static constexpr direction get(
                const d3::point& first,
                const d3::point& second,
                const d3::point& third,
                const d3::point& fourth)
        {
            return direction{d3::get_direction(first, second, third, fourth)};
        }


        // Protection
    private:
        constexpr direction() = default;

        // Data
        bool _inner;
    };


    // Dimensional aliases

    namespace d2
    {
        template<degree_of_freedom DegreeOfFreedom>
        using direction [[maybe_unused]] = il::direction<DegreeOfFreedom, 2>;
    }

    namespace d3
    {
        template<degree_of_freedom DegreeOfFreedom>
        using direction [[maybe_unused]] = il::direction<DegreeOfFreedom, 3>;
    }
}

#endif
