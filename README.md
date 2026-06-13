# About the repo
"LVGL v9 portfor ESP32-S3 with SPI SH1106 using nopnop2002/esp-idf-ssd1306 driver, achieving 100fps rendering on a 128x64 monochrome OLED."

Or if you prefer a slightly shorter version:

"ESP32-S3 + LVGL v9 + SPI SH1106 (nopnop2002 driver) base project hitting 100fps on 128x64 OLED."

The library citation format: <a href=https://github.com/nopnop2002/esp-idf-ssd1306> nopnop2002/esp-idf-ssd1306 </a>

## Build
```shell
git clone https://github.com/TNT569/esp-idf-sh1106-spi-demo.git
cd esp-idf-sh1106-spi-demo
idf.py flash 
