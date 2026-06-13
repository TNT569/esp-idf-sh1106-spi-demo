#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lvgl.h"
#include "sh1106_lvgl_port.h"
#include "lvgl_ui.h"

static const char *TAG = "APP_MAIN";


void app_main(void)
{
    // 一行代码完成显示端口初始化
    lv_display_t *disp = sh1106_lvgl_port_init(
        CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO,
        CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO
    );
    
    if (!disp) {
        ESP_LOGE(TAG, "Display port init failed! Halting.");
        return;
    }

     // 线程安全地创建 UI
    SemaphoreHandle_t mutex = sh1106_lvgl_get_mutex();
    xSemaphoreTake(mutex, portMAX_DELAY);
    load_lvgl_ui();
    xSemaphoreGive(mutex);

    ESP_LOGI(TAG, "Entering LVGL main loop...");
    while (1) {
        xSemaphoreTake(mutex, portMAX_DELAY);
        uint32_t time_till_next = lv_timer_handler();
        xSemaphoreGive(mutex);

        // 钳制延迟范围，防止 WDT 触发或 CPU 空转
        if (time_till_next < 10)   time_till_next = 10;
        if (time_till_next > 500) time_till_next = 500;

        vTaskDelay(pdMS_TO_TICKS(time_till_next));
    }
}