#!/bin/bash
# =============================================================================
# CMake构建脚本 - STM32F103项目
# =============================================================================

set -e  # 遇到错误立即退出

# 项目配置
PROJECT_NAME="stm32f103_cmake_demo"
BUILD_DIR="build"
CMAKE_BUILD_TYPE="Release"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 打印带颜色的消息
print_message() {
    echo -e "${2}[$(date +'%H:%M:%S')] $1${NC}"
}

print_error() {
    print_message "$1" "$RED"
}

print_success() {
    print_message "$1" "$GREEN"
}

print_info() {
    print_message "$1" "$BLUE"
}

print_warning() {
    print_message "$1" "$YELLOW"
}

# 检查工具链
check_toolchain() {
    print_info "检查ARM工具链..."
    
    if ! command -v arm-none-eabi-gcc &> /dev/null; then
        print_error "错误: arm-none-eabi-gcc 未找到!"
        print_info "请安装ARM工具链或设置环境变量 ARM_TOOLCHAIN_PATH"
        exit 1
    fi
    
    print_success "ARM工具链检查通过"
    arm-none-eabi-gcc --version | head -1
}

# 检查CMake
check_cmake() {
    print_info "检查CMake..."
    
    if ! command -v cmake &> /dev/null; then
        print_error "错误: cmake 未找到!"
        exit 1
    fi
    
    CMAKE_VERSION=$(cmake --version | head -1 | cut -d' ' -f3)
    print_success "CMake版本: $CMAKE_VERSION"
}

# 清理构建目录
clean_build() {
    print_info "清理构建目录..."
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        print_success "构建目录已清理"
    fi
}

# 创建构建目录
create_build_dir() {
    print_info "创建构建目录..."
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
}

# 配置项目
configure_project() {
    print_info "配置CMake项目..."
    
    cmake .. \
        -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    
    if [ $? -eq 0 ]; then
        print_success "项目配置完成"
    else
        print_error "项目配置失败"
        exit 1
    fi
}

# 构建项目
build_project() {
    print_info "构建项目..."
    
    make -j$(nproc) 2>&1 | tee build.log
    
    if [ ${PIPESTATUS[0]} -eq 0 ]; then
        print_success "项目构建完成"
    else
        print_error "项目构建失败，查看 build.log 获取详细信息"
        exit 1
    fi
}

# 显示构建结果
show_build_results() {
    print_info "构建结果:"
    
    if [ -f "${PROJECT_NAME}.elf" ]; then
        print_success "✓ ${PROJECT_NAME}.elf"
        arm-none-eabi-size "${PROJECT_NAME}.elf"
    fi
    
    if [ -f "${PROJECT_NAME}.hex" ]; then
        print_success "✓ ${PROJECT_NAME}.hex"
        ls -lh "${PROJECT_NAME}.hex"
    fi
    
    if [ -f "${PROJECT_NAME}.bin" ]; then
        print_success "✓ ${PROJECT_NAME}.bin"
        ls -lh "${PROJECT_NAME}.bin"
    fi
}

# 主函数
main() {
    print_info "开始构建STM32F103 CMake项目"
    print_info "=================================="
    
    # 检查依赖
    check_cmake
    check_toolchain
    
    # 处理命令行参数
    case "${1:-build}" in
        "clean")
            clean_build
            print_success "清理完成"
            exit 0
            ;;
        "rebuild")
            clean_build
            ;;
        "build")
            ;;
        *)
            print_error "未知参数: $1"
            print_info "用法: $0 [build|clean|rebuild]"
            exit 1
            ;;
    esac
    
    # 构建流程
    create_build_dir
    configure_project
    build_project
    show_build_results
    
    print_success "构建完成!"
    print_info "=================================="
}

# 执行主函数
main "$@"