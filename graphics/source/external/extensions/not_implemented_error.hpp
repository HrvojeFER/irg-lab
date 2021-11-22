#ifndef IRGLAB_NOT_IMPLEMENTED_ERROR_HPP
#define IRGLAB_NOT_IMPLEMENTED_ERROR_HPP


#include "external/pch.hpp"


namespace il
{
    class [[maybe_unused]] not_implemented_error final : public std::logic_error
    {
    public:
        [[nodiscard, maybe_unused]] explicit not_implemented_error(
                const std::optional <std::string_view> &function_name = std::nullopt,
                const std::optional <std::string_view> &reason = std::nullopt) :

                std::logic_error{_create_message(function_name, reason)}
        { }

    private:
        [[nodiscard]] static std::string _create_message(
                std::optional <std::string_view> function_name,
                std::optional <std::string_view> reason)
        {
            std::stringstream string_stream;
            string_stream << "Function is not implemented yet.";

            if (function_name.has_value())
            {
                string_stream << "Name: " << function_name.value();
            }

            if (reason.has_value())
            {
                string_stream << std::endl << "Reason: " << reason.value() << std::endl;
            }

            return string_stream.str();
        }
    };
}


#endif //IRGLAB_NOT_IMPLEMENTED_ERROR_HPP
