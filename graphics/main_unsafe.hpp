#pragma once

#include "pch.hpp"

#include "app_unsafe.hpp"

namespace irglab
{
    inline int main_unsafe()
    {
        irglab::app_unsafe app;

        try
        {
            app.run();
        }
        catch (const std::exception & e)
        {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }
}
