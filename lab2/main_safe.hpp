#ifndef MAIN_SAFE_HPP
#define MAIN_SAFE_HPP

#pragma once

#include "app_safe.hpp"

namespace irglab
{
    inline int main_safe()
    {
        try
        {
            const app_safe app;
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

#endif
