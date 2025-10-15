cmake_minimum_required(VERSION 3.16)
project(stm32_car C CXX ASM)

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
add_executable(stm32_car ${ASM_SOURCES} ${C_SOURCES} ${CXX_SOURCES})

# 设置链接选项
target_link_options(stm32_car PRIVATE
    -Wl,--gc-sections
    -Wl,--start-group
    -lc
    -lm
    -Wl,--end-group
    -T${LINKER_SCRIPT}
    -Wl,-Map=stm32_car.map
    -static
)

# 设置目标属性
set_target_properties(stm32_car PROPERTIES
    SUFFIX ".elf"
    OUTPUT_NAME "stm32_car"
    LINKER_LANGUAGE C
)

# 添加自定义目标来生成hex文件
add_custom_command(TARGET stm32_car POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:stm32_car> stm32_car.hex
    COMMENT "Generating HEX file"
)

# 添加自定义目标来生成bin文件
add_custom_command(TARGET stm32_car POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:stm32_car> stm32_car.bin
    COMMENT "Generating BIN file"
)

# ============================================================================
# 烧录配置 (Upload Configuration)
# ============================================================================

# 串口参数配置（可通过 cmake -DUPLOAD_PORT=COM7 修改）
set(UPLOAD_PORT "COM6" CACHE STRING "Serial port for uploading")
set(UPLOAD_BAUD "115200" CACHE STRING "Baud rate for serial upload")

# 检测可用的烧录工具
find_program(STM32FLASH stm32flash)
find_program(OPENOCD openocd)

# ----------------------------------------------------------------------------
# 串口 ISP 烧录（主要方式）- 使用 stm32flash
# ----------------------------------------------------------------------------
if(STM32FLASH)
    message(STATUS "Found stm32flash: ${STM32FLASH}")
    
    # 串口烧录目标（使用验证成功的配置）
    add_custom_target(flash_serial
        COMMAND ${STM32FLASH}
            -w ${CMAKE_BINARY_DIR}/stm32_car.bin
            -v                              # 验证写入
            -g 0x0                          # 烧录后跳转到 0x08000000
            -b ${UPLOAD_BAUD}               # 波特率
            -R                              # 使用 RTS/DTR 控制
            -i -dtr,rts,dtr:-rts           # DTR/RTS 控制序列（已验证）
            ${UPLOAD_PORT}
        DEPENDS stm32_car
        COMMENT "Flashing via Serial (ISP) to ${UPLOAD_PORT} at ${UPLOAD_BAUD} baud"
    )
    
    # 默认 flash 目标指向串口烧录
    add_custom_target(flash
        DEPENDS flash_serial
        COMMENT "Flashing to device (default: serial)"
    )
    
    # 读取芯片信息
    add_custom_target(serial_info
        COMMAND ${STM32FLASH} ${UPLOAD_PORT}
        COMMENT "Reading chip information via serial"
    )
    
    # 擦除芯片
    add_custom_target(serial_erase
        COMMAND ${STM32FLASH} -o ${UPLOAD_PORT}
        COMMENT "Erasing chip via serial"
    )
    
    # 备份固件
    add_custom_target(serial_backup
        COMMAND ${STM32FLASH}
            -r firmware_backup.bin
            -b ${UPLOAD_BAUD}
            ${UPLOAD_PORT}
        COMMENT "Backing up firmware via serial"
    )
    
else()
    message(WARNING "stm32flash not found! Serial flashing will not be available.")
    message(STATUS "Install: https://sourceforge.net/projects/stm32flash/")
    
    # 创建一个提示目标
    add_custom_target(flash
        COMMAND ${CMAKE_COMMAND} -E echo "Error: stm32flash not found!"
        COMMAND ${CMAKE_COMMAND} -E echo "Please install stm32flash to enable serial uploading."
        COMMENT "Flash tool not available"
    )
endif()

# ----------------------------------------------------------------------------
# OpenOCD + ST-Link 烧录（备用方式，需要 ST-Link 硬件）
# ----------------------------------------------------------------------------
if(OPENOCD)
    message(STATUS "Found OpenOCD: ${OPENOCD}")
    
    add_custom_target(flash_stlink
        COMMAND ${OPENOCD}
            -f interface/stlink.cfg
            -f target/stm32f1x.cfg
            -c "program $<TARGET_FILE:stm32_car> verify reset exit"
        DEPENDS stm32_car
        COMMENT "Flashing via ST-Link (requires ST-Link hardware)"
    )
    
    add_custom_target(flash_openocd
        DEPENDS flash_stlink
        COMMENT "Alias for flash_stlink"
    )
    
    # 启动 GDB 调试服务器
    add_custom_target(debug
        COMMAND ${OPENOCD}
            -f interface/stlink.cfg
            -f target/stm32f1x.cfg
        COMMENT "Starting OpenOCD GDB server on port 3333"
    )
    
    # 擦除芯片（OpenOCD）
    add_custom_target(erase_stlink
        COMMAND ${OPENOCD}
            -f interface/stlink.cfg
            -f target/stm32f1x.cfg
            -c "init; reset halt; stm32f1x mass_erase 0; exit"
        COMMENT "Erasing chip via ST-Link"
    )
    
else()
    message(STATUS "OpenOCD not found. ST-Link flashing will not be available.")
endif()

# ----------------------------------------------------------------------------
# 显示配置信息
# ----------------------------------------------------------------------------
message(STATUS "")
message(STATUS "=== Upload Configuration ===")
message(STATUS "Serial Port:  ${UPLOAD_PORT}")
message(STATUS "Baud Rate:    ${UPLOAD_BAUD}")
message(STATUS "stm32flash:   ${STM32FLASH}")
message(STATUS "OpenOCD:      ${OPENOCD}")
message(STATUS "")
message(STATUS "Available targets:")
if(STM32FLASH)
    message(STATUS "  - flash          : Upload via serial (default)")
    message(STATUS "  - flash_serial   : Upload via serial")
    message(STATUS "  - serial_info    : Read chip info via serial")
    message(STATUS "  - serial_erase   : Erase chip via serial")
    message(STATUS "  - serial_backup  : Backup firmware via serial")
endif()
if(OPENOCD)
    message(STATUS "  - flash_stlink   : Upload via ST-Link")
    message(STATUS "  - debug          : Start OpenOCD GDB server")
    message(STATUS "  - erase_stlink   : Erase chip via ST-Link")
endif()
message(STATUS "===========================")
message(STATUS "")
