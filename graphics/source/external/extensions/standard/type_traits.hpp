#ifndef IRGLAB_TYPE_TRAITS_HPP
#define IRGLAB_TYPE_TRAITS_HPP


#include "external/pch.hpp"


namespace il
{
    template<typename CheckingType, typename Type>
    struct [[maybe_unused]] is_one_of_b : std::is_same<CheckingType, Type>
    {
    };

    template<typename CheckingType, typename... Variants>
    struct [[maybe_unused]] is_one_of_b<CheckingType, std::variant<Variants...>>
            : std::disjunction<std::is_same<CheckingType, Variants>...>
    {
    };

    template<typename CheckingType, typename Type>
    [[maybe_unused]] constexpr bool is_one_of = is_one_of_b<CheckingType, Type>::value;


    template<typename T, typename = void>
    struct [[maybe_unused]] is_iterator_b : std::false_type
    {
    };

    template<typename T>
    struct [[maybe_unused]] is_iterator_b<
            T, std::void_t<
                    typename std::iterator_traits<T>::iterator_category>> : std::true_type
    {
    };

    template<typename T>
    [[maybe_unused]] constexpr bool is_iterator = is_iterator_b<T>::value;

    template<typename Iterator>
    using iterator_value_type [[maybe_unused]] = typename std::iterator_traits<Iterator>::value_type;

    template<typename BeginIterator, typename EndIterator, typename ValueType>
    [[maybe_unused]] constexpr bool have_value_type =
            std::is_same_v<iterator_value_type<BeginIterator>, ValueType> &&
            std::is_same_v<iterator_value_type<EndIterator>, ValueType>;


    template<typename T, typename = void>
    struct [[maybe_unused]] is_range_b : std::false_type
    {
    };

    template<typename T>
    struct [[maybe_unused]] is_range_b<
            T, std::enable_if_t<
                    is_iterator<decltype(std::declval<T>().begin())> &&
                    is_iterator<decltype(std::declval<T>().end())>, void>>
            : std::true_type
    {
    };

    template<typename T>
    constexpr bool is_range [[maybe_unused]] = is_range_b<T>::value;

    template<typename T>
    using range_value_type [[maybe_unused]] = iterator_value_type<decltype(std::declval<T>().begin())>;


#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedTemplateParameterInspection"


    template<typename Function, typename... Arguments>
    struct [[maybe_unused]] is_invokable_i
    {
    private:
#pragma clang diagnostic push
#pragma ide diagnostic ignored "NotImplementedFunctions"

        template<typename StaticOperator, typename... StaticArguments>
        [[maybe_unused]] static auto _test(int) ->
        decltype(std::declval<StaticOperator>()(std::declval<StaticArguments>()...), void(), std::true_type{ });

        template<typename StaticOperator, typename... StaticArguments>
        [[maybe_unused]] static auto _test(...) -> std::false_type;

#pragma clang diagnostic pop

    public:
        using type [[maybe_unused]] = decltype(_test<Function, Arguments...>(0));
    };


#pragma clang diagnostic pop

    template<typename Function, typename... Arguments>
    [[maybe_unused]] constexpr bool is_invokable = is_invokable_i<Function, Arguments...>::type::value;
}

#endif //IRGLAB_TYPE_TRAITS_HPP
