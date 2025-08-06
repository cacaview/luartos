# LuaRTOS API 文档

## 概述

LuaRTOS 是一个基于 ESP32-S3 的实时操作系统，集成了 Lua 解释器和 LVGL 图形库。本文档描述了 Lua 解释器提供的 API 接口。

## 内存管理

LuaRTOS 的 Lua 解释器专门针对 PSRAM 进行了优化：

- **自动 PSRAM 分配**: 大于 256 字节的内存分配会优先使用 PSRAM
- **回退机制**: 如果 PSRAM 不足，会自动回退到内部 RAM
- **内存监控**: 提供详细的内存使用统计信息

### 内存配置参数

```c
#define LUA_PSRAM_MIN_SIZE 256      // 使用 PSRAM 的最小分配大小
#define LUA_LARGE_ALLOC_SIZE 1024   // 大内存分配阈值
```

## LVGL 图形库 API

### 核心对象操作

#### 屏幕管理

```lua
-- 获取当前活动屏幕
local screen = lvgl.scr_act()

-- 创建新屏幕对象
local new_screen = lvgl.obj_create(nil)

-- 加载屏幕（带动画）
lvgl.scr_load_anim(screen, animation_type, time, delay, auto_delete)
```

#### 对象创建和配置

```lua
-- 创建基础对象
local obj = lvgl.obj_create(parent)

-- 设置对象大小和位置
lvgl.obj_set_size(obj, width, height)
lvgl.obj_set_pos(obj, x, y)

-- 对象对齐
lvgl.obj_align(obj, alignment, x_offset, y_offset)
lvgl.obj_align_to(obj, base_obj, alignment, x_offset, y_offset)
lvgl.obj_center(obj)

-- 对象配置
lvgl.obj_set_scrollbar_mode(obj, mode)
lvgl.obj_set_width(obj, width)
```

### 样式系统

#### 背景样式

```lua
-- 背景颜色和透明度
lvgl.obj_set_style_bg_color(obj, color_hex, selector)
lvgl.obj_set_style_bg_opa(obj, opacity, selector)
lvgl.obj_set_style_bg_grad_dir(obj, gradient_direction, selector)

-- 边框样式
lvgl.obj_set_style_border_width(obj, width, selector)
lvgl.obj_set_style_border_color(obj, color_hex, selector)
lvgl.obj_set_style_border_opa(obj, opacity, selector)
lvgl.obj_set_style_border_side(obj, side, selector)

-- 圆角和阴影
lvgl.obj_set_style_radius(obj, radius, selector)
lvgl.obj_set_style_shadow_width(obj, width, selector)
```

#### 文本样式

```lua
-- 文本颜色和字体
lvgl.obj_set_style_text_color(obj, color_hex, selector)
lvgl.obj_set_style_text_font(obj, font, selector)
lvgl.obj_set_style_text_opa(obj, opacity, selector)

-- 文本对齐和装饰
lvgl.obj_set_style_text_align(obj, alignment, selector)
lvgl.obj_set_style_text_decor(obj, decoration, selector)
lvgl.obj_set_style_text_letter_space(obj, space, selector)
lvgl.obj_set_style_text_line_space(obj, space, selector)
```

#### 填充（Padding）

```lua
-- 设置所有方向的填充
lvgl.obj_set_style_pad_all(obj, padding, selector)

-- 分别设置各方向填充
lvgl.obj_set_style_pad_top(obj, padding, selector)
lvgl.obj_set_style_pad_bottom(obj, padding, selector)
lvgl.obj_set_style_pad_left(obj, padding, selector)
lvgl.obj_set_style_pad_right(obj, padding, selector)
```

### 控件（Widgets）

#### 标签（Label）

```lua
-- 创建标签
local label = lvgl.label_create(parent)

-- 设置文本
lvgl.label_set_text(label, "Hello World")

-- 设置长文本模式
lvgl.label_set_long_mode(label, lvgl.LABEL_LONG_WRAP())
```

#### 按钮（Button）

```lua
-- 创建按钮
local btn = lvgl.btn_create(parent)

-- 在按钮上创建标签
local btn_label = lvgl.label_create(btn)
lvgl.label_set_text(btn_label, "按钮")
lvgl.obj_align(btn_label, lvgl.ALIGN_CENTER(), 0, 0)
```

#### 滑块（Slider）

```lua
-- 创建滑块
local slider = lvgl.slider_create(parent)

-- 设置滑块值
lvgl.slider_set_value(slider, value, animation)
```

#### 开关（Switch）

```lua
-- 创建开关
local switch = lvgl.switch_create(parent)
```

#### 进度条（Bar）

```lua
-- 创建进度条
local bar = lvgl.bar_create(parent)

-- 设置范围和值
lvgl.bar_set_range(bar, min, max)
lvgl.bar_set_value(bar, value, animation)
lvgl.bar_set_mode(bar, mode)
```

#### 跨度组（Spangroup）

```lua
-- 创建跨度组（富文本容器）
local spangroup = lvgl.spangroup_create(parent)

-- 配置跨度组
lvgl.spangroup_set_align(spangroup, alignment)
lvgl.spangroup_set_overflow(spangroup, overflow_mode)
lvgl.spangroup_set_mode(spangroup, mode)

-- 创建和设置跨度
local span = lvgl.spangroup_new_span(spangroup)
lvgl.span_set_text(span, "文本内容")

-- 刷新跨度组
lvgl.spangroup_refr_mode(spangroup)
```

#### 消息框（Message Box）

```lua
-- 创建消息框
local msgbox = lvgl.msgbox_create(parent, "标题", "消息内容", "确定", false)
```

#### 列表（List）

```lua
-- 创建列表
local list = lvgl.list_create(parent)

-- 添加文本项
local text_item = lvgl.list_add_text(list, "文本项")

-- 添加按钮项
local btn_item = lvgl.list_add_btn(list, icon, "按钮项")
```

### 事件处理

```lua
-- 添加事件回调
lvgl.obj_add_event_cb(obj, function(event)
    -- 事件处理代码
    print("事件触发")
end)
```

### 常量定义

#### 对齐常量

```lua
lvgl.ALIGN_CENTER()
lvgl.ALIGN_TOP_LEFT()
lvgl.ALIGN_TOP_MID()
lvgl.ALIGN_TOP_RIGHT()
lvgl.ALIGN_BOTTOM_LEFT()
lvgl.ALIGN_BOTTOM_MID()
lvgl.ALIGN_BOTTOM_RIGHT()
lvgl.ALIGN_OUT_TOP_MID()
```

#### 部分和状态常量

```lua
-- 对象部分
lvgl.PART_MAIN()
lvgl.PART_INDICATOR()
lvgl.PART_KNOB()

-- 对象状态
lvgl.STATE_DEFAULT()
lvgl.STATE_CHECKED()
```

#### 动画常量

```lua
lvgl.ANIM_OFF()
lvgl.ANIM_ON()
lvgl.SCR_LOAD_ANIM_NONE()
```

#### 字体常量

```lua
lvgl.font_montserrat_14()
lvgl.font_montserrat_16()
lvgl.font_montserrat_20()
```

#### 其他常量

```lua
-- 文本对齐
lvgl.TEXT_ALIGN_LEFT()
lvgl.TEXT_ALIGN_CENTER()
lvgl.TEXT_ALIGN_RIGHT()

-- 滚动条模式
lvgl.SCROLLBAR_MODE_OFF()

-- 标签长文本模式
lvgl.LABEL_LONG_WRAP()

-- 跨度模式
lvgl.SPAN_OVERFLOW_CLIP()
lvgl.SPAN_MODE_BREAK()

-- 进度条模式
lvgl.BAR_MODE_NORMAL()

-- 渐变方向
lvgl.GRAD_DIR_NONE()

-- 文本装饰
lvgl.TEXT_DECOR_NONE()

-- 边框侧面
lvgl.BORDER_SIDE_FULL()
```

### 工具函数

```lua
-- 颜色函数
local color = lvgl.color_hex(0xFF0000)  -- 红色
local white = lvgl.color_white()
local black = lvgl.color_black()

-- 强制刷新显示
lvgl.refr_now()

-- 清空对象的所有子对象
lvgl.obj_clean(obj)

-- 标记对象需要重绘
lvgl.obj_invalidate(obj)
```

## 系统 API

### SD 卡操作

```lua
-- 初始化 SD 卡
local success, message = system.sd_init()

-- 检查 SD 卡是否已挂载
local mounted = system.sd_is_mounted()

-- 获取 SD 卡信息
local info, error = system.sd_get_info()
-- info.total_bytes, info.free_bytes, info.used_bytes

-- 写入文件
local success, message = system.sd_write_file("filename.txt", "文件内容")

-- 读取文件
local content, error = system.sd_read_file("filename.txt")
```

### WiFi 操作

```lua
-- 初始化 WiFi
local success, message = system.wifi_init()

-- 扫描网络
local networks = system.wifi_scan()
-- networks[i].ssid, networks[i].rssi, networks[i].authmode

-- 连接网络
local success, message = system.wifi_connect("SSID", "密码")

-- 断开连接
local success = system.wifi_disconnect()

-- 检查连接状态
local connected = system.wifi_is_connected()

-- 获取IP地址
local ip = system.wifi_get_ip()
```

### 系统功能

```lua
-- 延时（毫秒）
system.delay(1000)

-- 获取空闲堆内存
local free_heap = system.get_free_heap()

-- 获取 PSRAM 大小
local psram_size = system.get_psram_size()

-- 重启系统
system.restart()
```

### 定时器

```lua
-- 创建定时器（周期毫秒，是否自动重载，回调函数）
local timer = system.timer_create(1000, true, function()
    print("定时器触发")
end)

-- 启动定时器
system.timer_start(timer)

-- 停止定时器
system.timer_stop(timer)
```

## 使用示例

### 创建简单的 UI

```lua
-- 获取当前屏幕
local screen = lvgl.scr_act()

-- 设置背景色
lvgl.obj_set_style_bg_color(screen, 0x003a57, lvgl.PART_MAIN())

-- 创建标题
local title = lvgl.label_create(screen)
lvgl.label_set_text(title, "LuaRTOS")
lvgl.obj_set_style_text_color(title, lvgl.color_white(), lvgl.PART_MAIN())
lvgl.obj_set_style_text_font(title, lvgl.font_montserrat_20(), lvgl.PART_MAIN())
lvgl.obj_align(title, lvgl.ALIGN_TOP_MID(), 0, 20)

-- 创建按钮
local btn = lvgl.btn_create(screen)
lvgl.obj_set_size(btn, 120, 50)
lvgl.obj_align(btn, lvgl.ALIGN_CENTER(), 0, 0)

local btn_label = lvgl.label_create(btn)
lvgl.label_set_text(btn_label, "点击我")
lvgl.obj_center(btn_label)

-- 添加按钮事件
lvgl.obj_add_event_cb(btn, function(e)
    print("按钮被点击！")
end)

-- 刷新显示
lvgl.refr_now()
```

### 系统监控

```lua
-- 显示系统信息
local function show_system_info()
    local free_heap = system.get_free_heap()
    local psram_size = system.get_psram_size()
    
    print("空闲堆内存: " .. free_heap .. " 字节")
    print("PSRAM 大小: " .. psram_size .. " 字节")
    
    if system.sd_is_mounted() then
        local info = system.sd_get_info()
        if info then
            print("SD 卡总容量: " .. info.total_bytes .. " 字节")
            print("SD 卡可用空间: " .. info.free_bytes .. " 字节")
        end
    end
    
    if system.wifi_is_connected() then
        local ip = system.wifi_get_ip()
        print("WiFi IP 地址: " .. (ip or "未知"))
    end
end

-- 创建定时器定期显示系统信息
local info_timer = system.timer_create(5000, true, show_system_info)
system.timer_start(info_timer)
```

## 注意事项

1. **内存管理**: Lua 对象由垃圾回收器自动管理，但 LVGL 对象需要手动管理
2. **事件处理**: 事件回调函数在 LVGL 任务上下文中执行，避免长时间阻塞操作
3. **文件系统**: SD 卡必须先初始化才能进行文件操作
4. **WiFi 连接**: 某些操作可能需要时间，建议使用异步方式处理
5. **内存优化**: 大型数据结构会自动分配到 PSRAM，小对象使用内部 RAM

## 错误处理

大多数系统函数返回布尔值表示成功/失败状态，以及可选的错误消息：

```lua
local success, error_msg = system.sd_init()
if not success then
    print("SD 卡初始化失败: " .. (error_msg or "未知错误"))
end
```

## 调试技巧

1. 使用 `print()` 函数输出调试信息
2. 监控内存使用情况
3. 使用定时器进行周期性检查
4. 检查函数返回值和错误消息
