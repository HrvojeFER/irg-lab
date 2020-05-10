#ifndef IRGLAB_TETRAHEDRON_HPP
#define IRGLAB_TETRAHEDRON_HPP


#include "pch.hpp"

#include "body.hpp"


namespace irglab::three_dimensional
{
	const auto tetrahedron =
		R"( # tetrahedron.obj created by hand.
			#
			g tetrahedron

			v 1.00 1.00 1.00
			v 2.00 1.00 1.00
			v 1.00 2.00 1.00
			v 1.00 1.00 2.00

			f 1 3 2
			f 1 4 3
			f 1 2 4
			f 2 3 4

			)"_body;
}

#endif
