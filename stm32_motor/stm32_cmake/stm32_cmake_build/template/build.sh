#!/bin/bash
# STM32 CMake 构建脚本 (Linux/macOS)
# 用法: ./build.sh [Debug|Release|ReleaseWithDebInfo]

set -e

echo "Building STM32 project with CMake..."

# 检查 ARM 工具链环境变量
if [ -z "$ARM_NONE_EABI_GCC_PATH" ]; then
    echo "Warning: ARM_NONE_EABI_GCC_PATH is not set"
    echo "Please set ARM_NONE_EABI_GCC_PATH to your ARM GCC toolchain directory"
    echo "Example: export ARM_NONE_EABI_GCC_PATH=/usr/bin"
fi

# 创建构建目录
mkdir -p build

# 进入构建目录
cd build

# 配置 CMake
cmake -DCMAKE_BUILD_TYPE=${1:-Debug} ..

# 构建
cmake --build .

# 检查构建结果
if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "Build complete!"
echo "Output files:"
ls -la *.elf *.hex *.bin 2>/dev/null || echo "No output files found"

# 返回到项目根目录
cd ..
