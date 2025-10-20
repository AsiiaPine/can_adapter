# Copyright (C) 2025 Anastasiia Stepanova <asiiapine@gmail.com>
# Distributed under the terms of the GPL v3 license, available in the file LICENSE.

cmake_minimum_required(VERSION 3.15.3)

# Pathes
set(ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(SRC_DIR ${ROOT_DIR}/src)
set(SRC_COMMON_DIR ${SRC_DIR}/common)
set(SRC_DRIVERS_DIR ${SRC_DIR}/drivers)
set(CMAKE_DIR ${ROOT_DIR}/cmake)


set(stm32cubeMxProjectPath  ${ROOT_DIR}/stm_ioc)
FILE(GLOB ldFile            ${stm32cubeMxProjectPath}/*_FLASH.ld)
FILE(GLOB coreSources       ${stm32cubeMxProjectPath}/Core/Src/*)
FILE(GLOB driversSources    ${stm32cubeMxProjectPath}/Drivers/*/*/*.c
                            ${stm32cubeMxProjectPath}/USB_Device/*/*.c
                            ${stm32cubeMxProjectPath}/Middlewares/*/*/*/*/*.c
                            ${stm32cubeMxProjectPath}/Middlewares/*/*/*/*/*/*.c
                            )
FILE(GLOB startupFile       ${stm32cubeMxProjectPath}/*.s
                            ${stm32cubeMxProjectPath}/Core/Startup/*.s
)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug mode is enabled")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g -DDEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -DDEBUG")
endif()

set(CMAKE_EXE_LINKER_FLAGS "-T${ldFile} -Wl,-Map=${PROJECT_NAME}.map,--cref")
include(${CMAKE_DIR}/toolchain-stm32g0b1.cmake)

# Set build dir based on hardware version and protocol
set(BUILD_ROOT_DIR ${ROOT_DIR}/build)
set(BUILD_SRC_DIR ${BUILD_ROOT_DIR}/src)
set(BUILD_OBJ_DIR ${BUILD_ROOT_DIR}/obj)
set(APPLICATION_DIR ${ROOT_DIR}/src/application)

list(APPEND APPLICATION_HEADERS ${ROOT_DIR}/src ${APPLICATION_DIR})
include(${APPLICATION_DIR}/CMakeLists.txt)

# Include peripheral sources (temporarily disabled due to HAL compatibility issues)
include(${ROOT_DIR}/src/peripheral/usb/CMakeLists.txt)
include(${ROOT_DIR}/src/peripheral/fdcan/CmakeLists.txt)

# Project
project(node CXX C ASM)

set(EXECUTABLE ${PROJECT_NAME})
add_executable(${EXECUTABLE}
    ${APPLICATION_SOURCES}

    ${coreSources}
    ${driversSources}
    ${startupFile}
)

target_include_directories(${EXECUTABLE} PRIVATE
    ${BUILD_SRC_DIR}
    ${APPLICATION_HEADERS}
    ${ROOT_DIR}/src/common

    ${stm32cubeMxProjectPath}/Core/Inc
    ${stm32cubeMxProjectPath}/Drivers/CMSIS/Include

    ${stm32cubeMxProjectPath}/Drivers/CMSIS/Device/ST/STM32F1xx/Include
    ${stm32cubeMxProjectPath}/Drivers/STM32F1xx_HAL_Driver/Inc
    ${stm32cubeMxProjectPath}/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy
    ${stm32cubeMxProjectPath}/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc/
    ${stm32cubeMxProjectPath}/Middlewares/ST/STM32_USB_Device_Library/Core/Inc/

    ${stm32cubeMxProjectPath}/Drivers/CMSIS/Device/ST/STM32G0xx/Include
    ${stm32cubeMxProjectPath}/Drivers/STM32G0xx_HAL_Driver/Inc
    ${stm32cubeMxProjectPath}/Drivers/STM32G0xx_HAL_Driver/Inc/Legacy
    ${stm32cubeMxProjectPath}/USB_Device/App
    ${stm32cubeMxProjectPath}/USB_Device/Target
)

add_custom_command(TARGET ${EXECUTABLE}
    POST_BUILD
    COMMAND ${CMAKE_SIZE} ${EXECUTABLE}
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE} ${BUILD_OBJ_DIR}/${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE} ${BUILD_OBJ_DIR}/${PROJECT_NAME}.bin
    COMMAND ${CMAKE_OBJCOPY} -I binary -O elf32-little ${EXECUTABLE} ${BUILD_OBJ_DIR}/${PROJECT_NAME}.elf
)
