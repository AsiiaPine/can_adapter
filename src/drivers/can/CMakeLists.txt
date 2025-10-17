# Copyright (C) 2025 Anastasiia Stepanova <asiiapine@gmail.com>
# Distributed under the terms of the GPL v3 license, available in the file LICENSE.

# Include guard
if(SRC_DRIVERS_CAN_DIR)
    return()
endif()
set(SRC_DRIVERS_CAN_DIR ${CMAKE_CURRENT_LIST_DIR})

list(APPEND APPLICATION_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/can.c
)

include(${ROOT_DIR}/Src/peripheral/fdcan/CMakeLists.txt)
