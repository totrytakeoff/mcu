# STM32 CMake构建系统

这个项目使用CMake构建系统来编译STM32F103RC微控制器项目，支持C和C++混合编程。

## 项目结构

```
build_cmake/
├── CMakeLists.txt          # 主构建配置文件
├── build.bat              # Windows构建脚本
├── cmake/
│   └── arm-none-eabi.cmake # ARM工具链配置文件
└── README.md              # 说明文档
```

## 系统要求

### 必需的软件

1. **CMake** (3.16或更高版本)
   - 下载地址: https://cmake.org/download/
   - 安装完成后确保cmake命令在PATH中

2. **ARM GCC工具链**
   - 选项1: 使用STM32CubeIDE内置的工具链
     - STM32CubeIDE: https://www.st.com/en/development-tools/stm32cubeide.html
     - 工具链路径: `D:/devtools/stm32cubeIDE/STM32CubeIDE_1.16.0/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.12.3.rel1.win32_1.0.200.202406191623/tools/bin`
   
   - 选项2: 使用独立的ARM GCC工具链
     - 下载地址: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm
     - 下载Windows版本并解压到某个目录（如 `C:\devtools\arm-none-eabi-gcc`）

3. **可选：构建工具**
   - MinGW Makefiles (用于Windows)
   - 或者使用Visual Studio (需要CMake支持)

### 可选的软件

1. **STM32CubeMX** (用于项目配置)
   - 下载地址: https://www.st.com/en/development-tools/stm32cubemx.html

2. **PlatformIO** (如果你喜欢使用VS Code)
   - 安装PlatformIO IDE扩展

## 安装步骤

### 1. 安装CMake

```powershell
# 使用winget安装（推荐）
winget install --id Kitware.CMake

# 或者从官网下载安装包安装
```

### 2. 安装ARM GCC工具链

#### 选项A: 使用STM32CubeIDE

1. 安装STM32CubeIDE
2. 工具链会自动安装，路径已经在 `cmake/arm-none-eabi.cmake` 中配置

#### 选项B: 使用独立工具链

1. 下载ARM GCC工具链
2. 解压到某个目录（如 `C:\devtools\arm-none-eabi-gcc`）
3. 设置环境变量：
   ```powershell
   # 在PowerShell中设置
   $env:ARM_NONE_EABI_GCC_PATH = "C:\devtools\arm-none-eabi-gcc\bin"
   # 或者永久添加到系统环境变量
   ```

### 3. 验证安装

```powershell
# 检查CMake版本
cmake --version

# 检查ARM工具链
arm-none-eabi-gcc --version
```

## 使用方法

### 方法1: 使用构建脚本（推荐）

```powershell
# 进入构建目录
cd build_cmake

# 运行构建脚本
build.bat
```

### 方法2: 手动使用CMake

```powershell
# 创建构建目录
mkdir build
cd build

# 配置项目
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-none-eabi.cmake -G "MinGW Makefiles" ..

# 编译项目
cmake --build . --config Debug

# 查看输出文件
dir *.elf *.hex *.bin
```

### 方法3: 使用Visual Studio

```powershell
# 创建构建目录
mkdir build
cd build

# 配置项目（使用Visual Studio生成器）
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-none-eabi.cmake -G "Visual Studio 16 2019" -A x64 ..

# 使用Visual Studio打开解决方案
# 然后在Visual Studio中选择构建项目
```

## 构建输出

构建完成后，你会在 `build` 目录中找到以下文件：

- `dpj.elf` - ELF格式的可执行文件
- `dpj.hex` - Intel HEX格式的文件（用于烧录）
- `dpj.bin` - 二进制格式的文件
- `dpj.map` - 内存映射文件

## 烧录到设备

### 使用ST-Link

```powershell
# 需要先安装OpenOCD或ST-Link工具
# 使用ST-Link Utility或命令行工具烧录
arm-none-eabi-gdb dpj.elf -ex "target remote localhost:4242" -ex "monitor reset halt" -ex "load" -ex "monitor reset" -ex "quit"
```

### 使用STM32CubeProgrammer

1. 打开STM32CubeProgrammer
2. 选择ST-Link作为调试器
3. 加载 `dpj.hex` 文件
4. 点击"Download"按钮

## 故障排除

### 常见问题

1. **找不到ARM工具链**
   - 检查 `ARM_NONE_EABI_GCC_PATH` 环境变量是否设置正确
   - 验证工具链路径是否正确

2. **编译错误**
   - 检查所有源文件路径是否正确
   - 确认包含路径设置正确

3. **链接错误**
   - 检查链接器脚本路径
   - 确认所有必要的库文件都已包含

### 调试构建

```powershell
# 启用详细输出
cmake --build . --config Debug --verbose

# 查看CMake配置
cmake -LAH
```

## 扩展和修改

### 添加新的源文件

在 `CMakeLists.txt` 中的相应源文件列表中添加新文件：

```cmake
# C源文件
set(C_SOURCES
    # 现有文件...
    Core/Src/new_file.c  # 添加新文件
)

# C++源文件
set(CXX_SOURCES
    # 现有文件...
    Core/Src/new_cpp_file.cpp  # 添加新文件
)
```

### 修改编译选项

```cmake
# 修改C编译标志
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -g")

# 修改C++编译标志
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -g -std=c++14")
```

### 添加新的库

```cmake
# 添加外部库
add_library(mylib STATIC
    external/mylib/src/file1.c
    external/mylib/src/file2.c
)

# 链接到目标
target_link_libraries(dpj.elf mylib)
```

## 许可证

本项目遵循原始项目的许可证。
