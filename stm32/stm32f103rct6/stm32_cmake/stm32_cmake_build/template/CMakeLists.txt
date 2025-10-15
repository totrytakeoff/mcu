# STM32 CMake 项目模板
# 适用于 STM32F1xx 系列

# 设置最低 CMake 版本要求
cmake_minimum_required(VERSION 3.16)

# 项目名称和语言
project(stm32_template C CXX ASM)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m3 -mthumb -ffunction-sections -fdata-sections")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=cortex-m3 -mthumb -ffunction-sections -fdata-sections")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -mcpu=cortex-m3 -mthumb")

# 添加编译定义
add_compile_definitions(DEBUG USE_HAL_DRIVER STM32F103xE)

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Include
)

# 汇编源文件
set(ASM_SOURCES
    ${CMAKE_SOURCE_DIR}/Core/Startup/startup_stm32f103rctx.s
)

# C 源文件
set(C_SOURCES
    ${CMAKE_SOURCE_DIR}/Core/Src/main.c
    ${CMAKE_SOURCE_DIR}/Core/Src/stm32f1xx_hal_msp.c
    ${CMAKE_SOURCE_DIR}/Core/Src/stm32f1xx_it.c
    ${CMAKE_SOURCE_DIR}/Core/Src/system_stm32f1xx.c
    ${CMAKE_SOURCE_DIR}/Core/Src/syscalls.c
    ${CMAKE_SOURCE_DIR}/Core/Src/sysmem.c
)

# C++ 源文件
set(CXX_SOURCES
    ${CMAKE_SOURCE_DIR}/Core/Src/main.cpp
)

# 链接器脚本
set(LINKER_SCRIPT STM32F103RCTX_FLASH.ld)

# 创建可执行文件
add_executable(stm32_template ${ASM_SOURCES} ${C_SOURCES} ${CXX_SOURCES})

# 设置链接选项
target_link_options(stm32_template PRIVATE
    -Wl,--gc-sections
    -Wl,--start-group
    -lc
    -lm
    -Wl,--end-group
    -T${LINKER_SCRIPT}
    -Wl,-Map=stm32_template.map
    -static
)

# 设置目标属性
set_target_properties(stm32_template PROPERTIES
    SUFFIX ".elf"
    OUTPUT_NAME "stm32_template"
    LINKER_LANGUAGE C
)

# 添加自定义目标来生成 hex 文件
add_custom_command(TARGET stm32_template POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:stm32_template> $<TARGET_FILE_BASE_NAME:stm32_template>.hex
    DEPENDS stm32_template
    COMMENT "Generating HEX file"
)

# 添加自定义目标来生成 bin 文件
add_custom_command(TARGET stm32_template POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:stm32_template> $<TARGET_FILE_BASE_NAME:stm32_template>.bin
    DEPENDS stm32_template
    COMMENT "Generating BIN file"
)

# 添加 flash 目标（可选）
add_custom_target(flash
    COMMAND arm-none-eabi-flash $<TARGET_FILE:stm32_template>
    DEPENDS stm32_template
    COMMENT "Flashing to device"
)
