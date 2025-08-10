# LuaRTOS 技术文档与API参考

## 目录

- [硬件连接指南](#硬件连接指南)
- [软件架构详解](#软件架构详解)
- [Lua API 完整参考](#lua-api-完整参考)
  - [系统 API](#系统-api)
  - [LVGL Lua 绑定](#lvgl-lua-绑定)
- [开发指南](#开发指南)
- [故障排除](#故障排除)
- [性能优化](#性能优化)
- [附录：API 快速参考](#附录-api-快速参考)

## 硬件连接指南

### ILI9488 显示屏连接

ESP32-S3与ILI9488 3.5寸TFT显示屏的连接方式：

```
ESP32-S3    ILI9488     说明
----------- --------    --------
GPIO11  --> MOSI        SPI数据输出
GPIO12  --> SCLK        SPI时钟信号
GPIO10  --> CS          片选信号
GPIO9   --> DC(A0)      数据/命令选择
GPIO8   --> RST         复位信号
GPIO7   --> BL          背光控制
3V3     --> VCC         3.3V电源
GND     --> GND         接地
```

**重要注意事项：**
- 显示屏使用SPI2_HOST，最大传输大小限制为16KB
- 实现了SPI分块传输，避免硬件限制错误
- 支持RGB888色彩模式，自动从RGB565转换

### XPT2046 触摸屏连接

```
ESP32-S3    XPT2046     说明
----------- --------    --------
GPIO13  --> T_DIN       触摸SPI数据输入
GPIO14  --> T_DO        触摸SPI数据输出
GPIO15  --> T_CLK       触摸SPI时钟
GPIO16  --> T_CS        触摸片选
GPIO17  --> T_IRQ       触摸中断信号
3V3     --> VCC         3.3V电源
GND     --> GND         接地
```

### SD卡模块连接

```
ESP32-S3    SD卡        说明
----------- --------    --------
GPIO9   --> CS          片选信号
GPIO11  --> SCK         SPI时钟
GPIO42  --> MOSI        SPI数据输出
GPIO41  --> MISO        SPI数据输入
3V3     --> VCC         3.3V电源
GND     --> GND         接地
```

**SD卡要求：**
- 使用SPI3_HOST，与显示屏分离避免冲突
- 最大传输大小：8KB
- 支持FAT32格式，容量≤32GB
- 必须使用10kΩ上拉电阻
- 推荐使用Class 10高速卡

### 电源和上拉电阻

**上拉电阻配置：**
```
信号线           上拉电阻    连接到
SD_CS           10kΩ       3.3V
SD_MOSI         10kΩ       3.3V
SD_MISO         10kΩ       3.3V
SD_SCK          10kΩ       3.3V
```

**电源去耦：**
- 每个3.3V供电点添加100nF陶瓷电容
- 主电源添加10µF电解电容
- 确保电源纹波<50mV

## 软件架构详解

### 内存管理架构

LuaRTOS采用双分配器架构，针对ESP32-S3的内存特点进行优化：

```
ESP32-S3 内存分布
┌─────────────────────────────────────────────────┐
│                ESP32-S3 内存架构                 │
├─────────────────────┬───────────────────────────┤
│   内置RAM (512KB)   │      PSRAM (8MB)         │
├─────────────────────┼───────────────────────────┤
│ • LVGL图形渲染      │ • Lua解释器和对象        │
│ • 显示缓冲区        │ • 用户脚本数据           │
│ • 系统关键任务      │ • 大型数据缓冲区         │
│ • 中断处理程序      │ • 文件读写缓冲区         │
│ • WiFi协议栈        │ • 临时大内存分配         │
└─────────────────────┴───────────────────────────┘
```

### 组件架构

```
luartos/
├── main/                           # 主程序入口
│   ├── main.c                     # 系统启动、任务管理、GUI初始化
│   ├── oobe_lua.lua               # 开箱即用体验脚本
│   └── main_simple_lua.h          # 嵌入式Lua脚本头文件
├── components/                     # 核心组件库
│   ├── lua/                       # Lua引擎组件
│   │   ├── src/                   # Lua 5.4.6 源码
│   │   ├── lua_engine.c/h         # Lua引擎包装器
│   │   ├── lvgl_bindings.c/h      # LVGL Lua绑定
│   │   ├── system_bindings.c/h    # 系统API Lua绑定
│   │   └── lua_psram_alloc.c/h    # PSRAM优化内存分配器
│   ├── sdcard/                    # SD卡驱动组件
│   │   ├── sdcard_driver.c/h      # SPI SD卡驱动实现
│   │   └── CMakeLists.txt         # 组件构建配置
│   ├── lvgl/                      # LVGL图形库
│   │   ├── lv_conf.h              # LVGL配置文件
│   │   └── lvgl_internal_alloc.c/h # LVGL内置RAM分配器
│   └── lvgl_esp32_drivers/        # ESP32显示驱动
│       ├── lvgl_helpers.c/h       # LVGL初始化辅助
│       ├── lvgl_spi_conf.h        # SPI配置
│       └── lvgl_tft/              # TFT驱动
│           ├── ili9488.c/h        # ILI9488显示驱动
│           └── disp_spi.c/h       # 显示SPI接口
└── GUI-Guider-Projects/           # 图形界面设计项目
    └── oobe/                      # OOBE界面设计文件
```

### SPI总线分配

| 设备类型 | SPI主机 | 传输大小限制 | 频率 | 用途 |
|----------|---------|-------------|------|------|
| 显示屏 | SPI2_HOST | 16KB (分块传输) | 40MHz | ILI9488图形显示 |
| 触摸屏 | SPI2_HOST | 共享显示屏 | 2MHz | XPT2046触摸检测 |
| SD卡 | SPI3_HOST | 8KB | 1MHz | 文件系统存储 |

### 任务管理

```c
// 主要任务分配
FreeRTOS任务          优先级    栈大小      用途
gui_task             5        8192       Lua脚本执行和GUI更新
lvgl_task            4        4096       LVGL界面刷新
wifi_task            3        4096       WiFi连接管理
idle_task            0        1024       系统空闲处理
```

## Lua API 完整参考

### 系统 API

#### 内存管理

```lua
-- 获取系统内存信息
local free_heap = system.get_free_heap()        -- 获取空闲堆内存(字节)
local psram_size = system.get_psram_size()      -- 获取PSRAM总大小(字节)

-- 系统控制
system.restart()                               -- 重启系统
system.delay(milliseconds)                     -- 延时(毫秒)
system.sleep(milliseconds)                     -- FreeRTOS 睡眠(毫秒), 不阻塞其他任务

-- 示例：内存监控
print("空闲内存:", system.get_free_heap(), "字节")
print("PSRAM大小:", system.get_psram_size(), "字节")
```

#### SD卡操作

```lua
-- 初始化 SD 卡
local success, message = system.sd_init()
if success then
    print("SD card initialized:", message)
end

-- 检查 SD 卡是否挂载
if system.sd_is_mounted() then
    print("SD card is mounted")
end

-- 获取 SD 卡信息
local info = system.sd_get_info()
if info then
    print("Total:", info.total_bytes, "Free:", info.free_bytes, "Used:", info.used_bytes)
end

-- 读写文件
local write_success = system.sd_write_file("test.txt", "Hello, World!")
if write_success then
    local content = system.sd_read_file("test.txt")
    print("File content:", content)
end
```

#### WiFi操作

```lua
-- WiFi初始化
local success, message = system.wifi_init()

-- 扫描网络
local networks = system.wifi_scan()
for i, network in ipairs(networks) do
    print("SSID:", network.ssid, "RSSI:", network.rssi, "认证:", network.authmode)
end

-- 连接网络
local success, message = system.wifi_connect("Your_SSID", "Your_Password")
if success then
    print("WiFi连接成功")
    local ip = system.wifi_get_ip()
    print("IP地址:", ip)
else
    print("WiFi连接失败:", message)
end

-- 断开连接
system.wifi_disconnect()

-- 检查连接状态
local connected = system.wifi_is_connected()

-- 获取WiFi状态
-- 返回 "not_initialized", "connected", "connecting", "disconnected"
local status = system.wifi_get_status()
print("WiFi 状态:", status)
```

#### 定时器

```lua
-- 创建定时器
local timer = system.timer_create(
    1000,    -- 周期(毫秒)
    true,    -- 自动重载
    function()
        print("定时器触发:", os.date())
    end
)

-- 控制定时器
system.timer_start(timer)  -- 启动
system.timer_stop(timer)   -- 停止

-- 单次定时器示例
local once_timer = system.timer_create(5000, false, function()
    print("5秒后执行一次")
end)
system.timer_start(once_timer)
```

### LVGL Lua 绑定

#### 核心对象操作

##### 屏幕管理

```lua
-- 获取当前屏幕
local screen = lvgl.scr_act()

-- 创建新屏幕
local new_screen = lvgl.obj_create(nil)

-- 屏幕切换（带动画）
-- anim_type: lvgl.SCR_LOAD_ANIM_NONE, lvgl.SCR_LOAD_ANIM_MOVE_*, etc.
-- time: 动画时长 (ms)
-- delay: 动画开始前的延迟 (ms)
-- auto_del: 动画结束后是否自动删除旧屏幕
lvgl.scr_load_anim(new_screen, lvgl.SCR_LOAD_ANIM_NONE, 200, 200, false)

-- 强制刷新显示
lvgl.refr_now()
```

##### 基础对象操作

```lua
-- 创建对象
local obj = lvgl.obj_create(parent)

-- 大小和位置
lvgl.obj_set_size(obj, width, height)
lvgl.obj_set_width(obj, width)
lvgl.obj_set_pos(obj, x, y)

-- 对齐方式
lvgl.obj_align(obj, lvgl.ALIGN_CENTER, x_offset, y_offset)
lvgl.obj_align_to(obj, base_obj, lvgl.ALIGN_TOP_MID, x_offset, y_offset)
lvgl.obj_center(obj)

-- 对象配置
lvgl.obj_set_scrollbar_mode(obj, lvgl.SCROLLBAR_MODE_OFF)
```

#### 样式系统

##### 背景和边框

```lua
-- 背景样式
lvgl.obj_set_style_bg_color(obj, 0xFF0000, lvgl.PART_MAIN)  -- 红色背景
lvgl.obj_set_style_bg_opa(obj, 128, lvgl.PART_MAIN)        -- 50%透明度

-- 边框样式
lvgl.obj_set_style_border_width(obj, 2, lvgl.PART_MAIN)
lvgl.obj_set_style_border_color(obj, 0x000000, lvgl.PART_MAIN)
lvgl.obj_set_style_radius(obj, 10, lvgl.PART_MAIN)         -- 圆角

-- 阴影效果
lvgl.obj_set_style_shadow_width(obj, 5, lvgl.PART_MAIN)
```

##### 文本样式

```lua
-- 文本颜色和字体
lvgl.obj_set_style_text_color(obj, 0xFFFFFF, lvgl.PART_MAIN)
lvgl.obj_set_style_text_font(obj, lvgl.font_montserrat_16, lvgl.PART_MAIN)

-- 文本对齐和间距
lvgl.obj_set_style_text_align(obj, lvgl.TEXT_ALIGN_CENTER, lvgl.PART_MAIN)
lvgl.obj_set_style_text_letter_space(obj, 2, lvgl.PART_MAIN)
lvgl.obj_set_style_text_line_space(obj, 5, lvgl.PART_MAIN)
```

##### 填充（Padding）

```lua
-- 统一填充
lvgl.obj_set_style_pad_all(obj, 10, lvgl.PART_MAIN)

-- 分别设置
lvgl.obj_set_style_pad_top(obj, 5, lvgl.PART_MAIN)
lvgl.obj_set_style_pad_bottom(obj, 5, lvgl.PART_MAIN)
lvgl.obj_set_style_pad_left(obj, 10, lvgl.PART_MAIN)
lvgl.obj_set_style_pad_right(obj, 10, lvgl.PART_MAIN)
```

#### 控件（Widgets）

##### 标签（Label）

```lua
-- 创建标签
local label = lvgl.label_create(parent)
lvgl.label_set_text(label, "Hello LuaRTOS!")

-- 长文本处理
lvgl.label_set_long_mode(label, lvgl.LABEL_LONG_WRAP)

-- 标签样式
lvgl.obj_set_style_text_color(label, lvgl.color_white(), lvgl.PART_MAIN)
lvgl.obj_set_style_text_font(label, lvgl.font_montserrat_20, lvgl.PART_MAIN)
```

##### 按钮（Button）

```lua
-- 创建按钮
local btn = lvgl.btn_create(parent)
lvgl.obj_set_size(btn, 120, 50)

-- 按钮标签
local btn_label = lvgl.label_create(btn)
lvgl.label_set_text(btn_label, "点击我")
lvgl.obj_center(btn_label)

-- 按钮样式
lvgl.obj_set_style_bg_color(btn, 0x0066CC, lvgl.PART_MAIN)
lvgl.obj_set_style_radius(btn, 5, lvgl.PART_MAIN)
```

##### 进度条（Bar）

```lua
local bar = lvgl.bar_create(parent)
lvgl.obj_set_size(bar, 200, 20)
lvgl.bar_set_mode(bar, lvgl.BAR_MODE_NORMAL)
lvgl.bar_set_range(bar, 0, 100)
lvgl.bar_set_value(bar, 50, lvgl.ANIM_OFF)
```

#### 事件处理

```lua
-- 为对象添加事件回调
lvgl.obj_add_event_cb(btn, function(event)
    local code = lvgl.event_get_code(event)
    local target = lvgl.event_get_target(event)
    -- 获取在 add_event_cb 中传递的用户数据
    local user_data = lvgl.event_get_user_data(event)

    if code == lvgl.EVENT_CLICKED then
        print("按钮被点击了!")
    end
end, my_data) -- my_data 是传递给回调的用户数据

-- 检查对象状态
local is_checked = lvgl.obj_has_state(switch, lvgl.STATE_CHECKED)

-- 添加/移除对象标志
lvgl.obj_add_flag(obj, lvgl.OBJ_FLAG_HIDDEN)    -- 隐藏对象
lvgl.obj_clear_flag(obj, lvgl.OBJ_FLAG_HIDDEN) -- 显示对象
```

## 开发指南

### 环境配置

#### ESP-IDF安装

```bash
# 1. 下载ESP-IDF
git clone -b v5.1 --recursive https://github.com/espressif/esp-idf.git
cd esp-idf

# 2. 安装工具链
./install.sh esp32s3

# 3. 设置环境变量
source export.sh
```

#### 项目配置

```bash
# 1. 克隆项目
git clone <your-repo-url> luartos
cd luartos

# 2. 配置项目
idf.py menuconfig

# 重要配置项：
# - Component config -> ESP32S3-Specific -> CPU frequency (240MHz)
# - Component config -> FreeRTOS -> Tick rate (1000Hz)  
# - Component config -> PSRAM -> Enable PSRAM
```

#### 编译和烧录

```bash
# 1. 清理构建
idf.py clean

# 2. 编译项目
idf.py build

# 3. 烧录固件
idf.py flash

# 4. 监控串口
idf.py monitor

# 5. 一键操作
idf.py build flash monitor
```

## 故障排除

### 常见问题和解决方案

#### 1. SPI传输错误

**问题**: `spi_master: check_trans_valid: txdata transfer > host maximum`

**解决方案**:
- 检查SPI传输大小设置
- 确认分块传输实现正确
- 验证硬件连接

#### 2. SD卡无法挂载

**问题**: SD卡初始化失败或挂载失败

**解决方案**:
- 验证硬件连接
- 确认上拉电阻（10kΩ）
- 检查SD卡格式（FAT32）
- 验证电源稳定性（3.3V ±0.1V）

## 性能优化

### 内存优化

#### 1. PSRAM优化

```c
// 配置Lua使用PSRAM
#define LUA_FORCE_PSRAM_SIZE 32  // 32字节以上使用PSRAM

// 在lua_psram_alloc.c中调整阈值
if (size >= LUA_FORCE_PSRAM_SIZE) {
    // 优先使用PSRAM
    ptr = heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
}
```

#### 2. Lua脚本优化

```lua
-- 避免频繁创建大对象
local reusable_table = {}

function optimized_function()
    -- 重用表而不是创建新的
    for k in pairs(reusable_table) do
        reusable_table[k] = nil
    end
end

-- 及时清理大对象
large_data = nil
collectgarbage("collect")
```

---

## 附录：API 快速参考

### 常用 LVGL 函数

#### 基础对象
```lua
local obj = lvgl.obj_create(parent)
lvgl.obj_set_pos(obj, x, y)
lvgl.obj_set_size(obj, w, h)
lvgl.obj_align(obj, lvgl.ALIGN_CENTER, 0, 0)
lvgl.obj_center(obj)
lvgl.obj_del(obj)
```

#### 事件处理
```lua
lvgl.obj_add_event_cb(obj, function(event)
    if lvgl.event_get_code(event) == lvgl.EVENT_CLICKED then
        -- 处理点击事件
    end
end, user_data)
```

### 常用系统函数

#### WiFi
```lua
system.wifi_init()
system.wifi_scan()
system.wifi_connect("SSID", "password")
system.wifi_is_connected()
system.wifi_get_ip()
```

#### SD 卡
```lua
system.sd_init()
system.sd_is_mounted()
system.sd_get_info()
system.sd_write_file("file.txt", "content")
system.sd_read_file("file.txt")
```

#### 系统控制
```lua
system.delay(1000)    -- 延时 1 秒
system.sleep(2000)    -- FreeRTOS 睡眠 2 秒
system.get_free_heap()
system.restart()
