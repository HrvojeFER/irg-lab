#include "pch.hpp"

#include "body_app.hpp"
#include "convex_polygon_app.hpp"


int main()
{	
	try
	{
		irglab::convex_polygon_app{ }.run();
	}
	catch (const std::exception& ex)
	{
		std::cerr << ex.what();
	}
	
	return EXIT_SUCCESS;
}
