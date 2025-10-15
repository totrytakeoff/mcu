@echo off
echo Building STM32 project with CMake...

REM 检查是否设置了ARM工具链路径
if "%ARM_NONE_EABI_GCC_PATH%"=="" (
    echo Warning: ARM_NONE_EABI_GCC_PATH is not set
    echo Please set ARM_NONE_EABI_GCC_PATH to your ARM GCC toolchain directory
    echo Example: set ARM_NONE_EABI_GCC_PATH=C:\devtools\arm-none-eabi-gcc
    echo.
)

REM 创建构建目录
if not exist build mkdir build
cd build

REM 配置CMake
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-none-eabi.cmake -G "MinGW Makefiles" ..

REM 编译项目
cmake --build . --config Debug

echo.
echo Build complete!
echo Output files:
dir /B *.elf *.hex *.bin

cd ..
