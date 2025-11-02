/**
 * @file    oled_display.cpp
 * @brief   SSD1315 OLED显示屏实现
 * @author  AI Assistant
 * @date    2024
 */

#include "oled_display.hpp"
#include <U8g2lib.h>  // u8g2 C++封装（包含C核心）
#include <stdarg.h>

/* ========== u8g2 HAL回调函数 ========== */

/**
 * @brief I2C发送回调函数（u8g2调用）
 * @param u8x8 u8x8对象指针
 * @param msg 消息类型
 * @param arg_int 整数参数
 * @param arg_ptr 指针参数
 * @return 成功返回1，失败返回0
 */
extern "C" uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    static uint8_t buffer[128];
    static uint8_t buf_idx = 0;
    uint8_t *data;
    
    switch(msg) {
        case U8X8_MSG_BYTE_INIT:
            // I2C已在main中初始化
            break;
            
        case U8X8_MSG_BYTE_START_TRANSFER:
            buf_idx = 0;
            break;
            
        case U8X8_MSG_BYTE_SEND:
            data = (uint8_t *)arg_ptr;
            while(arg_int > 0) {
                buffer[buf_idx++] = *data;
                data++;
                arg_int--;
            }
            break;
            
        case U8X8_MSG_BYTE_END_TRANSFER:
            // 发送I2C数据
            if (HAL_I2C_Master_Transmit(&hi2c2, 
                                        OLEDDisplay::I2C_ADDRESS << 1, 
                                        buffer, 
                                        buf_idx, 
                                        100) != HAL_OK) {
                return 0;
            }
            break;
            
        default:
            return 0;
    }
    
    return 1;
}

/**
 * @brief GPIO和延时回调函数（u8g2调用）
 * @param u8x8 u8x8对象指针
 * @param msg 消息类型
 * @param arg_int 整数参数
 * @param arg_ptr 指针参数
 * @return 成功返回1
 */
extern "C" uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch(msg) {
        case U8X8_MSG_DELAY_NANO:
            // 纳秒延时（忽略，STM32太快）
            break;
            
        case U8X8_MSG_DELAY_100NANO:
            // 100纳秒延时
            __NOP();
            break;
            
        case U8X8_MSG_DELAY_10MICRO:
            // 10微秒延时
            for(uint16_t n = 0; n < 72; n++) { __NOP(); }
            break;
            
        case U8X8_MSG_DELAY_MILLI:
            // 毫秒延时
            HAL_Delay(arg_int);
            break;
            
        case U8X8_MSG_GPIO_I2C_CLOCK:
        case U8X8_MSG_GPIO_I2C_DATA:
            // 硬件I2C不需要控制GPIO
            break;
            
        default:
            return 0;
    }
    
    return 1;
}

/* ========== OLEDDisplay类实现 ========== */

OLEDDisplay::OLEDDisplay() 
    : u8g2_(nullptr), initialized_(false) {
    memset(line_buffer_, 0, sizeof(line_buffer_));
}

OLEDDisplay::~OLEDDisplay() {
    if (u8g2_) {
        delete u8g2_;
    }
}

bool OLEDDisplay::init() {
    if (initialized_) {
        return true;
    }
    
    // 分配u8g2对象
    u8g2_ = new u8g2_t;
    if (!u8g2_) {
        return false;
    }
    
    // 初始化u8g2（SSD1306兼容SSD1315）
    // 使用硬件I2C，无旋转，128x64分辨率
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(
        u8g2_,
        U8G2_R0,  // 不旋转
        u8x8_byte_hw_i2c,
        u8x8_gpio_and_delay
    );
    
    // 设置I2C地址
    u8g2_SetI2CAddress(u8g2_, I2C_ADDRESS << 1);
    
    // 初始化显示
    u8g2_InitDisplay(u8g2_);
    u8g2_SetPowerSave(u8g2_, 0);  // 唤醒显示
    u8g2_ClearDisplay(u8g2_);
    
    // 设置字体（8像素高度，适合6行显示）
    u8g2_SetFont(u8g2_, u8g2_font_6x10_tf);
    
    initialized_ = true;
    return true;
}

void OLEDDisplay::clear() {
    if (!initialized_) return;
    u8g2_ClearBuffer(u8g2_);
}

void OLEDDisplay::show() {
    if (!initialized_) return;
    u8g2_SendBuffer(u8g2_);
}

void OLEDDisplay::setContrast(uint8_t value) {
    if (!initialized_) return;
    u8g2_SetContrast(u8g2_, value);
}

void OLEDDisplay::setPower(bool on) {
    if (!initialized_) return;
    u8g2_SetPowerSave(u8g2_, on ? 0 : 1);
}

void OLEDDisplay::setFont(const uint8_t *font) {
    if (!initialized_) return;
    u8g2_SetFont(u8g2_, font);
}

uint8_t OLEDDisplay::getLineY(uint8_t line) const {
    // 每行10像素高度（字体8px + 2px间距）
    return (line + 1) * 10;
}

void OLEDDisplay::printLine(uint8_t line, const char* text) {
    if (!initialized_ || line >= MAX_LINES) return;
    u8g2_DrawStr(u8g2_, 0, getLineY(line), text);
}

void OLEDDisplay::printAt(uint8_t x, uint8_t y, const char* text) {
    if (!initialized_) return;
    u8g2_DrawStr(u8g2_, x, y, text);
}

void OLEDDisplay::printfLine(uint8_t line, const char* format, ...) {
    if (!initialized_ || line >= MAX_LINES) return;
    
    va_list args;
    va_start(args, format);
    vsnprintf(line_buffer_, sizeof(line_buffer_), format, args);
    va_end(args);
    
    printLine(line, line_buffer_);
}

void OLEDDisplay::drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    if (!initialized_) return;
    u8g2_DrawLine(u8g2_, x0, y0, x1, y1);
}

void OLEDDisplay::drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    if (!initialized_) return;
    u8g2_DrawFrame(u8g2_, x, y, w, h);
}

void OLEDDisplay::drawBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    if (!initialized_) return;
    u8g2_DrawBox(u8g2_, x, y, w, h);
}

void OLEDDisplay::drawCircle(uint8_t x, uint8_t y, uint8_t r) {
    if (!initialized_) return;
    u8g2_DrawCircle(u8g2_, x, y, r, U8G2_DRAW_ALL);
}

void OLEDDisplay::showDebugInfo(const char* state, int speed, float position, int sensorValue) {
    clear();
    
    // 第0行：状态
    printfLine(0, "State: %s", state);
    
    // 第1行：速度
    printfLine(1, "Speed: %d", speed);
    
    // 第2行：位置偏差
    printfLine(2, "Pos: %.1f", position);
    
    // 第3行：传感器值
    printfLine(3, "Sensor: %d", sensorValue);
    
    show();
}

void OLEDDisplay::showPIDParams(float kp, float ki, float kd) {
    clear();
    
    printLine(0, "PID Parameters");
    printfLine(1, "Kp: %.3f", kp);
    printfLine(2, "Ki: %.3f", ki);
    printfLine(3, "Kd: %.3f", kd);
    
    show();
}

void OLEDDisplay::drawProgressBar(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t percentage) {
    if (!initialized_) return;
    
    // 限制百分比范围
    if (percentage > 100) percentage = 100;
    
    // 画外框
    drawRect(x, y, w, h);
    
    // 画填充
    uint8_t fill_width = (w - 2) * percentage / 100;
    if (fill_width > 0) {
        drawBox(x + 1, y + 1, fill_width, h - 2);
    }
}

void OLEDDisplay::showWelcome() {
    clear();
    
    // 居中显示欢迎信息
    printAt(20, 20, "STM32 Car");
    printAt(10, 35, "Line Follower");
    printAt(30, 50, "v1.0");
    
    show();
}

void OLEDDisplay::showCalibration() {
    clear();
    
    printLine(0, "Calibrating...");
    printLine(2, "Move sensor");
    printLine(3, "over black");
    printLine(4, "and white");
    
    // 显示进度条（示例）
    drawProgressBar(10, 55, 108, 8, 50);
    
    show();
}
