include(${CMAKE_DIR}/toolchain-stm32g0b1.cmake)

set(stm32cubeMxProjectPath  ${ROOT_DIR}/stm_ioc)
FILE(GLOB ldFile            ${stm32cubeMxProjectPath}/*_FLASH.ld)

# Set linker script and map file
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T${ldFile} -Wl,-Map=${PROJECT_NAME}.map,--cref")
FILE(GLOB coreSources       ${stm32cubeMxProjectPath}/Core/Src/*)
FILE(GLOB driversSources    ${stm32cubeMxProjectPath}/Drivers/*/*/*.c
                            ${stm32cubeMxProjectPath}/USB_Device/*/*.c
                            ${stm32cubeMxProjectPath}/Middlewares/*/*/*/*/*.c
                            ${stm32cubeMxProjectPath}/Middlewares/*/*/*/*/*/*.c
                            ${stm32cubeMxProjectPath}/Middlewares/*/*/*/*/*/*/*.c
                            )
FILE(GLOB startupFile       ${stm32cubeMxProjectPath}/*.s
                            ${stm32cubeMxProjectPath}/Core/Startup/*.s
)

add_executable(${EXECUTABLE}
    ${APPLICATION_SOURCES}

    ${coreSources}
    ${driversSources}
    ${startupFile}
    ${ROOT_DIR}/src/platform_specific/stm32g0b1/systemcalls.c
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
    ${stm32cubeMxProjectPath}/Middlewares/ST/STM32_USB_Device_Library/Class/CompositeBuilder/Inc/
    ${stm32cubeMxProjectPath}/Middlewares/ST/STM32_USB_Device_Library/Core/Inc/

    ${stm32cubeMxProjectPath}/Drivers/CMSIS/Device/ST/STM32G0xx/Include
    ${stm32cubeMxProjectPath}/Drivers/STM32G0xx_HAL_Driver/Inc
    ${stm32cubeMxProjectPath}/Drivers/STM32G0xx_HAL_Driver/Inc/Legacy
    ${stm32cubeMxProjectPath}/USB_Device/App
    ${stm32cubeMxProjectPath}/USB_Device/Target
)
