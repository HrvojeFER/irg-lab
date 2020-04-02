#ifndef GRAPHICS_EXTENSIONS_HPP
#define GRAPHICS_EXTENSIONS_HPP


#include "pch.hpp"


namespace irglab
{
	template<typename InnerType>
	[[nodiscard]] std::vector<std::reference_wrapper<const InnerType>> dereference_handles(
		const std::vector<vk::UniqueHandle<InnerType, vk::DispatchLoaderDynamic>>& handles)
	{
		std::vector<std::reference_wrapper<const InnerType>> result{};

		for (const auto& handle : handles)
		{
			result.push_back(std::cref(*handle));
		}

		return result;
	}
}


#endif
