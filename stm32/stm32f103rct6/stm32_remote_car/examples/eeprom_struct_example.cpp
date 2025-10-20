/**
 * @file    eeprom_struct_example.cpp
 * @brief   EEPROM结构体读写示例（带CRC校验）
 * @author  AI Assistant
 * @date    2024
 * 
 * 本示例演示：
 * 1. 结构体的读写
 * 2. CRC校验的使用
 * 3. 实际应用场景：保存PID参数
 */

#include "eeprom.hpp"
#include "debug.hpp"
#include "stm32f1xx_hal.h"

extern "C" {
void SystemClock_Config(void);
}

/* ========== 数据结构定义 ========== */

/**
 * @brief PID参数结构体
 */
struct PIDParams {
    float kp;           ///< 比例系数
    float ki;           ///< 积分系数
    float kd;           ///< 微分系数
    float max_output;   ///< 最大输出
    float min_output;   ///< 最小输出
};

/**
 * @brief 巡线配置参数
 */
struct LineFollowConfig {
    uint16_t sensor_threshold[8];  ///< 8路传感器阈值
    float base_speed;               ///< 基础速度
    float turn_gain;                ///< 转向增益
    bool invert_sensors;            ///< 是否反转传感器
};

/**
 * @brief 系统配置
 */
struct SystemConfig {
    uint32_t magic_number;      ///< 魔术数字（用于验证配置有效性）
    uint8_t version;            ///< 配置版本
    uint8_t mode;               ///< 运行模式
    uint16_t runtime_hours;     ///< 运行时间（小时）
};

/* ========== 常量定义 ========== */

constexpr uint32_t CONFIG_MAGIC = 0xDEADBEEF;  ///< 配置魔术数字

/* EEPROM地址分配 */
constexpr uint8_t ADDR_SYSTEM_CONFIG = 0x00;   ///< 系统配置地址
constexpr uint8_t ADDR_PID_PARAMS = 0x20;      ///< PID参数地址
constexpr uint8_t ADDR_LINE_CONFIG = 0x40;     ///< 巡线配置地址

/* ========== 主程序 ========== */

extern "C" int main(void) {
    /* 1. HAL库初始化 */
    HAL_Init();
    SystemClock_Config();
    
    /* 2. 初始化调试串口 */
    MX_USART2_UART_Init();
    Debug_Enable();
    
    Debug_Printf("\r\n========== EEPROM结构体示例 ==========\r\n");
    
    /* 3. 初始化EEPROM */
    EEPROM eeprom;
    
    if (!eeprom.init()) {
        Debug_Printf("[ERROR] EEPROM初始化失败！\r\n");
        while (1);
    }
    Debug_Printf("[OK] EEPROM初始化成功\r\n\r\n");
    
    /* ===== 示例1: PID参数保存与读取 ===== */
    
    Debug_Printf("===== 示例1: PID参数（带CRC校验）=====\r\n");
    
    // 写入PID参数
    PIDParams pid_write = {
        .kp = 1.5f,
        .ki = 0.5f,
        .kd = 0.2f,
        .max_output = 100.0f,
        .min_output = -100.0f
    };
    
    Debug_Printf("写入PID参数:\r\n");
    Debug_Printf("  Kp = %.2f\r\n", pid_write.kp);
    Debug_Printf("  Ki = %.2f\r\n", pid_write.ki);
    Debug_Printf("  Kd = %.2f\r\n", pid_write.kd);
    Debug_Printf("  Max = %.2f\r\n", pid_write.max_output);
    Debug_Printf("  Min = %.2f\r\n", pid_write.min_output);
    
    if (eeprom.writeStructCRC(ADDR_PID_PARAMS, pid_write)) {
        Debug_Printf("[OK] PID参数写入成功（已附加CRC校验）\r\n");
    } else {
        Debug_Printf("[ERROR] PID参数写入失败\r\n");
    }
    
    // 读取PID参数
    PIDParams pid_read;
    if (eeprom.readStructCRC(ADDR_PID_PARAMS, pid_read)) {
        Debug_Printf("[OK] PID参数读取成功（CRC校验通过）\r\n");
        Debug_Printf("读取PID参数:\r\n");
        Debug_Printf("  Kp = %.2f\r\n", pid_read.kp);
        Debug_Printf("  Ki = %.2f\r\n", pid_read.ki);
        Debug_Printf("  Kd = %.2f\r\n", pid_read.kd);
        Debug_Printf("  Max = %.2f\r\n", pid_read.max_output);
        Debug_Printf("  Min = %.2f\r\n", pid_read.min_output);
        
        // 验证数据
        if (pid_read.kp == pid_write.kp && 
            pid_read.ki == pid_write.ki && 
            pid_read.kd == pid_write.kd) {
            Debug_Printf("[OK] 数据验证通过！\r\n");
        }
    } else {
        Debug_Printf("[ERROR] PID参数读取失败或CRC校验失败\r\n");
    }
    
    HAL_Delay(1000);
    
    /* ===== 示例2: 巡线配置保存 ===== */
    
    Debug_Printf("\r\n===== 示例2: 巡线配置 =====\r\n");
    
    LineFollowConfig line_config_write;
    
    // 设置传感器阈值
    for (int i = 0; i < 8; i++) {
        line_config_write.sensor_threshold[i] = 2000 + i * 100;
    }
    line_config_write.base_speed = 50.0f;
    line_config_write.turn_gain = 1.2f;
    line_config_write.invert_sensors = false;
    
    Debug_Printf("写入巡线配置:\r\n");
    Debug_Printf("  基础速度: %.1f\r\n", line_config_write.base_speed);
    Debug_Printf("  转向增益: %.2f\r\n", line_config_write.turn_gain);
    Debug_Printf("  传感器阈值: ");
    for (int i = 0; i < 8; i++) {
        Debug_Printf("%d ", line_config_write.sensor_threshold[i]);
    }
    Debug_Printf("\r\n");
    
    if (eeprom.writeStructCRC(ADDR_LINE_CONFIG, line_config_write)) {
        Debug_Printf("[OK] 巡线配置写入成功\r\n");
    } else {
        Debug_Printf("[ERROR] 巡线配置写入失败\r\n");
    }
    
    LineFollowConfig line_config_read;
    if (eeprom.readStructCRC(ADDR_LINE_CONFIG, line_config_read)) {
        Debug_Printf("[OK] 巡线配置读取成功（CRC校验通过）\r\n");
    } else {
        Debug_Printf("[ERROR] 巡线配置读取失败或CRC校验失败\r\n");
    }
    
    HAL_Delay(1000);
    
    /* ===== 示例3: 系统配置（首次使用检测）===== */
    
    Debug_Printf("\r\n===== 示例3: 系统配置（魔术数字验证）=====\r\n");
    
    SystemConfig sys_config;
    
    // 尝试读取系统配置
    if (eeprom.readStructCRC(ADDR_SYSTEM_CONFIG, sys_config)) {
        // CRC校验通过，检查魔术数字
        if (sys_config.magic_number == CONFIG_MAGIC) {
            Debug_Printf("[OK] 发现有效配置\r\n");
            Debug_Printf("  版本: %d\r\n", sys_config.version);
            Debug_Printf("  模式: %d\r\n", sys_config.mode);
            Debug_Printf("  运行时间: %d 小时\r\n", sys_config.runtime_hours);
            
            // 增加运行时间
            sys_config.runtime_hours++;
            eeprom.writeStructCRC(ADDR_SYSTEM_CONFIG, sys_config);
            Debug_Printf("[INFO] 运行时间已更新为 %d 小时\r\n", sys_config.runtime_hours);
        } else {
            Debug_Printf("[INFO] EEPROM未初始化，写入默认配置\r\n");
            goto write_default_config;
        }
    } else {
        Debug_Printf("[INFO] EEPROM数据无效，写入默认配置\r\n");
        
write_default_config:
        sys_config.magic_number = CONFIG_MAGIC;
        sys_config.version = 1;
        sys_config.mode = 0;
        sys_config.runtime_hours = 0;
        
        if (eeprom.writeStructCRC(ADDR_SYSTEM_CONFIG, sys_config)) {
            Debug_Printf("[OK] 默认配置写入成功\r\n");
        }
    }
    
    /* 7. 完成 */
    Debug_Printf("\r\n========== 示例运行完成 ==========\r\n");
    Debug_Printf("提示：\r\n");
    Debug_Printf("  1. 重新上电后，运行时间会自动累加\r\n");
    Debug_Printf("  2. CRC校验可以检测数据是否损坏\r\n");
    Debug_Printf("  3. 魔术数字可以判断EEPROM是否已初始化\r\n");
    
    while (1) {
        HAL_Delay(1000);
    }
}
