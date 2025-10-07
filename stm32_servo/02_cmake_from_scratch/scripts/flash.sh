#!/bin/bash
# =============================================================================
# 烧录脚本 - STM32F103项目
# =============================================================================

set -e

# 项目配置
PROJECT_NAME="stm32f103_cmake_demo"
BUILD_DIR="build"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

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

# 检查OpenOCD
check_openocd() {
    print_info "检查OpenOCD..."
    
    if ! command -v openocd &> /dev/null; then
        print_error "错误: openocd 未找到!"
        print_info "请安装OpenOCD或确保其在PATH中"
        exit 1
    fi
    
    print_success "OpenOCD检查通过"
    openocd --version 2>&1 | head -1
}

# 检查ST-Link
check_stlink() {
    print_info "检查ST-Link连接..."
    
    # 尝试连接ST-Link
    openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c "init; exit" &>/dev/null
    
    if [ $? -eq 0 ]; then
        print_success "ST-Link连接正常"
    else
        print_warning "ST-Link连接检查失败，请确保:"
        print_warning "1. ST-Link已连接到PC"
        print_warning "2. 目标板已上电"
        print_warning "3. 连接线正常"
    fi
}

# 检查hex文件
check_hex_file() {
    HEX_FILE="${BUILD_DIR}/${PROJECT_NAME}.hex"
    
    if [ ! -f "$HEX_FILE" ]; then
        print_error "错误: hex文件不存在: $HEX_FILE"
        print_info "请先构建项目: ./scripts/build.sh"
        exit 1
    fi
    
    print_success "找到hex文件: $HEX_FILE"
    ls -lh "$HEX_FILE"
}

# 烧录程序
flash_program() {
    print_info "开始烧录程序..."
    
    HEX_FILE="${BUILD_DIR}/${PROJECT_NAME}.hex"
    
    openocd \
        -f interface/stlink.cfg \
        -f target/stm32f1x.cfg \
        -c "program $HEX_FILE verify reset exit"
    
    if [ $? -eq 0 ]; then
        print_success "程序烧录成功!"
    else
        print_error "程序烧录失败!"
        exit 1
    fi
}

# 擦除芯片
erase_chip() {
    print_info "擦除芯片..."
    
    openocd \
        -f interface/stlink.cfg \
        -f target/stm32f1x.cfg \
        -c "init; halt; stm32f1x mass_erase 0; exit"
    
    if [ $? -eq 0 ]; then
        print_success "芯片擦除成功!"
    else
        print_error "芯片擦除失败!"
        exit 1
    fi
}

# 读取芯片信息
read_chip_info() {
    print_info "读取芯片信息..."
    
    openocd \
        -f interface/stlink.cfg \
        -f target/stm32f1x.cfg \
        -c "init; halt; flash info 0; exit"
}

# 启动调试服务器
start_debug_server() {
    print_info "启动OpenOCD调试服务器..."
    print_info "GDB端口: 3333"
    print_info "Telnet端口: 4444"
    print_info "按Ctrl+C退出"
    
    openocd \
        -f interface/stlink.cfg \
        -f target/stm32f1x.cfg
}

# 显示帮助信息
show_help() {
    echo "STM32F103 烧录脚本"
    echo ""
    echo "用法: $0 [command]"
    echo ""
    echo "命令:"
    echo "  flash    - 烧录程序 (默认)"
    echo "  erase    - 擦除芯片"
    echo "  info     - 读取芯片信息"
    echo "  debug    - 启动调试服务器"
    echo "  help     - 显示帮助信息"
    echo ""
    echo "示例:"
    echo "  $0 flash    # 烧录程序"
    echo "  $0 erase    # 擦除芯片"
    echo "  $0 debug    # 启动调试服务器"
}

# 主函数
main() {
    print_info "STM32F103 烧录工具"
    print_info "=================="
    
    # 处理命令行参数
    case "${1:-flash}" in
        "flash")
            check_openocd
            check_hex_file
            check_stlink
            flash_program
            ;;
        "erase")
            check_openocd
            check_stlink
            erase_chip
            ;;
        "info")
            check_openocd
            check_stlink
            read_chip_info
            ;;
        "debug")
            check_openocd
            check_stlink
            start_debug_server
            ;;
        "help"|"-h"|"--help")
            show_help
            exit 0
            ;;
        *)
            print_error "未知命令: $1"
            show_help
            exit 1
            ;;
    esac
    
    print_info "=================="
}

# 执行主函数
main "$@"