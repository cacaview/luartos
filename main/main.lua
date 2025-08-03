-- main.lua - LVGL 演示程序
-- 这个文件展示了如何使用Lua调用LVGL的各种功能

print("Starting Lua LVGL demo...")

-- 获取当前屏幕
local scr = lvgl.scr_act()

-- 设置背景颜色为深蓝色
lvgl.obj_set_style_bg_color(scr, 0x003a57, lvgl.PART_MAIN())
print("Set background color to dark blue")

-- 强制刷新屏幕
lvgl.obj_invalidate(scr)
lvgl.refr_now()
print("Force refresh screen")

-- 创建标题标签
print("Creating title label...")
local title = lvgl.label_create(scr)
if title then
    lvgl.label_set_text(title, "ESP32-S3 + Lua + LVGL")
    lvgl.obj_set_style_text_color(title, lvgl.color_white(), lvgl.PART_MAIN())
    lvgl.obj_set_style_text_font(title, lvgl.font_montserrat_14(), lvgl.PART_MAIN())
    lvgl.obj_align(title, lvgl.ALIGN_TOP_MID(), 0, 10)
    print("Title label created successfully")
else
    print("ERROR: Title label creation failed")
end

-- 创建可点击按钮
print("Creating button...")
local btn = lvgl.btn_create(scr)
if btn then
    lvgl.obj_set_size(btn, 120, 50)
    lvgl.obj_align(btn, lvgl.ALIGN_CENTER(), -80, -50)
    
    local btn_label = lvgl.label_create(btn)
    if btn_label then
        lvgl.label_set_text(btn_label, "Lua Button")
        lvgl.obj_center(btn_label)
        print("Button created successfully")
    else
        print("ERROR: Button label creation failed")
    end
else
    print("ERROR: Button creation failed")
end

-- 创建滑块
print("Creating slider...")
local slider = lvgl.slider_create(scr)
if slider then
    lvgl.obj_set_size(slider, 200, 20)
    lvgl.obj_align(slider, lvgl.ALIGN_CENTER(), 0, 0)
    lvgl.slider_set_value(slider, 50, lvgl.ANIM_OFF())
    
    local slider_label = lvgl.label_create(scr)
    if slider_label then
        lvgl.label_set_text(slider_label, "Lua Slider: 50%")
        lvgl.obj_align_to(slider_label, slider, lvgl.ALIGN_OUT_TOP_MID(), 0, -10)
        print("Slider created successfully")
    else
        print("ERROR: Slider label creation failed")
    end
else
    print("ERROR: Slider creation failed")
end

-- 创建开关
print("Creating switch...")
local sw = lvgl.switch_create(scr)
if sw then
    lvgl.obj_align(sw, lvgl.ALIGN_CENTER(), 80, -50)
    
    local sw_label = lvgl.label_create(scr)
    if sw_label then
        lvgl.label_set_text(sw_label, "Lua Switch")
        lvgl.obj_align_to(sw_label, sw, lvgl.ALIGN_OUT_TOP_MID(), 0, -10)
        print("Switch created successfully")
    else
        print("ERROR: Switch label creation failed")
    end
else
    print("ERROR: Switch creation failed")
end

-- 创建进度条
print("Creating progress bar...")
local bar = lvgl.bar_create(scr)
if bar then
    lvgl.obj_set_size(bar, 200, 20)
    lvgl.obj_align(bar, lvgl.ALIGN_CENTER(), 0, 50)
    lvgl.bar_set_value(bar, 70, lvgl.ANIM_OFF())
    
    local bar_label = lvgl.label_create(scr)
    if bar_label then
        lvgl.label_set_text(bar_label, "Lua Progress: 70%")
        lvgl.obj_align_to(bar_label, bar, lvgl.ALIGN_OUT_TOP_MID(), 0, -10)
        print("Progress bar created successfully")
    else
        print("ERROR: Progress bar label creation failed")
    end
else
    print("ERROR: Progress bar creation failed")
end

-- 创建信息面板
print("Creating info panel...")
local info_panel = lvgl.obj_create(scr)
if info_panel then
    lvgl.obj_set_size(info_panel, 400, 60)
    lvgl.obj_align(info_panel, lvgl.ALIGN_BOTTOM_MID(), 0, -20)
    lvgl.obj_set_style_bg_color(info_panel, 0x2e7d32, lvgl.PART_MAIN())
    lvgl.obj_set_style_border_width(info_panel, 2, lvgl.PART_MAIN())
    lvgl.obj_set_style_border_color(info_panel, lvgl.color_white(), lvgl.PART_MAIN())
    
    local info_text = lvgl.label_create(info_panel)
    if info_text then
        lvgl.label_set_text(info_text, "Status: Lua Runtime Active\\nResolution: 480x320 (Landscape)\\nFramework: LVGL + Lua")
        lvgl.obj_set_style_text_color(info_text, lvgl.color_white(), lvgl.PART_MAIN())
        lvgl.obj_center(info_text)
        print("Info panel created successfully")
    else
        print("ERROR: Info panel text creation failed")
    end
else
    print("ERROR: Info panel creation failed")
end

print("All Lua UI elements created successfully")

-- 最终刷新
lvgl.obj_invalidate(scr)
lvgl.refr_now()
print("Final refresh completed")

print("Lua LVGL demo initialization completed!")

-- 定义一个可以被C代码调用的更新函数
function update_ui()
    print("UI update called from Lua")
    -- 这里可以添加动态更新UI的代码
end

-- 定义一个清理函数
function cleanup_ui()
    print("Cleaning up Lua UI...")
    lvgl.obj_clean(scr)
    print("Lua UI cleaned up")
end
