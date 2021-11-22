#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedLocalVariable"

#ifndef IRGLAB_ALGORITHM_HPP
#define IRGLAB_ALGORITHM_HPP


#include "external/pch.hpp"

#include "type_traits.hpp"


namespace il
{
    [[nodiscard, maybe_unused]] inline size_t factorial(const size_t n)
    {
        if (n == 0) return 1;

        return n * factorial(n - 1);
    }

    [[nodiscard, maybe_unused]] inline size_t number_of_combinations(const size_t n, const size_t r)
    {
        return factorial(n) / (factorial(r) * factorial(n - r));
    }


    template<typename ScalarType, std::enable_if_t<std::is_arithmetic_v<ScalarType>, int> = 0>
    [[nodiscard, maybe_unused]] size_t highest_bit_position(ScalarType scalar)
    {
        size_t result = -1;

        while (scalar >>= 1)
        {
            result++;
        }

        return result;
    }

    template<
            typename BitmaskType, std::enable_if_t<
                    std::is_enum_v<BitmaskType> and std::is_arithmetic_v<std::underlying_type_t<BitmaskType>>,
                    int> = 0>
    [[nodiscard, maybe_unused]] size_t highest_bit_position(BitmaskType bitmask)
    {
        using underlying_type = std::underlying_type_t<BitmaskType>;
        auto underlying_bitmask = static_cast<underlying_type>(bitmask);
        return highest_bit_position<underlying_type>(underlying_bitmask);
    }


    template<
            typename ForwardIterator, std::enable_if_t<
                    is_iterator<ForwardIterator> &&
                    std::is_convertible_v<
                            typename std::iterator_traits<ForwardIterator>::iterator_category,
                            std::forward_iterator_tag> &&
                    is_invokable<std::equal_to<>, typename std::iterator_traits<ForwardIterator>::value_type>,
                    int> = 0>
    [[nodiscard, maybe_unused]] ForwardIterator remove_duplicates(ForwardIterator first, ForwardIterator last)
    {
        auto new_last = first;

        for (auto current = first ; current != last ; ++current)
        {
            if (std::find(first, new_last, *current) == new_last)
            {
                if (new_last != current) *new_last = *current;
                ++new_last;
            }
        }

        return new_last;
    }


    template<typename InnerType>
    [[nodiscard, maybe_unused]] static std::vector<std::reference_wrapper<const InnerType>>
    dereference_vulkan_handles(
            const std::vector<vk::UniqueHandle<InnerType, vk::DispatchLoaderDynamic>>& handles)
    {
        std::vector<std::reference_wrapper<const InnerType>> result{ };

        for (const auto& handle : handles)
        {
            result.emplace_back(std::cref(*handle));
        }

        return result;
    }
}

#endif //IRGLAB_ALGORITHM_HPP

#pragma clang diagnostic pop
