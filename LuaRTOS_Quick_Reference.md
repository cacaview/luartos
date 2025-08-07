# LuaRTOS 快速参考

## 常用 LVGL 函数

### 基础对象
```lua
local obj = lvgl.obj_create(parent)
lvgl.obj_set_pos(obj, x, y)
lvgl.obj_set_size(obj, w, h)
lvgl.obj_align(obj, lvgl.ALIGN_CENTER(), 0, 0)
lvgl.obj_center(obj)
lvgl.obj_del(obj)
```

### 控件创建
```lua
-- 标签
local label = lvgl.label_create(parent)
lvgl.label_set_text(label, "Text")

-- 按钮
local btn = lvgl.btn_create(parent)
local btn_label = lvgl.label_create(btn)
lvgl.obj_center(btn_label)

-- 进度条
local bar = lvgl.bar_create(parent)
lvgl.bar_set_value(bar, 50, lvgl.ANIM_OFF())

-- 开关
local switch = lvgl.switch_create(parent)

-- 列表
local list = lvgl.list_create(parent)
local item = lvgl.list_add_btn(list, lvgl.SYMBOL_WIFI(), "Item")

-- 消息框
local msgbox = lvgl.msgbox_create(parent, "Title", "Message", {"OK", "Cancel"})

-- 文本输入
local textarea = lvgl.textarea_create(parent)
local keyboard = lvgl.keyboard_create(parent)
```

### 事件处理
```lua
lvgl.obj_add_event_cb(obj, function(event)
    if lvgl.event_get_code(event) == lvgl.EVENT_CLICKED() then
        -- 处理点击事件
    end
end)
```

### 样式设置
```lua
-- 背景
lvgl.obj_set_style_bg_color(obj, 0xff0000, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
lvgl.obj_set_style_bg_opa(obj, 255, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())

-- 边框
lvgl.obj_set_style_border_width(obj, 2, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
lvgl.obj_set_style_border_color(obj, 0x2195f6, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())

-- 圆角和内边距
lvgl.obj_set_style_radius(obj, 10, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
lvgl.obj_set_style_pad_all(obj, 10, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())

-- 文本
lvgl.obj_set_style_text_color(obj, 0x000000, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
lvgl.obj_set_style_text_font(obj, lvgl.font_montserrat_16(), lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
```

## 常用系统函数

### WiFi
```lua
system.wifi_init()
system.wifi_scan()
system.wifi_connect("SSID", "password")
system.wifi_is_connected()
system.wifi_get_ip()
system.wifi_get_status()
system.wifi_disconnect()
```

### SD 卡
```lua
system.sd_init()
system.sd_is_mounted()
system.sd_get_info()
system.sd_format()
system.sd_check_status()
system.sd_write_file("file.txt", "content")
system.sd_read_file("file.txt")
```

### 系统控制
```lua
system.delay(1000)    -- 延时 1 秒
system.sleep(2000)    -- FreeRTOS 睡眠 2 秒
system.get_free_heap()
system.get_psram_size()
system.restart()
```

## 常用常量

### 对齐
```lua
lvgl.ALIGN_CENTER()
lvgl.ALIGN_TOP_MID()
lvgl.ALIGN_LEFT_MID()
lvgl.ALIGN_RIGHT_MID()
lvgl.ALIGN_BOTTOM_MID()
```

### 事件
```lua
lvgl.EVENT_CLICKED()
lvgl.EVENT_VALUE_CHANGED()
```

### 状态
```lua
lvgl.STATE_DEFAULT()
lvgl.STATE_CHECKED()
```

### 符号
```lua
lvgl.SYMBOL_WIFI()
lvgl.SYMBOL_OK()
lvgl.SYMBOL_CLOSE()
```

### 字体
```lua
lvgl.font_montserrat_12()
lvgl.font_montserrat_14()
lvgl.font_montserrat_16()
lvgl.font_montserrat_20()
```

### 标志
```lua
lvgl.OBJ_FLAG_HIDDEN()
```

## 快速模板

### 带回调的按钮
```lua
local function create_button(parent, text, x, y, w, h, callback)
    local btn = lvgl.btn_create(parent)
    lvgl.obj_set_pos(btn, x, y)
    lvgl.obj_set_size(btn, w, h)
    
    local label = lvgl.label_create(btn)
    lvgl.label_set_text(label, text)
    lvgl.obj_center(label)
    
    if callback then
        lvgl.obj_add_event_cb(btn, function(event)
            if lvgl.event_get_code(event) == lvgl.EVENT_CLICKED() then
                callback()
            end
        end)
    end
    
    return btn
end
```

### 进度对话框
```lua
local function show_progress_dialog(title, initial_value)
    local dialog = lvgl.obj_create(lvgl.scr_act())
    lvgl.obj_set_size(dialog, 300, 150)
    lvgl.obj_center(dialog)
    
    local label = lvgl.label_create(dialog)
    lvgl.label_set_text(label, title)
    lvgl.obj_align(label, lvgl.ALIGN_TOP_MID(), 0, 20)
    
    local bar = lvgl.bar_create(dialog)
    lvgl.obj_set_size(bar, 250, 20)
    lvgl.obj_align(bar, lvgl.ALIGN_CENTER(), 0, 0)
    lvgl.bar_set_value(bar, initial_value, lvgl.ANIM_OFF())
    
    return dialog, bar
end
```

### 简单消息框
```lua
local function show_message(title, message, callback)
    local msgbox = lvgl.msgbox_create(lvgl.scr_act(), title, message, {"OK"})
    lvgl.obj_set_size(msgbox, 250, 150)
    
    if callback then
        lvgl.obj_add_event_cb(lvgl.msgbox_get_btns(msgbox), function(event)
            if lvgl.event_get_code(event) == lvgl.EVENT_CLICKED() then
                lvgl.obj_del(msgbox)
                callback()
            end
        end)
    end
    
    return msgbox
end
```

## 调试技巧

### 内存监控
```lua
local function log_memory()
    print("Free heap:", system.get_free_heap())
    print("PSRAM size:", system.get_psram_size())
end
```

### 事件调试
```lua
local function debug_events(obj, name)
    lvgl.obj_add_event_cb(obj, function(event)
        print(name .. " event:", lvgl.event_get_code(event))
    end)
end
```

### 错误处理
```lua
local function safe_call(func, ...)
    local success, result = pcall(func, ...)
    if not success then
        print("Error:", result)
        return nil
    end
    return result
end
```

## 最佳实践

1. **内存管理**: 使用局部变量，及时删除不需要的对象
2. **事件处理**: 避免在回调中执行耗时操作
3. **屏幕刷新**: 批量更新后调用 `lvgl.refr_now()`
4. **错误处理**: 使用 `pcall` 包装可能失败的操作
5. **PSRAM 优化**: 大多数数据自动分配到 PSRAM，无需特殊处理

---

*参见完整 API 文档: LuaRTOS_API_Documentation.md*
