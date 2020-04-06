#ifndef GRAPHICS_PCH_HPP
#define GRAPHICS_PCH_HPP


// Language "features"
#include <stdexcept>
#include <functional>
#include <optional>

// Streams, IO, strings
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cctype>
#include <regex>

// UINT32_MAX and UINT64 needed
#include <cstdint>

// Collections and such
#include <algorithm>
#include <vector>
#include <array>
#include <unordered_set>
#include <map>
#include <set>

// Sleep
#include <chrono>
#include <thread>


// GLFW includes vulkan.h with the following macro.
// needed to create a drawing drawing_surface for Vulkan.
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

// Math
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm.hpp"

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include "vulkan/vulkan.hpp"


#endif
