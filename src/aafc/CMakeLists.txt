file(GLOB_RECURSE submodules "submodules/*.cpp" "submodules/*.h")

add_library(aafc SHARED aafc.cpp ${submodules})

include_directories(${aafc_SOURCE_DIR}/include)
include_directories(${PROJECT_BINARY_DIR}/include)