# ESP32-S3 ILI9488 TFT显示屏与LVGL演示

本项目演示了如何在ESP32-S3上使用ILI9488 TFT显示屏和触摸功能，结合LVGL图形库创建用户界面。

## 硬件要求

- ESP32-S3 开发板
- ILI9488 3.5寸 TFT 显示屏 (320x480, 带触摸)
- XPT2046 触摸控制器

## 引脚连接

### ILI9488 显示屏连接

```
ESP32-S3    ILI9488
----------- --------
GPIO11  --> MOSI (SDA)
GPIO12  --> SCLK (SCL)
GPIO10  --> CS
GPIO9   --> DC (A0)
GPIO8   --> RST
GPIO7   --> BL (背光)
3V3     --> VCC
GND     --> GND
```

### XPT2046 触摸屏连接

```
ESP32-S3    XPT2046
----------- --------
GPIO13  --> T_DIN (MOSI)
GPIO14  --> T_DO (MISO)
GPIO15  --> T_CLK (SCLK)
GPIO16  --> T_CS
GPIO17  --> T_IRQ
3V3     --> VCC
GND     --> GND
```

## 软件环境

- ESP-IDF v4.4 或更新版本
- LVGL v8.3

## 安装和编译

1. 克隆LVGL库到组件目录：

```bash
cd components/lvgl
git clone --branch release/v8.3 --depth 1 https://github.com/lvgl/lvgl.git
```

2. 配置项目：

```bash
idf.py menuconfig
```

在菜单中配置：

- Component config -> ESP32S3-Specific -> CPU frequency (建议240MHz)
- Component config -> FreeRTOS -> Tick rate (建议1000Hz)

3. 编译项目：

```bash
idf.py build
```

4. 烧录到设备：

```bash
idf.py flash monitor
```

## 功能特性

- **显示驱动**: 支持ILI9488 320x480分辨率，RGB565色彩格式
- **触摸输入**: XPT2046电阻式触摸屏支持
- **LVGL界面**: 包含多种UI元素的演示
  - 按钮 (可点击反馈)
  - 滑动条 (实时值显示)
  - 开关 (状态切换)
  - 进度条
  - 信息面板

## 代码结构

```
main/
├── main.c              # 主程序入口
├── ili9488_driver.c/h  # ILI9488显示屏驱动
├── touch_driver.c/h    # 触摸屏驱动
└── lvgl_demo.c/h       # LVGL演示界面

components/
└── lvgl/
    ├── CMakeLists.txt  # LVGL组件配置
    ├── lv_conf.h       # LVGL配置文件
    └── lvgl/           # LVGL源码目录
```

## 自定义配置

## 性能优化

1. **SPI频率**: 显示屏SPI设置为40MHz，触摸屏为2MHz
2. **缓冲区**: 使用双缓冲提高刷新性能
3. **任务优先级**: LVGL任务设置为高优先级保证流畅性

## 故障排除

1. **显示异常**: 检查SPI连接和引脚配置
2. **触摸不响应**: 确认触摸中断引脚连接和上拉电阻
3. **编译错误**: 确保已正确下载LVGL源码

## 扩展功能

- 添加更多LVGL控件
- 实现图片显示功能
- 添加WiFi配置界面
- 集成传感器数据显示

## 参考资料

- [ESP-IDF编程指南](https://docs.espressif.com/projects/esp-idf/)
- [LVGL文档](https://docs.lvgl.io/)
- [ILI9488数据手册](https://www.displayfuture.com/Display/datasheet/controller/ILI9488.pdf)
- [XPT2046数据手册](https://www.ti.com/lit/ds/symlink/xpt2046.pdf)
