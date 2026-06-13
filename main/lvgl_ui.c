#include"lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// 动画执行回调适配器（签名必须严格匹配 lv_anim_exec_xcb_t）
static void anim_bar_set_value(void *var, int32_t v)
{
    lv_bar_set_value((lv_obj_t *)var, v, LV_ANIM_OFF);
}

void load_lvgl_ui(void){

    // 线程安全地创建 UI
    
    // 1. 创建标签显示状态
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_source_han_sans_sc_14_cjk);
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "动画中");
    lv_obj_add_style(label, &style, 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 5);

    // 2. 创建进度条并设置样式
    lv_obj_t *bar = lv_bar_create(lv_screen_active());
    lv_obj_set_size(bar, 100, 10);               // 适配 128x64 的尺寸
    lv_obj_align(bar, LV_ALIGN_CENTER, 0, 0);
    lv_bar_set_range(bar, 0, 100);
    lv_bar_set_value(bar, 0, LV_ANIM_OFF);       // 初始值

    // 3. 创建弹性动画 (核心测试点)
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, bar);                    // 绑定到进度条
    lv_anim_set_values(&a, 0, 100);              // 从 0 到 100
    lv_anim_set_duration(&a, 1500);              // 单次周期 1.5s
    lv_anim_set_playback_duration(&a, 1500);     // 回放周期 1.5s
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE); // 无限循环
    lv_anim_set_path_cb(&a, lv_anim_path_overshoot); // ⭐ 弹性过冲曲线
    lv_anim_set_exec_cb(&a, anim_bar_set_value); // 执行回调
    lv_anim_start(&a);
}