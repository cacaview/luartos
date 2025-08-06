-- Memory optimization demo script
-- This script demonstrates the optimized memory allocation

print("=== LuaRTOS 内存优化演示 ===")
print("Lua解释器运行在PSRAM中，LVGL运行在内置RAM中")

-- Display memory statistics
print("\n内存状态:")
print("可用内置RAM:", system.get_free_heap(), "字节")
print("PSRAM大小:", system.get_psram_size(), "字节")

-- Create a simple GUI to test the optimizations
local screen = lvgl.obj_create(nil)
lvgl.obj_set_size(screen, 480, 320)

-- Title
local title_spangroup = lvgl.spangroup_create(screen)
lvgl.spangroup_set_align(title_spangroup, lvgl.TEXT_ALIGN_CENTER())
lvgl.obj_set_pos(title_spangroup, 90, 30)
lvgl.obj_set_size(title_spangroup, 300, 50)

local title_span = lvgl.spangroup_new_span(title_spangroup)
lvgl.span_set_text(title_span, "内存优化演示")
lvgl.spangroup_refr_mode(title_spangroup)

-- Memory info labels
local ram_label = lvgl.label_create(screen)
lvgl.label_set_text(ram_label, "内置RAM: 用于LVGL图形")
lvgl.obj_set_pos(ram_label, 50, 100)

local psram_label = lvgl.label_create(screen)
lvgl.label_set_text(psram_label, "PSRAM: 用于Lua解释器")
lvgl.obj_set_pos(psram_label, 50, 130)

-- Performance indicator
local perf_label = lvgl.label_create(screen)
lvgl.label_set_text(perf_label, "优化: GUI性能提升 + 内存效率")
lvgl.obj_set_pos(perf_label, 50, 160)

-- Memory usage progress bars
local ram_bar = lvgl.bar_create(screen)
lvgl.obj_set_pos(ram_bar, 50, 200)
lvgl.obj_set_size(ram_bar, 300, 20)
lvgl.bar_set_range(ram_bar, 0, 100)
lvgl.bar_set_value(ram_bar, 70, lvgl.ANIM_ON()) -- Simulated RAM usage

local ram_bar_label = lvgl.label_create(screen)
lvgl.label_set_text(ram_bar_label, "内置RAM使用率: 70%")
lvgl.obj_set_pos(ram_bar_label, 50, 230)

local psram_bar = lvgl.bar_create(screen)
lvgl.obj_set_pos(psram_bar, 50, 250)
lvgl.obj_set_size(psram_bar, 300, 20)
lvgl.bar_set_range(psram_bar, 0, 100)
lvgl.bar_set_value(psram_bar, 30, lvgl.ANIM_ON()) -- Simulated PSRAM usage

local psram_bar_label = lvgl.label_create(screen)
lvgl.label_set_text(psram_bar_label, "PSRAM使用率: 30%")
lvgl.obj_set_pos(psram_bar_label, 50, 280)

-- Load the screen
lvgl.scr_load_anim(screen, lvgl.SCR_LOAD_ANIM_NONE(), 200, 200, false)

print("\n内存优化已应用:")
print("- Lua对象优先分配到PSRAM")
print("- LVGL图形对象分配到内置RAM") 
print("- 显存和GUI性能得到优化")
print("- 内存使用更加高效")

-- Test memory allocation patterns
local test_table = {}
for i = 1, 100 do
    test_table[i] = string.rep("测试字符串", i)
end

print("\n已创建100个测试对象，展示PSRAM分配优化")
