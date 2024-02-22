include(FetchContent)

FetchContent_Declare(
  portaudio
  GIT_REPOSITORY "https://github.com/PortAudio/portaudio.git"
  GIT_TAG "v19.7.0"
)

set(PA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(PA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(PA_INSTALL_LIB OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(portaudio)

add_executable(plyr aafc_player.cpp)
target_link_libraries(plyr PRIVATE portaudio_static)

include_directories(${aafc_SOURCE_DIR}/include)
include_directories(${PROJECT_BINARY_DIR}/include)