# =============================================================================
# STM32F103 CMake Project - 从零开始的空项目模板
# =============================================================================

cmake_minimum_required(VERSION 3.22)

# 设置工具链文件（必须在project()之前）
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/arm-none-eabi-gcc.cmake)

# 项目定义
project(stm32f103_cmake_demo 
    VERSION 1.0.0
    DESCRIPTION "STM32F103 CMake Demo from Scratch"
    LANGUAGES C ASM
)

# 设置C标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 芯片定义
set(MCU_FAMILY STM32F1xx)
set(MCU_MODEL STM32F103xE)
set(CPU_PARAMETERS 
    -mcpu=cortex-m3
    -mthumb
    -mfloat-abi=soft
)

# 编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CPU_PARAMETERS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdata-sections -ffunction-sections")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -g3")

# 链接选项
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${CPU_PARAMETERS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--print-memory-usage")

# 定义宏
add_definitions(
    -D${MCU_MODEL}
    -DUSE_HAL_DRIVER
    -DHSE_VALUE=8000000
)

# 包含路径
include_directories(
    src
    include
    drivers/CMSIS/Include
    drivers/CMSIS/Device/ST/STM32F1xx/Include
    drivers/STM32F1xx_HAL_Driver/Inc
    drivers/STM32F1xx_HAL_Driver/Inc/Legacy
)

# HAL库源文件
file(GLOB_RECURSE HAL_SOURCES 
    drivers/STM32F1xx_HAL_Driver/Src/*.c
)

# 排除模板文件
list(FILTER HAL_SOURCES EXCLUDE REGEX ".*template.*")

# 项目源文件
set(PROJECT_SOURCES
    # 启动文件
    drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/gcc/startup_stm32f103xe.s
    
    # 系统文件
    drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/system_stm32f1xx.c
    
    # 用户源文件
    src/main.c
    src/stm32f1xx_hal_msp.c
    src/stm32f1xx_it.c
    src/pwm_driver.c
    src/gpio_driver.c
    src/clock_config.c
)

# 创建可执行文件
add_executable(${PROJECT_NAME}.elf 
    ${PROJECT_SOURCES}
    ${HAL_SOURCES}
)

# 链接脚本
set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/STM32F103VETx_FLASH.ld)
set_target_properties(${PROJECT_NAME}.elf PROPERTIES
    LINK_DEPENDS ${LINKER_SCRIPT}
)
target_link_options(${PROJECT_NAME}.elf PRIVATE 
    -T${LINKER_SCRIPT}
    -Wl,-Map=${PROJECT_NAME}.map
)

# 生成hex和bin文件
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.bin
    COMMENT "Building ${PROJECT_NAME}.hex and ${PROJECT_NAME}.bin"
)

# 显示大小信息
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}.elf>
    COMMENT "Showing size information"
)

# 自定义目标：烧录程序
add_custom_target(flash
    COMMAND openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c "program ${PROJECT_NAME}.hex verify reset exit"
    DEPENDS ${PROJECT_NAME}.elf
    COMMENT "Flashing ${PROJECT_NAME}.hex to target"
)

# 自定义目标：调试
add_custom_target(debug
    COMMAND openocd -f interface/stlink.cfg -f target/stm32f1x.cfg
    COMMENT "Starting OpenOCD for debugging"
)

# 自定义目标：清理额外文件
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove -f ${PROJECT_NAME}.hex ${PROJECT_NAME}.bin ${PROJECT_NAME}.map
    COMMENT "Cleaning all generated files"
)