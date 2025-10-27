# Copyright (C) 2025 Anastasiia Stepanova <asiiapine@gmail.com>
# Distributed under the terms of the GPL v3 license, available in the file LICENSE.

# Include guard
if(SRC_PERIPHERAL_IWDG_DIR)
    return()
endif()
set(SRC_PERIPHERAL_IWDG_DIR ${CMAKE_CURRENT_LIST_DIR})

list(APPEND APPLICATION_SOURCES
    ${PLATFORM_DIR}/iwdg.cpp
)
