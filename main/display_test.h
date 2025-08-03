#ifndef DISPLAY_TEST_H
#define DISPLAY_TEST_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 显示填充测试 - 循环显示不同颜色
 */
void display_fill_test(void);

/**
 * @brief 简单显示测试 - 显示文本
 */
void display_simple_test(void);

/**
 * @brief 显示图案测试 - 显示多个矩形
 */
void display_pattern_test(void);

#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_TEST_H */
