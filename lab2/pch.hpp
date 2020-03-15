#ifndef LAB2_PCH_HPP
#define LAB2_PCH_HPP

#pragma once


#include <stdexcept>
#include <functional>
#include <optional>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

// UINT32_MAX and UINT64 needed
#include <cstdint>
// strcmp
#include <cstring>

#include <algorithm>
#include <vector>
#include <array>
#include <unordered_set>
#include <map>
#include <list>


// GLFW includes Vulkan with the following macro.
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm.hpp"

#include "vulkan/vulkan.hpp"


#endif
