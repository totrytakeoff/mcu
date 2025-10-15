# TLE100 遥控器官方 Demo 分析

## 📋 硬件信息

### MCU
- **型号**: AT89S52 / STC89C52 (兼容)
- **架构**: 8051 (MCS-51)
- **时钟**: 11.0592MHz (典型) 或 12MHz
- **内存**: 
  - RAM: 256 字节
  - Flash: 8KB
- **串口**: 1 个 UART

### E49 无线模块引脚
- **M0**: P1.1
- **M1**: P1.2
- **模式**: 透传模式 (M0=0, M1=0)
- **TX/RX**: 通过 UART 连接

---

## 🎮 按键映射

### 方向键 (P0 口)
| 按键 | GPIO | 功能 | 发送字符 |
|------|------|------|---------|
| Left | P0.0 | 左转 | 'L' |
| Right | P0.1 | 右转 | 'R' |
| Forward | P0.2 | 前进 | 'F' |
| Back | P0.3 | 后退 | 'B' |

### 速度调节键 (P0 口)
| 按键 | GPIO | 功能 | 发送字符 |
|------|------|------|---------|
| UpSpeed | P0.4 | 加速 | 'U' |
| DownSpeed | P0.5 | 减速 | 'D' |

### 功能键 (P0/P1 口)
| 按键 | GPIO | 功能 | 发送字符 |
|------|------|------|---------|
| F1 | P0.6 | 功能1 | 'W' |
| F2 | P0.7 | 功能2 | 'X' |
| F3 | P1.3 | 功能3 | 'Y' |
| F4 | P1.4 | 功能4 | 'Z' |

### 开关 (P2 口) - 未使用
| 开关 | GPIO |
|------|------|
| S1-S8 | P2.0-P2.7 |

---

## 🔧 核心功能分析

### 1. UART 初始化 (9600 波特率)
```c
void init_9600(void)
{
    TMOD = 0x20;      // Timer1 模式2 (8位自动重装)
    TH1 = 0xFD;       // 波特率 9600 (11.0592MHz 晶振)
    TL1 = 0xFD;
    SCON = 0x50;      // 串口模式1 (8位 UART)
    PCON &= 0xef;     // 不倍频
    TR1 = 1;          // 启动 Timer1
    IE = 0x0;         // 禁用中断 (轮询模式)
}
```

**关键参数**：
- 波特率: 9600
- 数据位: 8
- 停止位: 1
- 校验: 无
- 模式: 轮询发送 (非中断)

### 2. E49 模块配置
```c
WX_M0 = 0;  // P1.1 = 0
WX_M1 = 0;  // P1.2 = 0
// 透传模式 (00)
```

### 3. 按键检测逻辑
```c
while(1)
{
    if(Left==0)    send_char('L');
    if(Right==0)   send_char('R');
    if(Forward==0) send_char('F');
    if(Back==0)    send_char('B');
    if(UpSpeed==0) send_char('U');
    if(DownSpeed==0) send_char('D');
    if(F1==0)      send_char('W');
    if(F2==0)      send_char('X');
    if(F3==0)      send_char('Y');
    if(F4==0)      send_char('Z');
    
    delaynms(200);  // 延时 200ms
}
```

**特点**：
- 按键低电平有效 (按下 = 0)
- 无按键去抖
- 简单轮询检测
- 每个循环延时 200ms → **发送间隔约 200ms，而非 1 秒！**

---

## ⚠️ 发现的问题

### 1. 发送间隔
- 代码中延时是 **200ms**，不是 1 秒
- 你测试时可能是遥控器电池电量低，或者延时被修改过

### 2. 无按键去抖
- 可能导致一次按下发送多次
- 需要添加去抖逻辑

### 3. 多键同时按下
- 如果同时按多个键，会连续发送多个字符
- 例如：同时按 F+L 会发送 "FL"

### 4. 无按键优先级
- 按键检测顺序：L → R → F → B → U → D → W → X → Y → Z
- 同时按下时，先检测到的先发送

---

## 🎯 改进目标

### 1. 添加按键去抖
- 软件去抖：连续检测 10-20ms
- 避免误触发

### 2. 单键发送
- 只发送一个按键（优先级最高的）
- 避免多键冲突

### 3. 持续按压检测
- 首次按下：立即发送
- 持续按压：每隔 100-200ms 重复发送
- 松开：立即停止

### 4. 增强功能
- 组合键支持 (例如：F+L = 前进+左转)
- LED 状态指示
- 低功耗模式

---

## 📊 PlatformIO 移植计划

### 项目配置
```ini
[env:STC89C52]
platform = intel_mcs51
board = STC89C52RC  ; 或 AT89S52
framework = 

; 编译选项
build_flags = 
    -DFOSC=11059200UL  ; 11.0592MHz 晶振

; 串口配置
upload_protocol = stcgal
upload_speed = 115200
monitor_speed = 9600
```

### 目录结构
```
remote_control_pio/
├── platformio.ini
├── include/
│   ├── config.h          # 引脚定义
│   └── uart.h            # UART 函数声明
├── src/
│   ├── main.c            # 主程序
│   ├── uart.c            # UART 实现
│   ├── keys.c            # 按键检测
│   └── e49.c             # E49 控制
└── docs/
    └── README.md
```

---

## 🔑 关键代码片段

### UART 发送 (原理)
```c
void send_char(unsigned char txd)
{
    SBUF = txd;       // 写入发送缓冲区
    while(!TI);       // 等待发送完成 (TI = 发送中断标志)
    TI = 0;           // 清除标志
}
```

### 延时函数 (11.0592MHz)
```c
void delay1ms()
{
    unsigned char i, j;
    for(i=0; i<10; i++)
        for(j=0; j<33; j++);
}

void delaynms(unsigned int n)
{
    unsigned int i;
    for(i=0; i<n; i++)
        delay1ms();
}
```

---

## 🚀 下一步

1. ✅ 分析完成
2. ⏭️ 创建 PlatformIO 项目
3. ⏭️ 移植代码
4. ⏭️ 添加改进功能
5. ⏭️ 测试验证

**准备好开始创建项目了！**
