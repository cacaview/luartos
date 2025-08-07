# LuaRTOS API 文档

## 概述

LuaRTOS 是一个为 ESP32-S3 开发板定制的 Lua 解释器，专门针对嵌入式系统和图形界面开发进行了优化。它集成了 LVGL 图形库和系统功能，为 OOBE（开箱即用体验）和应用程序开发提供了完整的 Lua 运行环境。

## 特性

- **PSRAM 优化**：优先使用 PSRAM 进行内存分配，为显存留出足够的内部 RAM 空间
- **LVGL 集成**：完整的 LVGL 图形库 Lua 绑定
- **系统功能**：WiFi、SD卡、系统控制等硬件接口
- **事件处理**：支持触摸和按钮事件的异步处理
- **实时刷新**：针对 GUI 应用优化的显示更新机制

## 内存管理

### PSRAM 优先策略

LuaRTOS 使用智能内存分配器，优先使用 PSRAM：

```lua
-- 大多数 Lua 对象和数据结构都会分配在 PSRAM 中
local large_table = {}
for i = 1, 10000 do
    large_table[i] = "This string will be allocated in PSRAM"
end

-- 系统会自动选择最佳的内存区域
local function get_memory_info()
    -- 获取内存使用统计
    local total, psram, internal = system.get_memory_stats()
    print("Total:", total, "PSRAM:", psram, "Internal:", internal)
end
```

### 内存统计

```lua
-- 获取系统内存信息
local free_heap = system.get_free_heap()
local psram_size = system.get_psram_size()

print("Free heap:", free_heap, "bytes")
print("PSRAM size:", psram_size, "bytes")
```

## LVGL 图形库

### 基础对象操作

```lua
-- 获取当前屏幕
local screen = lvgl.scr_act()

-- 创建对象
local obj = lvgl.obj_create(screen)
lvgl.obj_set_size(obj, 200, 100)
lvgl.obj_set_pos(obj, 50, 50)

-- 对象对齐
lvgl.obj_align(obj, lvgl.ALIGN_CENTER(), 0, 0)
lvgl.obj_center(obj)
```

### 控件创建

#### 标签 (Label)

```lua
local label = lvgl.label_create(parent)
lvgl.label_set_text(label, "Hello, World!")
lvgl.label_set_long_mode(label, lvgl.LABEL_LONG_WRAP())

-- 设置字体和颜色
lvgl.obj_set_style_text_font(label, lvgl.font_montserrat_16(), lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
lvgl.obj_set_style_text_color(label, 0x000000, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
```

#### 按钮 (Button)

```lua
local btn = lvgl.btn_create(parent)
lvgl.obj_set_size(btn, 100, 50)

local btn_label = lvgl.label_create(btn)
lvgl.label_set_text(btn_label, "Click Me")
lvgl.obj_center(btn_label)

-- 按钮事件处理
lvgl.obj_add_event_cb(btn, function(event)
    if lvgl.event_get_code(event) == lvgl.EVENT_CLICKED() then
        print("Button clicked!")
    end
end)
```

#### 进度条 (Bar)

```lua
local bar = lvgl.bar_create(parent)
lvgl.obj_set_size(bar, 200, 20)
lvgl.bar_set_mode(bar, lvgl.BAR_MODE_NORMAL())
lvgl.bar_set_range(bar, 0, 100)
lvgl.bar_set_value(bar, 50, lvgl.ANIM_OFF())
```

#### 开关 (Switch)

```lua
local switch = lvgl.switch_create(parent)
lvgl.obj_set_size(switch, 50, 25)

-- 开关状态检查
if lvgl.obj_has_state(switch, lvgl.STATE_CHECKED()) then
    print("Switch is ON")
end
```

#### 列表 (List)

```lua
local list = lvgl.list_create(parent)
lvgl.obj_set_size(list, 200, 300)

-- 添加列表项
local item1 = lvgl.list_add_text(list, "Text Item")
local item2 = lvgl.list_add_btn(list, lvgl.SYMBOL_WIFI(), "WiFi Network")

-- 列表项事件
lvgl.obj_add_event_cb(item2, function(event)
    if lvgl.event_get_code(event) == lvgl.EVENT_CLICKED() then
        print("WiFi item clicked")
    end
end)
```

#### 消息框 (Message Box)

```lua
local msgbox = lvgl.msgbox_create(parent, "Title", "Message text", {"Yes", "No"})
lvgl.obj_set_size(msgbox, 300, 200)

-- 处理按钮点击
lvgl.obj_add_event_cb(lvgl.msgbox_get_btns(msgbox), function(event)
    if lvgl.event_get_code(event) == lvgl.EVENT_CLICKED() then
        local btn_id = lvgl.event_get_target(event)
        if btn_id == 0 then
            print("Yes clicked")
        else
            print("No clicked")
        end
        lvgl.obj_del(msgbox)
    end
end)
```

#### 文本区域和键盘 (Textarea & Keyboard)

```lua
local textarea = lvgl.textarea_create(parent)
lvgl.obj_set_size(textarea, 300, 40)
lvgl.textarea_set_text(textarea, "")

local keyboard = lvgl.keyboard_create(parent)
lvgl.obj_set_size(keyboard, 300, 120)

-- 获取输入的文本
local text = lvgl.textarea_get_text(textarea)
print("Input text:", text)
```

### 样式设置

#### 背景样式

```lua
-- 背景颜色和透明度
lvgl.obj_set_style_bg_color(obj, 0xff0000, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
lvgl.obj_set_style_bg_opa(obj, 255, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
lvgl.obj_set_style_bg_grad_dir(obj, lvgl.GRAD_DIR_NONE(), lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
```

#### 边框样式

```lua
-- 边框设置
lvgl.obj_set_style_border_width(obj, 2, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
lvgl.obj_set_style_border_color(obj, 0x2195f6, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
lvgl.obj_set_style_border_opa(obj, 255, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
lvgl.obj_set_style_border_side(obj, lvgl.BORDER_SIDE_FULL(), lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
```

#### 内边距和圆角

```lua
-- 内边距
lvgl.obj_set_style_pad_all(obj, 10, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
lvgl.obj_set_style_pad_top(obj, 5, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())

-- 圆角
lvgl.obj_set_style_radius(obj, 10, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
```

#### 文本样式

```lua
-- 文本颜色和字体
lvgl.obj_set_style_text_color(obj, 0x000000, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
lvgl.obj_set_style_text_font(obj, lvgl.font_montserrat_16(), lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
lvgl.obj_set_style_text_align(obj, lvgl.TEXT_ALIGN_CENTER(), lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
```

### 常量

#### 对齐常量

```lua
lvgl.ALIGN_CENTER()
lvgl.ALIGN_TOP_MID()
lvgl.ALIGN_LEFT_MID()
lvgl.ALIGN_RIGHT_MID()
lvgl.ALIGN_BOTTOM_MID()
```

#### 状态常量

```lua
lvgl.STATE_DEFAULT()
lvgl.STATE_CHECKED()
```

#### 事件常量

```lua
lvgl.EVENT_CLICKED()
lvgl.EVENT_VALUE_CHANGED()
```

#### 符号常量

```lua
lvgl.SYMBOL_WIFI()
lvgl.SYMBOL_OK()
lvgl.SYMBOL_CLOSE()
```

#### 字体常量

```lua
lvgl.font_montserrat_12()
lvgl.font_montserrat_14()
lvgl.font_montserrat_16()
lvgl.font_montserrat_20()
```

### 对象标志

```lua
-- 隐藏/显示对象
lvgl.obj_add_flag(obj, lvgl.OBJ_FLAG_HIDDEN())
lvgl.obj_clear_flag(obj, lvgl.OBJ_FLAG_HIDDEN())

-- 删除对象
lvgl.obj_del(obj)
```

### 屏幕刷新

```lua
-- 强制刷新屏幕
lvgl.refr_now()

-- 使对象无效，触发重绘
lvgl.obj_invalidate(obj)
```

## 系统功能

### WiFi 管理

```lua
-- 初始化 WiFi
local success, message = system.wifi_init()
if success then
    print("WiFi initialized:", message)
end

-- 扫描 WiFi 网络
local networks = system.wifi_scan()
if networks then
    for i, network in ipairs(networks) do
        print("SSID:", network.ssid, "RSSI:", network.rssi)
    end
end

-- 连接 WiFi
local connected = system.wifi_connect("SSID", "password")
if connected then
    print("Connected to WiFi")
    local ip = system.wifi_get_ip()
    print("IP address:", ip)
end

-- 检查连接状态
if system.wifi_is_connected() then
    print("WiFi is connected")
end

-- 断开连接
system.wifi_disconnect()

-- 获取 WiFi 状态
local status = system.wifi_get_status()
print("WiFi status:", status) -- "connected", "connecting", "disconnected", "not_initialized"
```

### SD 卡管理

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

-- 格式化 SD 卡（模拟）
local formatted, message = system.sd_format()
if formatted then
    print("SD card formatted:", message)
end

-- 检查 SD 卡状态
local status = system.sd_check_status()
print("SD mounted:", status.mounted, "Status:", status.status)

-- 读写文件
local write_success = system.sd_write_file("test.txt", "Hello, World!")
if write_success then
    local content = system.sd_read_file("test.txt")
    print("File content:", content)
end
```

### 系统控制

```lua
-- 延时函数
system.delay(1000) -- 延时 1 秒

-- FreeRTOS 睡眠（不阻塞其他任务）
system.sleep(2000) -- 睡眠 2 秒

-- 获取系统信息
local free_heap = system.get_free_heap()
local psram_size = system.get_psram_size()
print("Free heap:", free_heap, "PSRAM size:", psram_size)

-- 重启系统
system.restart()
```

### 定时器

```lua
-- 创建定时器
local timer_success, timer_handle = system.timer_create(function()
    print("Timer callback executed")
end, 1000) -- 1秒后执行

if timer_success then
    print("Timer created with handle:", timer_handle)
end

-- 启动定时器
system.timer_start(timer_handle)

-- 停止定时器
system.timer_stop(timer_handle)
```

## 事件处理

### 事件回调模式

```lua
-- 基础事件处理
lvgl.obj_add_event_cb(obj, function(event)
    local code = lvgl.event_get_code(event)
    local target = lvgl.event_get_target(event)
    
    if code == lvgl.EVENT_CLICKED() then
        print("Object clicked")
    elseif code == lvgl.EVENT_VALUE_CHANGED() then
        print("Value changed")
    end
end)
```

### 简化的按钮创建助手

```lua
-- 创建带回调的按钮（推荐模式）
local function create_button_with_callback(parent, text, x, y, w, h, callback)
    local btn = lvgl.btn_create(parent)
    lvgl.obj_set_pos(btn, x, y)
    lvgl.obj_set_size(btn, w, h)
    
    local btn_label = lvgl.label_create(btn)
    lvgl.label_set_text(btn_label, text)
    lvgl.obj_center(btn_label)
    
    if callback then
        lvgl.obj_add_event_cb(btn, function(event)
            if lvgl.event_get_code(event) == lvgl.EVENT_CLICKED() then
                callback()
            end
        end)
    end
    
    return btn, btn_label
end

-- 使用示例
local my_button = create_button_with_callback(screen, "Test", 100, 100, 120, 50, function()
    print("My button was clicked!")
end)
```

## 最佳实践

### 1. 内存使用优化

```lua
-- 优先使用局部变量
local function create_ui()
    local screen = lvgl.scr_act()
    local container = lvgl.obj_create(screen)
    -- ... 其他 UI 元素
    return container
end

-- 及时清理不需要的对象
if old_dialog then
    lvgl.obj_del(old_dialog)
    old_dialog = nil
end
```

### 2. 事件处理

```lua
-- 避免在事件回调中执行耗时操作
lvgl.obj_add_event_cb(btn, function(event)
    if lvgl.event_get_code(event) == lvgl.EVENT_CLICKED() then
        -- 好的做法：设置标志，在主循环中处理
        app_state.button_clicked = true
        
        -- 避免：直接执行耗时操作
        -- system.sleep(5000) -- 不要这样做
    end
end)
```

### 3. 屏幕刷新

```lua
-- 批量更新后手动刷新
lvgl.label_set_text(label1, "New text 1")
lvgl.label_set_text(label2, "New text 2")
lvgl.bar_set_value(progress, 75, lvgl.ANIM_OFF())
lvgl.refr_now() -- 一次性刷新所有变化
```

### 4. 错误处理

```lua
-- 处理可能失败的操作
local success, result = pcall(system.wifi_connect, "SSID", "password")
if not success then
    print("WiFi connection failed:", result)
    -- 显示错误信息给用户
end
```

## 调试技巧

### 1. 内存监控

```lua
-- 定期检查内存使用
local function log_memory_usage(stage)
    local free = system.get_free_heap()
    local psram = system.get_psram_size()
    print(string.format("[%s] Free: %d, PSRAM: %d", stage, free, psram))
end

log_memory_usage("Before UI creation")
create_ui()
log_memory_usage("After UI creation")
```

### 2. 事件调试

```lua
-- 通用事件调试器
local function debug_event_callback(event)
    local code = lvgl.event_get_code(event)
    local target = lvgl.event_get_target(event)
    print("Event:", code, "Target:", target)
end

-- 为调试添加到任何对象
lvgl.obj_add_event_cb(obj, debug_event_callback)
```

### 3. 性能监控

```lua
-- 简单的性能测量
local function measure_time(func, name)
    local start_time = system.get_time() -- 如果有时间函数
    func()
    local end_time = system.get_time()
    print(name .. " took " .. (end_time - start_time) .. " ms")
end

measure_time(function()
    create_complex_ui()
end, "UI Creation")
```

## 示例项目

### 简单的 Hello World

```lua
print("Hello from LuaRTOS!")

local screen = lvgl.scr_act()
lvgl.obj_set_style_bg_color(screen, 0x003a57, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())

local label = lvgl.label_create(screen)
lvgl.label_set_text(label, "Hello, LuaRTOS!")
lvgl.obj_set_style_text_color(label, 0xffffff, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
lvgl.obj_align(label, lvgl.ALIGN_CENTER(), 0, 0)

lvgl.refr_now()
```

### 交互式计数器

```lua
local screen = lvgl.scr_act()
local counter = 0

local label = lvgl.label_create(screen)
lvgl.label_set_text(label, "Count: 0")
lvgl.obj_align(label, lvgl.ALIGN_CENTER(), 0, -50)

local btn_up = lvgl.btn_create(screen)
lvgl.obj_set_size(btn_up, 100, 50)
lvgl.obj_align(btn_up, lvgl.ALIGN_CENTER(), -60, 0)

local btn_up_label = lvgl.label_create(btn_up)
lvgl.label_set_text(btn_up_label, "+")
lvgl.obj_center(btn_up_label)

local btn_down = lvgl.btn_create(screen)
lvgl.obj_set_size(btn_down, 100, 50)
lvgl.obj_align(btn_down, lvgl.ALIGN_CENTER(), 60, 0)

local btn_down_label = lvgl.label_create(btn_down)
lvgl.label_set_text(btn_down_label, "-")
lvgl.obj_center(btn_down_label)

-- 更新计数器显示
local function update_counter()
    lvgl.label_set_text(label, "Count: " .. counter)
end

-- 按钮事件
lvgl.obj_add_event_cb(btn_up, function(event)
    if lvgl.event_get_code(event) == lvgl.EVENT_CLICKED() then
        counter = counter + 1
        update_counter()
    end
end)

lvgl.obj_add_event_cb(btn_down, function(event)
    if lvgl.event_get_code(event) == lvgl.EVENT_CLICKED() then
        counter = counter - 1
        update_counter()
    end
end)

lvgl.refr_now()
```

## 版本信息

- **LuaRTOS 版本**: 1.0.0
- **Lua 版本**: 5.4.x
- **LVGL 版本**: 8.x
- **ESP-IDF 版本**: 5.x
- **目标平台**: ESP32-S3

## 更新日志

### v1.0.0 (2025-01-07)

- 初始版本发布
- 完整的 LVGL Lua 绑定
- PSRAM 优化的内存分配器
- 系统功能集成（WiFi、SD卡、系统控制）
- OOBE 系统实现
- 完整的 API 文档

## 联系和支持

如有问题或建议，请通过以下方式联系：

- GitHub Issues: [luartos/issues](https://github.com/cacaview/luartos/issues)
- 文档更新: 请提交 Pull Request

---

*本文档描述了 LuaRTOS API 的完整功能。建议结合示例代码和 OOBE 实现来学习最佳实践。*
