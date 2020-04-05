#ifndef GRAPHICS_BODY_APP
#define GRAPHICS_BODY_APP


#include "pch.hpp"

#include "app_base.hpp"
#include "body.hpp"


namespace irglab
{
	struct body_app final : app_base
	{
		explicit body_app() : app_base("Body") {}
	};
}

#endif
