#include "pch.hpp"

template <class MathType>
void print(MathType value)
{
	std::cout << glm::to_string(value) << std::endl;
}

template <class ... MathType>
void print_many(MathType ... params)
{
	for (const auto& value : { params ... })
	{
		print(value);
	}
}

auto const hv1 = glm::vec3{-1, 4, -1};

auto const v1 = glm::vec3{2, 3, -4} + hv1;

auto const s = dot(hv1, v1);

auto const v2 = cross(v1, glm::vec3{2, 2, 4});

auto const v3 = normalize(v2);

auto const v4 = -v2;


auto const hm1 =
	glm::mat3x3
	{
		1, 2, 3,
		2, 1, 3,
		4, 5, 1
	};
auto const hm2 =
	glm::mat3x3
	{
		-1, 2, -3,
		5, -2, 7,
		-4, -1, 3
	};

auto const m1 = hm1 + hm2;

auto const m2 = hm1 * transpose(hm2);

auto const m3 = hm1 * inverse(hm2);

auto const v =
	glm::vec4{1, 2, 3, 1} *
	glm::mat4x4
	{
		1, 0, 0, 0,
		0, 2, 0, 0,
		0, 0, 1, 0,
		2, 3, 3, 1
	};


inline glm::vec3 solve_equation(const glm::mat3x3& coefficients, const glm::vec3& solutions)
{
	return solutions * inverse(coefficients);
}


inline glm::vec3 get_barycentric(const glm::mat3x3& triangle, const glm::vec3& point)
{
	return point * inverse(triangle);
}

int main()  // NOLINT(bugprone-exception-escape)
{

	print(transpose(hm1));
	print(transpose(hm1)[0]);
	print(transpose(hm1) * transpose(hm1)[0]);
	print(transpose(hm1)[0] * transpose(hm1));
	
	return EXIT_SUCCESS;
}