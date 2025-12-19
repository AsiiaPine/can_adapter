# Copyright (C) 2025 Anastasiia Stepanova <asiiapine@gmail.com>
# Distributed under the terms of the GPL v3 license, available in the file LICENSE.

# Include guard
if(SRC_PERIPHERAL_USB_DIR)
    return()
endif()
set(SRC_PERIPHERAL_USB_DIR ${CMAKE_CURRENT_LIST_DIR})

list(APPEND APPLICATION_SOURCES
    ${PLATFORM_DIR}/usb.cpp
    ${PLATFORM_DIR}/usb_stubs.cpp
)
function(generate_random_pid)
    # Generate a random number in the range of 0 to 65535 (16-bit PID)
    execute_process(
        COMMAND bash -c "echo $((RANDOM % 65536))"
        OUTPUT_VARIABLE UNIQUE_PID
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(UNIQUE_PID ${UNIQUE_PID} PARENT_SCOPE)
endfunction()
# Initialize random seed

generate_random_pid()

add_compile_definitions(
    -DUSBD_VID=1155
    -DUSBD_PID=${UNIQUE_PID}
    -DUSBD_LANGID_STRING=1033
    -DUSBD_INTERFACE_STRING="CDC Interface"
    -DUSBD_PRODUCT_STRING="STM32 CanAdapter"
    -DUSBD_CONFIGURATION_STRING="CDC Config"
    -DUSBD_MANUFACTURER_STRING="RaccoonLab"
    -DREDEFINED_USB_DESC
)
