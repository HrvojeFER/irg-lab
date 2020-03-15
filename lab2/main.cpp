#include "app.hpp"

int main()
{
    try
    {
        irglab::app app;
	    app.run();
    }
    catch (const std::exception & e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}