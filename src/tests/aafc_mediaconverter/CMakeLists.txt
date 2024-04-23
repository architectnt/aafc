include(FetchContent)

FetchContent_Declare(
  libsndfile
  GIT_REPOSITORY "https://github.com/libsndfile/libsndfile.git"
  GIT_TAG "1.2.2"
  EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(libsndfile)

add_executable(aud2aafc aafc_mediaconverter.cpp)
target_link_libraries(aud2aafc PRIVATE SndFile::sndfile)

include_directories(${aafc_SOURCE_DIR}/include)
include_directories(${PROJECT_BINARY_DIR}/include)