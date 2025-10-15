# ARM GCC 工具链配置文件
# 用于 STM32 嵌入式项目

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

# 设置目标属性
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# 禁用 C++ 标准库
set(CMAKE_CXX_STANDARD_LIBRARIES "")

# 禁用运行时库
set(CMAKE_C_STANDARD_LIBRARIES "")

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 设置构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# 设置编译器特定选项
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    # 添加通用编译选项
    set(COMMON_FLAGS "-fno-common -ffunction-sections -fdata-sections")
    
    # 根据构建类型设置优化选项
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(COMMON_FLAGS "${COMMON_FLAGS} -g3 -O0")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(COMMON_FLAGS "${COMMON_FLAGS} -Os")
    elseif(CMAKE_BUILD_TYPE STREQUAL "ReleaseWithDebInfo")
        set(COMMON_FLAGS "${COMMON_FLAGS} -Os -g3")
    endif()
    
    # 应用编译选项
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS}" CACHE STRING "C Compiler flags" FORCE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS}" CACHE STRING "C++ Compiler flags" FORCE)
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${COMMON_FLAGS}" CACHE STRING "ASM Compiler flags" FORCE)
    
    # 添加警告选项
    set(WARNING_FLAGS "-Wall -Wextra")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS}" CACHE STRING "C Compiler flags" FORCE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS}" CACHE STRING "C++ Compiler flags" FORCE)
    
    # 禁用一些不必要的警告
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter" CACHE STRING "C Compiler flags" FORCE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter" CACHE STRING "C++ Compiler flags" FORCE)
endif()

# 设置链接器选项
set(LINKER_FLAGS "-Wl,--gc-sections -static")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKER_FLAGS}" CACHE STRING "Linker flags" FORCE)

# 清理缓存变量
set(CMAKE_C_FLAGS_CACHE "")
set(CMAKE_CXX_FLAGS_CACHE "")
set(CMAKE_ASM_FLAGS_CACHE "")
set(CMAKE_EXE_LINKER_FLAGS_CACHE "")
