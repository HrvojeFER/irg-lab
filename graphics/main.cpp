#include "pch.hpp"


// ReSharper disable CppUnusedIncludeDirective
#include "animation_app.hpp"
#include "convex_polygon_app.hpp"
#include "fractal_app.hpp"


int main()
{	
	irglab::fractal_app{ }.run();
	
	return EXIT_SUCCESS;
}
