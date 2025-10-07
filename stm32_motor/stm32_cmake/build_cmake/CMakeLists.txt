cmake_minimum_required(VERSION 3.16)
project(dpj C CXX ASM)

# 设置C++标准
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
    ../Core/Inc
    ../Drivers/STM32F1xx_HAL_Driver/Inc
    ../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy
    ../Drivers/CMSIS/Include
    ../Drivers/CMSIS/Device/ST/STM32F1xx/Include
)

# 汇编源文件
set(ASM_SOURCES
    ../Core/Startup/startup_stm32f103rctx.s
)

# C源文件
set(C_SOURCES
    ../Core/Src/adc.c
    ../Core/Src/colour_GY33.c
    ../Core/Src/gpio.c
    ../Core/Src/i2c.c
    ../Core/Src/stm32f1xx_hal_msp.c
    ../Core/Src/stm32f1xx_it.c
    ../Core/Src/syscalls.c
    ../Core/Src/sysmem.c
    ../Core/Src/system_stm32f1xx.c
    ../Core/Src/tim.c
    ../Core/Src/usart.c
    ../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c
    ../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_adc.c
    ../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_adc_ex.c
    ../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c
    ../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dma.c
    ../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_exti.c
    ../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash.c
    ../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash_ex.c
    ../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c
    ../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio_ex.c
    ../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_i2c.c
    ../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pwr.c
    ../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c
    ../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc_ex.c
    ../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim.c
    ../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim_ex.c
    ../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_uart.c
)

# C++源文件
set(CXX_SOURCES
    ../Core/Src/main.cpp
    ../Core/Src/motor.cpp
)

# 链接器脚本
set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/STM32F103RCTX_FLASH.ld)

# 创建可执行文件
add_executable(dpj ${ASM_SOURCES} ${C_SOURCES} ${CXX_SOURCES})

# 设置链接选项
target_link_options(dpj PRIVATE
    -Wl,--gc-sections
    -Wl,--start-group
    -lc
    -lm
    -Wl,--end-group
    -T${LINKER_SCRIPT}
    -Wl,-Map=dpj.map
    -static
)

# 设置目标属性
set_target_properties(dpj PROPERTIES
    SUFFIX ".elf"
    OUTPUT_NAME "dpj"
    LINKER_LANGUAGE C
)

# 添加自定义目标来生成hex文件
add_custom_command(TARGET dpj POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:dpj> dpj.hex
    COMMENT "Generating HEX file"
)

# 添加自定义目标来生成bin文件
add_custom_command(TARGET dpj POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:dpj> dpj.bin
    COMMENT "Generating BIN file"
)

# 添加flash目标（可选）
add_custom_target(flash
    COMMAND arm-none-eabi-flash $<TARGET_FILE:dpj>
    DEPENDS dpj
    COMMENT "Flashing to device"
)
