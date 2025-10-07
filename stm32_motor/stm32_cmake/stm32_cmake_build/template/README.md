# STM32 CMake 项目模板

这是一个使用 CMake 构建 STM32 项目的模板，支持 C 和 C++ 混合编程。

## 项目结构

```
template/
├── Core/
│   ├── Inc/                    # Core 头文件
│   │   ├── main.hpp
│   │   └── ...
│   ├── Src/                    # Core 源文件
│   │   ├── main.cpp
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
├── STM32F103RCTX_FLASH.ld      # 链接器脚本
├── build.bat                   # Windows 构建脚本
├── build.sh                    # Linux/macOS 构建脚本
└── README.md                   # 本文件
```

## 快速开始

### 1. 复制模板

```bash
# 创建新项目目录
mkdir my_stm32_project
cd my_stm32_project

# 复制模板文件
cp -r /path/to/stm32_cmake_build/template/* .
```

### 2. 配置项目

编辑 `CMakeLists.txt` 文件，根据需要修改以下配置：

```cmake
# 修改项目名称
project(my_stm32_project C CXX ASM)

# 修改芯片型号
add_compile_definitions(STM32F103xE)

# 修改链接器脚本
set(LINKER_SCRIPT STM32F103RCTX_FLASH.ld)
```

### 3. 设置工具链路径

#### Windows
```cmd
set ARM_NONE_EABI_GCC_PATH=C:\devtools\arm-none-eabi-gcc\bin
```

#### Linux/macOS
```bash
export ARM_NONE_EABI_GCC_PATH=/usr/bin
```

### 4. 构建项目

#### Windows
```cmd
build.bat Debug
```

#### Linux/macOS
```bash
chmod +x build.sh
./build.sh Debug
```

### 5. 查看输出文件

构建完成后，在 `build` 目录中会生成以下文件：

- `my_stm32_project.elf` - ELF 格式的可执行文件
- `my_stm32_project.hex` - Intel HEX 格式的烧录文件
- `my_stm32_project.bin` - 二进制格式的烧录文件
- `my_stm32_project.map` - 内存映射文件

## 构建配置

支持以下构建类型：

- `Debug` - 调试版本，包含调试信息，无优化
- `Release` - 发布版本，最高优化，无调试信息
- `ReleaseWithDebInfo` - 发布版本，包含调试信息，最高优化

## 自定义配置

### 添加新的源文件

1. 将源文件添加到相应的目录
2. 在 `CMakeLists.txt` 中添加源文件：

```cmake
# 添加新的 C 源文件
list(APPEND C_SOURCES
    ${CMAKE_SOURCE_DIR}/Core/Src/my_module.c
)

# 添加新的 C++ 源文件
list(APPEND CXX_SOURCES
    ${CMAKE_SOURCE_DIR}/Core/Src/my_class.cpp
)
```

### 添加新的头文件

```cmake
# 添加新的包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/Core/Inc/my_module.h
)
```

### 修改编译选项

```cmake
# 添加编译定义
add_compile_definitions(MY_FEATURE=1)

# 修改编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -my-custom-flag")
```

## IDE 集成

### Visual Studio Code

1. 安装 CMake Tools 扩展
2. 打开项目文件夹
3. 选择工具链文件：`CMake/arm-none-eabi.cmake`

### CLion

1. 打开项目
2. 设置工具链：File → Settings → Build, Execution, Deployment → CMake → Toolchains
3. 添加新的工具链，指定编译器路径和工具链文件

### Eclipse

1. 安装 CDT 插件
2. 创建 CMake 项目
3. 配置工具链和构建选项

## 故障排除

### 常见问题

1. **找不到 ARM 工具链**
   - 确保 ARM GCC 工具链已安装
   - 检查 `ARM_NONE_EABI_GCC_PATH` 环境变量设置

2. **编译错误**
   - 检查包含路径是否正确
   - 确认源文件路径是否正确
   - 检查编译选项是否合适

3. **链接错误**
   - 检查链接器脚本是否正确
   - 确认所有必要的源文件都已包含
   - 检查库文件是否正确链接

### 调试技巧

1. **查看详细的编译命令**
   ```bash
   cmake --build build --verbose
   ```

2. **清理构建**
   ```bash
   rm -rf build/
   ```

3. **重新配置**
   ```bash
   cd build
   cmake ..
   ```

## 许可证

本项目基于 MIT 许可证，详见 LICENSE 文件。

## 贡献

欢迎提交 Issue 和 Pull Request 来改进这个模板。

## 相关链接

- [CMake 官方文档](https://cmake.org/documentation/)
- [STM32 HAL 库文档](https://www.st.com/en/embedded-software/x-cube-hal.html)
- [ARM GCC 工具链](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm)
