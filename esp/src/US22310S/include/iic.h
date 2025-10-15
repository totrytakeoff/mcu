#ifndef IIC_H
#define IIC_H

#include <Arduino.h>
#include <Wire.h>

// US22310S超声波模块I2C地址定义
// 根据官方文档：模块I2C总线地址为：0x72
// 这是8位地址格式（包含读写位），Arduino Wire库使用7位地址
// 转换关系：7位地址 = 8位地址 >> 1
// 0x72 >> 1 = 0x39 (7位写地址，Wire库自动添加写位0)
// 0x73 >> 1 = 0x39 (7位读地址，Wire库自动添加读位1)
#define US22310S_I2C_ADDR   0x39  // 7位I2C地址
#define US22310S_WRITE_ADDR 0x72  // 8位写地址（文档中的地址）
#define US22310S_READ_ADDR  0x73  // 8位读地址（0x72 + 0x01）

// 指令码定义
#define CMD_SINGLE_MEASURE 0x50   // 单次测量指令
#define CMD_THRESHOLD_MODE 0x51   // 阈值中断模式指令

// NodeMCU默认I2C引脚定义
#define NODEMCU_SCL_PIN 5   // D1 (GPIO5)
#define NODEMCU_SDA_PIN 4   // D2 (GPIO4)

class US22310S_I2C {
private:
    uint8_t _scl_pin;
    uint8_t _sda_pin;
    
public:
    // 构造函数
    US22310S_I2C(uint8_t scl_pin = NODEMCU_SCL_PIN, uint8_t sda_pin = NODEMCU_SDA_PIN);
    
    // 初始化I2C
    void begin();
    
    // 写数据到I2C设备
    bool write(uint8_t device_addr, uint8_t *data, uint8_t length);
    
    // 从I2C设备读取数据
    bool read(uint8_t device_addr, uint8_t *data, uint8_t length);
    
    // 发送单次测量命令
    bool startSingleMeasurement();
    
    // 设置阈值中断模式
    bool setThresholdMode(uint8_t threshold_mm);
    
    // 读取测量距离（单字节模式）
    bool readDistance(uint8_t *distance_mm);
    
    // 读取测量距离（双字节模式，支持更大距离）
    bool readDistance16(uint16_t *distance_mm);
    
    // 检查设备是否在线
    bool checkDevice();
};

#endif // IIC_H
