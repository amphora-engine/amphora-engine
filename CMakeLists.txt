cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	file(TO_CMAKE_PATH $ENV{VCPKG_ROOT} VCPKG_ROOT)
	set(CMAKE_TOOLCHAIN_FILE ${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
	set(VCPKG_LIBRARY_LINKAGE static)
endif()

if(APPLE)
    set(CMAKE_THREAD_LIBS_INIT "-lpthread")
endif()

set(AMPHORA_ENGINE_SUFFIX "vanilla")
set(AMPHORA_ENGINE_VERSION_STRING "v0.2.1-alpha")

project(amphora LANGUAGES C)

set(CMAKE_C_STANDARD 17)

add_compile_definitions(ENGINE_SUFFIX="${AMPHORA_ENGINE_SUFFIX}")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_definitions(VERSION_STRING="${AMPHORA_ENGINE_VERSION_STRING}-dbg")
else()
	add_compile_definitions(VERSION_STRING="${AMPHORA_ENGINE_VERSION_STRING}")
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_definitions(DEBUG)
	if(WIN32)
		string(APPEND CMAKE_C_FLAGS " /W4 /WX")
	else()
		string(APPEND CMAKE_C_FLAGS " -Wall -Wpedantic -Wextra")
	endif()
endif()

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
if(LINUX)
	find_package(unofficial-libuuid CONFIG REQUIRED)
endif()

include_directories(
	include
	${SDL2_INCLUDE_DIRS}
)

add_subdirectory(src)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set_target_properties(amphora PROPERTIES OUTPUT_NAME "${CMAKE_PROJECT_NAME}-${AMPHORA_ENGINE_SUFFIX}-${AMPHORA_ENGINE_VERSION_STRING}-dbg")
else()
	set_target_properties(amphora PROPERTIES OUTPUT_NAME "${CMAKE_PROJECT_NAME}-${AMPHORA_ENGINE_SUFFIX}-${AMPHORA_ENGINE_VERSION_STRING}")
endif()
