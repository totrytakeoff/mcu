/**
 * @file    eeprom.hpp
 * @brief   24C02 EEPROM封装类（类型安全、简单易用）
 * @author  AI Assistant
 * @date    2024
 * 
 * 功能特性：
 * - ✅ 类型安全：支持基本类型和结构体的读写
 * - ✅ 自动管理：自动处理写延迟和页边界
 * - ✅ 数据校验：支持CRC校验防止数据损坏
 * - ✅ 简单易用：一行代码完成读写操作
 * 
 * 使用示例：
 * @code
 * // 初始化
 * EEPROM eeprom;
 * if (!eeprom.init()) {
 *     // 初始化失败
 * }
 * 
 * // 写入单个变量
 * float kp = 1.5f;
 * eeprom.write(0x00, kp);
 * 
 * // 读取单个变量
 * float kp_read = 0.0f;
 * eeprom.read(0x00, kp_read);
 * 
 * // 写入结构体
 * struct PIDParams {
 *     float kp, ki, kd;
 * };
 * PIDParams params = {1.5f, 0.5f, 0.2f};
 * eeprom.writeStruct(0x10, params);
 * 
 * // 读取结构体（带CRC校验）
 * PIDParams params_read;
 * if (eeprom.readStructCRC(0x10, params_read)) {
 *     // 数据有效
 * } else {
 *     // 数据损坏或未初始化
 * }
 * @endcode
 * 
 * 内存布局建议：
 * - 0x00-0x0F: 基本配置参数（16字节）
 * - 0x10-0x3F: PID参数等（48字节）
 * - 0x40-0x7F: 传感器校准数据（64字节）
 * - 0x80-0xFF: 用户自定义数据（128字节）
 */

#ifndef __EEPROM_HPP
#define __EEPROM_HPP

#include "stm32f1xx_hal.h"
#include "i2c.h"
#include <stdint.h>
#include <string.h>

/**
 * @class EEPROM
 * @brief 24C02 EEPROM封装类
 * 
 * 硬件参数：
 * - 型号: 24C02
 * - 容量: 256字节
 * - 地址范围: 0x00 ~ 0xFF
 * - 页大小: 8字节
 * - 器件地址: 0x50 (7-bit)
 * - 写周期: 5ms
 */
class EEPROM {
public:
    /* ========== 常量定义 ========== */
    
    // 注意：根据你的硬件配置，A0=GND, A1=3.3V, A2=GND
    // 地址计算：1 0 1 0 A2 A1 A0 = 1 0 1 0 0 1 0 = 0x52
    static constexpr uint16_t DEVICE_ADDRESS = 0x52 << 1;  ///< 器件地址（HAL库使用8位地址）
    static constexpr uint16_t MEMORY_SIZE = 256;           ///< 存储容量（字节）
    static constexpr uint16_t PAGE_SIZE = 8;               ///< 页大小（字节）
    static constexpr uint16_t WRITE_DELAY_MS = 5;          ///< 写周期延迟（毫秒）
    static constexpr uint32_t I2C_TIMEOUT_MS = 100;        ///< I2C超时时间（毫秒）
    
    /* ========== 构造与初始化 ========== */
    
    /**
     * @brief 构造函数
     */
    EEPROM() : initialized_(false) {}
    
    /**
     * @brief 初始化EEPROM
     * @return true 初始化成功
     * @return false 初始化失败
     * 
     * @note 会自动调用MX_I2C2_Init()初始化I2C总线
     */
    bool init();
    
    /**
     * @brief 检查EEPROM是否已初始化
     * @return true 已初始化
     */
    bool isInitialized() const { return initialized_; }
    
    /* ========== 基本读写操作 ========== */
    
    /**
     * @brief 写入单字节
     * @param address 内存地址 (0x00-0xFF)
     * @param data 要写入的数据
     * @return true 写入成功
     * @return false 写入失败或地址越界
     */
    bool writeByte(uint8_t address, uint8_t data);
    
    /**
     * @brief 读取单字节
     * @param address 内存地址 (0x00-0xFF)
     * @param data 读取的数据
     * @return true 读取成功
     * @return false 读取失败或地址越界
     */
    bool readByte(uint8_t address, uint8_t& data);
    
    /**
     * @brief 写入多字节（自动处理页边界）
     * @param address 起始地址
     * @param data 数据指针
     * @param length 数据长度
     * @return true 写入成功
     * @return false 写入失败或越界
     * 
     * @note 自动处理页边界，无需手动分页
     */
    bool writeBytes(uint8_t address, const uint8_t* data, uint16_t length);
    
    /**
     * @brief 读取多字节
     * @param address 起始地址
     * @param data 数据缓冲区
     * @param length 读取长度
     * @return true 读取成功
     * @return false 读取失败或越界
     */
    bool readBytes(uint8_t address, uint8_t* data, uint16_t length);
    
    /* ========== 模板化读写（类型安全）========== */
    
    /**
     * @brief 写入任意类型数据
     * @tparam T 数据类型
     * @param address 内存地址
     * @param value 要写入的值
     * @return true 写入成功
     * @return false 写入失败
     * 
     * @example
     * @code
     * float kp = 1.5f;
     * eeprom.write(0x00, kp);
     * 
     * int counter = 100;
     * eeprom.write(0x10, counter);
     * @endcode
     */
    template<typename T>
    bool write(uint8_t address, const T& value) {
        return writeBytes(address, reinterpret_cast<const uint8_t*>(&value), sizeof(T));
    }
    
    /**
     * @brief 读取任意类型数据
     * @tparam T 数据类型
     * @param address 内存地址
     * @param value 读取的值
     * @return true 读取成功
     * @return false 读取失败
     * 
     * @example
     * @code
     * float kp;
     * eeprom.read(0x00, kp);
     * 
     * int counter;
     * eeprom.read(0x10, counter);
     * @endcode
     */
    template<typename T>
    bool read(uint8_t address, T& value) {
        return readBytes(address, reinterpret_cast<uint8_t*>(&value), sizeof(T));
    }
    
    /* ========== 结构体读写（带CRC校验）========== */
    
    /**
     * @brief 写入结构体（不带CRC）
     * @tparam T 结构体类型
     * @param address 起始地址
     * @param data 结构体引用
     * @return true 写入成功
     * @return false 写入失败
     */
    template<typename T>
    bool writeStruct(uint8_t address, const T& data) {
        return write(address, data);
    }
    
    /**
     * @brief 读取结构体（不带CRC）
     * @tparam T 结构体类型
     * @param address 起始地址
     * @param data 结构体引用
     * @return true 读取成功
     * @return false 读取失败
     */
    template<typename T>
    bool readStruct(uint8_t address, T& data) {
        return read(address, data);
    }
    
    /**
     * @brief 写入结构体（带CRC校验）
     * @tparam T 结构体类型
     * @param address 起始地址
     * @param data 结构体引用
     * @return true 写入成功
     * @return false 写入失败或空间不足
     * 
     * @note 会在数据后附加1字节CRC-8校验码
     */
    template<typename T>
    bool writeStructCRC(uint8_t address, const T& data) {
        // 检查空间是否足够（数据 + 1字节CRC）
        if (address + sizeof(T) + 1 > MEMORY_SIZE) {
            return false;
        }
        
        // 计算CRC
        uint8_t crc = calculateCRC(reinterpret_cast<const uint8_t*>(&data), sizeof(T));
        
        // 写入数据
        if (!write(address, data)) {
            return false;
        }
        
        // 写入CRC
        return writeByte(address + sizeof(T), crc);
    }
    
    /**
     * @brief 读取结构体（带CRC校验）
     * @tparam T 结构体类型
     * @param address 起始地址
     * @param data 结构体引用
     * @return true 读取成功且CRC校验通过
     * @return false 读取失败或CRC校验失败
     * 
     * @note 会验证数据后的CRC校验码
     */
    template<typename T>
    bool readStructCRC(uint8_t address, T& data) {
        // 检查地址范围
        if (address + sizeof(T) + 1 > MEMORY_SIZE) {
            return false;
        }
        
        // 读取数据
        if (!read(address, data)) {
            return false;
        }
        
        // 读取CRC
        uint8_t stored_crc = 0;
        if (!readByte(address + sizeof(T), stored_crc)) {
            return false;
        }
        
        // 验证CRC
        uint8_t calculated_crc = calculateCRC(reinterpret_cast<const uint8_t*>(&data), sizeof(T));
        return (stored_crc == calculated_crc);
    }
    
    /* ========== 辅助功能 ========== */
    
    /**
     * @brief 清除EEPROM（全部写0xFF）
     * @return true 清除成功
     * @return false 清除失败
     * 
     * @warning 此操作会擦除所有数据，耗时约1.3秒
     */
    bool clear();
    
    /**
     * @brief 填充指定区域
     * @param address 起始地址
     * @param value 填充值
     * @param length 填充长度
     * @return true 填充成功
     * @return false 填充失败
     */
    bool fill(uint8_t address, uint8_t value, uint16_t length);
    
    /**
     * @brief 检查设备是否在线
     * @return true 设备在线
     * @return false 设备离线
     */
    bool isDeviceReady();
    
private:
    /* ========== 私有成员 ========== */
    
    bool initialized_;  ///< 初始化标志
    
    /**
     * @brief 检查地址范围
     * @param address 起始地址
     * @param length 数据长度
     * @return true 地址有效
     */
    bool checkAddressRange(uint16_t address, uint16_t length) const {
        return (address + length <= MEMORY_SIZE);
    }
    
    /**
     * @brief 等待写周期完成
     */
    void waitForWriteCycle() {
        HAL_Delay(WRITE_DELAY_MS);
    }
    
    /**
     * @brief 计算CRC-8校验码
     * @param data 数据指针
     * @param length 数据长度
     * @return CRC-8校验码
     * 
     * @note 使用CRC-8-CCITT多项式: x^8 + x^2 + x + 1 (0x07)
     */
    uint8_t calculateCRC(const uint8_t* data, uint16_t length);
};

#endif /* __EEPROM_HPP */

