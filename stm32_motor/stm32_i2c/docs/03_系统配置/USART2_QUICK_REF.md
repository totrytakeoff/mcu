# USART2 串口调试 - 快速参考卡

## 🔌 硬件接线（3根线）

```
CH340模块    STM32板子
  GND   ---->  GND
  TXD   ---->  PA3 (RX2)
  RXD   ---->  PA2 (TX2)
```

VCC **不接**！

---

## ⚙️ 串口设置

```
波特率: 115200
数据位: 8
停止位: 1
校验位: 无
```

---

## 💻 代码模板

### 初始化
```c
MX_USART2_UART_Init();
HAL_Delay(100);
```

### 发送数据
```c
// 字符串
USART2_Print("Hello\r\n");

// 格式化输出
USART2_Printf("值: %d\r\n", 123);
USART2_Printf("温度: %.1f°C\r\n", 25.6f);
USART2_Printf("指令: %c\r\n", 'F');
```

---

## 📊 格式化占位符

| 类型 | 占位符 | 示例 |
|------|--------|------|
| 整数 | `%d` | `%d` → `123` |
| 无符号整数 | `%u` | `%u` → `255` |
| 长整型 | `%lu` | `%lu` → `1000000` |
| 浮点数 | `%.2f` | `%.2f` → `3.14` |
| 字符 | `%c` | `%c` → `A` |
| 字符串 | `%s` | `%s` → `Hello` |
| 十六进制 | `%X` | `%02X` → `FF` |

---

## 🎯 E49 集成示例

```c
// E49接收回调
void onE49DataReceived(uint8_t data) {
    USART2_Printf("收到: %c\r\n", data);
    
    switch(data) {
        case 'F': USART2_Print("前进\r\n"); break;
        case 'B': USART2_Print("后退\r\n"); break;
        case 'L': USART2_Print("左转\r\n"); break;
        case 'R': USART2_Print("右转\r\n"); break;
    }
}
```

---

## ⚠️ 注意

- ✅ 使用 `\r\n` 换行
- ✅ 缓冲区限制 256 字节
- ❌ 不要在高频中断中用
- ❌ VCC 不要接

---

**完整文档**: `USART2_DEBUG_GUIDE.md`
