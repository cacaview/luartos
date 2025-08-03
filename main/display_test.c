#include "display_test.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "lvgl.h"

static const char *TAG = "DISPLAY_TEST";

void display_fill_test(void) {
    ESP_LOGI(TAG, "开始显示填充测试...");
    
    // 获取当前屏幕
    lv_obj_t * scr = lv_scr_act();
    
    // 清空屏幕
    lv_obj_clean(scr);
    
    // 设置为红色背景
    ESP_LOGI(TAG, "设置红色背景...");
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xFF0000), LV_PART_MAIN);
    lv_obj_invalidate(scr);
    lv_refr_now(NULL);
    
    // 等待2秒
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 设置为绿色背景
    ESP_LOGI(TAG, "设置绿色背景...");
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x00FF00), LV_PART_MAIN);
    lv_obj_invalidate(scr);
    lv_refr_now(NULL);
    
    // 等待2秒
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 设置为蓝色背景
    ESP_LOGI(TAG, "设置蓝色背景...");
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x0000FF), LV_PART_MAIN);
    lv_obj_invalidate(scr);
    lv_refr_now(NULL);
    
    // 等待2秒
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 设置为白色背景
    ESP_LOGI(TAG, "设置白色背景...");
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_invalidate(scr);
    lv_refr_now(NULL);
    
    // 等待2秒
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 设置为黑色背景
    ESP_LOGI(TAG, "设置黑色背景...");
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_invalidate(scr);
    lv_refr_now(NULL);
    
    ESP_LOGI(TAG, "显示填充测试完成");
}

void display_simple_test(void) {
    ESP_LOGI(TAG, "开始简单显示测试...");
    
    // 获取当前屏幕
    lv_obj_t * scr = lv_scr_act();
    
    // 清空屏幕
    lv_obj_clean(scr);
    
    // 设置背景颜色
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000080), LV_PART_MAIN);
    
    // 创建一个标签
    lv_obj_t * label = lv_label_create(scr);
    if (label != NULL) {
        lv_label_set_text(label, "显示测试\nDisplay Test\n123456789");
        lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        ESP_LOGI(TAG, "测试标签创建成功");
    } else {
        ESP_LOGE(TAG, "测试标签创建失败");
    }
    
    // 强制刷新
    lv_obj_invalidate(scr);
    lv_refr_now(NULL);
    
    ESP_LOGI(TAG, "简单显示测试完成");
}

void display_pattern_test(void) {
    ESP_LOGI(TAG, "开始显示图案测试...");
    
    // 获取当前屏幕
    lv_obj_t * scr = lv_scr_act();
    
    // 清空屏幕
    lv_obj_clean(scr);
    
    // 设置背景颜色
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x202020), LV_PART_MAIN);
    
    // 创建多个矩形作为测试图案
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            lv_obj_t * rect = lv_obj_create(scr);
            if (rect != NULL) {
                lv_obj_set_size(rect, 60, 60);
                lv_obj_set_pos(rect, 10 + i * 70, 10 + j * 70);
                
                // 设置不同的颜色
                uint32_t color = (i * 3 + j + 1) * 0x202020;
                lv_obj_set_style_bg_color(rect, lv_color_hex(color), LV_PART_MAIN);
                lv_obj_set_style_border_width(rect, 2, LV_PART_MAIN);
                lv_obj_set_style_border_color(rect, lv_color_white(), LV_PART_MAIN);
            }
        }
    }
    
    // 强制刷新
    lv_obj_invalidate(scr);
    lv_refr_now(NULL);
    
    ESP_LOGI(TAG, "显示图案测试完成");
}
