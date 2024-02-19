cmake_minimum_required (VERSION 3.11)
project (aafc VERSION 2 LANGUAGES C CXX)
add_subdirectory(src)
add_subdirectory(resources)
add_subdirectory(include)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)