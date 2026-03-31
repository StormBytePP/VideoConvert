cmake_minimum_required(VERSION 3.30)

# set the project name and version
project("StormByte Video Convert"
	VERSION 1.5.5.1
	DESCRIPTION "This is a program to handle automatic ffmpeg conversions between downloaded videos in order to format them to be properly stored"
	HOMEPAGE_URL "https://github.com/StormBytePP/StormByte-videoconvert"
)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -Wall -Wextra -Werror -Wnon-virtual-dtor -pedantic -pedantic-errors -g3 -fno-inline -O0")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	message(STATUS "Setting build type to Release as none was specified.")
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Encoder support
option(ENABLE_HEVC 			"Enable HEVC/H265 encoder"					ON)
option(ENABLE_AAC 			"Enable AAC encoder" 						ON)
option(ENABLE_FDKAAC 		"Enable Fraunhoffer AAC encoder" 			OFF)
option(ENABLE_AC3 			"Enable AC3 encoder" 						ON)
option(ENABLE_EAC3 			"Enable Enhanced AC3 encoder" 				ON)
option(ENABLE_OPUS 			"Enable Opus encoder"						ON)
option(ENABLE_STATIC		"Build static libs along with shared libs"	OFF)

if (ENABLE_HEVC)
	add_compile_definitions("ENABLE_HEVC")
endif()

if (ENABLE_AAC)
	add_compile_definitions("ENABLE_AAC")
endif()

if (ENABLE_FDKAAC)
	add_compile_definitions("ENABLE_FDKAAC")
endif()

if (ENABLE_AC3)
	add_compile_definitions("ENABLE_AC3")
endif()

if (ENABLE_EAC3)
	add_compile_definitions("ENABLE_EAC3")
endif()

if (ENABLE_OPUS)
	add_compile_definitions("ENABLE_OPUS")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "11.0.0")
    message(FATAL_ERROR "GCC needs to be at least version 11.0.0 but found ${CMAKE_CXX_COMPILER_VERSION}")
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "13.0.0")
    message(FATAL_ERROR "Clang needs to be at least version 13.0.0 but found ${CMAKE_CXX_COMPILER_VERSION}")
  endif()
endif()

find_package (SQLite3 REQUIRED)
find_package (Config++ REQUIRED)
find_package(Boost REQUIRED)
find_package (Git REQUIRED)
find_package (JsonCpp REQUIRED)

find_program(FFMPEG_EXECUTABLE ffmpeg REQUIRED)
mark_as_advanced(FFMPEG_EXECUTABLE)
find_program(FFPROBE_EXECUTABLE ffprobe REQUIRED)
mark_as_advanced(FFPROBE_EXECUTABLE)

# Manually set VERSION_UPDATE_FROM_GIT to true to update version from last tag
include(${PROJECT_SOURCE_DIR}/cmake/GetVersionFromGitTag.cmake)

# Project and version info
file(CONFIGURE OUTPUT ${CMAKE_BINARY_DIR}/generated/definitions.h CONTENT "#pragma once

#define PROGRAM_NAME			\"StormByte-VideoConvert\"
#define PROGRAM_DESCRIPTION		\"${PROJECT_DESCRIPTION}\"
#define PROGRAM_AUTHOR			\"David C. Manuelda a.k.a StormByte <stormbyte@gmail.com>\"
#define PROGRAM_VERSION			\"${${PROJECT_NAME}_VERSION_STRING}\"
#define PROJECT_URI				\"${PROJECT_HOMEPAGE_URL}\"
#define COMPILER_NAME			\"${CMAKE_CXX_COMPILER_ID}\"
#define COMPILER_VERSION		\"${CMAKE_CXX_COMPILER_VERSION}\"
")

string(TOUPPER ${CMAKE_BUILD_TYPE} build_type)
if (build_type STREQUAL DEBUG)
	file(APPEND ${CMAKE_BINARY_DIR}/generated/definitions.h "#define COMPILER_FLAGS			\"${CMAKE_CXX_FLAGS_DEBUG}\"")
elseif(build_type STREQUAL RELWITHDEBINFO)
	file(APPEND ${CMAKE_BINARY_DIR}/generated/definitions.h "#define COMPILER_FLAGS			\"${CMAKE_CXX_FLAGS_RELWITHDEBINFO}\"")
elseif(build_type STREQUAL MINSIZEREL)
	file(APPEND ${CMAKE_BINARY_DIR}/generated/definitions.h "#define COMPILER_FLAGS			\"${CMAKE_CXX_FLAGS_MINSIZEREL}\"")
elseif(build_type STREQUAL RELEASE)
	file(APPEND ${CMAKE_BINARY_DIR}/generated/definitions.h "#define COMPILER_FLAGS			\"${CMAKE_CXX_FLAGS_RELEASE}\"")
else()
	file(APPEND ${CMAKE_BINARY_DIR}/generated/definitions.h "#define COMPILER_FLAGS			\"${CMAKE_CXX_FLAGS}\"")
endif()

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/generated)
include_directories(${CMAKE_BINARY_DIR}/generated)
include_directories(lib)
include_directories(frontend)
add_subdirectory(lib)
add_subdirectory(frontend)
