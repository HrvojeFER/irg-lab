#ifndef IRGLAB_CURVE_HPP
#define IRGLAB_CURVE_HPP


#include "external/external.hpp"

#include "primitive/primitive.hpp"


namespace il
{
    // Type traits

    [[nodiscard, maybe_unused]] constexpr bool is_curve_description_supported(
            small_natural_number dimension_count)
    {
        return is_vector_size_supported(dimension_count);
    }


    // Base

    template<small_natural_number DimensionCount>
    class [[maybe_unused]] curve
    {
        // Traits and types

    public:
        [[maybe_unused]] static constexpr small_natural_number dimension_count = DimensionCount;

        using control_point [[maybe_unused]] = cartesian_coordinates<dimension_count>;


        // Constructors and related methods

        // Only std::initializer_list constructor because I want to discourage the use of curves with
        // a lot of control points because that would greatly affect performance.
        template<
#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedLocalVariable"
                typename Dummy = void, std::enable_if_t<
                        std::is_same_v<Dummy, void> && is_curve_description_supported(dimension_count), int> = 0>
#pragma clang diagnostic pop
        [[nodiscard, maybe_unused]] explicit curve(std::initializer_list<control_point> control_points) :
                _control_points{control_points}
        { }


        // Non-modifiers

        [[nodiscard, maybe_unused]] cartesian_coordinates<dimension_count> operator()(
                const rational_number parameter)
        {
            cartesian_coordinates<dimension_count> result{ };

            for (natural_number i = 0 ; i < _control_points.size() ; ++i)
                result += _control_points[i] * _get_bernstein_polynomial_result(
                        i, _control_points.size() - 1, parameter);

            return result;
        }


        // Implementation details

    private:
        [[nodiscard]] static rational_number _get_bernstein_polynomial_result(
                const natural_number index,
                const natural_number control_point_count,
                const rational_number parameter)
        {
            return static_cast<rational_number>(
                    number_of_combinations(control_point_count, index) *
                    glm::pow(parameter, index) *
                    glm::pow(1 - parameter, control_point_count - index));
        }


        // Data

        std::vector<control_point> _control_points;
    };
}


// Dimensional aliases

namespace il::d2
{
    using curve [[maybe_unused]] = il::curve<dimension_count>;
}

namespace il::d3
{
    using curve [[maybe_unused]] = il::curve<dimension_count>;
}


#endif
