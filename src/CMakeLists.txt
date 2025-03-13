cmake_minimum_required(VERSION 3.10)
set(TARGET quartz)

# ==================================================================================================
# Options
# ==================================================================================================

# ==================================================================================================
# Variables
# ==================================================================================================
set(QUARTZ_HAS_WASAPI FALSE)
set(QUARTZ_HAS_DIRECTSOUND FALSE)
set(QUARTZ_PLATFORM_DEFINES "")
set(QUARTZ_TARGET_TYPE SHARED)

if (WIN32)
	set(QUARTZ_HAS_WASAPI ${QUARTZ_BUILD_WITH_WASAPI})
	set(QUARTZ_HAS_DIRECTSOUND ${QUARTZ_BUILD_WITH_DIRECSOUND})
	set(QUARTZ_PLATFORM_DEFINES QUARTZ_PLATFORM_WIN32 QUARTZ_SHARED_LIBRARY NOMINMAX)
endif()


if (EMSCRIPTEN)
	set(QUARTZ_PLATFORM_DEFINES QUARTZ_PLATFORM_WEB)
	set(QUARTZ_TARGET_TYPE STATIC)
endif()

# ==================================================================================================
# Dependencies
# ==================================================================================================

# ==================================================================================================
# Sources
# ==================================================================================================
file(GLOB SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/null/*.c
)

file(GLOB HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/null/*.h
)

file (GLOB PUBLIC_HEADERS
	${QUARTZ_DIR_API}/quartz.h
)

if (QUARTZ_HAS_WASAPI)
	file(GLOB WASAPI_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/wasapi/*.c
	)

	file(GLOB WASAPI_HEADERS
		${CMAKE_CURRENT_SOURCE_DIR}/wasapi/*.h
	)

	list(APPEND SOURCES ${WASAPI_SOURCES})
	list(APPEND HEADERS ${WASAPI_HEADERS})
	list(APPEND QUARTZ_PLATFORM_DEFINES QUARTZ_BACKEND_WASAPI)
endif()

if (QUARTZ_HAS_DIRECTSOUND)
	file(GLOB DIRECTSOUND_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/directsound/*.c
	)

	file(GLOB DIRECTSOUND_HEADERS
		${CMAKE_CURRENT_SOURCE_DIR}/directsound/*.h
	)

	list(APPEND SOURCES ${DIRECTSOUND_SOURCES})
	list(APPEND HEADERS ${DIRECTSOUND_HEADERS})
	list(APPEND QUARTZ_PLATFORM_DEFINES QUARTZ_BACKEND_DIRECTSOUND)
endif()

# ==================================================================================================
# Target
# ==================================================================================================
add_library(${TARGET} ${QUARTZ_TARGET_TYPE} ${SOURCES} ${HEADERS} ${PUBLIC_HEADERS})

set_target_properties(${TARGET} PROPERTIES DEBUG_POSTFIX d)
set_target_properties(${TARGET} PROPERTIES PUBLIC_HEADER ${PUBLIC_HEADERS})

if (WIN32)
	set_target_properties(${TARGET} PROPERTIES PREFIX "")
	set_target_properties(${TARGET} PROPERTIES IMPORT_PREFIX "")
	set_target_properties(${TARGET} PROPERTIES IMPORT_SUFFIX ".lib")
else()
	set_target_properties(${TARGET} PROPERTIES IMPORT_SUFFIX ".a")
endif()

# ==================================================================================================
# Includes
# ==================================================================================================
target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${QUARTZ_DIR_API})

# ==================================================================================================
# Preprocessor
# ==================================================================================================
target_compile_definitions(${TARGET} PRIVATE ${QUARTZ_PLATFORM_DEFINES})

# ==================================================================================================
# Linker
# ==================================================================================================

# ==================================================================================================
# Libraries
# ==================================================================================================

# ==================================================================================================
# Installation
# ==================================================================================================
install(
	TARGETS ${TARGET}
	EXPORT ${TARGET}
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	INCLUDES DESTINATION include
	PUBLIC_HEADER DESTINATION include
)
