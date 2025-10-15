cmake_minimum_required(VERSION 3.15)

# 项目名称和语言
project(basic_blink C CXX ASM)

# 设置C标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 设置C++标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找工具链文件
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../CMake/arm-none-eabi.cmake)

# 设置构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# 编译定义
add_compile_definitions(
    STM32F103xE
    USE_HAL_DRIVER
)

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy
)

# 源文件
set(C_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/system_stm32f1xx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_exti.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pwr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_adc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_adc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_tim.c
)

set(ASM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Startup/startup_stm32f103rctx.s
)

# 可执行文件
add_executable(${PROJECT_NAME}.elf ${C_SOURCES} ${ASM_SOURCES})

# 链接器脚本
set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F103RCTX_FLASH.ld)
target_link_options(${PROJECT_NAME}.elf PRIVATE
    -Wl,-T${LINKER_SCRIPT}
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map
    --specs=nosys.specs
    --specs=nano.specs
    -static
    -u_printf_float
    -u_scanf_float
    -Wl,--start-group -lc -lm -Wl,--end-group
)

# 设置输出目录
set_target_properties(${PROJECT_NAME}.elf PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# 添加自定义目标来生成hex和bin文件
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
    COMMENT "Generating HEX and BIN files"
)

# 安装规则
install(TARGETS ${PROJECT_NAME}.elf
    RUNTIME DESTINATION bin
)

install(FILES ${PROJECT_NAME}.hex ${PROJECT_NAME}.bin
    DESTINATION bin
)
