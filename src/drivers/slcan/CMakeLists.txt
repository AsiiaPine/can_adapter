# Copyright (C) 2025 Anastasiia Stepanova <asiiapine@gmail.com>
# Distributed under the terms of the GPL v3 license, available in the file LICENSE.

# Include guard
if(SRC_DRIVERS_CAN_DIR)
    return()
endif()
set(SRC_DRIVERS_CAN_DIR ${CMAKE_CURRENT_LIST_DIR})

list(APPEND APPLICATION_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/slcan.cpp
)

# Include peripheral sources (temporarily disabled due to HAL compatibility issues)
include(${ROOT_DIR}/src/peripheral/usb/CMakeLists.txt)
include(${ROOT_DIR}/src/peripheral/fdcan/CMakeLists.txt)
