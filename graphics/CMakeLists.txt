cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(irglab)
set(CMAKE_CXX_STANDARD 17)


if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
else()
    message(WARNING "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first.")
endif()


add_executable(irglab source/main.cpp)
conan_target_link_libraries(irglab ${CONAN_LIBS})
target_include_directories(irglab
        PRIVATE
            source)
target_precompile_headers(irglab
        PRIVATE
            source/external/pch.hpp)
