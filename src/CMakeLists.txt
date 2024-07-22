if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_OSX_ARCHITECTURES arm64;x86_64)
endif()

set(MSVC_FLAGSREL "/Oxyi /Ob2 /OPT:REF /fp:fast /MD")
set(GCCCLANG_FLAGSREL "-s -Ofast -ffast-math -flto -finline-functions")

if("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
    if ("${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17 /MP")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${MSVC_FLAGSREL}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fPIC -std=c++17")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${GCCCLANG_FLAGSREL}")
    endif()
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU" )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fPIC -std=c++17")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${GCCCLANG_FLAGSREL}")
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC" )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17 /MP")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${MSVC_FLAGSREL}")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(aafc)
add_subdirectory(tests)
set(commands aafc_shared plyr aud2aafc aftcreator)

if(WIN32)
    set(PLATFORM_FOLDER "win")
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_FOLDER "linux")
elseif(APPLE)
    set(PLATFORM_FOLDER "macos")
else()
    set(PLATFORM_FOLDER "that_one_platform_LOL")
    message("what the")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${PLATFORM_FOLDER})
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/bin/${PLATFORM_FOLDER})
install(TARGETS ${commands}
    RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/bin/${PLATFORM_FOLDER}
    LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/bin/${PLATFORM_FOLDER}
    ARCHIVE DESTINATION ""
)

install(TARGETS aafc_static
    ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/bin/${PLATFORM_FOLDER}
    LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/bin/${PLATFORM_FOLDER}
    RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/bin/${PLATFORM_FOLDER}
)