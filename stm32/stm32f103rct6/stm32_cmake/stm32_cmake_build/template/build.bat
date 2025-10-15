@echo off
REM STM32 CMake 构建脚本 (Windows)
REM 用法: build.bat [Debug|Release|ReleaseWithDebInfo]

echo Building STM32 project with CMake...

REM 检查 ARM 工具链环境变量
if not defined ARM_NONE_EABI_GCC_PATH (
    echo Warning: ARM_NONE_EABI_GCC_PATH is not set
    echo Please set ARM_NONE_EABI_GCC_PATH to your ARM GCC toolchain directory
    echo Example: set ARM_NONE_EABI_GCC_PATH=C:\devtools\arm-none-eabi-gcc
)

REM 创建构建目录
if not exist build mkdir build

REM 进入构建目录
cd build

REM 配置 CMake
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=%1 ..

REM 构建
cmake --build . --config %1

REM 检查构建结果
if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    exit /b 1
)

echo Build complete!
echo Output files:
dir *.elf *.hex *.bin 2>nul || echo No output files found

REM 返回到项目根目录
cd ..
