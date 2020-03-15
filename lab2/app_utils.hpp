#ifndef APP_UTILS_HPP
#define APP_UTILS_HPP

#pragma once

#include "pch.hpp"

namespace irglab
{
	template<
        typename SubsetValueType,
		typename SupersetValueType,
		typename SubsetIteratorType,
		typename SupersetIteratorType,
		typename PredicateType>
    bool is_subset(
        const SubsetIteratorType subset,
        const SupersetIteratorType superset,
        PredicateType equality_comparer)
    {
        return std::all_of(
            subset.begin(),
            subset.end(),
            [superset, equality_comparer](SupersetValueType subset_value)
            {
                return std::find_if(
                    superset.begin(),
                    superset.end(),
                    [subset_value, equality_comparer](SupersetValueType superset_value)
                    {
                        return equality_comparer(subset_value, superset_value);
                    });
            });
    }

    inline std::vector<char> read_shader_file(const std::string& path)
    {
        std::ifstream file(path, std::ios::ate | std::ios::binary);

    	if (!file.is_open())
    	{
            throw std::runtime_error("Failed to open file from path '" + path + "'.");
    	}

        const auto file_size = file.tellg();
        std::vector<char> buffer(static_cast<size_t>(file_size));
        file.seekg(0);
        file.read(buffer.data(), static_cast<std::streamsize>(file_size));

        return buffer;
    }
}

#endif
