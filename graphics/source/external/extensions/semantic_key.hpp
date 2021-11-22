#ifndef IRGLAB_SEMANTIC_KEY_HPP
#define IRGLAB_SEMANTIC_KEY_HPP


#include "external/pch.hpp"


namespace il
{
    struct [[maybe_unused]] semantic_key
    {
        [[nodiscard, maybe_unused]] semantic_key() : _inner{_inner_counter++}
        { }

        [[nodiscard, maybe_unused]] static semantic_key start()
        {
            _inner_counter = 0;
            return {};
        }

        [[nodiscard, maybe_unused]] bool operator==(const semantic_key& other) const
        {
            return _inner == other._inner;
        }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "google-explicit-constructor"

        [[nodiscard, maybe_unused]] operator size_t() const
        {
            return _inner;
        }

#pragma clang diagnostic pop

        friend std::hash<semantic_key>;

    private:
        static inline size_t _inner_counter{0};
        const size_t _inner{0};
    };


    template<typename ValueType>
    using semantic_map [[maybe_unused]] =
    std::unordered_map<semantic_key, ValueType>;

    template<typename ValueType>
    using semantic_vector [[maybe_unused]] = std::vector<ValueType>;

    template<typename ValueType, size_t Size>
    using semantic_array [[maybe_unused]] = std::array<ValueType, Size>;
}

template<>
struct [[maybe_unused]] std::hash<il::semantic_key>
{
    [[maybe_unused]] size_t operator()(const il::semantic_key& key) const noexcept
    { return key._inner; }
};


#endif //IRGLAB_SEMANTIC_KEY_HPP
