#include "pch.hpp"

#include "main_safe.hpp"
#include "main_unsafe.hpp"

#include "window.hpp"

int main()
{
	const irglab::window window{ "dummy" };

	window.loop();

	return EXIT_SUCCESS;
}
