#ifndef IRGLAB_SFINAE_MACROS_HPP
#define IRGLAB_SFINAE_MACROS_HPP


#include "external/pch.hpp"


#define ENABLE_IF(condition) std::enable_if_t<condition, int> = 0

#define ENABLE_IF_TEMPLATE(condition) \
        template< \
                typename Dummy = void, std::enable_if_t< \
                        std::is_same_v<Dummy, void> && \
                        condition, \
                        int> = 0>


#endif //IRGLAB_SFINAE_MACROS_HPP
