file(GLOB_RECURSE submodules "submodules/*.c" "submodules/*.h")

add_library(aafc_shared SHARED "aafc.c" "lib.h" ${submodules})
set_target_properties(aafc_shared PROPERTIES OUTPUT_NAME "aafc")

add_library(aafc_static STATIC "aafc.c" "lib.h" ${submodules})
target_compile_definitions(aafc_shared PRIVATE BUILDING_SHARED_LIBRARY)
set_target_properties(aafc_static PROPERTIES OUTPUT_NAME "staafc")

include_directories(
	${aafc_SOURCE_DIR}/include
	${aafc_SOURCE_DIR}/include/aafc)
include_directories(
	${PROJECT_BINARY_DIR}/include
	${PROJECT_BINARY_DIR}/include/aafc)