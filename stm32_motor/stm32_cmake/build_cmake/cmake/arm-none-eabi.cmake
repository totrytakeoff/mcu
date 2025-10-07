# ARM Cortex-M Toolchain file for CMake
# This file configures CMake to use the ARM GCC toolchain

# Set the system name
set(CMAKE_SYSTEM_NAME Generic)

# Set the toolchain prefix
if(NOT DEFINED ARM_NONE_EABI_GCC_PATH)
    # Default path for STM32CubeIDE
    set(ARM_NONE_EABI_GCC_PATH "D:/devtools/stm32cubeIDE/STM32CubeIDE_1.16.0/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.12.3.rel1.win32_1.0.200.202406191623/tools/bin")
endif()

# Find the toolchain
set(CMAKE_C_COMPILER "${ARM_NONE_EABI_GCC_PATH}/arm-none-eabi-gcc.exe")
set(CMAKE_CXX_COMPILER "${ARM_NONE_EABI_GCC_PATH}/arm-none-eabi-g++.exe")
set(CMAKE_ASM_COMPILER "${ARM_NONE_EABI_GCC_PATH}/arm-none-eabi-gcc.exe")
set(CMAKE_OBJCOPY "${ARM_NONE_EABI_GCC_PATH}/arm-none-eabi-objcopy.exe")
set(CMAKE_OBJDUMP "${ARM_NONE_EABI_GCC_PATH}/arm-none-eabi-objdump.exe")
set(CMAKE_SIZE "${ARM_NONE_EABI_GCC_PATH}/arm-none-eabi-size.exe")

# Set the target processor
set(CMAKE_C_COMPILER_TARGET "arm-none-eabi")
set(CMAKE_CXX_COMPILER_TARGET "arm-none-eabi")

# Set CMake to not try to run compilers
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Set common compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fdata-sections -ffunction-sections")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fdata-sections -ffunction-sections")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -x assembler-with-cpp")

# Set linker flags
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections -Wl,--start-group -lc -lm -Wl,--end-group")

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set find commands
set(CMAKE_FIND_PACKAGE_NO_SYSTEM_PACKAGE ON)
set(CMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY ON)

# Don't use system package repositories
set(CMAKE_FIND_PACKAGE_USE_CMAKE_SYSTEM_PATH OFF)
