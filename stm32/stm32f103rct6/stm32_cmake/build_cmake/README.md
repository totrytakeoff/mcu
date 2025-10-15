# STM32 CMake 构建目录

## 🚀 快速开始

### 配置和编译

```bash
# 1. 配置项目
cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake ..

# 2. 编译
make

# 3. 烧录
make flash
```

### 修改串口

```bash
cmake -DUPLOAD_PORT=COM7 -DUPLOAD_BAUD=115200 ..
```

## 📋 常用命令

```bash
make              # 编译
make flash        # 烧录（串口）
make serial_info  # 读取芯片信息
make clean        # 清理
```

## 📚 详细文档

完整使用说明请查看：[CMAKE_UPLOAD_GUIDE.md](../CMAKE_UPLOAD_GUIDE.md)

## ✅ 验证配置

串口烧录使用的 DTR/RTS 序列：`-dtr,rts,dtr:-rts`

此配置已在 PlatformIO 项目中验证可用。