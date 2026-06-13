#include "sh1106_lvgl_port.h"
#include"lvgl.h"
#include "ssd1306.h"
#include "esp_log.h"
#include "freertos/task.h"
#include"esp_timer.h"
#include <string.h>
#include "esp_timer.h"

// // 1. 定义全局/静态计数变量
// static volatile uint32_t s_frame_cnt = 0;
// static int64_t s_last_fps_time_us = 0;

// /**
//  * @brief 在每次 LVGL flush 完成后调用此函数
//  * 如果你用的是 LVGL v8，请在你的 disp_flush 回调函数的最后调用它；
//  * 如果你用的是 LVGL v9，请在 LV_EVENT_RENDER_READY 事件中调用它。
//  */
// void fps_counter_tick(void)
// {
//     s_frame_cnt++;
//     int64_t now_us = esp_timer_get_time();
    
//     // 每 1000ms (1秒) 打印并重置一次
//     if ((now_us - s_last_fps_time_us) >= 1000000) {
//         printf("[PERF] Real FPS: %lu\n", (unsigned long)s_frame_cnt);
        
//         // TODO: 也可以在这里用你现有的 ssd1306 驱动把 FPS 画到屏幕上
//         // char buf[16]; snprintf(buf, sizeof(buf), "FPS:%lu", s_frame_cnt);
//         // ssd1306_draw_string(...); 
        
//         s_frame_cnt = 0;
//         s_last_fps_time_us = now_us;
//     }
// } 

static const char *TAG = "SH1106_PORT";
static SSD1306_t dev;
static SemaphoreHandle_t s_lvgl_mutex = NULL;

// L8 全屏渲染缓冲 (8KB) + SH1106 页式显存缓冲 (1KB)
static uint8_t s_lvgl_full_buf[128 * 64];
// void *s_lvgl_full_buf = NULL;
static uint8_t s_oled_page_buf[128 * 8];

static void IRAM_ATTR lvgl_tick_timer_cb(void *arg)
{
    // lv_tick_inc 是线程安全的原子操作，可在中断中直接调用
    lv_tick_inc(2); 
}

// 在 sh1106_lvgl_port_init() 中初始化定时器
static void init_lvgl_hw_tick(void)
{
    const esp_timer_create_args_t timer_args = {
        .callback = &lvgl_tick_timer_cb,
        .name = "lvgl_hw_tick"
    };
    
    esp_timer_handle_t lvgl_timer;
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &lvgl_timer));
    // 2000us = 2ms，恢复 LVGL 最佳时间基准
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_timer, 2000)); 
    
    ESP_LOGI(TAG, "LVGL HW Tick started (2ms interval, ISR based)");
}


// ================= Flush 回调 =================
static void sh1106_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    memset(s_oled_page_buf, 0, sizeof(s_oled_page_buf));

    for (int y = area->y1; y <= area->y2; y++) {
        int page = y / 8;
        int bit_pos = y % 8;
        int32_t w = area->x2 - area->x1 + 1;

        for (int x = area->x1; x <= area->x2; x++) {
            int32_t local_x = x - area->x1;
            int32_t local_y = y - area->y1;
            
            // L8 格式无 Palette 头，直接索引像素值
            if (px_map[local_y * w + local_x] > 127) {
                s_oled_page_buf[page * 128 + x] |= (1 << bit_pos);
            }
        }
    }

    // 全量发送 8 页数据，规避 SH1106 局部刷新缺陷
    for (int p = 0; p < 8; p++) {
        ssd1306_display_image(&dev, p, 0, &s_oled_page_buf[p * 128], 128);
    }

    lv_display_flush_ready(disp);
    // fps_counter_tick();  
}


// ================= 公开接口 =================
lv_display_t* sh1106_lvgl_port_init(int mosi_gpio, int sclk_gpio, 
                                     int cs_gpio, int dc_gpio, int rst_gpio)
{
    //分配堆内存
    // 1. 硬件初始化与自检
    ESP_LOGI(TAG, "Init SH1106 Hardware...");
    spi_master_init(&dev, mosi_gpio, sclk_gpio, cs_gpio, dc_gpio, rst_gpio);
    ssd1306_init(&dev, 128, 64);
    ssd1306_clear_screen(&dev, false);
    
    ssd1306_display_text(&dev, 0, "HW OK", 5, false);
    vTaskDelay(pdMS_TO_TICKS(800));
    ssd1306_clear_screen(&dev, false);

    // 2. LVGL 核心初始化
    ESP_LOGI(TAG, "Init LVGL v9 Display Port...");
    lv_init();
    
    s_lvgl_mutex = xSemaphoreCreateMutex();
    if (!s_lvgl_mutex) {
        ESP_LOGE(TAG, "Failed to create LVGL mutex!");
        return NULL;
    }

    lv_display_t *disp = lv_display_create(128, 64);
    if (!disp) {
        ESP_LOGE(TAG, "Failed to create LVGL display!");
        return NULL;
    }

    lv_display_set_color_format(disp, LV_COLOR_FORMAT_L8);
    lv_display_set_buffers(disp, s_lvgl_full_buf, NULL, 
                           sizeof(s_lvgl_full_buf), LV_DISPLAY_RENDER_MODE_FULL);
    lv_display_set_flush_cb(disp, sh1106_flush_cb);

    // 3. 启动 Tick 任务 (绑定 CPU1)
    init_lvgl_hw_tick();

    ESP_LOGI(TAG, "SH1106 LVGL Port initialized successfully.");
    return disp;
}

SemaphoreHandle_t sh1106_lvgl_get_mutex(void)
{
    return s_lvgl_mutex;
}