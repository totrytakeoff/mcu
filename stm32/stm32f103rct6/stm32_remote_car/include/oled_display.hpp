/**
 * @file    oled_display.hpp
 * @brief   SSD1315 OLED显示屏封装（使用u8g2库）
 * @author  AI Assistant
 * @date    2024
 * 
 * 硬件参数：
 * - 屏幕型号：0.96寸 OLED（SSD1315驱动）
 * - 分辨率：128x64
 * - 接口：4线I2C
 * - 连接：使用I2C2（PB10=SCL, PB11=SDA）
 * - I2C地址：0x3C (默认)
 * 
 * 性能指标：
 * - Flash占用：约9-10KB
 * - RAM占用：约1-2KB（帧缓冲）
 * - 刷新时间：10-15ms（100kHz I2C）
 * 
 * 使用示例：
 * @code
 * // 初始化
 * OLEDDisplay oled;
 * oled.init();
 * 
 * // 显示文本
 * oled.clear();
 * oled.printLine(0, "Speed: 50");
 * oled.printLine(1, "Pos: -12.5");
 * oled.show();
 * 
 * // 显示调试信息
 * oled.showDebugInfo("Running", 45, -8.5f, 2500);
 * @endcode
 */

#ifndef __OLED_DISPLAY_HPP
#define __OLED_DISPLAY_HPP

#include "stm32f1xx_hal.h"
#include "i2c.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// u8g2库前向声明
struct u8g2_struct;
typedef struct u8g2_struct u8g2_t;

/**
 * @class OLEDDisplay
 * @brief SSD1315 OLED显示屏封装类
 */
class OLEDDisplay {
public:
    /* ========== 常量定义 ========== */
    static constexpr uint8_t I2C_ADDRESS = 0x3C;  ///< I2C地址（7位）
    static constexpr uint8_t WIDTH = 128;          ///< 屏幕宽度
    static constexpr uint8_t HEIGHT = 64;          ///< 屏幕高度
    static constexpr uint8_t MAX_LINES = 6;        ///< 最大行数（使用8号字体）
    
    /* ========== 构造与初始化 ========== */
    
    /**
     * @brief 构造函数
     */
    OLEDDisplay();
    
    /**
     * @brief 析构函数
     */
    ~OLEDDisplay();
    
    /**
     * @brief 初始化OLED显示屏
     * @return true 初始化成功
     * @return false 初始化失败
     * 
     * @note 会自动初始化u8g2库和I2C总线
     */
    bool init();
    
    /**
     * @brief 检查是否已初始化
     * @return true 已初始化
     */
    bool isInitialized() const { return initialized_; }
    
    /* ========== 基本绘图操作 ========== */
    
    /**
     * @brief 清空显示缓冲区
     */
    void clear();
    
    /**
     * @brief 刷新显示（将缓冲区内容发送到屏幕）
     */
    void show();
    
    /**
     * @brief 设置显示对比度
     * @param value 对比度值 (0-255)
     */
    void setContrast(uint8_t value);
    
    /**
     * @brief 开启/关闭显示
     * @param on true=开启, false=关闭
     */
    void setPower(bool on);
    
    /**
     * @brief 设置字体
     * @param font 字体指针，可以使用u8g2的任意字体，例如：
     *             u8g2_font_5x7_tf        - 小字体
     *             u8g2_font_6x10_tf       - 中字体（默认）
     *             u8g2_font_10x20_tf      - 大字体
     *             u8g2_font_ncenB08_tr    - 8号粗体
     *             u8g2_font_unifont_t_chinese1 - 中文字体
     */
    void setFont(const uint8_t *font);
    
    /* ========== 文本显示 ========== */
    
    /**
     * @brief 在指定行显示文本（自动换行）
     * @param line 行号 (0-5)
     * @param text 要显示的文本
     */
    void printLine(uint8_t line, const char* text);
    
    /**
     * @brief 在指定位置显示文本
     * @param x X坐标 (像素)
     * @param y Y坐标 (像素)
     * @param text 要显示的文本
     */
    void printAt(uint8_t x, uint8_t y, const char* text);
    
    /**
     * @brief 在指定行显示格式化文本（类似printf）
     * @param line 行号 (0-5)
     * @param format 格式化字符串
     * @param ... 可变参数
     */
    void printfLine(uint8_t line, const char* format, ...);
    
    /* ========== 图形绘制 ========== */
    
    /**
     * @brief 画线
     * @param x0, y0 起点坐标
     * @param x1, y1 终点坐标
     */
    void drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
    
    /**
     * @brief 画矩形
     * @param x, y 左上角坐标
     * @param w, h 宽度和高度
     */
    void drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
    
    /**
     * @brief 画实心矩形
     * @param x, y 左上角坐标
     * @param w, h 宽度和高度
     */
    void drawBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
    
    /**
     * @brief 画圆
     * @param x, y 圆心坐标
     * @param r 半径
     */
    void drawCircle(uint8_t x, uint8_t y, uint8_t r);
    
    /* ========== 高级功能 ========== */
    
    /**
     * @brief 显示巡线车调试信息（一站式显示）
     * @param state 系统状态文本
     * @param speed 当前速度
     * @param position 线位置偏差
     * @param sensorValue 传感器原始值
     */
    void showDebugInfo(const char* state, int speed, float position, int sensorValue);
    
    /**
     * @brief 显示PID参数
     * @param kp, ki, kd PID三个参数
     */
    void showPIDParams(float kp, float ki, float kd);
    
    /**
     * @brief 显示进度条
     * @param x, y 左上角坐标
     * @param w, h 宽度和高度
     * @param percentage 进度百分比 (0-100)
     */
    void drawProgressBar(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t percentage);
    
    /**
     * @brief 显示欢迎界面
     */
    void showWelcome();
    
    /**
     * @brief 显示校准界面
     */
    void showCalibration();
    
private:
    u8g2_t* u8g2_;           ///< u8g2对象指针
    bool initialized_;       ///< 初始化标志
    char line_buffer_[32];   ///< 行缓冲区（用于格式化）
    
    /**
     * @brief 获取行的Y坐标
     * @param line 行号 (0-5)
     * @return Y坐标（像素）
     */
    uint8_t getLineY(uint8_t line) const;
};

#endif /* __OLED_DISPLAY_HPP */
