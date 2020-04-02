#include "pch.hpp"

#include "app.hpp"


int main()
{
	try
	{
		irglab::app{}.run();
	}
	catch (const std::exception& ex)
	{
		std::cerr << ex.what();
	}
	
	return EXIT_SUCCESS;
}
