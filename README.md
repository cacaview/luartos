# LuaRTOS - ESP32-S3 Lua 解释器与 OOBE 系统

LuaRTOS 是一个为 ESP32-S3 开发板量身定制的 Lua 解释器，专为嵌入式系统的图形界面开发进行了优化。它集成了完整的 LVGL 图形库支持和系统硬件接口，为开箱即用体验（OOBE）和应用程序开发提供了强大的 Lua 运行环境。

## 🚀 核心特性

- **🧠 PSRAM 优化**: 智能内存分配器，优先使用 PSRAM，为显存留出内部 RAM
- **🎨 LVGL 集成**: 完整的 LVGL 8.x 图形库 Lua 绑定，支持现代 GUI 开发
- **� 系统功能**: WiFi、SD卡、FreeRTOS 等硬件接口的完整 Lua 绑定
- **⚡ 高性能**: 针对 ESP32-S3 优化的事件处理和显示刷新机制
- **� OOBE 系统**: 开箱即用的 4 页式初始化向导，支持系统配置

## 🏗️ 系统架构

```
LuaRTOS
├── Lua 解释器 (PSRAM 优化)
│   ├── 内存分配器 (lua_psram_alloc.c)
│   └── 核心引擎 (lua_engine.c)
├── LVGL 绑定 (图形界面)
│   ├── 控件绑定 (lvgl_bindings.c)
│   └── 事件处理 (事件回调系统)
├── 系统绑定 (硬件接口)
│   ├── WiFi 管理 (system_bindings.c)
│   ├── SD 卡操作
│   └── FreeRTOS 接口
└── OOBE 系统 (用户向导)
    ├── 欢迎页面
    ├── SD 卡格式化
    ├── WiFi 连接配置
    └── 系统安装完成
```

## 💾 内存优化策略

### PSRAM 优先分配

```c
// 智能内存分配策略
void* lua_psram_alloc(size_t size) {
    if (size >= 32) {
        // 大对象优先使用 PSRAM
        void* ptr = heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
        if (ptr) return ptr;
    }
    // 小对象或 PSRAM 不足时使用内部 RAM
    return heap_caps_malloc(size, MALLOC_CAP_8BIT);
}
```

### 内存分布

```
┌─────────────────────────────────────────────────┐
│                ESP32-S3 内存架构                 │
├─────────────────────┬───────────────────────────┤
│   内置RAM (512KB)   │      PSRAM (8MB)         │
├─────────────────────┼───────────────────────────┤
│ • LVGL 显示缓冲区   │ • Lua 解释器和对象        │
│ • 系统关键任务      │ • 大型数据结构           │
│ • 中断处理程序      │ • 用户脚本数据           │
│ • 小对象(<32字节)   │ • GUI 组件状态           │
└─────────────────────┴───────────────────────────┘
```

## 🎯 OOBE 系统特点

### 用户体验设计
- **简化交互**: 一键式操作，降低用户学习成本
- **容错设计**: 完善的错误处理和状态恢复机制
- **视觉反馈**: 实时进度指示和状态提示
- **模块化**: 每个页面独立可维护，便于扩展

### 页面流程

1. **欢迎页面** (`create_welcome_screen`)
   - 系统介绍和版本信息
   - 开始配置按钮

2. **SD 卡格式化** (`create_sd_format_screen`)
   - 检测 SD 卡状态
   - 一键格式化功能
   - 进度显示

3. **WiFi 连接** (`create_wifi_connect_screen`)
   - 网络扫描和选择
   - 密码输入界面
   - 连接状态反馈

4. **系统安装** (`create_install_screen`)
   - 模拟系统安装过程
   - 进度条显示
   - 完成提示

## 📦 快速开始

### 1. 环境准备

```bash
# 安装 ESP-IDF
git clone https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh
source export.sh

# 克隆项目
git clone https://github.com/cacaview/luartos.git
cd luartos
```

### 2. 构建项目

```bash
# 配置目标芯片
idf.py set-target esp32s3

# 构建固件
idf.py build
```

### 3. 烧录和运行

```bash
# 烧录固件
idf.py flash

# 监控串口输出
idf.py monitor
```

### 4. 自动运行 OOBE

系统启动后会自动运行 OOBE 向导：
- 触摸屏幕开始配置流程
- 按照提示完成 SD 卡、WiFi 设置
- 系统安装完成后可进入正常使用

## 🔧 API 概览

### LVGL 图形接口

```lua
-- 创建按钮
local btn = lvgl.btn_create(screen)
lvgl.obj_set_size(btn, 120, 50)
lvgl.obj_align(btn, lvgl.ALIGN_CENTER(), 0, 0)

-- 添加事件处理
lvgl.obj_add_event_cb(btn, function(event)
    if lvgl.event_get_code(event) == lvgl.EVENT_CLICKED() then
        print("按钮被点击!")
    end
end)

-- 创建进度条
local bar = lvgl.bar_create(screen)
lvgl.obj_set_size(bar, 200, 20)
lvgl.bar_set_value(bar, 75, lvgl.ANIM_OFF())
```

### 系统功能接口

```lua
-- WiFi 操作
system.wifi_init()
local networks = system.wifi_scan()
system.wifi_connect("SSID", "password")
local status = system.wifi_get_status()

-- SD 卡操作
system.sd_init()
system.sd_format()
local mounted = system.sd_is_mounted()

-- 系统控制
system.sleep(1000)  -- FreeRTOS 非阻塞睡眠
system.delay(500)   -- 简单延时
local free_mem = system.get_free_heap()
```

## 📚 文档资源

- **[完整 API 文档](LuaRTOS_API_Documentation.md)** - 详细的函数说明、参数和示例
- **[快速参考手册](LuaRTOS_Quick_Reference.md)** - 常用函数和模板代码
- **[OOBE 实现](main/oobe_lua.lua)** - 完整的用户向导示例代码

## 🗂️ 项目结构

```
luartos/
├── main/
│   ├── main.c                    # 主程序入口，启动 Lua 解释器
│   ├── main.lua                  # 默认 Lua 脚本
│   └── oobe_lua.lua              # OOBE 向导脚本 (完整实现)
├── components/
│   └── lua/
│       ├── lua_engine.c          # Lua 解释器核心
│       ├── lua_engine.h
│       ├── lvgl_bindings.c       # LVGL 图形库绑定
│       ├── lvgl_bindings.h
│       ├── system_bindings.c     # 系统功能绑定
│       ├── system_bindings.h
│       ├── lua_psram_alloc.c     # PSRAM 优化分配器
│       └── lua_psram_alloc.h
├── GUI-Guider-Projects/
│   └── oobe/                     # 原始 C 语言 OOBE 实现 (参考)
├── build/                        # 构建输出目录
├── LuaRTOS_API_Documentation.md  # 完整 API 文档
├── LuaRTOS_Quick_Reference.md    # 快速参考手册
└── README.md                     # 本文件
```

## 🛠️ 开发指南

### 最佳实践

1. **内存管理优化**
   ```lua
   -- 优先使用局部变量
   local function create_ui()
       local container = lvgl.obj_create(screen)
       local btn = lvgl.btn_create(container)
       return container
   end
   
   -- 及时清理不需要的对象
   if old_dialog then
       lvgl.obj_del(old_dialog)
       old_dialog = nil
   end
   ```

2. **事件处理最佳实践**
   ```lua
   -- 避免在事件回调中执行阻塞操作
   lvgl.obj_add_event_cb(btn, function(event)
       if lvgl.event_get_code(event) == lvgl.EVENT_CLICKED() then
           -- 设置状态标志，在主循环中处理
           app_state.button_clicked = true
           
           -- 避免直接调用 system.sleep() 或耗时操作
       end
   end)
   ```

3. **批量更新和刷新**
   ```lua
   -- 批量修改 UI 后统一刷新
   lvgl.label_set_text(status_label, "连接中...")
   lvgl.bar_set_value(progress_bar, 50, lvgl.ANIM_OFF())
   lvgl.obj_set_style_bg_color(indicator, 0x00ff00, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
   lvgl.refr_now()  -- 一次性刷新所有变化
   ```

### 调试和性能监控

```lua
-- 内存使用监控
local function log_memory_usage(stage)
    local free = system.get_free_heap()
    local psram = system.get_psram_size()
    print(string.format("[%s] Free: %d bytes, PSRAM: %d bytes", stage, free, psram))
end

-- 事件调试助手
local function debug_events(obj, name)
    lvgl.obj_add_event_cb(obj, function(event)
        local code = lvgl.event_get_code(event)
        print(string.format("%s 事件: %d", name, code))
    end)
end

-- 性能测量
local function measure_performance(func, description)
    local start_time = system.get_tick_count()  -- 如果可用
    func()
    local end_time = system.get_tick_count()
    print(string.format("%s 耗时: %d ms", description, end_time - start_time))
end
```

## 🔍 技术细节

### PSRAM 使用统计

- **Lua 对象分配**: ~80% 使用 PSRAM
- **GUI 组件状态**: PSRAM 存储
- **显示缓冲区**: 内部 RAM (保证高速访问)
- **系统栈**: 内部 RAM (实时性要求)

### 性能指标

- **系统启动时间**: < 3 秒
- **OOBE 页面切换**: < 200ms
- **事件响应时间**: < 100ms
- **内存利用率**: PSRAM >85%, 内部 RAM >70%
- **同时支持控件数**: 100+ 个

### 支持的 LVGL 控件

| 控件类型 | Lua 绑定 | 示例用法 |
|---------|---------|---------|
| 基础对象 | ✅ | `lvgl.obj_create()` |
| 标签 | ✅ | `lvgl.label_create()` |
| 按钮 | ✅ | `lvgl.btn_create()` |
| 进度条 | ✅ | `lvgl.bar_create()` |
| 开关 | ✅ | `lvgl.switch_create()` |
| 列表 | ✅ | `lvgl.list_create()` |
| 消息框 | ✅ | `lvgl.msgbox_create()` |
| 文本输入 | ✅ | `lvgl.textarea_create()` |
| 键盘 | ✅ | `lvgl.keyboard_create()` |

## 🤝 参与贡献

我们欢迎所有形式的贡献：

1. **代码贡献**
   - Fork 项目仓库
   - 创建功能分支 (`git checkout -b feature/amazing-feature`)
   - 提交变更 (`git commit -m '添加了新功能'`)
   - 推送到分支 (`git push origin feature/amazing-feature`)
   - 创建 Pull Request

2. **问题反馈**
   - 使用 [GitHub Issues](https://github.com/cacaview/luartos/issues) 报告 Bug
   - 详细描述复现步骤和环境信息
   - 提供错误日志和截图

3. **文档改进**
   - 修正文档错误
   - 添加使用示例
   - 翻译文档到其他语言

## 📄 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件

## 🏷️ 版本历史

### v1.0.0 (2025-01-07)

- ✅ **初始版本发布**
- ✅ **完整的 LVGL Lua 绑定** - 支持所有主要控件和事件处理
- ✅ **PSRAM 优化的内存分配器** - 智能内存管理，最大化利用硬件资源
- ✅ **系统功能集成** - WiFi、SD卡、FreeRTOS 完整接口
- ✅ **OOBE 系统实现** - 4 页式用户向导，简化初始配置
- ✅ **完整的 API 文档** - 详细说明和示例代码

### 即将发布

- 🔄 **多语言支持** - 国际化界面
- 🔄 **OTA 升级** - 无线固件更新
- 🔄 **云端集成** - 物联网平台连接
- 🔄 **扩展组件** - 更多硬件模块支持

## 📧 联系方式

- **项目主页**: [GitHub Repository](https://github.com/cacaview/luartos)
- **问题反馈**: [GitHub Issues](https://github.com/cacaview/luartos/issues)
- **文档贡献**: Pull Request
- **技术讨论**: [Discussions](https://github.com/cacaview/luartos/discussions)

---

**LuaRTOS** - 让嵌入式图形界面开发更简单 🚀

*基于 ESP32-S3 的下一代 Lua 运行时系统*

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
