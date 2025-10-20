/**
 * @file    eeprom_pid_config.cpp
 * @brief   EEPROM实战示例：PID参数保存与加载
 * @author  AI Assistant
 * @date    2024
 * 
 * 本示例演示：
 * 1. 保存和加载PID参数
 * 2. 首次使用检测（魔术数字）
 * 3. 版本兼容性处理
 * 4. 数据完整性校验（CRC）
 * 
 * 应用场景：
 * - 巡线PID参数持久化
 * - 差速转向PID参数持久化
 * - 支持通过串口动态调参后保存
 */

#include "eeprom.hpp"
#include "debug.hpp"
#include "stm32f1xx_hal.h"

extern "C" {
void SystemClock_Config(void);
}

/* ========== 配置结构体定义 ========== */

/**
 * @brief PID参数配置（版本2）
 */
struct __attribute__((packed)) PIDConfigV2 {
    // 元数据
    uint32_t magic_number;      ///< 魔术数字（用于验证配置有效性）
    uint8_t version;            ///< 配置版本号（当前版本=2）
    
    // 巡线PID
    float line_kp;              ///< 巡线比例系数
    float line_ki;              ///< 巡线积分系数
    float line_kd;              ///< 巡线微分系数
    
    // 速度PID
    float speed_kp;             ///< 速度比例系数
    float speed_ki;             ///< 速度积分系数
    float speed_kd;             ///< 速度微分系数
    
    // 运行参数
    float base_speed;           ///< 基础速度
    float max_speed;            ///< 最大速度
    
    // 统计信息（V2新增）
    uint32_t total_runtime_sec; ///< 总运行时间（秒）
    uint16_t save_count;        ///< 保存次数
};

/* ========== 常量定义 ========== */

constexpr uint32_t PID_CONFIG_MAGIC = 0xCAFEBABE;  ///< 配置魔术数字
constexpr uint8_t PID_CONFIG_VERSION = 2;          ///< 当前配置版本
constexpr uint8_t ADDR_PID_CONFIG = 0x00;          ///< PID配置存储地址

/**
 * @brief 默认PID配置
 */
const PIDConfigV2 DEFAULT_PID_CONFIG = {
    .magic_number = PID_CONFIG_MAGIC,
    .version = PID_CONFIG_VERSION,
    
    // 默认巡线PID
    .line_kp = 1.5f,
    .line_ki = 0.0f,
    .line_kd = 0.3f,
    
    // 默认速度PID
    .speed_kp = 1.0f,
    .speed_ki = 0.1f,
    .speed_kd = 0.0f,
    
    // 默认运行参数
    .base_speed = 40.0f,
    .max_speed = 80.0f,
    
    // 统计信息
    .total_runtime_sec = 0,
    .save_count = 0
};

/* ========== 全局变量 ========== */

EEPROM eeprom;
PIDConfigV2 current_config;

/* ========== 辅助函数 ========== */

/**
 * @brief 加载PID配置
 * @return true 加载成功
 * @return false 加载失败，已恢复默认值
 */
bool loadPIDConfig() {
    Debug_Printf("\r\n[CONFIG] 正在加载PID配置...\r\n");
    
    // 尝试从EEPROM读取配置
    if (eeprom.readStructCRC(ADDR_PID_CONFIG, current_config)) {
        Debug_Printf("[CONFIG] 配置读取成功（CRC校验通过）\r\n");
        
        // 验证魔术数字
        if (current_config.magic_number == PID_CONFIG_MAGIC) {
            Debug_Printf("[CONFIG] 配置有效（魔术数字匹配）\r\n");
            Debug_Printf("[CONFIG] 配置版本: %d\r\n", current_config.version);
            Debug_Printf("[CONFIG] 保存次数: %d\r\n", current_config.save_count);
            Debug_Printf("[CONFIG] 运行时间: %lu 秒\r\n", current_config.total_runtime_sec);
            
            // 检查版本兼容性
            if (current_config.version < PID_CONFIG_VERSION) {
                Debug_Printf("[CONFIG] 检测到旧版本配置，升级到版本 %d\r\n", PID_CONFIG_VERSION);
                // 这里可以做版本迁移
                current_config.version = PID_CONFIG_VERSION;
                // V1->V2: 补充新增字段默认值
                current_config.total_runtime_sec = 0;
                current_config.save_count = 0;
                return false;  // 返回false以触发保存
            }
            
            return true;
        } else {
            Debug_Printf("[CONFIG] 配置无效（魔术数字不匹配）\r\n");
            Debug_Printf("[CONFIG] 可能是首次使用或数据损坏\r\n");
        }
    } else {
        Debug_Printf("[CONFIG] 配置读取失败或CRC校验失败\r\n");
    }
    
    // 加载失败，使用默认配置
    Debug_Printf("[CONFIG] 加载默认PID配置\r\n");
    current_config = DEFAULT_PID_CONFIG;
    return false;
}

/**
 * @brief 保存PID配置
 * @return true 保存成功
 */
bool savePIDConfig() {
    Debug_Printf("\r\n[CONFIG] 正在保存PID配置...\r\n");
    
    // 更新保存计数
    current_config.save_count++;
    
    // 写入EEPROM（带CRC校验）
    if (eeprom.writeStructCRC(ADDR_PID_CONFIG, current_config)) {
        Debug_Printf("[CONFIG] 配置保存成功（第 %d 次）\r\n", current_config.save_count);
        return true;
    } else {
        Debug_Printf("[CONFIG] 配置保存失败！\r\n");
        return false;
    }
}

/**
 * @brief 打印当前PID配置
 */
void printPIDConfig() {
    Debug_Printf("\r\n========== 当前PID配置 ==========\r\n");
    Debug_Printf("巡线PID:\r\n");
    Debug_Printf("  Kp = %.3f\r\n", current_config.line_kp);
    Debug_Printf("  Ki = %.3f\r\n", current_config.line_ki);
    Debug_Printf("  Kd = %.3f\r\n", current_config.line_kd);
    Debug_Printf("\r\n");
    
    Debug_Printf("速度PID:\r\n");
    Debug_Printf("  Kp = %.3f\r\n", current_config.speed_kp);
    Debug_Printf("  Ki = %.3f\r\n", current_config.speed_ki);
    Debug_Printf("  Kd = %.3f\r\n", current_config.speed_kd);
    Debug_Printf("\r\n");
    
    Debug_Printf("运行参数:\r\n");
    Debug_Printf("  基础速度 = %.1f\r\n", current_config.base_speed);
    Debug_Printf("  最大速度 = %.1f\r\n", current_config.max_speed);
    Debug_Printf("\r\n");
    
    Debug_Printf("统计信息:\r\n");
    Debug_Printf("  总运行时间 = %lu 秒 (%.1f 小时)\r\n", 
                 current_config.total_runtime_sec,
                 current_config.total_runtime_sec / 3600.0f);
    Debug_Printf("  配置保存次数 = %d\r\n", current_config.save_count);
    Debug_Printf("===================================\r\n");
}

/**
 * @brief 模拟参数调整
 */
void tuneParameters() {
    Debug_Printf("\r\n[TUNE] 开始参数调优...\r\n");
    
    // 模拟调整巡线PID
    current_config.line_kp = 2.0f;
    current_config.line_kd = 0.5f;
    Debug_Printf("[TUNE] 调整巡线PID: Kp=%.2f, Kd=%.2f\r\n", 
                 current_config.line_kp, current_config.line_kd);
    
    // 模拟调整速度
    current_config.base_speed = 50.0f;
    Debug_Printf("[TUNE] 调整基础速度: %.1f\r\n", current_config.base_speed);
    
    Debug_Printf("[TUNE] 参数调优完成\r\n");
}

/**
 * @brief 恢复出厂设置
 */
void factoryReset() {
    Debug_Printf("\r\n[RESET] 恢复出厂设置...\r\n");
    
    // 保留统计信息
    uint32_t runtime = current_config.total_runtime_sec;
    uint16_t saves = current_config.save_count;
    
    // 恢复默认配置
    current_config = DEFAULT_PID_CONFIG;
    
    // 恢复统计信息
    current_config.total_runtime_sec = runtime;
    current_config.save_count = saves;
    
    Debug_Printf("[RESET] 出厂设置已恢复\r\n");
}

/* ========== 主程序 ========== */

extern "C" int main(void) {
    /* 1. HAL库初始化 */
    HAL_Init();
    SystemClock_Config();
    
    /* 2. 初始化调试串口 */
    MX_USART2_UART_Init();
    Debug_Enable();
    
    Debug_Printf("\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("   EEPROM PID配置管理系统示例\r\n");
    Debug_Printf("========================================\r\n");
    
    /* 3. 初始化EEPROM */
    Debug_Printf("\r\n[INIT] 正在初始化EEPROM...\r\n");
    if (!eeprom.init()) {
        Debug_Printf("[ERROR] EEPROM初始化失败！\r\n");
        Debug_Printf("[ERROR] 请检查硬件连接\r\n");
        while (1);
    }
    Debug_Printf("[OK] EEPROM初始化成功\r\n");
    
    /* 4. 加载配置 */
    bool config_valid = loadPIDConfig();
    
    if (!config_valid) {
        Debug_Printf("[INFO] 将在3秒后保存默认配置...\r\n");
        HAL_Delay(3000);
        savePIDConfig();
    }
    
    /* 5. 打印当前配置 */
    printPIDConfig();
    HAL_Delay(2000);
    
    /* 6. 演示场景1：参数调优后保存 */
    Debug_Printf("\r\n========== 场景1: 参数调优 ==========\r\n");
    Debug_Printf("[INFO] 模拟用户通过串口调整PID参数...\r\n");
    HAL_Delay(1000);
    
    tuneParameters();
    HAL_Delay(1000);
    
    Debug_Printf("[INFO] 用户输入保存命令...\r\n");
    HAL_Delay(1000);
    
    savePIDConfig();
    printPIDConfig();
    HAL_Delay(2000);
    
    /* 7. 演示场景2：运行时间累加 */
    Debug_Printf("\r\n========== 场景2: 运行时间统计 ==========\r\n");
    Debug_Printf("[INFO] 模拟系统运行60秒...\r\n");
    
    for (int i = 0; i < 6; i++) {
        HAL_Delay(1000);
        current_config.total_runtime_sec += 10;
        Debug_Printf("[RUNTIME] 运行时间: %lu 秒\r\n", current_config.total_runtime_sec);
    }
    
    Debug_Printf("[INFO] 系统关机前保存配置...\r\n");
    savePIDConfig();
    HAL_Delay(2000);
    
    /* 8. 演示场景3：恢复出厂设置 */
    Debug_Printf("\r\n========== 场景3: 恢复出厂设置 ==========\r\n");
    Debug_Printf("[INFO] 用户请求恢复出厂设置...\r\n");
    HAL_Delay(1000);
    
    factoryReset();
    savePIDConfig();
    printPIDConfig();
    HAL_Delay(2000);
    
    /* 9. 演示场景4：断电重启恢复 */
    Debug_Printf("\r\n========== 场景4: 模拟断电重启 ==========\r\n");
    Debug_Printf("[INFO] 重新加载配置（模拟重启）...\r\n");
    HAL_Delay(1000);
    
    loadPIDConfig();
    printPIDConfig();
    
    /* 10. 完成 */
    Debug_Printf("\r\n========================================\r\n");
    Debug_Printf("   示例运行完成\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("\r\n提示：\r\n");
    Debug_Printf("  1. 断电后重新上电，配置会自动加载\r\n");
    Debug_Printf("  2. 运行时间会持续累加\r\n");
    Debug_Printf("  3. CRC校验确保数据完整性\r\n");
    Debug_Printf("  4. 魔术数字判断是否首次使用\r\n");
    
    while (1) {
        HAL_Delay(1000);
    }
}
