#include "pch.hpp"

template <class MathType>
void print(MathType value)
{
	std::cout << glm::to_string(value) << std::endl;
}

template <class ... MathType>
void print_many(MathType ... params)
{
	for (auto value : {params...})
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

int main()
{
	print_many(v1, v2, v3, v4);
	std::cout << s << std::endl;
	std::cout << std::endl;
	print_many(m1, m2, m3);
	std::cout << std::endl;
	print_many(v);

	
	float x1, y1, z1, r1;
	float x2, y2, z2, r2;
	float x3, y3, z3, r3;

	std::cout << std::endl << "1. equation" << std::endl;
	std::cin >> x1 >> y1 >> z1 >> r1;
	std::cout << std::endl << "2. equation" << std::endl;
	std::cin >> x2 >> y2 >> z2 >> r2;
	std::cout << std::endl << "3. equation" << std::endl;
	std::cin >> x3 >> y3 >> z3 >> r3;

	print(solve_equation(
		glm::mat3x3
		{
			x1, y1, z1,
			x2, y2, z2,
			x3, y3, z3
		},
		glm::vec3
		{
			r1, r2, r3
		})
	);

	
	std::cout << std::endl << "Triangle vertex A" << std::endl;
	std::cin >> x1 >> y1 >> z1;
	std::cout << std::endl << "Triangle vertex B" << std::endl;
	std::cin >> x2 >> y2 >> z2;
	std::cout << std::endl << "Triangle vertex C" << std::endl;
	std::cin >> x3 >> y3 >> z3;
	std::cout << std::endl << "Point" << std::endl;
	std::cin >> r1 >> r2 >> r3;
	std::cout << std::endl;

	print(get_barycentric(
		glm::mat3x3
		{
			x1, x2, x3,
			y1, y2, y3,
			z1, z2, z3
		},
		glm::vec3
		{
			r1, r2, r3
		})
	);

	
	return EXIT_SUCCESS;
}