# LuaRTOS 技术文档与API参考

## 目录

- [硬件连接指南](#硬件连接指南)
- [软件架构详解](#软件架构详解)
- [Lua API 完整参考](#lua-api-完整参考)
- [LVGL Lua 绑定](#lvgl-lua-绑定)
- [系统 API](#系统-api)
- [开发指南](#开发指南)
- [故障排除](#故障排除)
- [性能优化](#性能优化)

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
│   ├── memory_demo.lua            # 内存优化演示脚本
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
system.sleep(milliseconds)                     -- 睡眠(毫秒), 与delay功能相同

-- 示例：内存监控
print("空闲内存:", system.get_free_heap(), "字节")
print("PSRAM大小:", system.get_psram_size(), "字节")
```

#### SD卡操作 (sdcard 模块)

```lua
-- 引入sdcard模块
local sdcard = require("sdcard")

-- 初始化
local ok, err = sdcard.init()
if not ok then
    print("SD卡初始化失败:", err)
    return
end

-- 挂载文件系统
-- sdcard.mount([mount_point], [max_files])
local ok, err = sdcard.mount()
if not ok then
    print("SD卡挂载失败:", err)
    return
end

-- 检查挂载状态
if sdcard.is_mounted() then
    print("SD卡已挂载")
end

-- 写文件
-- sdcard.write_file(filename, data) -> ok, bytes_written | false, error
local ok, bytes_written = sdcard.write_file("test.txt", "Hello from LuaRTOS!")
if ok then
    print("成功写入", bytes_written, "字节")
end

-- 读文件
-- sdcard.read_file(filename) -> content | nil, error
local content, err = sdcard.read_file("test.txt")
if content then
    print("读取内容:", content)
end

-- 列出文件
-- sdcard.list_files([path]) -> { {name="...", size=...}, ... } | nil, error
local files, err = sdcard.list_files("/")
if files then
    for _, file in ipairs(files) do
        print(string.format("- %s (%d bytes)", file.name, file.size))
    end
end

-- 获取使用情况
-- sdcard.get_usage() -> { total=..., used=..., free=... } | nil, error
local usage, err = sdcard.get_usage()
if usage then
    print(string.format("总空间: %.2f MB, 已用: %.2f MB, 可用: %.2f MB",
          usage.total / 1024 / 1024,
          usage.used / 1024 / 1024,
          usage.free / 1024 / 1024))
end

-- 获取SD卡信息
-- sdcard.get_info() -> { name="...", type="...", ... } | nil, error
local info, err = sdcard.get_info()
if info then
    print("卡名称:", info.name)
    print("卡类型:", info.type)
    print("容量:", info.capacity_mb, "MB")
end

-- 删除文件
local ok, err = sdcard.delete_file("test.txt")

-- 卸载
sdcard.unmount()
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

## LVGL Lua 绑定

### 核心对象操作

#### 屏幕管理

```lua
-- 获取当前屏幕
local screen = lvgl.scr_act()

-- 创建新屏幕
local new_screen = lvgl.obj_create(nil)

-- 屏幕切换（带动画）
lvgl.scr_load_anim(new_screen, lvgl.SCR_LOAD_ANIM_NONE(), 200, 200, false)

-- 强制刷新显示
lvgl.refr_now()
```

#### 基础对象操作

```lua
-- 创建对象
local obj = lvgl.obj_create(parent)

-- 大小和位置
lvgl.obj_set_size(obj, width, height)
lvgl.obj_set_width(obj, width)
lvgl.obj_set_pos(obj, x, y)

-- 对齐方式
lvgl.obj_align(obj, lvgl.ALIGN_CENTER(), x_offset, y_offset)
lvgl.obj_align_to(obj, base_obj, lvgl.ALIGN_TOP_MID(), x_offset, y_offset)
lvgl.obj_center(obj)

-- 对象配置
lvgl.obj_set_scrollbar_mode(obj, lvgl.SCROLLBAR_MODE_OFF())
```

### 样式系统

#### 背景和边框

```lua
-- 背景样式
lvgl.obj_set_style_bg_color(obj, 0xFF0000, lvgl.PART_MAIN())  -- 红色背景
lvgl.obj_set_style_bg_opa(obj, 128, lvgl.PART_MAIN())        -- 50%透明度

-- 边框样式
lvgl.obj_set_style_border_width(obj, 2, lvgl.PART_MAIN())
lvgl.obj_set_style_border_color(obj, 0x000000, lvgl.PART_MAIN())
lvgl.obj_set_style_radius(obj, 10, lvgl.PART_MAIN())         -- 圆角

-- 阴影效果
lvgl.obj_set_style_shadow_width(obj, 5, lvgl.PART_MAIN())
```

#### 文本样式

```lua
-- 文本颜色和字体
lvgl.obj_set_style_text_color(obj, 0xFFFFFF, lvgl.PART_MAIN())
lvgl.obj_set_style_text_font(obj, lvgl.font_montserrat_16(), lvgl.PART_MAIN())

-- 文本对齐和间距
lvgl.obj_set_style_text_align(obj, lvgl.TEXT_ALIGN_CENTER(), lvgl.PART_MAIN())
lvgl.obj_set_style_text_letter_space(obj, 2, lvgl.PART_MAIN())
lvgl.obj_set_style_text_line_space(obj, 5, lvgl.PART_MAIN())
```

#### 填充（Padding）

```lua
-- 统一填充
lvgl.obj_set_style_pad_all(obj, 10, lvgl.PART_MAIN())

-- 分别设置
lvgl.obj_set_style_pad_top(obj, 5, lvgl.PART_MAIN())
lvgl.obj_set_style_pad_bottom(obj, 5, lvgl.PART_MAIN())
lvgl.obj_set_style_pad_left(obj, 10, lvgl.PART_MAIN())
lvgl.obj_set_style_pad_right(obj, 10, lvgl.PART_MAIN())
```

### 控件（Widgets）

#### 标签（Label）

```lua
-- 创建标签
local label = lvgl.label_create(parent)
lvgl.label_set_text(label, "Hello LuaRTOS!")

-- 长文本处理
lvgl.label_set_long_mode(label, lvgl.LABEL_LONG_WRAP())

-- 标签样式
lvgl.obj_set_style_text_color(label, lvgl.color_white(), lvgl.PART_MAIN())
lvgl.obj_set_style_text_font(label, lvgl.font_montserrat_20(), lvgl.PART_MAIN())
```

#### 按钮（Button）

```lua
-- 创建按钮
local btn = lvgl.btn_create(parent)
lvgl.obj_set_size(btn, 120, 50)

-- 按钮标签
local btn_label = lvgl.label_create(btn)
lvgl.label_set_text(btn_label, "点击我")
lvgl.obj_center(btn_label)

-- 按钮样式
lvgl.obj_set_style_bg_color(btn, 0x0066CC, lvgl.PART_MAIN())
lvgl.obj_set_style_radius(btn, 5, lvgl.PART_MAIN())
```

#### 滑块（Slider）

```lua
-- 创建滑块
local slider = lvgl.slider_create(parent)
lvgl.obj_set_size(slider, 200, 20)

-- 设置范围和值
-- lvgl.slider_set_range(slider, 0, 100) -- 注意: 此函数在当前绑定中未实现
lvgl.slider_set_value(slider, 50, lvgl.ANIM_OFF())

-- 滑块样式
lvgl.obj_set_style_bg_color(slider, 0xCCCCCC, lvgl.PART_MAIN())
lvgl.obj_set_style_bg_color(slider, 0x0066CC, lvgl.PART_INDICATOR())
```

#### 开关（Switch）

```lua
-- 创建开关
local switch = lvgl.switch_create(parent)
lvgl.obj_set_size(switch, 60, 30)

-- 开关样式
lvgl.obj_set_style_bg_color(switch, 0xCCCCCC, lvgl.PART_MAIN())
lvgl.obj_set_style_bg_color(switch, 0x00AA00, lvgl.PART_INDICATOR())
```

#### 进度条（Bar）

```lua
-- 创建进度条
local bar = lvgl.bar_create(parent)
lvgl.obj_set_size(bar, 200, 20)

-- 设置范围和值
lvgl.bar_set_range(bar, 0, 100)
lvgl.bar_set_value(bar, 75, lvgl.ANIM_ON())

-- 进度条样式
lvgl.obj_set_style_bg_color(bar, 0xEEEEEE, lvgl.PART_MAIN())
lvgl.obj_set_style_bg_color(bar, 0x00AA00, lvgl.PART_INDICATOR())
```

#### 文本区域（Text Area）

```lua
-- 创建文本区域
local ta = lvgl.textarea_create(parent)
lvgl.obj_set_size(ta, 200, 80)
lvgl.textarea_set_text(ta, "Initial text")

-- 获取文本
local text = lvgl.textarea_get_text(ta)
print("Textarea content:", text)
```

#### 键盘（Keyboard）

```lua
-- 创建键盘
local kb = lvgl.keyboard_create(parent)

-- 通常与文本区域关联
-- lvgl.keyboard_set_textarea(kb, ta) -- 此绑定当前缺失，需要添加
```

#### 消息框（Msgbox）

```lua
-- 创建消息框
local mbox = lvgl.msgbox_create(
    parent,
    "提示",
    "这是一个消息框。",
    "确定", -- 按钮文本
    true -- 添加关闭按钮
)
lvgl.obj_center(mbox)
```

#### 跨度组（Spangroup）- 富文本

```lua
-- 创建跨度组
local spangroup = lvgl.spangroup_create(parent)
lvgl.obj_set_size(spangroup, 300, 100)

-- 配置跨度组
lvgl.spangroup_set_align(spangroup, lvgl.TEXT_ALIGN_CENTER())
lvgl.spangroup_set_overflow(spangroup, lvgl.SPAN_OVERFLOW_CLIP())
lvgl.spangroup_set_mode(spangroup, lvgl.SPAN_MODE_BREAK())

-- 添加文本跨度
local span = lvgl.spangroup_new_span(spangroup)
lvgl.span_set_text(span, "这是富文本内容")

-- 刷新显示
lvgl.spangroup_refr_mode(spangroup)
```

#### 列表（List）

```lua
-- 创建列表
local list = lvgl.list_create(parent)
lvgl.obj_set_size(list, 200, 300)

-- 添加文本项
local text_item = lvgl.list_add_text(list, "分组标题")

-- 添加按钮项
local btn_item = lvgl.list_add_btn(list, nil, "列表项目1")
```

### 事件处理

```lua
-- 为对象添加事件回调
lvgl.obj_add_event_cb(btn, function(event)
    local code = lvgl.event_get_code(event)
    local target = lvgl.event_get_target(event)

    if code == lvgl.EVENT_CLICKED() then
        print("按钮被点击了!")
        -- 切换标签文本
        -- 假设btn_label在作用域内
        -- if lvgl.label_get_text(btn_label) == "点击我" then
        --     lvgl.label_set_text(btn_label, "已点击")
        -- else
        --     lvgl.label_set_text(btn_label, "点击我")
        -- end
    end
end)

-- 检查对象状态
local is_checked = lvgl.obj_has_state(switch, lvgl.STATE_CHECKED())

-- 添加/移除对象标志
lvgl.obj_add_flag(obj, lvgl.OBJ_FLAG_HIDDEN())    -- 隐藏对象
lvgl.obj_clear_flag(obj, lvgl.OBJ_FLAG_HIDDEN()) -- 显示对象
```

### 颜色和常量

#### 颜色函数

```lua
-- 创建颜色
local red = lvgl.color_hex(0xFF0000)
local white = lvgl.color_white()
local black = lvgl.color_black()

-- 使用颜色
lvgl.obj_set_style_bg_color(obj, red, lvgl.PART_MAIN())
```

#### 对齐常量

```lua
-- 基本对齐
lvgl.ALIGN_CENTER()       -- 居中
lvgl.ALIGN_TOP_LEFT()     -- 左上角
lvgl.ALIGN_TOP_MID()      -- 顶部中间
lvgl.ALIGN_TOP_RIGHT()    -- 右上角
lvgl.ALIGN_BOTTOM_LEFT()  -- 左下角
lvgl.ALIGN_BOTTOM_MID()   -- 底部中间
lvgl.ALIGN_BOTTOM_RIGHT() -- 右下角

-- 外部对齐
lvgl.ALIGN_OUT_TOP_MID()  -- 外部顶部中间
```

#### 其他常量

```lua
-- 部分选择器
lvgl.PART_MAIN()          -- 主要部分
lvgl.PART_INDICATOR()     -- 指示器部分

-- 动画
lvgl.ANIM_OFF()           -- 关闭动画
lvgl.ANIM_ON()            -- 开启动画

-- 字体
lvgl.font_montserrat_14() -- 14号字体
lvgl.font_montserrat_16() -- 16号字体
lvgl.font_montserrat_20() -- 20号字体
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

### 自定义开发

#### 添加新的Lua API

1. **在system_bindings.h中声明函数**：
```c
int my_custom_function(lua_State* L);
```

2. **在system_bindings.c中实现函数**：
```c
int my_custom_function(lua_State* L) {
    const char* param = luaL_checkstring(L, 1);
    
    // 你的功能实现
    printf("Custom function called with: %s\n", param);
    
    lua_pushboolean(L, true);
    lua_pushstring(L, "Success");
    return 2;  // 返回值数量
}
```

3. **在函数数组中注册**：
```c
static const luaL_Reg system_functions[] = {
    // ... 现有函数
    {"my_custom", my_custom_function},
    {NULL, NULL}
};
```

4. **在Lua中使用**：
```lua
local success, message = system.my_custom("test parameter")
```

#### 修改LVGL配置

编辑 `components/lvgl/lv_conf.h`：

```c
// 内存配置
#define LV_MEM_SIZE    (128 * 1024U)    // 增加LVGL内存

// 显示配置
#define LV_COLOR_DEPTH     16           // RGB565色彩深度
#define LV_DPI             130          // DPI设置

// 字体配置
#define LV_FONT_MONTSERRAT_14    1      // 启用14号字体
#define LV_FONT_MONTSERRAT_16    1      // 启用16号字体
```

#### 创建自定义界面

```lua
-- 创建自定义界面函数
function create_custom_screen()
    local screen = lvgl.obj_create(nil)
    lvgl.obj_set_style_bg_color(screen, 0x001122, lvgl.PART_MAIN())
    
    -- 添加标题
    local title = lvgl.label_create(screen)
    lvgl.label_set_text(title, "自定义界面")
    lvgl.obj_set_style_text_color(title, lvgl.color_white(), lvgl.PART_MAIN())
    lvgl.obj_align(title, lvgl.ALIGN_TOP_MID(), 0, 20)
    
    -- 添加内容区域
    local content = lvgl.obj_create(screen)
    lvgl.obj_set_size(content, 400, 200)
    lvgl.obj_align(content, lvgl.ALIGN_CENTER(), 0, 0)
    
    return screen
end

-- 使用自定义界面
local my_screen = create_custom_screen()
lvgl.scr_load_anim(my_screen, lvgl.SCR_LOAD_ANIM_NONE(), 200, 200, false)
```

## 故障排除

### 常见问题和解决方案

#### 1. 编译错误

**问题**: `lvgl_internal_alloc.c: conflicting types`
```bash
components/lvgl/lvgl_internal_alloc.c:13:3: error: conflicting types for 'lvgl_memory_stats_t'
```

**解决方案**: 
- 确保头文件和源文件中的结构体定义一致
- 删除重复的typedef定义

#### 2. SPI传输错误

**问题**: `spi_master: check_trans_valid: txdata transfer > host maximum`

**解决方案**:
- 检查SPI传输大小设置
- 确认分块传输实现正确
- 验证硬件连接

#### 3. SD卡无法挂载

**问题**: SD卡初始化失败或挂载失败

**解决方案**:
```bash
# 检查清单：
1. 验证硬件连接
2. 确认上拉电阻（10kΩ）
3. 检查SD卡格式（FAT32）
4. 验证电源稳定性（3.3V ±0.1V）
5. 使用高质量SD卡
```

#### 4. 显示问题

**问题**: 显示屏无显示或显示异常

**解决方案**:
```lua
-- 检查显示初始化
print("显示初始化状态:", lvgl.scr_act())

-- 强制刷新
lvgl.refr_now()

-- 检查背光控制
-- GPIO7应该输出高电平
```

#### 5. 内存不足

**问题**: 系统运行时内存不足

**解决方案**:
```lua
-- 监控内存使用
function monitor_memory()
    local free_heap = system.get_free_heap()
    local psram_size = system.get_psram_size()
    
    print("空闲内存:", free_heap)
    print("PSRAM:", psram_size)
    
    if free_heap < 50000 then
        print("警告：内存不足！")
    end
end

-- 定期检查
local timer = system.timer_create(5000, true, monitor_memory)
system.timer_start(timer)
```

#### 6. WiFi连接失败

**问题**: WiFi无法连接或连接不稳定

**解决方案**:
```lua
-- WiFi调试
function debug_wifi()
    local networks = system.wifi_scan()
    if #networks == 0 then
        print("未发现WiFi网络，检查天线")
        return
    end
    
    for i, network in ipairs(networks) do
        print("发现网络:", network.ssid, "信号强度:", network.rssi)
    end
    
    -- 重试连接
    for retry = 1, 3 do
        local success = system.wifi_connect("SSID", "Password")
        if success then
            print("WiFi连接成功")
            break
        else
            print("重试连接 #", retry)
            system.delay(2000)
        end
    end
end
```

### 调试技巧

#### 1. 串口调试

```c
// 在C代码中添加调试输出
ESP_LOGI("TAG", "调试信息: %d", value);
ESP_LOGW("TAG", "警告信息");
ESP_LOGE("TAG", "错误信息");
```

```lua
-- 在Lua代码中添加调试输出
print("调试:", variable)
print("状态:", system.get_free_heap())
```

#### 2. 内存调试

```lua
-- 创建内存监控函数
function debug_memory()
    collectgarbage("collect")  -- 强制垃圾回收
    
    local heap = system.get_free_heap()
    local psram = system.get_psram_size()
    
    print(string.format("内存状态 - 堆:%d, PSRAM:%d", heap, psram))
end

-- 在关键位置调用
debug_memory()
```

#### 3. LVGL调试

```lua
-- LVGL对象调试
function debug_lvgl_object(obj)
    if obj then
        print("LVGL对象创建成功")
    else
        print("LVGL对象创建失败")
    end
end

local label = lvgl.label_create(parent)
debug_lvgl_object(label)
```

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

#### 2. LVGL内存优化

```c
// 在lv_conf.h中配置
#define LV_MEM_CUSTOM 1
#define LV_MEM_CUSTOM_ALLOC   lvgl_internal_malloc
#define LV_MEM_CUSTOM_FREE    lvgl_internal_free

// 优化显示缓冲区
#define DISP_BUF_LINES 20  // 减少缓冲区行数
```

#### 3. Lua脚本优化

```lua
-- 避免频繁创建大对象
local reusable_table = {}

function optimized_function()
    -- 重用表而不是创建新的
    for k in pairs(reusable_table) do
        reusable_table[k] = nil
    end
    
    -- 使用reusable_table
end

-- 及时清理大对象
large_data = nil
collectgarbage("collect")
```

### 显示性能优化

#### 1. SPI分块传输优化

```c
// 在ili9488.c中优化块大小
#define MAX_CHUNK_SIZE 8192  // 8KB块大小

// 优化传输逻辑
while (remaining > 0) {
    size_t chunk_size = (remaining > MAX_CHUNK_SIZE) ? MAX_CHUNK_SIZE : remaining;
    disp_spi_send_colors(data_ptr + sent, chunk_size);
    
    sent += chunk_size;
    remaining -= chunk_size;
    
    if (remaining > 0) {
        disp_wait_for_pending_transactions();
    }
}
```

#### 2. 显示缓冲区优化

```c
// 减少显示缓冲区大小以提高响应性
#ifdef CONFIG_SPIRAM
    #define DISP_BUF_LINES 20  // 20行缓冲区
#else
    #define DISP_BUF_LINES 10  // 10行缓冲区
#endif
```

### 系统性能监控

```lua
-- 创建性能监控
function create_performance_monitor()
    local last_time = 0
    local frame_count = 0
    
    return function()
        frame_count = frame_count + 1
        local current_time = system.get_tick_count()
        
        if current_time - last_time >= 1000 then  -- 每秒统计
            local fps = frame_count * 1000 / (current_time - last_time)
            print(string.format("FPS: %.1f, 内存: %d", fps, system.get_free_heap()))
            
            frame_count = 0
            last_time = current_time
        end
    end
end

local monitor = create_performance_monitor()

-- 在主循环中调用
function main_loop()
    -- 界面更新代码
    lvgl.refr_now()
    
    -- 性能监控
    monitor()
end
```

#### 从SD卡加载Lua模块

在从SD卡加载包含多个模块的复杂Lua应用时，我们发现并解决了一个核心问题。以下是相关的配置要求和最终实现方案。

**1. 关键配置：启用长文件名支持 (LFN)**

在默认的ESP-IDF配置中，FAT文件系统可能未开启对长文件名的支持，这会导致Lua的`require`函数无法通过标准路径（如`"APP.main.gui_guider"`）找到文件。

**解决方案**:
必须在项目配置中启用LFN支持。
```bash
# 1. 运行 menuconfig
idf.py menuconfig

# 2. 导航至
Component config ---> FAT Filesystem support --->

# 3. 选择长文件名支持模式
(X) Long filename buffer in heap
```
选择“在堆中为长文件名分配缓冲区”(`Long filename buffer in heap`)是最安全的选择，可以避免堆栈溢出风险。

**2. 最终实现方案：预加载机制**

为了彻底解决文件系统在运行时可能出现的任何问题，并提高模块加载性能，系统采用了“预加载”机制。

- **工作原理**: 在`main/main.c`中，系统启动时会首先将所有应用所需的`.lua`模块从SD卡一次性读取到内存中。
- **注入Lua环境**: 然后，这些从内存中加载的模块会被编译成Lua字节码，并直接存入Lua的`package.preload`表中。
- **无缝`require`**: 当Lua代码执行`require("some.module")`时，它会首先在`package.preload`中查找，并立即找到已经加载好的模块，无需再次访问文件系统。

**`main.c`中的实现示例**:
```c
// 定义需要预加载的模块列表
const char* modules_to_preload[][2] = {
    {"APP.main.gui_guider", "/sdcard/APP/main/gui_guider.lua"},
    {"APP.main.events_init", "/sdcard/APP/main/events_init.lua"},
    // ... 其他模块
    {NULL, NULL}
};

// 循环预加载
bool all_preloaded = true;
for (int i = 0; modules_to_preload[i][0] != NULL; i++) {
    if (!preload_module(g_lua_state, modules_to_preload[i][0], modules_to_preload[i][1])) {
        all_preloaded = false;
        break;
    }
}

// 如果全部预加载成功，则执行主脚本
if (all_preloaded) {
    lua_engine_exec_file(g_lua_state, "/sdcard/APP/main/main.lua");
}
```
这个方案不仅解决了文件加载问题，也为应用启动提供了性能优化。

### API绑定和代码转换注记

在将C代码（尤其是由GUI Guider生成的代码）转换为Lua时，需要注意以下几点：

**1. 缺失的API绑定**

- **`lv_anim` 动画API**: 当前的Lua绑定中**没有**包含对`lv_anim`动画系统的完整支持。因此，所有与`lv_anim_t`、`lv_anim_set_exec_cb`、`lv_anim_start`等相关的C代码都无法直接转换。在`gui_guider.lua`中的`ui_animation`函数是一个空实现占位符，以提醒开发者此限制。
- **`lv_keyboard_set_textarea`**: 此函数绑定当前也缺失，无法直接将键盘控件与文本区域关联。

**2. C到Lua的转换要点**

- **样式（Styles）**: 在C中，`lv_style_t`是一个需要初始化的结构体。在Lua中，样式可以被看作是一个普通的table。当需要“重置”一个样式时，只需将table中的所有键值对设为`nil`即可。
- **指针和引用**: C代码中大量使用指针来修改传递的变量（例如`bool * old_scr_del_ref`）。在Lua中，由于数字和布尔值是按值传递的，无法直接模拟这种行为。一种解决方法是使用只有一个字段的table（例如`{ value = true }`）来模拟引用传递。
- **常量和枚举**: C中的枚举和宏定义（如`LV_SCR_LOAD_ANIM_NONE`）在Lua中被绑定为返回相应值的函数（如`lvgl.SCR_LOAD_ANIM_NONE()`）。

这个技术文档涵盖了LuaRTOS的所有技术细节，从硬件连接到软件架构，从API参考到开发指南，以及完整的故障排除和性能优化指导。开发者可以根据这个文档快速上手并深入开发基于LuaRTOS的项目。

### 新增 LVGL 绑定 (近期更新)

为了支持更复杂的UI功能，我们新增了以下LVGL API的Lua绑定。

| 函数/常量 | 类型 | 描述 |
| :--- | :--- | :--- |
| `lvgl.textarea_set_placeholder_text(ta, text)` | 函数 | 设置文本区域的占位符文本。 |
| `lvgl.textarea_set_max_length(ta, length)` | 函数 | 设置文本区域的最大字符长度。 |
| `lvgl.pct(value)` | 函数 | 用于设置基于父对象尺寸的百分比值。例如 `lvgl.pct(50)` 代表50%。 |
| `lvgl.SCROLLBAR_MODE_ON()` | 常量 | 启用滚动条。 |
| `lvgl.ALIGN_OUT_BOTTOM_LEFT()` | 常量 | 在对象外部左下角对齐。 |
| `lvgl.DIR_TOP()` | 常量 | 表示顶部方向，用于Tabview等控件。 |
