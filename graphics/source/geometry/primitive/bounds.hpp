#ifndef IRGLAB_BOUNDS_HPP
#define IRGLAB_BOUNDS_HPP


#include "external/external.hpp"

#include "primitives.hpp"


namespace il
{
    // Declaration

    template<small_natural_number DimensionCount>
    struct [[maybe_unused]] bounds;


    // Dimensional aliases

    namespace d2
    {
        using bounds [[maybe_unused]] = il::bounds<dimension_count>;
    }
    namespace d3
    {
        using bounds [[maybe_unused]] = il::bounds<dimension_count>;
    }



    // 2D

    template<>
    struct [[maybe_unused]] bounds<2>
    {
        // Constructors and related methods

        [[nodiscard, maybe_unused]] explicit bounds(
                rational_number x_min = rational_number_max, rational_number x_max = rational_number_min,
                rational_number y_min = rational_number_max, rational_number y_max = rational_number_min) :
                _x_min{std::move(x_min)}, _x_max{std::move(x_max)},
                _y_min{std::move(y_min)}, _y_max{std::move(y_max)}
        { }


        // Union

        [[nodiscard, maybe_unused]] bounds operator|(const point<2>& point) const noexcept
        {
            auto new_bounds{*this};

            if (point.x < this->_x_min) new_bounds._x_min = point.x;
            if (point.x > this->_x_max) new_bounds._x_max = point.x;

            if (point.y < this->_y_min) new_bounds._y_min = point.y;
            if (point.y > this->_y_max) new_bounds._y_max = point.y;

            return new_bounds;
        }

        [[maybe_unused]] constexpr void operator|=(const point<2>& point) noexcept
        {
            if (point.x < this->_x_min) this->_x_min = point.x;
            if (point.x > this->_x_max) this->_x_max = point.x;

            if (point.y < this->_y_min) this->_y_min = point.y;
            if (point.y > this->_y_max) this->_y_max = point.y;
        }


        [[nodiscard, maybe_unused]] bounds operator|(const bounds& old_bounds) const noexcept
        {
            auto new_bounds{*this};

            if (old_bounds._x_min < this->_x_min) new_bounds._x_min = old_bounds._x_min;
            if (old_bounds._x_max > this->_x_max) new_bounds._x_max = old_bounds._x_max;

            if (old_bounds._y_min < this->_y_min) new_bounds._y_min = old_bounds._y_min;
            if (old_bounds._y_max > this->_y_max) new_bounds._y_max = old_bounds._y_max;

            return new_bounds;
        }

        [[maybe_unused]] constexpr void operator|=(const bounds& bounds) noexcept
        {
            if (bounds._x_min < this->_x_min) this->_x_min = bounds._x_min;
            if (bounds._x_max > this->_x_max) this->_x_max = bounds._x_max;

            if (bounds._y_min < this->_y_min) this->_y_min = bounds._y_min;
            if (bounds._y_max > this->_y_max) this->_y_max = bounds._y_max;
        }


        // Non-modifiers

        using difference = cartesian_coordinates<2>;

        [[nodiscard, maybe_unused]] difference get_difference() const noexcept
        {
            return {this->_x_max - this->_x_min, this->_y_max - this->_y_min};
        }

        using center = cartesian_coordinates<2>;

        [[nodiscard, maybe_unused]] center get_center() const noexcept
        {
            return
                    {
                            this->_x_min + (this->_x_max - this->_x_min) / 2.0f,
                            this->_y_min + (this->_y_max - this->_y_min) / 2.0f
                    };
        }


        [[nodiscard, maybe_unused]] friend std::ostream& operator<<(
                std::ostream& output_stream, const bounds& bounds)
        {
            return output_stream <<
                                 "Min x: " << bounds._x_min << "Max x: " << bounds._x_max << std::endl <<
                                 "Min y: " << bounds._y_min << "Max y: " << bounds._y_max << std::endl <<
                                 std::endl;
        }


        // Data

    private:
        rational_number _x_min;
        rational_number _x_max;

        rational_number _y_min;
        rational_number _y_max;
    };



    // 3D

    template<>
    struct [[maybe_unused]] bounds<3>
    {
        // Constructors and related methods

        [[nodiscard, maybe_unused]] explicit bounds(
                rational_number x_min = rational_number_max, rational_number x_max = rational_number_min,
                rational_number y_min = rational_number_max, rational_number y_max = rational_number_min,
                rational_number z_min = rational_number_max, rational_number z_max = rational_number_min) :

                _x_min{std::move(x_min)}, _x_max{std::move(x_max)},
                _y_min{std::move(y_min)}, _y_max{std::move(y_max)},
                _z_min{std::move(z_min)}, _z_max{std::move(z_max)}
        { }


        // Union

        [[nodiscard, maybe_unused]] bounds operator|(const point<3>& point) const noexcept
        {
            auto new_bounds{*this};

            if (point.x < this->_x_min) new_bounds._x_min = point.x;
            if (point.x > this->_x_max) new_bounds._x_max = point.x;

            if (point.y < this->_y_min) new_bounds._y_min = point.y;
            if (point.y > this->_y_max) new_bounds._y_max = point.y;

            if (point.z < this->_z_min) new_bounds._z_min = point.z;
            if (point.z > this->_z_max) new_bounds._z_max = point.z;

            return new_bounds;
        }

        [[maybe_unused]] constexpr void operator|=(const point<3>& point) noexcept
        {
            if (point.x < this->_x_min) this->_x_min = point.x;
            if (point.x > this->_x_max) this->_x_max = point.x;

            if (point.y < this->_y_min) this->_y_min = point.y;
            if (point.y > this->_y_max) this->_y_max = point.y;

            if (point.z < this->_z_min) this->_z_min = point.z;
            if (point.z > this->_z_max) this->_z_max = point.z;
        }


        [[nodiscard, maybe_unused]] bounds operator|(const bounds& old_bounds) const noexcept
        {
            auto new_bounds{*this};

            if (old_bounds._x_min < this->_x_min) new_bounds._x_min = old_bounds._x_min;
            if (old_bounds._x_max > this->_x_max) new_bounds._x_max = old_bounds._x_max;

            if (old_bounds._y_min < this->_y_min) new_bounds._y_min = old_bounds._y_min;
            if (old_bounds._y_max > this->_y_max) new_bounds._y_max = old_bounds._y_max;

            if (old_bounds._z_min < this->_z_min) new_bounds._z_min = old_bounds._z_min;
            if (old_bounds._z_max > this->_z_max) new_bounds._z_max = old_bounds._z_max;

            return new_bounds;
        }

        [[maybe_unused]] constexpr void operator|=(const bounds& bounds) noexcept
        {
            if (bounds._x_min < this->_x_min) this->_x_min = bounds._x_min;
            if (bounds._x_max > this->_x_max) this->_x_max = bounds._x_max;

            if (bounds._y_min < this->_y_min) this->_y_min = bounds._y_min;
            if (bounds._y_max > this->_y_max) this->_y_max = bounds._y_max;

            if (bounds._z_min < this->_z_min) this->_z_min = bounds._z_min;
            if (bounds._z_max > this->_z_max) this->_z_max = bounds._z_max;
        }


        // Non-modifiers

        using difference = cartesian_coordinates<3>;

        [[nodiscard, maybe_unused]] difference get_difference() const noexcept
        {
            return
                    {
                            this->_x_max - this->_x_min,
                            this->_y_max - this->_y_min,
                            this->_z_max - this->_z_min
                    };
        }

        using center = cartesian_coordinates<3>;

        [[nodiscard, maybe_unused]] center get_center() const noexcept
        {
            return
                    {
                            this->_x_min + (this->_x_max - this->_x_min) / 2.0f,
                            this->_y_min + (this->_y_max - this->_y_min) / 2.0f,
                            this->_z_min + (this->_z_max - this->_z_min) / 2.0f
                    };
        }


        [[nodiscard, maybe_unused]] friend std::ostream& operator<<(
                std::ostream& output_stream, const bounds& bounds)
        {
            return output_stream <<
                                 "Min x: " << bounds._x_min << "Max x: " << bounds._x_max << std::endl <<
                                 "Min y: " << bounds._y_min << "Max y: " << bounds._y_max << std::endl <<
                                 "Min z: " << bounds._z_min << "Max z: " << bounds._z_max << std::endl <<
                                 std::endl;
        }


        // Data

    private:
        rational_number _x_min;
        rational_number _x_max;

        rational_number _y_min;
        rational_number _y_max;

        rational_number _z_min;
        rational_number _z_max;
    };

}

#endif
