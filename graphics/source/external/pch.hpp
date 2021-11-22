#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedMacroInspection"

#ifndef IRGLAB_PCH_HPP
#define IRGLAB_PCH_HPP



// C++ Standard Library

// Language features
#include <stdexcept>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>
#include <any>
#include <variant>

// Streams, IO, strings
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cctype>
#include <regex>

// UINT32_MAX and UINT64 needed
#include <cstdint>

// STL and algorithms
#include <algorithm>
#include <vector>
#include <array>
#include <unordered_set>
#include <map>
#include <set>

// Threading
#include <chrono>
#include <thread>



// GLFW - Windows and IO
// GLFW includes vulkan.h with the following macro.
// Needed to create a drawing surface for Vulkan.
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"


// GLM - Math
#define GLM_FORCE_RADIANS
#define GLM_FORCE_SIZE_T_LENGTH
#if defined(NDEBUG)
#define GLM_FORCE_INTRINSICS
#define GLM_CONFIG_ALIGNED_GENTYPES
#endif
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"


// Vulkan - Graphics
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include "vulkan/vulkan.hpp"


#endif

#pragma clang diagnostic pop
