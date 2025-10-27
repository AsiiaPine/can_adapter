# Copyright (C) 2025 Anastasiia Stepanova <asiiapine@gmail.com>
# Distributed under the terms of the GPL v3 license, available in the file LICENSE.

cmake_minimum_required(VERSION 3.15.3)

# Pathes
set(ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(SRC_DIR ${ROOT_DIR}/src)
set(SRC_COMMON_DIR ${SRC_DIR}/common)
set(SRC_DRIVERS_DIR ${SRC_DIR}/drivers)
set(CMAKE_DIR ${ROOT_DIR}/cmake)

if (NOT PLATFORM)
    message(FATAL_ERROR "PLATFORM is not set")
endif()

if (PLATFORM STREQUAL "STM32G0B1")
    set(PLATFORM_DIR ${ROOT_DIR}/src/platform_specific/stm32g0b1)
elseif(PLATFORM STREQUAL "UBUNTU")
    set(PLATFORM_DIR ${ROOT_DIR}/src/platform_specific/ubuntu)
else()
    message(FATAL_ERROR "Unknown platform")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug mode is enabled")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g -DDEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -DDEBUG")
endif()

# Set build dir based on hardware version and protocol
set(BUILD_ROOT_DIR ${ROOT_DIR}/build/${PLATFORM})
set(BUILD_SRC_DIR ${BUILD_ROOT_DIR}/src)
set(BUILD_OBJ_DIR ${BUILD_ROOT_DIR}/obj)
set(APPLICATION_DIR ${ROOT_DIR}/src/application)

list(APPEND APPLICATION_HEADERS ${ROOT_DIR}/src ${APPLICATION_DIR})

# Project
project(node CXX C ASM)

set(EXECUTABLE ${PROJECT_NAME})

# Linker flags will be set in platform-specific CMakeLists.txt

include(${APPLICATION_DIR}/CMakeLists.txt)

if(PLATFORM STREQUAL "UBUNTU")
else()
    add_custom_command(TARGET ${EXECUTABLE}
        POST_BUILD
        COMMAND ${CMAKE_SIZE} ${EXECUTABLE}
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE} ${BUILD_OBJ_DIR}/${PROJECT_NAME}.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE} ${BUILD_OBJ_DIR}/${PROJECT_NAME}.bin
        COMMAND ${CMAKE_OBJCOPY} -I binary -O elf32-little ${EXECUTABLE} ${BUILD_OBJ_DIR}/${PROJECT_NAME}.elf
    )
endif()
