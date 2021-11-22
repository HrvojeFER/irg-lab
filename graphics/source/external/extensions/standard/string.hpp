#ifndef IRGLAB_STRING_HPP
#define IRGLAB_STRING_HPP

#include "external/pch.hpp"

namespace il
{
    [[maybe_unused]] inline void left_trim(std::string &s)
    {
        s.erase(
                s.begin(),
                std::find_if(
                        s.begin(), s.end(),
                        [](const int character)
                        {
                            return !std::isspace(character);
                        }));
    }

    [[maybe_unused]] inline void right_trim(std::string &string)
    {
        string.erase(
                std::find_if(
                        string.rbegin(), string.rend(),
                        [](const int character)
                        {
                            return !std::isspace(character);
                        }).base(),
                string.end());
    }

    [[maybe_unused]] inline void trim(std::string &s)
    {
        left_trim(s);
        right_trim(s);
    }
}

#endif
