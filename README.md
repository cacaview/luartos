# LuaRTOS - ESP32-S3 嵌入式Lua运行时系统

一个基于ESP32-S3的嵌入式Lua运行时系统，集成了SD卡存储、LVGL图形界面和WiFi功能，提供完整的IoT开发解决方案。

## 🌟 项目特色

- **🚀 Lua 5.4 脚本引擎** - 支持动态脚本执行和交互式编程
- **📱 LVGL图形界面** - 现代化的嵌入式GUI框架，支持触摸操作
- **💾 SD卡文件系统** - 通过SPI接口实现完整的文件读写操作
- **📡 WiFi网络支持** - 内置WiFi连接和网络功能
- **⚡ 内存优化** - 智能双分配器架构，最大化利用PSRAM
- **🔧 模块化设计** - 清晰的组件分离和API设计

## 🛠️ 硬件配置

### 主控制器
- **芯片**: ESP32-S3
- **开发框架**: ESP-IDF 5.x
- **编程语言**: C + Lua
- **内存**: 支持PSRAM扩展

### 显示模块
- **显示屏**: ILI9488 3.5寸 TFT (480x320)
- **触摸**: XPT2046电阻式触摸控制器
- **接口**: SPI2_HOST

### SD卡模块
- **接口**: SPI3_HOST
- **支持格式**: FAT32, ≤32GB
- **引脚配置**:
  ```
  CS   → GPIO 9    SCK  → GPIO 11
  MOSI → GPIO 42   MISO → GPIO 41
  ```

## 🏗️ 软件架构

### 内存优化架构
```
┌─────────────────────────────────────────────────┐
│                ESP32-S3 内存架构                 │
├─────────────────────┬───────────────────────────┤
│   内置RAM (512KB)   │      PSRAM (8MB)         │
├─────────────────────┼───────────────────────────┤
│ • LVGL图形渲染      │ • Lua解释器和对象        │
│ • 系统关键任务      │ • 大型数据缓冲区         │
│ • 中断处理程序      │ • 用户脚本数据           │
└─────────────────────┴───────────────────────────┘
```

### 核心组件
```
luartos/
├── main/                    # 主程序入口
│   ├── main.c              # 系统启动和任务管理
│   ├── oobe_lua.lua        # 开箱即用体验脚本
│   └── memory_demo.lua     # 内存优化演示
├── components/              # 核心组件库
│   ├── lua/                # Lua引擎 + LVGL绑定
│   ├── sdcard/             # SD卡驱动组件
│   ├── lvgl/               # LVGL图形库
│   └── lvgl_esp32_drivers/ # ESP32显示驱动
└── GUI-Guider-Projects/     # 图形界面设计项目
```

## 🚀 快速开始

### 环境准备
```bash
# 1. 安装ESP-IDF 5.x
git clone -b v5.1 https://github.com/espressif/esp-idf.git
cd esp-idf && ./install.sh && source export.sh

# 2. 克隆项目
git clone <your-repo-url> luartos
cd luartos

# 3. 配置项目
idf.py menuconfig
```

### 编译和烧录
```bash
# 编译项目
idf.py build

# 烧录到设备
idf.py flash monitor
```

## 💡 功能演示

### OOBE（开箱即用体验）
系统启动后自动运行OOBE流程，包含：
- **系统初始化界面** - 显示启动状态
- **WiFi配置界面** - 扫描和连接网络
- **SD卡管理界面** - 文件系统操作
- **安装完成界面** - 系统就绪提示

### Lua脚本示例
```lua
-- 创建简单界面
local screen = lvgl.scr_act()
lvgl.obj_set_style_bg_color(screen, 0x003a57, lvgl.PART_MAIN())

-- 添加标题
local title = lvgl.label_create(screen)
lvgl.label_set_text(title, "Hello LuaRTOS!")
lvgl.obj_align(title, lvgl.ALIGN_TOP_MID(), 0, 20)

-- 添加交互按钮
local btn = lvgl.btn_create(screen)
lvgl.obj_set_size(btn, 120, 50)
local btn_label = lvgl.label_create(btn)
lvgl.label_set_text(btn_label, "点击我")
```

## 📋 系统特性

### 🔧 技术特性
- **SPI分块传输** - 解决硬件传输限制，支持大数据显示
- **智能内存分配** - LVGL使用内置RAM，Lua使用PSRAM
- **异步任务管理** - 多任务并发处理，响应流畅
- **错误恢复机制** - 自动检测和处理硬件异常

### 🎨 界面特性
- **触摸交互** - 支持按钮、滑块、开关等交互控件
- **动画效果** - 流畅的屏幕切换和界面动画
- **主题支持** - 可自定义颜色、字体和样式
- **响应式布局** - 适配不同分辨率显示屏

### 💾 存储特性
- **FAT32文件系统** - 兼容PC和其他设备
- **热插拔支持** - 运行时插拔SD卡
- **文件管理** - 完整的文件读写、删除、列表操作
- **容量监控** - 实时显示存储空间使用情况

## 🔍 性能指标

| 性能指标 | 数值 | 说明 |
|----------|------|------|
| 启动时间 | <3秒 | 从上电到界面显示 |
| 内存使用 | ~200KB | 系统核心占用（不含缓冲区） |
| 显示刷新 | 60fps | LVGL界面刷新率 |
| SD卡速度 | 1MB/s | SPI模式读写速度 |
| WiFi连接 | <10秒 | 网络扫描和连接时间 |

## 🛡️ 稳定性保障

### 硬件兼容性
- ✅ 支持多种ESP32-S3开发板
- ✅ 兼容标准ILI9488显示屏
- ✅ 支持Class 10高速SD卡
- ✅ 电源波动自动适应

### 软件可靠性
- 🔒 看门狗保护机制
- 🔄 自动错误恢复
- 📊 实时系统监控
- 🧪 全面测试覆盖

## 📚 文档和支持

- **📖 技术文档**: 查看 [WIKI.md](WIKI.md) 获取详细技术文档和API参考
- **🔧 硬件连接**: 详细的引脚配置和硬件设置指南
- **💻 API参考**: 完整的Lua API和系统函数说明
- **🚀 开发教程**: 从入门到高级的开发指导

## 🤝 贡献

我们欢迎社区贡献！请通过以下方式参与：

1. 🍴 Fork本项目
2. 🌿 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 💾 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 📤 推送到分支 (`git push origin feature/AmazingFeature`)
5. 🔀 打开Pull Request

## 📄 许可证

本项目采用MIT许可证 - 详见 [LICENSE](LICENSE) 文件

## 🙏 致谢

- [ESP-IDF](https://github.com/espressif/esp-idf) - 强大的ESP32开发框架
- [LVGL](https://github.com/lvgl/lvgl) - 优秀的嵌入式图形库
- [Lua](https://www.lua.org/) - 轻量级脚本语言
- [颜along](https://www.bilibili.com/opus/1022316329467641890) - SD卡SPI实现参考

---

**项目状态**: ✅ 稳定版本 | **最后更新**: 2025年8月6日 | **版本**: v1.0.0

🔗 **快速链接**: [技术文档](WIKI.md) | [API参考](WIKI.md#lua-api) | [硬件指南](WIKI.md#硬件连接指南)
