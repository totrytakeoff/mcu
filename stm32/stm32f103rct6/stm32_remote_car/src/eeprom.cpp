/**
 * @file    eeprom.cpp
 * @brief   24C02 EEPROM封装实现
 * @author  AI Assistant
 * @date    2024
 */

#include "eeprom.hpp"

/**
 * @brief 初始化EEPROM
 */
bool EEPROM::init() {
    // 1. 初始化I2C总线
    MX_I2C2_Init();
    
    // 2. 检查设备是否在线
    if (!isDeviceReady()) {
        initialized_ = false;
        return false;
    }
    
    initialized_ = true;
    return true;
}

/**
 * @brief 写入单字节
 */
bool EEPROM::writeByte(uint8_t address, uint8_t data) {
    if (!initialized_ || !checkAddressRange(address, 1)) {
        return false;
    }
    
    // I2C写入：先发送内存地址，再发送数据
    uint8_t buffer[2] = {address, data};
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(
        &hi2c2,
        DEVICE_ADDRESS,
        buffer,
        2,
        I2C_TIMEOUT_MS
    );
    
    if (status != HAL_OK) {
        return false;
    }
    
    // 等待写周期完成
    waitForWriteCycle();
    return true;
}

/**
 * @brief 读取单字节
 */
bool EEPROM::readByte(uint8_t address, uint8_t& data) {
    if (!initialized_ || !checkAddressRange(address, 1)) {
        return false;
    }
    
    // 1. 发送内存地址
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(
        &hi2c2,
        DEVICE_ADDRESS,
        &address,
        1,
        I2C_TIMEOUT_MS
    );
    
    if (status != HAL_OK) {
        return false;
    }
    
    // 2. 读取数据
    status = HAL_I2C_Master_Receive(
        &hi2c2,
        DEVICE_ADDRESS,
        &data,
        1,
        I2C_TIMEOUT_MS
    );
    
    return (status == HAL_OK);
}

/**
 * @brief 写入多字节（自动处理页边界）
 */
bool EEPROM::writeBytes(uint8_t address, const uint8_t* data, uint16_t length) {
    if (!initialized_ || !checkAddressRange(address, length) || data == nullptr) {
        return false;
    }
    
    uint16_t bytes_written = 0;
    
    while (bytes_written < length) {
        // 计算当前页剩余空间
        uint16_t current_address = address + bytes_written;
        uint16_t page_offset = current_address % PAGE_SIZE;
        uint16_t page_remaining = PAGE_SIZE - page_offset;
        
        // 计算本次写入的字节数（不跨页边界）
        uint16_t bytes_to_write = (length - bytes_written) < page_remaining 
                                  ? (length - bytes_written) 
                                  : page_remaining;
        
        // 准备写入缓冲区：[地址][数据...]
        uint8_t buffer[PAGE_SIZE + 1];
        buffer[0] = current_address;
        memcpy(&buffer[1], &data[bytes_written], bytes_to_write);
        
        // I2C写入
        HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(
            &hi2c2,
            DEVICE_ADDRESS,
            buffer,
            bytes_to_write + 1,
            I2C_TIMEOUT_MS
        );
        
        if (status != HAL_OK) {
            return false;
        }
        
        // 等待写周期完成
        waitForWriteCycle();
        
        bytes_written += bytes_to_write;
    }
    
    return true;
}

/**
 * @brief 读取多字节
 */
bool EEPROM::readBytes(uint8_t address, uint8_t* data, uint16_t length) {
    if (!initialized_ || !checkAddressRange(address, length) || data == nullptr) {
        return false;
    }
    
    // 1. 发送起始地址
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(
        &hi2c2,
        DEVICE_ADDRESS,
        &address,
        1,
        I2C_TIMEOUT_MS
    );
    
    if (status != HAL_OK) {
        return false;
    }
    
    // 2. 顺序读取多个字节
    status = HAL_I2C_Master_Receive(
        &hi2c2,
        DEVICE_ADDRESS,
        data,
        length,
        I2C_TIMEOUT_MS
    );
    
    return (status == HAL_OK);
}

/**
 * @brief 清除EEPROM（全部写0xFF）
 */
bool EEPROM::clear() {
    if (!initialized_) {
        return false;
    }
    
    // 分页填充0xFF
    uint8_t buffer[PAGE_SIZE];
    memset(buffer, 0xFF, PAGE_SIZE);
    
    for (uint16_t addr = 0; addr < MEMORY_SIZE; addr += PAGE_SIZE) {
        if (!writeBytes(addr, buffer, PAGE_SIZE)) {
            return false;
        }
    }
    
    return true;
}

/**
 * @brief 填充指定区域
 */
bool EEPROM::fill(uint8_t address, uint8_t value, uint16_t length) {
    if (!initialized_ || !checkAddressRange(address, length)) {
        return false;
    }
    
    // 使用页缓冲区填充
    uint8_t buffer[PAGE_SIZE];
    memset(buffer, value, PAGE_SIZE);
    
    uint16_t bytes_written = 0;
    
    while (bytes_written < length) {
        uint16_t bytes_to_write = (length - bytes_written) < PAGE_SIZE 
                                  ? (length - bytes_written) 
                                  : PAGE_SIZE;
        
        if (!writeBytes(address + bytes_written, buffer, bytes_to_write)) {
            return false;
        }
        
        bytes_written += bytes_to_write;
    }
    
    return true;
}

/**
 * @brief 检查设备是否在线
 */
bool EEPROM::isDeviceReady() {
    // 使用HAL库的设备就绪检测
    // 尝试3次，每次超时100ms
    HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(
        &hi2c2,
        DEVICE_ADDRESS,
        3,
        I2C_TIMEOUT_MS
    );
    
    return (status == HAL_OK);
}

/**
 * @brief 计算CRC-8校验码
 * @note 使用CRC-8-CCITT多项式: 0x07
 */
uint8_t EEPROM::calculateCRC(const uint8_t* data, uint16_t length) {
    uint8_t crc = 0x00;
    
    for (uint16_t i = 0; i < length; i++) {
        crc ^= data[i];
        
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc <<= 1;
            }
        }
    }
    
    return crc;
}
