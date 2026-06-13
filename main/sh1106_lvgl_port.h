#ifndef SH1106_LVGL_PORT_H
#define SH1106_LVGL_PORT_H
#include"freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "misc/lv_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化 SH1106 LVGL 显示端口
 * @param mosi_gpio SPI MOSI 引脚
 * @param sclk_gpio SPI SCLK 引脚
 * @param cs_gpio   SPI CS 引脚
 * @param dc_gpio   DC 引脚
 * @param rst_gpio  Reset 引脚
 * @return lv_display_t* LVGL 显示句柄，失败返回 NULL
 */
lv_display_t* sh1106_lvgl_port_init(int mosi_gpio, int sclk_gpio, 
                                     int cs_gpio, int dc_gpio, int rst_gpio);

/**
 * @brief 获取 LVGL 全局互斥锁句柄
 * @note  所有 LVGL API 调用前必须获取此锁
 */
SemaphoreHandle_t sh1106_lvgl_get_mutex(void);

#ifdef __cplusplus
}
#endif

#endif // SH1106_LVGL_PORT_H