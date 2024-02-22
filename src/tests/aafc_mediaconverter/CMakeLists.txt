include(FetchContent)

FetchContent_Declare(
  libsndfile
  GIT_REPOSITORY "https://github.com/libsndfile/libsndfile.git"
  GIT_TAG "1.2.2"
)


set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
set(INSTALL_DOCS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(libsndfile)

add_executable(aud2aafc aafc_mediaconverter.cpp)
target_link_libraries(aud2aafc PRIVATE SndFile::sndfile)

include_directories(${aafc_SOURCE_DIR}/include)
include_directories(${PROJECT_BINARY_DIR}/include)