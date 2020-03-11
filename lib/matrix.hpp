#ifndef MATRIX_HPP
#define MATRIX_HPP

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include "vector.hpp"

namespace irgmath
{
	template<int Height, int Width>
	struct matrix
	{
	private:
		const std::array<std::array<value_t, Width>, Height> inner_;

		constexpr matrix<Height, Width>(std::array<std::array<value_t, Width>, Height>) const noexcept;
	public:
		constexpr matrix<Height, Width>(std::initializer_list<std::initializer_list<value_t>>) const;
		
		constexpr vector<Width> row(unsigned int) const noexcept;
		constexpr vector<Height> column(unsigned int) const noexcept;

		constexpr matrix<Height, Width> transposed() const noexcept;
	};


	template<int Size>
	struct square_matrix : matrix<Size, Size>
	{
	private:
		const value_t determinant_;
		
		constexpr square_matrix<Size>(std::array<std::array<value_t, Size>, Size>) const noexcept;
	public:
		constexpr square_matrix<Size>(std::initializer_list<std::initializer_list<value_t>>) const;
	};


	template<int Height, int Width>
	constexpr matrix<Height, Width> operator-(const matrix<Height, Width>&) noexcept;

	template<int Height, int Width>
	constexpr matrix<Height, Width> operator+(const matrix<Height, Width>&, const matrix<Height, Width>&) noexcept;

	template<int Height, int Width>
	constexpr matrix<Height, Width> operator-(const matrix<Height, Width>&, const matrix<Height, Width>&) noexcept;

	template<int ResultHeight, int Common, int ResultWidth>
	constexpr matrix<ResultHeight, ResultWidth> operator *(const matrix<ResultHeight, Common>&, const matrix<Common, ResultWidth>&) noexcept;

	
	template<int Height, int Width>
	constexpr vector<Width> operator*(const vector<Width>&, const matrix<Height, Width>&) noexcept;

	template<int Height, int Width>
	constexpr vector<Height> operator*(const matrix<Width, Height>&, const vector<Height>&) noexcept;

	template<int Height, int Width>
	constexpr matrix<Height, Width> operator*(const value_t&, const matrix<Height, Width>&) noexcept;
}

#endif
