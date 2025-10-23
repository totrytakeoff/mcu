/**
 * @file    line_follower_parabolic_test.cpp
 * @brief   抛物线拟合法巡线测试示例
 * @author  AI Assistant
 * @date    2024
 * 
 * @description
 * 测试抛物线拟合算法的精度和性能
 * 使用实际采集的传感器数据验证位置计算
 */

#include <cstdint>
#include <cmath>
#include "debug.hpp"
#include "stm32f1xx_hal.h"

/* ========== 抛物线拟合位置计算函数（测试用） ========== */

/**
 * @brief 抛物线拟合法计算线位置（独立测试函数）
 */
float calculateLinePositionParabolic(uint16_t sensor_data[8], bool white_line_mode = true) {
    // 传感器位置常量
    const float SENSOR_POSITIONS[8] = {
        -1000.0f, -714.0f, -428.0f, -142.0f,
         142.0f,   428.0f,  714.0f,  1000.0f
    };
    const float SENSOR_SPACING = 286.0f;
    const float ADC_MAX = 4095.0f;

    // 1. 数据预处理
    float values[8];
    if (white_line_mode) {
        // 黑底白线：反转数值
        for (int i = 0; i < 8; i++) {
            values[i] = ADC_MAX - sensor_data[i];
        }
    } else {
        for (int i = 0; i < 8; i++) {
            values[i] = sensor_data[i];
        }
    }

    // 2. 找到峰值
    int peak_idx = 0;
    float peak_value = values[0];
    for (int i = 1; i < 8; i++) {
        if (values[i] > peak_value) {
            peak_value = values[i];
            peak_idx = i;
        }
    }

    // 3. 边界检查
    if (peak_idx == 0 || peak_idx == 7) {
        return SENSOR_POSITIONS[peak_idx];
    }

    // 4. 三点抛物线拟合
    float y0 = values[peak_idx - 1];
    float y1 = values[peak_idx];
    float y2 = values[peak_idx + 1];

    float denominator = 2.0f * (y0 - 2.0f * y1 + y2);

    if (fabsf(denominator) < 0.001f) {
        // 退化为加权平均
        float weighted_sum = y0 * (-1.0f) + y1 * 0.0f + y2 * 1.0f;
        float total_weight = y0 + y1 + y2;
        if (total_weight < 0.001f) {
            return SENSOR_POSITIONS[peak_idx];
        }
        float offset = weighted_sum / total_weight;
        return SENSOR_POSITIONS[peak_idx] + offset * SENSOR_SPACING;
    }

    // 5. 计算顶点偏移
    float offset = (y0 - y2) / denominator;
    if (offset > 1.0f) offset = 1.0f;
    if (offset < -1.0f) offset = -1.0f;

    // 6. 计算最终位置
    float final_position = SENSOR_POSITIONS[peak_idx] + offset * SENSOR_SPACING;
    if (final_position > 1000.0f) final_position = 1000.0f;
    if (final_position < -1000.0f) final_position = -1000.0f;

    return final_position;
}

/* ========== 测试数据 ========== */

// 测试用例1：小车居中（来自实际数据）
uint16_t test_center_data[8] = {1469, 1064, 716, 332, 346, 604, 998, 1344};

// 测试用例2：线偏左
uint16_t test_left_data[8] = {1000, 500, 300, 280, 1200, 1400, 1500, 1600};

// 测试用例3：线偏右
uint16_t test_right_data[8] = {1600, 1500, 1400, 1200, 280, 300, 500, 1000};

// 测试用例4：纯黑底
uint16_t test_black_data[8] = {1597, 1541, 1547, 1497, 1510, 1525, 1550, 1584};

// 测试用例5：纯白底
uint16_t test_white_data[8] = {566, 402, 293, 263, 281, 355, 479, 717};

/* ========== 测试函数 ========== */

void testParabolicAlgorithm() {
    Debug_Printf("\r\n========== 抛物线拟合算法测试 ==========\r\n\r\n");

    // 测试1：居中位置
    Debug_Printf("【测试1】小车居中（实际数据）\r\n");
    Debug_Printf("传感器数据: %d %d %d %d %d %d %d %d\r\n",
                 test_center_data[0], test_center_data[1], test_center_data[2], test_center_data[3],
                 test_center_data[4], test_center_data[5], test_center_data[6], test_center_data[7]);
    float pos1 = calculateLinePositionParabolic(test_center_data, true);
    Debug_Printf("计算位置: %.2f (预期: 接近0)\r\n", pos1);
    Debug_Printf("结果: %s\r\n\r\n", (fabsf(pos1) < 100.0f) ? "✅ 通过" : "❌ 失败");

    // 测试2：线偏左
    Debug_Printf("【测试2】线偏左\r\n");
    Debug_Printf("传感器数据: %d %d %d %d %d %d %d %d\r\n",
                 test_left_data[0], test_left_data[1], test_left_data[2], test_left_data[3],
                 test_left_data[4], test_left_data[5], test_left_data[6], test_left_data[7]);
    float pos2 = calculateLinePositionParabolic(test_left_data, true);
    Debug_Printf("计算位置: %.2f (预期: 负值)\r\n", pos2);
    Debug_Printf("结果: %s\r\n\r\n", (pos2 < -100.0f) ? "✅ 通过" : "❌ 失败");

    // 测试3：线偏右
    Debug_Printf("【测试3】线偏右\r\n");
    Debug_Printf("传感器数据: %d %d %d %d %d %d %d %d\r\n",
                 test_right_data[0], test_right_data[1], test_right_data[2], test_right_data[3],
                 test_right_data[4], test_right_data[5], test_right_data[6], test_right_data[7]);
    float pos3 = calculateLinePositionParabolic(test_right_data, true);
    Debug_Printf("计算位置: %.2f (预期: 正值)\r\n", pos3);
    Debug_Printf("结果: %s\r\n\r\n", (pos3 > 100.0f) ? "✅ 通过" : "❌ 失败");

    // 测试4：纯黑底（无线）
    Debug_Printf("【测试4】纯黑底（无白线）\r\n");
    Debug_Printf("传感器数据: %d %d %d %d %d %d %d %d\r\n",
                 test_black_data[0], test_black_data[1], test_black_data[2], test_black_data[3],
                 test_black_data[4], test_black_data[5], test_black_data[6], test_black_data[7]);
    float pos4 = calculateLinePositionParabolic(test_black_data, true);
    Debug_Printf("计算位置: %.2f (方差应该很小)\r\n", pos4);
    Debug_Printf("说明: 应该触发丢线检测\r\n\r\n");

    // 测试5：纯白底（无线）
    Debug_Printf("【测试5】纯白底（无黑线）\r\n");
    Debug_Printf("传感器数据: %d %d %d %d %d %d %d %d\r\n",
                 test_white_data[0], test_white_data[1], test_white_data[2], test_white_data[3],
                 test_white_data[4], test_white_data[5], test_white_data[6], test_white_data[7]);
    float pos5 = calculateLinePositionParabolic(test_white_data, true);
    Debug_Printf("计算位置: %.2f (方差应该很小)\r\n", pos5);
    Debug_Printf("说明: 应该触发丢线检测\r\n\r\n");

    Debug_Printf("========== 测试完成 ==========\r\n\r\n");
}

/* ========== 性能测试 ========== */

void performanceTest() {
    Debug_Printf("\r\n========== 性能测试 ==========\r\n\r\n");

    const int TEST_ITERATIONS = 1000;
    uint32_t start_time = HAL_GetTick();

    for (int i = 0; i < TEST_ITERATIONS; i++) {
        calculateLinePositionParabolic(test_center_data, true);
    }

    uint32_t end_time = HAL_GetTick();
    uint32_t elapsed = end_time - start_time;

    Debug_Printf("执行次数: %d\r\n", TEST_ITERATIONS);
    Debug_Printf("总耗时: %lu ms\r\n", elapsed);
    Debug_Printf("平均耗时: %.3f ms\r\n", (float)elapsed / TEST_ITERATIONS);
    Debug_Printf("理论更新频率: %.1f Hz\r\n", 1000.0f / ((float)elapsed / TEST_ITERATIONS));
    Debug_Printf("\r\n========== 性能测试完成 ==========\r\n\r\n");
}

/* ========== 主测试函数 ========== */

extern "C" int main(void) {
    // HAL初始化
    HAL_Init();
    
    // 初始化调试串口
    Debug_Enable();
    
    Debug_Printf("\r\n\r\n");
    Debug_Printf("╔════════════════════════════════════════╗\r\n");
    Debug_Printf("║   抛物线拟合巡线算法测试程序           ║\r\n");
    Debug_Printf("╚════════════════════════════════════════╝\r\n");
    Debug_Printf("\r\n");

    // 延迟一下确保串口稳定
    HAL_Delay(1000);

    // 运行算法测试
    testParabolicAlgorithm();

    // 运行性能测试
    performanceTest();

    Debug_Printf("【提示】测试完成，你可以：\r\n");
    Debug_Printf("  1. 查看位置计算是否准确\r\n");
    Debug_Printf("  2. 确认算法性能满足实时要求（建议>50Hz）\r\n");
    Debug_Printf("  3. 将此算法集成到实际巡线系统\r\n\r\n");

    // 循环等待
    while (1) {
        HAL_Delay(1000);
    }
}
