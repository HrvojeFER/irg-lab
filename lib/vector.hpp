#ifndef VECTOR_HPP
#define VECTOR_HPP

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include "matrix.hpp"

namespace irgmath
{
	typedef float value_t;

	template<int Length>
	struct vector
	{
		template<int Height, int Width>
		friend matrix<Height, Width>;

	private:
		const std::array<value_t, Length> inner_;
		const value_t normalized_;

		constexpr vector<Length>(const std::array<value_t, Length>&) const noexcept;

		constexpr vector<Length> for_each(value_t (*fn)(const int&, const value_t&)) const noexcept;
		constexpr value_t aggregate(value_t (*fn)(const value_t&)) const noexcept;
	public:
		template<typename ... ValueT>
		constexpr vector(ValueT... values) const;
		
		constexpr value_t operator [](unsigned int i) const
		{
			return i < Length ? inner_[i] : throw std::out_of_range("Vector index out of range.");
		}

		constexpr value_t normalized() const noexcept;

		constexpr value_t projection(const vector<Length>&) const noexcept;
	};

	
	template<int Length>
	constexpr vector<Length> operator-(const vector<Length>&) noexcept;

	template<int Length>
	constexpr vector<Length> operator+(const vector<Length>&, const vector<Length>&) noexcept;

	template<int Length>
	constexpr  vector<Length> operator-(const vector<Length>&, const vector<Length>&) noexcept;

	template<int Length>
	constexpr value_t operator*(const vector<Length>&, const vector<Length>&) noexcept;

	template<int Length>
	constexpr vector<Length> operator*(const value_t&, const vector<Length>&) noexcept;

	
	template<int Length>
	constexpr value_t normalized(const vector<Length>&) noexcept;

	template<int Length>
	constexpr value_t angle_between(const vector<Length>&, const vector<Length>&) noexcept;

	
	constexpr vector<3> cross_product(const vector<3>&, const vector<3>&) noexcept;
}

#endif
