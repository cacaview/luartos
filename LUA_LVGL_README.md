# LuaRTOS - ESP32 LVGL + Lua 集成

这个项目为ESP32-S3实现了Lua对LVGL的完整绑定，允许使用Lua脚本来创建和控制LVGL图形界面。

## 功能特性

- **完整的LVGL Lua绑定**: 支持LVGL的核心功能，包括对象创建、样式设置、布局等
- **动态界面更新**: 可以在运行时通过更新Lua脚本来改变界面，无需重新编译固件
- **内存优化**: 针对ESP32-S3的PSRAM进行了优化
- **易于扩展**: 模块化设计，可以轻松添加更多LVGL功能绑定

## 项目结构

```
├── components/
│   └── lua/                    # Lua组件
│       ├── src/                # Lua 5.4.6 源码
│       ├── lua_engine.c/h      # Lua引擎包装器
│       ├── lvgl_bindings.c/h   # LVGL的Lua绑定
│       └── CMakeLists.txt      # Lua组件构建配置
├── main/
│   ├── main.c                  # 主程序（已修改为运行Lua脚本）
│   ├── main.lua                # Lua演示脚本（可选，用于文件系统）
│   └── main_lua.h              # 嵌入式Lua脚本（编译到固件中）
└── CMakeLists.txt              # 项目构建配置
```

## 支持的LVGL功能

### 核心对象操作
- `lvgl.scr_act()` - 获取当前屏幕
- `lvgl.obj_create(parent)` - 创建对象
- `lvgl.obj_set_size(obj, width, height)` - 设置对象大小
- `lvgl.obj_set_pos(obj, x, y)` - 设置对象位置
- `lvgl.obj_align(obj, align, x_ofs, y_ofs)` - 对齐对象
- `lvgl.obj_center(obj)` - 居中对象
- `lvgl.obj_clean(obj)` - 清空对象
- `lvgl.obj_invalidate(obj)` - 标记对象需要重绘

### 样式设置
- `lvgl.obj_set_style_bg_color(obj, color, selector)` - 设置背景颜色
- `lvgl.obj_set_style_text_color(obj, color, selector)` - 设置文本颜色
- `lvgl.obj_set_style_text_font(obj, font, selector)` - 设置字体
- `lvgl.obj_set_style_border_width(obj, width, selector)` - 设置边框宽度
- `lvgl.obj_set_style_border_color(obj, color, selector)` - 设置边框颜色

### 控件创建
- `lvgl.label_create(parent)` - 创建标签
- `lvgl.label_set_text(label, text)` - 设置标签文本
- `lvgl.btn_create(parent)` - 创建按钮
- `lvgl.slider_create(parent)` - 创建滑块
- `lvgl.slider_set_value(slider, value, anim)` - 设置滑块值
- `lvgl.switch_create(parent)` - 创建开关
- `lvgl.bar_create(parent)` - 创建进度条
- `lvgl.bar_set_value(bar, value, anim)` - 设置进度条值

### 实用函数
- `lvgl.color_hex(hex_value)` - 创建十六进制颜色
- `lvgl.color_white()` - 白色
- `lvgl.refr_now()` - 立即刷新显示

### 常量
- `lvgl.PART_MAIN()` - 主要部分选择器
- `lvgl.ALIGN_CENTER()` - 居中对齐
- `lvgl.ALIGN_TOP_MID()` - 顶部中间对齐
- `lvgl.ALIGN_BOTTOM_MID()` - 底部中间对齐
- `lvgl.ALIGN_OUT_TOP_MID()` - 外部顶部中间对齐
- `lvgl.ANIM_OFF()` - 关闭动画
- `lvgl.font_montserrat_14()` - Montserrat 14号字体

## Lua脚本示例

```lua
-- 获取当前屏幕
local scr = lvgl.scr_act()

-- 设置背景颜色
lvgl.obj_set_style_bg_color(scr, 0x003a57, lvgl.PART_MAIN())

-- 创建标题
local title = lvgl.label_create(scr)
lvgl.label_set_text(title, "Hello from Lua!")
lvgl.obj_set_style_text_color(title, lvgl.color_white(), lvgl.PART_MAIN())
lvgl.obj_align(title, lvgl.ALIGN_TOP_MID(), 0, 20)

-- 创建按钮
local btn = lvgl.btn_create(scr)
lvgl.obj_set_size(btn, 120, 50)
lvgl.obj_align(btn, lvgl.ALIGN_CENTER(), 0, 0)

local btn_label = lvgl.label_create(btn)
lvgl.label_set_text(btn_label, "Click Me")
lvgl.obj_center(btn_label)

-- 刷新显示
lvgl.refr_now()
```

## 编译和运行

1. 确保已安装ESP-IDF开发环境
2. 克隆项目并进入目录
3. 配置项目：`idf.py menuconfig`
4. 编译项目：`idf.py build`
5. 烧录到ESP32-S3：`idf.py flash monitor`

## 自定义Lua脚本

### 方法1：修改嵌入式脚本
编辑 `main/main_lua.h` 中的 `main_lua_script` 字符串，然后重新编译固件。

### 方法2：使用文件系统（推荐）
1. 配置SPIFFS或其他文件系统
2. 将 `main.lua` 文件上传到文件系统
3. 修改 `run_lua_demo()` 函数以从文件系统加载脚本

## 扩展功能

要添加更多LVGL功能的Lua绑定：

1. 在 `lvgl_bindings.h` 中声明新函数
2. 在 `lvgl_bindings.c` 中实现绑定函数
3. 将函数添加到 `lvgl_functions` 数组中
4. 重新编译项目

## 内存使用

- Lua虚拟机：约 50KB RAM
- LVGL绑定：约 20KB RAM
- 显示缓冲区：使用PSRAM（如果可用）

## 故障排除

1. **Lua脚本执行失败**：检查ESP32串口输出中的Lua错误信息
2. **显示问题**：确认LVGL驱动配置正确
3. **内存不足**：启用PSRAM或减少显示缓冲区大小

## 注意事项

- 当前实现为演示版本，某些高级LVGL功能可能需要额外的绑定
- 事件处理系统尚未完全实现
- 建议在生产环境中添加更完善的错误处理

## 许可证

本项目基于Lua和LVGL的许可证条款。具体请参考各自的许可证文件。
