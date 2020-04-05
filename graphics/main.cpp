#include "pch.hpp"

#include "app_base.hpp"


int main()
{
	try
	{
		irglab::app_base{}.run();
	}
	catch (const std::exception& ex)
	{
		std::cerr << ex.what();
	}
	
	return EXIT_SUCCESS;
}
