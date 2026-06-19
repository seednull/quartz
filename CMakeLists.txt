cmake_minimum_required(VERSION 3.10)
project(quartz VERSION 1.0.0)

# ==================================================================================================
# Global options
# ==================================================================================================
option(QUARTZ_BUILD_SAMPLES "Build samples" TRUE)
option(QUARTZ_BUILD_WITH_WASAPI "Build with WASAPI backend" TRUE)
option(QUARTZ_BUILD_WITH_DIRECTSOUND "Build with Direct Sound backend" TRUE)
option(QUARTZ_BUILD_WITH_WEBAUDIO "Build with WebAudio backend" TRUE)

# ==================================================================================================
# Global Variables
# ==================================================================================================
enable_language(C)

set(QUARTZ_DIR_THIRDPARTY ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)
set(QUARTZ_DIR_API ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(QUARTZ_DIR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
	set(QUARTZ_PLATFORM windows)
	set(QUARTZ_ABI x64)
endif()

if (EMSCRIPTEN)
	set(QUARTZ_PLATFORM emscripten)
	set(QUARTZ_ABI wasm)
	set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

message("Platform: ${QUARTZ_PLATFORM}")
message("ABI: ${QUARTZ_ABI}")

# ==================================================================================================
# General compile flags
# ==================================================================================================
if (WIN32)
	set(CMAKE_C_FLAGS "/W4 /external:W3 /wd4996 /D_CRT_SECURE_NO_WARNINGS")
	set(CMAKE_CXX_FLAGS "/W4 /external:W3 /wd4996 /D_CRT_SECURE_NO_WARNINGS /EHsc /GR-")
endif()

# ==================================================================================================
# General includes
# ==================================================================================================

# ==================================================================================================
# General libraries
# ==================================================================================================

# ==================================================================================================
# Sub-projects
# ==================================================================================================
add_subdirectory(src)

if (QUARTZ_BUILD_SAMPLES)
	add_subdirectory(samples/01_enumerate_devices)
	add_subdirectory(samples/02_buffers)
	add_subdirectory(samples/03_resamplers)
endif()
