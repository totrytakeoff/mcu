/**
 * @file    debug_config.h
 * @brief   调试系统配置文件
 * @author  AI Assistant
 * @date    2024
 * 
 * 在此文件中配置调试系统的各种选项
 */

#ifndef DEBUG_CONFIG_H
#define DEBUG_CONFIG_H

/* ========== 全局调试开关 ========== */

/**
 * @brief 全局调试开关（编译时控制）
 * 1 = 启用调试功能
 * 0 = 完全禁用调试功能（节省代码空间）
 */
#define DEBUG_GLOBAL_ENABLE         1

/**
 * @brief 默认调试状态（运行时初始状态）
 * 1 = 启动时自动启用调试输出
 * 0 = 启动时禁用调试输出（需手动启用）
 */
#define DEBUG_DEFAULT_ENABLED       1


/* ========== 模块调试开关 ========== */

/**
 * @brief 各模块独立调试开关
 * 1 = 启用该模块的调试输出
 * 0 = 禁用该模块的调试输出
 */
#define DEBUG_MOTOR_ENABLE          1       // 电机模块调试
#define DEBUG_SENSOR_ENABLE         1       // 传感器模块调试
#define DEBUG_BLUETOOTH_ENABLE      0       // 蓝牙模块调试
#define DEBUG_WIRELESS_ENABLE       0       // 无线模块调试
#define DEBUG_LINE_FOLLOW_ENABLE    1       // 巡线模块调试
#define DEBUG_SYSTEM_ENABLE         1       // 系统信息调试


/* ========== 调试输出选项 ========== */

/**
 * @brief 启动信息输出
 * 1 = 启动时输出系统信息横幅
 * 0 = 不输出启动信息
 */
#define DEBUG_SHOW_STARTUP_BANNER   1

/**
 * @brief 系统状态信息
 * 1 = 输出详细系统状态（时钟、芯片ID等）
 * 0 = 不输出系统状态
 */
#define DEBUG_SHOW_SYSTEM_STATUS    1

/**
 * @brief 主循环调试信息
 * 1 = 在主循环中输出调试信息
 * 0 = 主循环中不输出调试信息（推荐）
 */
#define DEBUG_SHOW_LOOP_INFO        0

/**
 * @brief 时间戳功能
 * 1 = 调试信息前添加时间戳 [1234ms]
 * 0 = 不添加时间戳
 */
#define DEBUG_SHOW_TIMESTAMP        1


/* ========== 性能配置 ========== */

/**
 * @brief 调试缓冲区大小（字节）
 * 较大的缓冲区可以处理更长的字符串，但占用更多RAM
 * 推荐值: 128-512
 */
#define DEBUG_BUFFER_SIZE           256

/**
 * @brief UART发送超时时间（毫秒）
 * 发送调试数据的最大等待时间
 */
#define DEBUG_UART_TIMEOUT          1000

/**
 * @brief 调试输出限流（毫秒）
 * 两次调试输出之间的最小间隔，用于防止刷屏
 * 0 = 不限流
 */
#define DEBUG_MIN_INTERVAL          0


/* ========== 高级选项 ========== */

/**
 * @brief 使用颜色输出（ANSI转义码）
 * 1 = 使用颜色标记不同级别的信息（需要终端支持）
 * 0 = 纯文本输出
 */
#define DEBUG_USE_COLOR             0

/**
 * @brief 调试级别过滤
 * 1 = 启用调试级别过滤功能
 * 0 = 禁用级别过滤（所有调试信息都输出）
 */
#define DEBUG_USE_LEVEL_FILTER      0

/**
 * @brief 默认调试级别
 * 0 = 无输出
 * 1 = 仅错误 (ERROR)
 * 2 = 错误+警告 (WARN)
 * 3 = 错误+警告+信息 (INFO)
 * 4 = 全部 (DEBUG)
 */
#define DEBUG_DEFAULT_LEVEL         3

/**
 * @brief 包含文件名和行号
 * 1 = 调试信息包含源文件名和行号
 * 0 = 不包含文件名和行号
 */
#define DEBUG_SHOW_FILE_LINE        0


/* ========== 调试宏定义 ========== */

#if DEBUG_GLOBAL_ENABLE

    /* 模块调试宏 */
    #if DEBUG_MOTOR_ENABLE
        #define DEBUG_MOTOR(...)        Debug_Printf("[MOTOR] " __VA_ARGS__)
    #else
        #define DEBUG_MOTOR(...)        ((void)0)
    #endif

    #if DEBUG_SENSOR_ENABLE
        #define DEBUG_SENSOR(...)       Debug_Printf("[SENSOR] " __VA_ARGS__)
    #else
        #define DEBUG_SENSOR(...)       ((void)0)
    #endif

    #if DEBUG_BLUETOOTH_ENABLE
        #define DEBUG_BT(...)           Debug_Printf("[BT] " __VA_ARGS__)
    #else
        #define DEBUG_BT(...)           ((void)0)
    #endif

    #if DEBUG_WIRELESS_ENABLE
        #define DEBUG_WIRELESS(...)     Debug_Printf("[WIRELESS] " __VA_ARGS__)
    #else
        #define DEBUG_WIRELESS(...)     ((void)0)
    #endif

    #if DEBUG_LINE_FOLLOW_ENABLE
        #define DEBUG_LINE(...)         Debug_Printf("[LINE] " __VA_ARGS__)
    #else
        #define DEBUG_LINE(...)         ((void)0)
    #endif

    #if DEBUG_SYSTEM_ENABLE
        #define DEBUG_SYSTEM(...)       Debug_Printf("[SYSTEM] " __VA_ARGS__)
    #else
        #define DEBUG_SYSTEM(...)       ((void)0)
    #endif

    /* 错误和警告总是输出 */
    #define DEBUG_ERROR(...)            Debug_Print_Always("[ERROR] " __VA_ARGS__)
    #define DEBUG_WARN(...)             Debug_Print_Always("[WARN] " __VA_ARGS__)
    #define DEBUG_INFO(...)             Debug_Printf("[INFO] " __VA_ARGS__)

#else
    /* 全局调试禁用时，所有宏都变成空操作 */
    #define DEBUG_MOTOR(...)            ((void)0)
    #define DEBUG_SENSOR(...)           ((void)0)
    #define DEBUG_BT(...)               ((void)0)
    #define DEBUG_WIRELESS(...)         ((void)0)
    #define DEBUG_LINE(...)             ((void)0)
    #define DEBUG_SYSTEM(...)           ((void)0)
    #define DEBUG_ERROR(...)            ((void)0)
    #define DEBUG_WARN(...)             ((void)0)
    #define DEBUG_INFO(...)             ((void)0)
#endif


/* ========== 时间戳宏 ========== */

#if DEBUG_SHOW_TIMESTAMP
    #define DEBUG_TIMESTAMP()           Debug_Printf("[%lu ms] ", HAL_GetTick())
#else
    #define DEBUG_TIMESTAMP()           ((void)0)
#endif


/* ========== 文件行号宏 ========== */

#if DEBUG_SHOW_FILE_LINE
    #define DEBUG_FILE_LINE()           Debug_Printf("<%s:%d> ", __FILE__, __LINE__)
#else
    #define DEBUG_FILE_LINE()           ((void)0)
#endif


/* ========== 调试断言宏 ========== */

#if DEBUG_GLOBAL_ENABLE
    #define DEBUG_ASSERT(expr) \
        do { \
            if (!(expr)) { \
                Debug_Print_Always("[ASSERT] %s:%d - %s\r\n", __FILE__, __LINE__, #expr); \
                while(1); \
            } \
        } while(0)
#else
    #define DEBUG_ASSERT(expr)          ((void)0)
#endif


/* ========== 使用示例 ========== */

/*
使用方法：

1. 基本使用：
   DEBUG_MOTOR("速度设置为: %d\r\n", speed);
   DEBUG_SENSOR("传感器值: %d\r\n", value);
   
2. 错误和警告：
   DEBUG_ERROR("初始化失败\r\n");
   DEBUG_WARN("电压过低: %.2fV\r\n", voltage);
   
3. 断言：
   DEBUG_ASSERT(speed >= 0 && speed <= 100);
   
4. 带时间戳：
   DEBUG_TIMESTAMP();
   DEBUG_MOTOR("电机启动\r\n");
   // 输出: [1234 ms] [MOTOR] 电机启动
   
5. 条件调试：
   #if DEBUG_MOTOR_ENABLE
       // 只在电机调试启用时编译此代码
   #endif
*/

#endif /* DEBUG_CONFIG_H */
