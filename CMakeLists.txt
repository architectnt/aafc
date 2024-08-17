cmake_minimum_required(VERSION 3.11) # 3.11 because of the tools
project(aafc VERSION 3 LANGUAGES C CXX)

option(BUILD_TOOLS "Build AAFC tools" OFF)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

function(msvc_set_flags)
	set(MSVC_FLAGSREL "/Oxyi /Ob2 /OPT:REF /fp:fast /MD")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Zi")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${MSVC_FLAGSREL}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${MSVC_FLAGSREL}")
endfunction()

function(gcc_clang_set_flags)
    set(GCCCLANG_FLAGSREL "-s -Ofast -ffast-math -flto -finline-functions")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -fPIC")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${GCCCLANG_FLAGSREL}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fPIC")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${GCCCLANG_FLAGSREL}")
endfunction()

if("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
    if ("${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC")
        msvc_set_flags()
    else()
        gcc_clang_set_flags()
    endif()
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU" )
    gcc_clang_set_flags()
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC" )
    msvc_set_flags()
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(aafcheadr 
	include/aafc.h
	include/aafc/common.h
	include/aafc/helpers.h
)

set(AAFC_SRC
	${aafcheadr}
	src/aafc/aafc.c
	src/aafc/lib.h

	src/aafc/common.c
	src/aafc/submodules/filetable.h
	src/aafc/submodules/filetable.c

	src/aafc/submodules/modifiers.h
	src/aafc/submodules/modifiers.c

	src/aafc/submodules/pcm.h
	src/aafc/submodules/pcm.c

	src/aafc/submodules/adpcm.h
	src/aafc/submodules/adpcm.c

	src/aafc/submodules/dpcm.h
	src/aafc/submodules/dpcm.c

	src/aafc/submodules/sfpcm.h
	src/aafc/submodules/sfpcm.c

	src/aafc/submodules/ulaw.h
	src/aafc/submodules/ulaw.c
)

if(BUILD_SHARED_LIBS)
	message(STATUS "Building as shared library")
	add_library(aafc SHARED ${AAFC_SRC})
	target_compile_definitions(aafc PRIVATE BUILDING_SHARED_LIBRARY)
else()
	message(STATUS "Building as static library")
	add_library(aafc STATIC ${AAFC_SRC})
endif()

add_library(AAFC::aafc ALIAS aafc)

target_include_directories(aafc
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	PRIVATE
		src
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>
)

set_target_properties(aafc PROPERTIES
	PUBLIC_HEADER "${aafcheadr}"
)

if(BUILD_TOOLS) # insane
	include(FetchContent)
	set(BUILD_SHARED_LIBS OFF)

	FetchContent_Declare(
		portaudio
		GIT_REPOSITORY "https://github.com/PortAudio/portaudio.git"
		GIT_TAG "v19.7.0"
		EXCLUDE_FROM_ALL
	)

	FetchContent_Declare(
		libsndfile
		GIT_REPOSITORY "https://github.com/libsndfile/libsndfile.git"
		GIT_TAG "1.2.2"
		EXCLUDE_FROM_ALL
	)

	FetchContent_MakeAvailable(portaudio)
	FetchContent_MakeAvailable(libsndfile)

	set(toolsinc
		include/tests.h
		"include/tests/fileutils.h"
		include/tests/libaafcfunc.h
	)

	add_executable(plyr ${toolsinc} ${aafcheadr} src/tests/aafc_player/aafc_player.cpp)
	target_link_libraries(plyr PRIVATE portaudio_static)

	add_executable(aud2aafc ${toolsinc} ${aafcheadr} src/tests/aafc_mediaconverter/aafc_mediaconverter.cpp)
	target_link_libraries(aud2aafc PRIVATE SndFile::sndfile)

	add_executable(aftcreator ${toolsinc} ${aafcheadr} src/tests/aftcreator/aftcreator.cpp)

	include_directories(${aafc_SOURCE_DIR}/include)
	include_directories(${PROJECT_BINARY_DIR}/include)

	set(tools plyr aftcreator aud2aafc)
endif()

if(WIN32)
  if(BUILD_TOOLS)
    target_sources(plyr PRIVATE ${PROJECT_SOURCE_DIR}/resources/win32/plyr_resource.rc)
	target_sources(aud2aafc PRIVATE ${PROJECT_SOURCE_DIR}/resources/win32/aud2aafc_resource.rc)
  endif()
  target_sources(aafc PRIVATE ${PROJECT_SOURCE_DIR}/resources/win32/aafc_resource.rc)
endif()


install(TARGETS aafc ${tools}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)