# STM32 CMake 构建系统详细指南

## 目录
1. [概述](#概述)
2. [CMake 构建系统的优势](#cmake-构建系统的优势)
3. [项目结构](#项目结构)
4. [CMakeLists.txt 详细解析](#cmakeliststxt-详细解析)
5. [构建配置](#构建配置)
6. [编译选项和优化](#编译选项和优化)
7. [链接配置](#链接配置)
8. [生成文件](#生成文件)
9. [调试和发布配置](#调试和发布配置)
10. [常见问题和解决方案](#常见问题和解决方案)
11. [最佳实践](#最佳实践)
12. [模板使用指南](#模板使用指南)

## 概述

本文档详细介绍如何使用 CMake 构建 STM32 项目。CMake 是一个跨平台的构建系统生成器，能够为各种 IDE 和构建工具生成构建文件。对于 STM32 项目，CMake 提供了比传统 IDE 更灵活、更可移植的构建解决方案。

### 为什么选择 CMake？

- **跨平台支持**：可以在 Windows、Linux、macOS 上使用相同的构建配置
- **IDE 兼容性**：支持 Visual Studio、VS Code、CLion、Eclipse 等多种 IDE
- **版本控制友好**：生成的构建文件可以忽略，只提交源代码
- **模块化设计**：可以轻松管理复杂的项目结构
- **自动化构建**：支持持续集成和自动化构建

## CMake 构建系统的优势

### 1. 可移植性
- 不依赖于特定的 IDE（如 STM32CubeIDE）
- 可以在任何安装了 CMake 和 ARM 工具链的系统上构建
- 支持不同的操作系统和构建环境

### 2. 灵活性
- 可以轻松添加或删除源文件
- 支持多种构建配置（Debug、Release、ReleaseWithDebInfo）
- 可以自定义编译选项和链接选项

### 3. 可维护性
- 清晰的项目结构
- 模块化的构建配置
- 易于理解和修改的构建脚本

### 4. 自动化支持
- 支持持续集成（CI/CD）
- 可以集成到各种构建系统中
- 支持自动化测试和部署

## 项目结构

典型的 STM32 CMake 项目结构如下：

```
stm32_project/
├── Core/
│   ├── Inc/                    # Core 头文件
│   │   ├── main.hpp
│   │   ├── stm32f1xx_hal_conf.h
│   │   └── ...
│   ├── Src/                    # Core 源文件
│   │   ├── main.cpp
│   │   ├── stm32f1xx_hal_msp.c
│   │   ├── stm32f1xx_it.c
│   │   └── ...
│   └── Startup/                # 启动文件
│       └── startup_stm32f103rctx.s
├── Drivers/                    # STM32 HAL 驱动
│   ├── CMSIS/
│   │   ├── Include/
│   │   └── Device/ST/STM32F1xx/Include/
│   └── STM32F1xx_HAL_Driver/
│       ├── Inc/
│       └── Src/
├── CMakeLists.txt              # 主构建文件
├── CMake/                      # CMake 模块和工具链文件
│   └── arm-none-eabi.cmake
├── build.sh                    # 构建脚本（Linux/macOS）
└── build.bat                   # 构建脚本（Windows）
```

## CMakeLists.txt 详细解析

### 基本设置

```cmake
# 设置最低 CMake 版本要求
cmake_minimum_required(VERSION 3.16)

# 项目名称和语言
project(stm32_project C CXX ASM)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

### 编译选项配置

```cmake
# 设置编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m3 -mthumb -ffunction-sections -fdata-sections")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=cortex-m3 -mthumb -ffunction-sections -fdata-sections")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -mcpu=cortex-m3 -mthumb")

# 添加编译定义
add_compile_definitions(DEBUG USE_HAL_DRIVER STM32F103xE)
```

### 包含目录设置

```cmake
# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Include
)
```

### 源文件配置

```cmake
# 汇编源文件
set(ASM_SOURCES
    ${CMAKE_SOURCE_DIR}/Core/Startup/startup_stm32f103rctx.s
)

# C 源文件
set(C_SOURCES
    ${CMAKE_SOURCE_DIR}/Core/Src/main.cpp
    ${CMAKE_SOURCE_DIR}/Core/Src/stm32f1xx_hal_msp.c
    ${CMAKE_SOURCE_DIR}/Core/Src/stm32f1xx_it.c
    ${CMAKE_SOURCE_DIR}/Core/Src/system_stm32f1xx.c
    # ... 其他 C 源文件
)

# C++ 源文件
set(CXX_SOURCES
    ${CMAKE_SOURCE_DIR}/Core/Src/main.cpp
    # ... 其他 C++ 源文件
)
```

### 链接器脚本配置

```cmake
# 链接器脚本
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32F103RCTX_FLASH.ld)
```

### 创建可执行文件

```cmake
# 创建可执行文件
add_executable(stm32_project ${ASM_SOURCES} ${C_SOURCES} ${CXX_SOURCES})
```

### 链接选项配置

```cmake
# 设置链接选项
target_link_options(stm32_project PRIVATE
    -Wl,--gc-sections
    -Wl,--start-group
    -lc
    -lm
    -Wl,--end-group
    -T${LINKER_SCRIPT}
    -Wl,-Map=stm32_project.map
    -static
)
```

### 目标属性设置

```cmake
# 设置目标属性
set_target_properties(stm32_project PROPERTIES
    SUFFIX ".elf"
    OUTPUT_NAME "stm32_project"
    LINKER_LANGUAGE C
)
```

### 自定义目标

```cmake
# 添加自定义目标来生成 hex 文件
add_custom_command(TARGET stm32_project POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:stm32_project> stm32_project.hex
    DEPENDS stm32_project
    COMMENT "Generating HEX file"
)

# 添加自定义目标来生成 bin 文件
add_custom_command(TARGET stm32_project POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:stm32_project> stm32_project.bin
    DEPENDS stm32_project
    COMMENT "Generating BIN file"
)

# 添加 flash 目标（可选）
add_custom_target(flash
    COMMAND arm-none-eabi-flash $<TARGET_FILE:stm32_project>
    DEPENDS stm32_project
    COMMENT "Flashing to device"
)
```

## 构建配置

### 多配置支持

```cmake
# 支持多种构建配置
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG=1)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g3")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(NDEBUG=1)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
endif()
```

### 工具链文件

创建 `CMake/arm-none-eabi.cmake` 文件：

```cmake
# 设置 C 编译器
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_C_COMPILER_ID GNU)

# 设置 C++ 编译器
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_CXX_COMPILER_ID GNU)

# 设置汇编编译器
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

# 设置目标系统
set(CMAKE_SYSTEM_NAME Generic)

# 禁用自动查找程序
set(CMAKE_FIND_PACKAGE_NO_SYSTEM_PACKAGE ON)

# 设置编译器选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "C Compiler flags")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING "C++ Compiler flags")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS}" CACHE STRING "ASM Compiler flags")

# 设置链接器选项
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}" CACHE STRING "Linker flags")
```

## 编译选项和优化

### 基本编译选项

```cmake
# 基本编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m3 -mthumb")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=cortex-m3 -mthumb")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -mcpu=cortex-m3 -mthumb")

# 优化选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")

# 警告选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
```

### 调试选项

```cmake
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g3")
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -g3")
endif()
```

### 发布选项

```cmake
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
endif()
```

## 链接配置

### 链接器脚本

```cmake
# 链接器脚本
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32F103RCTX_FLASH.ld)

# 设置链接选项
target_link_options(stm32_project PRIVATE
    -Wl,--gc-sections
    -Wl,--start-group
    -lc
    -lm
    -Wl,--end-group
    -T${LINKER_SCRIPT}
    -Wl,-Map=stm32_project.map
    -static
)
```

### 内存布局

确保链接器脚本正确设置：

```ld
/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = ORIGIN(RAM) + LENGTH(RAM);    /* end of "RAM" Ram type memory */

_Min_Heap_Size = 0x200;                /* required amount of heap  */
_Min_Stack_Size = 0x400;               /* required amount of stack */

/* Memories definition */
MEMORY
{
  RAM    (xrw)    : ORIGIN = 0x20000000,   LENGTH = 20K
  FLASH    (rx)    : ORIGIN = 0x08000000,   LENGTH = 256K
}

/* Sections */
SECTIONS
{
  /* The startup code into "FLASH" Rom type memory */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH

  /* The program code and other data into "FLASH" Rom type memory */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH

  /* Constant data into "FLASH" Rom type memory */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >FLASH

  .ARM.extab   : { 
    . = ALIGN(4);
    *(.ARM.extab* .gnu.linkonce.armextab.*)
    . = ALIGN(4);
  } >FLASH
  
  .ARM : {
    . = ALIGN(4);
    *(.ARM.exidx*)
    . = ALIGN(4);
  } >FLASH
  
  .preinit_array     :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
    . = ALIGN(4);
  } >FLASH
  
  .init_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array))
    PROVIDE_HIDDEN (__init_array_end = .);
    . = ALIGN(4);
  } >FLASH
  
  .fini_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array))
    PROVIDE_HIDDEN (__fini_array_end = .);
    . = ALIGN(4);
  } >FLASH

  /* Used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections into "RAM" Ram type memory */
  .data : 
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >RAM AT> FLASH

  /* Uninitialized data section into "RAM" Ram type memory */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss section */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >RAM

  /* User_heap_stack section, used to check that there is enough "RAM"  left */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM

  /* Remove information from the compiler libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }
}
```

## 生成文件

### HEX 文件生成

```cmake
# 添加自定义目标来生成 hex 文件
add_custom_command(TARGET stm32_project POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:stm32_project> $<TARGET_FILE_BASE_NAME:stm32_project>.hex
    DEPENDS stm32_project
    COMMENT "Generating HEX file"
)
```

### BIN 文件生成

```cmake
# 添加自定义目标来生成 bin 文件
add_custom_command(TARGET stm32_project POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:stm32_project> $<TARGET_FILE_BASE_NAME:stm32_project>.bin
    DEPENDS stm32_project
    COMMENT "Generating BIN file"
)
```

### SREC 文件生成

```cmake
# 添加自定义目标来生成 srec 文件
add_custom_command(TARGET stm32_project POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O srec $<TARGET_FILE:stm32_project> $<TARGET_FILE_BASE_NAME:stm32_project>.srec
    DEPENDS stm32_project
    COMMENT "Generating SREC file"
)
```

## 调试和发布配置

### Debug 配置

```cmake
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG=1)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g3")
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -g3")
    
    # 启用调试信息
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -gdwarf-2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -gdwarf-2")
    
    # 禁用优化
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-inline")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-inline")
endif()
```

### Release 配置

```cmake
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(NDEBUG=1)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
    
    # 启用链接时优化
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")
    
    # 减少调试信息
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g1")
endif()
```

### ReleaseWithDebInfo 配置

```cmake
if(CMAKE_BUILD_TYPE STREQUAL "ReleaseWithDebInfo")
    add_compile_definitions(NDEBUG=1)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
    
    # 保留调试信息
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3")
endif()
```

## 常见问题和解决方案

### 1. 链接器脚本路径问题

**问题**：链接器找不到链接器脚本文件
```cmake
# 错误示例
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32F103RCTX_FLASH.ld)
```

**解决方案**：确保路径正确
```cmake
# 正确示例
set(LINKER_SCRIPT STM32F103RCTX_FLASH.ld)
```

### 2. 包含目录问题

**问题**：编译器找不到头文件
```cmake
# 错误示例
include_directories(Core/Inc)
```

**解决方案**：使用绝对路径
```cmake
# 正确示例
include_directories(
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include
)
```

### 3. 工具链路径问题

**问题**：找不到 ARM 工具链
```cmake
# 解决方案：设置工具链路径
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/CMake/arm-none-eabi.cmake)
```

### 4. 编译选项冲突

**问题**：编译选项冲突导致编译失败
```cmake
# 解决方案：清理编译选项
set(CMAKE_C_FLAGS "")
set(CMAKE_CXX_FLAGS "")
set(CMAKE_ASM_FLAGS "")

# 然后添加必要的选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m3 -mthumb")
```

### 5. 链接错误

**问题**：链接时找不到符号
```cmake
# 解决方案：确保所有必要的库都被链接
target_link_options(stm32_project PRIVATE
    -Wl,--start-group
    -lc
    -lm
    -lnosys
    -Wl,--end-group
)
```

## 最佳实践

### 1. 项目结构管理

```cmake
# 使用变量管理路径
set(CORE_INC_DIR ${CMAKE_SOURCE_DIR}/Core/Inc)
set(CORE_SRC_DIR ${CMAKE_SOURCE_DIR}/Core/Src)
set(DRIVERS_INC_DIR ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc)
set(DRIVERS_SRC_DIR ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src)

# 包含目录
include_directories(
    ${CORE_INC_DIR}
    ${DRIVERS_INC_DIR}
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Include
)
```

### 2. 源文件管理

```cmake
# 使用 glob 模式获取源文件
file(GLOB_RECURSE CORE_SOURCES 
    "${CORE_SRC_DIR}/*.c"
    "${CORE_SRC_DIR}/*.cpp"
)

file(GLOB_RECURSE DRIVERS_SOURCES 
    "${DRIVERS_SRC_DIR}/*.c"
)

# 过滤不需要的文件
list(FILTER CORE_SOURCES EXCLUDE REGEX ".*\.test\.c$")
list(FILTER DRIVERS_SOURCES EXCLUDE REGEX ".*\.template\.c$")
```

### 3. 条件编译

```cmake
# 根据条件添加编译定义
if(USE_HAL_DRIVER)
    add_compile_definitions(USE_HAL_DRIVER)
endif()

if(STM32F103xE)
    add_compile_definitions(STM32F103xE)
endif()
```

### 4. 版本管理

```cmake
# 设置项目版本
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

# 添加版本信息到编译定义
add_compile_definitions(PROJECT_VERSION="${VERSION}")
```

### 5. 依赖管理

```cmake
# 查找依赖包
find_package(CMSIS REQUIRED)
find_package(STM32_HAL_Driver REQUIRED)

# 链接依赖
target_link_libraries(stm32_project
    CMSIS::STM32F1xx
    STM32_HAL_Driver::STM32F1xx_HAL
)
```

## 模板使用指南

### 1. 创建新项目

```bash
# 创建项目目录
mkdir my_stm32_project
cd my_stm32_project

# 复制模板文件
cp -r /path/to/stm32_cmake_build/template/* .

# 配置项目
mkdir build
cd build
cmake ..
make
```

### 2. 配置项目

编辑 `CMakeLists.txt` 文件：

```cmake
# 修改项目名称
project(my_stm32_project C CXX ASM)

# 修改芯片型号
add_compile_definitions(STM32F103xE)

# 修改链接器脚本
set(LINKER_SCRIPT STM32F103RCTX_FLASH.ld)
```

### 3. 添加源文件

```cmake
# 添加新的源文件
list(APPEND C_SOURCES
    ${CMAKE_SOURCE_DIR}/Core/Src/my_module.c
)

# 添加新的头文件
include_directories(
    ${CMAKE_SOURCE_DIR}/Core/Inc/my_module.h
)
```

### 4. 构建项目

```bash
# Debug 构建
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Release 构建
cmake -DCMAKE_BUILD_TYPE=Release ..
make

# 清理构建
make clean
```

### 5. 生成烧录文件

```bash
# 生成 HEX 文件
make my_stm32_project.hex

# 生成 BIN 文件
make my_stm32_project.bin

# 烧录到设备
make flash
```

### 6. 集成到 IDE

#### Visual Studio Code

创建 `.vscode/cmake-kits.json`：

```json
{
    "name": "STM32 GCC",
    "compilers": {
        "C": "arm-none-eabi-gcc",
        "CXX": "arm-none-eabi-g++"
    },
    "toolchainFile": "${workspaceFolder}/CMake/arm-none-eabi.cmake"
}
```

#### CLion

1. 打开项目
2. 选择 "File" -> "Settings" -> "Build, Execution, Deployment" -> "CMake"
3. 设置工具链文件路径
4. 重新加载 CMake 项目

### 7. 持续集成

创建 `.github/workflows/build.yml`：

```yaml
name: STM32 Build

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v2
    
    - name: Install ARM GCC
      run: sudo apt-get install gcc-arm-none-eabi
    
    - name: Configure CMake
      run: cmake -B build -DCMAKE_BUILD_TYPE=Debug
    
    - name: Build
      run: cmake --build build
    
    - name: Upload artifacts
      uses: actions/upload-artifact@v2
      with:
        name: firmware
        path: build/*.hex
```

## 总结

CMake 为 STM32 项目提供了强大而灵活的构建解决方案。通过本文档的详细介绍，您应该能够：

1. 理解 CMake 构建系统的工作原理
2. 配置和定制 STM32 项目的构建过程
3. 解决常见的构建问题
4. 遵循最佳实践来管理项目
5. 使用模板快速创建新项目

CMake 的学习曲线可能有些陡峭，但一旦掌握了基本概念，您会发现它为项目开发带来了极大的便利和灵活性。
