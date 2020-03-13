#ifndef LAB2_PCH_HPP
#define LAB2_PCH_HPP

#pragma once


#include <stdexcept>
#include <functional>
#include <optional>

#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
// UINT32_MAX and UINT64 needed
#include <cstdint>
#include <algorithm>

#include <vector>
#include <array>
#include <set>
#include <map>


// GLFW includes Vulkan with the following macro.
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm.hpp"

// Added so RAII can be added later.
// #include "vulkan/vulkan.hpp"


#endif
