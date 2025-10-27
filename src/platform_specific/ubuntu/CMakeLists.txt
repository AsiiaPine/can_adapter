# Copyright (C) 2025 Anastasiia Stepanova <asiiapine@gmail.com>
# Distributed under the terms of the GPL v3 license, available in the file LICENSE.
include(${CMAKE_DIR}/toolchain-ubuntu.cmake)

# Include guard
list(APPEND PLATFORM_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/main.c
)

list(APPEND APPLICATION_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/fdcan.cpp
    ${CMAKE_CURRENT_LIST_DIR}/led.cpp
    ${CMAKE_CURRENT_LIST_DIR}/iwdg.cpp
    ${CMAKE_CURRENT_LIST_DIR}/usb.cpp
    ${CMAKE_CURRENT_LIST_DIR}/usb_stubs.cpp
)

add_executable(${EXECUTABLE}
                ${PLATFORM_SOURCES}
                ${APPLICATION_SOURCES})

target_include_directories(${EXECUTABLE} PRIVATE
    ${BUILD_SRC_DIR}
    ${APPLICATION_HEADERS}
    ${ROOT_DIR}/src/common
    ${CMAKE_CURRENT_LIST_DIR}
)
