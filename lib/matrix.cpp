#include "pch.hpp"

#include "matrix.hpp"
#include "vector.hpp"

namespace irgmath
{
	template <int Height, int Width>
	constexpr matrix<Height, Width>::matrix(std::array<std::array<value_t, Width>, Height> values) const noexcept
		: inner_(values) { }

	template <int Height, int Width>
	constexpr matrix<Height, Width>::matrix(std::initializer_list<std::initializer_list<value_t>> rows) const
		: inner_ {}
	{
		static_assert(rows.size() <= Height, "Row count should be equal to or lower than the matrix height.");
		
		auto i = 0, j = 0;
		for (const auto& row : rows)
		{
			static_assert(row.size() <= Width, "Column count should be equal to or lower than the matrix width.");
			
			inner_[i] = {};
			for (const auto& value : row)
			{
				inner_[i][j] = value;
				++i;
			}
			++j;
		}
	}

	
	template <int Height, int Width>
	constexpr vector<Width> matrix<Height, Width>::row(unsigned int i) const noexcept
	{
		static_assert(i < Height, "Row index must be lower than the matrix height.");

		vector<Width> result(inner_[i]);
		return result;
	}

	template <int Height, int Width>
	constexpr vector<Height> matrix<Height, Width>::column(unsigned int j) const noexcept
	{
		static_assert(j < Width, "Column index must be lower than the matrix width.");

		vector<Height> result {};
		for (auto i = 0 ; i < Height ; ++i)
		{
			result.inner_[i] = inner_[i][j];
		}

		return result;
	}

	template <int Height, int Width>
	constexpr matrix<Height, Width> matrix<Height, Width>::transposed() const noexcept
	{
		matrix<Height, Width> result {};
		for (auto i = 0 ; i < Height ; ++i)
		{
			for (auto j = 0 ; j < Width ; ++j)
			{
				result.inner_[i][j] = inner_[j][i];
			}
		}

		return result;
	}

	
	template <int Size>
	constexpr square_matrix<Size>::square_matrix(std::array<std::array<value_t, Size>, Size> values) const noexcept
		: matrix<Size, Size>(values), determinant_(irgmath::determinant(*this)) { }

	template<int Size>
	constexpr square_matrix<Size>::square_matrix(std::initializer_list<std::initializer_list<value_t>> rows) const
		: matrix<Size, Size>(rows), determinant_(irgmath::determinant(*this)) { }

	
	template <int Height, int Width>
	constexpr matrix<Height, Width> operator-(const matrix<Height, Width>& mat) noexcept
	{
	}

	template <int Height, int Width>
	constexpr matrix<Height, Width> operator+(const matrix<Height, Width>& mat1, const matrix<Height, Width>& mat2) noexcept
	{
	}

	template <int Height, int Width>
	constexpr matrix<Height, Width> operator-(const matrix<Height, Width>& first, const matrix<Height, Width>& second) noexcept
	{
	}

	
	template <int ResultHeight, int Common, int ResultWidth>
	constexpr matrix<ResultHeight, ResultWidth> operator*(const matrix<ResultHeight, Common>& first,
		const matrix<Common, ResultWidth>& second) noexcept
	{
	}

	template <int Height, int Width>
	constexpr vector<Width> operator*(const vector<Width>& first_vec, const matrix<Height, Width>& second_mat) noexcept
	{
	}

	template <int Height, int Width>
	constexpr vector<Height> operator*(const matrix<Width, Height>& first_mat, const vector<Height>& second_vec) noexcept
	{
	}

	template <int Height, int Width>
	constexpr matrix<Height, Width> operator*(const value_t& value, const matrix<Height, Width>& mat) noexcept
	{
	}
}
