#include "pch.hpp"

#include "vector.hpp"

namespace irgmath
{
	template <int Length>
	template<typename ... ValueT>
	constexpr vector<Length>::vector(ValueT... values) const
		: inner_ { values... }, normalized_(irgmath::normalized(*this)) { }

	template<int Length>
	constexpr vector<Length>::vector(const std::array<value_t, Length>& values) const noexcept
		: inner_(values), normalized_(irgmath::normalized(*this)) { }
	
	template<int Length>
	constexpr vector<Length> vector<Length>::for_each(value_t(*fn)(const int&, const value_t&)) const noexcept
	{
		vector<Length> result {};
		for (auto i = 0; i < Length; ++i)
		{
			result.inner_[i] = fn(i, this[i]);
		}

		return result;
	}

	template<int Length>
	constexpr value_t vector<Length>::aggregate(value_t(*fn)(const value_t&)) const noexcept
	{
		value_t result = 0;
		for (const auto& value : inner_)
		{
			result += fn(value);
		}
		
		return result;
	}

	template <int Length>
	constexpr value_t vector<Length>::normalized() const noexcept
	{
		return normalized_;
	}

	template <int Length>
	constexpr value_t vector<Length>::projection(const vector<Length>& vec) const noexcept
	{
		return (*this * vec) / (vec.normalized_);
	}

	
	template<int Length>
	constexpr vector<Length> operator-(const vector<Length>& vec) noexcept
	{
		return vec.for_each([](const int&, const value_t& value) { return -value; });
	}

	template <int Length>
	constexpr vector<Length> operator+(const vector<Length>& vector1, const vector<Length>& vector2) noexcept
	{
		return vector1.for_each([vector1, vector2](const int& i, const value_t&) { return vector1[i] + vector2[i]; });
	}

	template<int Length>
	constexpr vector<Length> operator-(const vector<Length>& first, const vector<Length>& second) noexcept
	{
		return first + (-second);
	}

	template <int Length>
	constexpr value_t operator*(const vector<Length>& vector1, const vector<Length>& vector2) noexcept
	{
		return vector1.aggregate([vector1, vector2](const int& i, const value_t&) { return vector1[i] * vector2[i]; });
	}

	template <int Length>
	constexpr vector<Length> operator*(const value_t& factor, const vector<Length>& vec) noexcept
	{
		return vec.for_each([factor](const int&, const value_t& value) { return factor * value; });
	}


	template<int Length>
	constexpr value_t normalized(const vector<Length>& vec) noexcept
	{
		return std::sqrt(vec.aggregate([](const value_t& value) { return value * value; }));
	}

	template <int Length>
	constexpr value_t angle_between(const vector<Length>& vector1, const vector<Length>& vector2) noexcept
	{
		return (vector1 * vector2) / (vector1.normalized_ * vector2.normalized_);
	}

	
	constexpr vector<3> cross_product(const vector<3>& first, const vector<3>& second) noexcept
	{
		const vector<3> result
		{
			first[2] * second[3] - first[3] * second[2],
			first[3] * second[1] - first[1] * second[3],
			first[1] * second[2] - first[2] * second[1]
		};
		
		return result;
	}
}
