-- oobe_lua.lua - Out-of-Box Experience in Lua
-- This replaces the C implementation with a pure Lua OOBE flow

print("Starting LuaRTOS OOBE...")

-- Global variables
local current_screen = nil
local ui = {}
local oobe_state = {
    screen_index = 0,
    sd_available = false,
    wifi_connected = false,
    installation_progress = 0
}

-- Helper function to create styled container
local function create_container(parent)
    local cont = lvgl.obj_create(parent)
    lvgl.obj_set_size(cont, 480, 320)
    lvgl.obj_set_pos(cont, 0, 0)
    lvgl.obj_set_scrollbar_mode(cont, lvgl.SCROLLBAR_MODE_OFF())
    
    -- Container styling
    lvgl.obj_set_style_border_width(cont, 2, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_border_opa(cont, 255, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_border_color(cont, 0x2195f6, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_border_side(cont, lvgl.BORDER_SIDE_FULL(), lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_radius(cont, 0, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_bg_opa(cont, 255, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_bg_color(cont, 0xffffff, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_bg_grad_dir(cont, lvgl.GRAD_DIR_NONE(), lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_pad_all(cont, 0, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_shadow_width(cont, 0, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    
    return cont
end

-- Helper function to create styled title
local function create_title(parent, text, x, y)
    local spangroup = lvgl.spangroup_create(parent)
    lvgl.spangroup_set_align(spangroup, lvgl.TEXT_ALIGN_LEFT())
    lvgl.spangroup_set_overflow(spangroup, lvgl.SPAN_OVERFLOW_CLIP())
    lvgl.spangroup_set_mode(spangroup, lvgl.SPAN_MODE_BREAK())
    
    local span = lvgl.spangroup_new_span(spangroup)
    lvgl.span_set_text(span, text)
    -- Note: Style functions for spans are not yet implemented in bindings
    
    lvgl.obj_set_pos(spangroup, x or 190, y or 30)
    lvgl.obj_set_size(spangroup, 200, 50)
    
    -- Style the spangroup container
    lvgl.obj_set_style_border_width(spangroup, 0, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_radius(spangroup, 0, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_bg_opa(spangroup, 0, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_pad_all(spangroup, 0, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_shadow_width(spangroup, 0, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    
    lvgl.spangroup_refr_mode(spangroup)
    return spangroup
end

-- Helper function to create styled button
local function create_button(parent, text, x, y, width, height, callback)
    local btn = lvgl.btn_create(parent)
    lvgl.obj_set_pos(btn, x, y)
    lvgl.obj_set_size(btn, width or 100, height or 50)
    
    local btn_label = lvgl.label_create(btn)
    lvgl.label_set_text(btn_label, text)
    lvgl.label_set_long_mode(btn_label, lvgl.LABEL_LONG_WRAP())
    lvgl.obj_align(btn_label, lvgl.ALIGN_CENTER(), 0, 0)
    lvgl.obj_set_style_pad_all(btn, 0, lvgl.STATE_DEFAULT())
    lvgl.obj_set_width(btn_label, 100) -- Use percentage equivalent
    
    -- Button styling
    lvgl.obj_set_style_bg_opa(btn, 255, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_bg_color(btn, 0x2195f6, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_bg_grad_dir(btn, lvgl.GRAD_DIR_NONE(), lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_border_width(btn, 0, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_radius(btn, 5, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_shadow_width(btn, 0, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_text_color(btn, 0xffffff, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_text_font(btn, lvgl.font_montserrat_16(), lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_text_opa(btn, 255, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_text_align(btn, lvgl.TEXT_ALIGN_CENTER(), lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    
    if callback then
        lvgl.obj_add_event_cb(btn, function(e)
            callback()
        end)
    end
    
    return btn
end

-- Helper function to create styled label
local function create_label(parent, text, x, y, width, height)
    local label = lvgl.label_create(parent)
    lvgl.label_set_text(label, text)
    lvgl.label_set_long_mode(label, lvgl.LABEL_LONG_WRAP())
    lvgl.obj_set_pos(label, x, y)
    lvgl.obj_set_size(label, width or 200, height or 36)
    
    -- Label styling
    lvgl.obj_set_style_border_width(label, 0, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_radius(label, 0, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_text_color(label, 0x000000, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_text_font(label, lvgl.font_montserrat_16(), lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_text_opa(label, 255, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_text_letter_space(label, 0, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_text_line_space(label, 0, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_text_align(label, lvgl.TEXT_ALIGN_LEFT(), lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_bg_opa(label, 0, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_pad_all(label, 0, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_shadow_width(label, 0, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    
    return label
end

-- Screen 0: System Initialization
local function setup_screen_0()
    print("Setting up screen 0: System Initialization")
    
    local screen = lvgl.obj_create(nil)
    lvgl.obj_set_size(screen, 480, 320)
    lvgl.obj_set_scrollbar_mode(screen, lvgl.SCROLLBAR_MODE_OFF())
    lvgl.obj_set_style_bg_opa(screen, 0, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    
    local cont = create_container(screen)
    local title = create_title(cont, "系统初始化")
    
    local start_btn = create_button(cont, "开始", 190, 135, 100, 50, function()
        print("Start button clicked, moving to screen 1")
        load_screen(1)
    end)
    
    ui.screen_0 = {
        screen = screen,
        container = cont,
        title = title,
        start_btn = start_btn
    }
    
    return screen
end

-- Screen 1: SD Card Check
local function setup_screen_1()
    print("Setting up screen 1: SD Card Check")
    
    local screen = lvgl.obj_create(nil)
    lvgl.obj_set_size(screen, 480, 320)
    lvgl.obj_set_scrollbar_mode(screen, lvgl.SCROLLBAR_MODE_OFF())
    lvgl.obj_set_style_bg_opa(screen, 0, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    
    local cont = create_container(screen)
    local title = create_title(cont, "检查SD卡")
    
    local status_label = create_label(cont, "正在检查SD卡...", 132, 127, 178, 36)
    
    local next_btn = create_button(cont, "下一步", 190, 200, 100, 50, function()
        print("Next button clicked, moving to screen 2")
        load_screen(2)
    end)
    
    ui.screen_1 = {
        screen = screen,
        container = cont,
        title = title,
        status_label = status_label,
        next_btn = next_btn
    }
    
    -- Check SD card asynchronously
    system.timer_create(1000, false, function()
        local success, msg = system.sd_init()
        if success then
            oobe_state.sd_available = true
            lvgl.label_set_text(status_label, "SD卡状态：可用")
        else
            lvgl.label_set_text(status_label, "SD卡状态：不可用 - " .. (msg or "未知错误"))
        end
    end)
    
    return screen
end

-- Screen 2: WiFi Setup
local function setup_screen_2()
    print("Setting up screen 2: WiFi Setup")
    
    local screen = lvgl.obj_create(nil)
    lvgl.obj_set_size(screen, 480, 320)
    lvgl.obj_set_scrollbar_mode(screen, lvgl.SCROLLBAR_MODE_OFF())
    lvgl.obj_set_style_bg_opa(screen, 0, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    
    local cont = create_container(screen)
    local title = create_title(cont, "连接至网络")
    
    local wifi_switch = lvgl.switch_create(cont)
    lvgl.obj_set_pos(wifi_switch, 180, 99)
    lvgl.obj_set_size(wifi_switch, 40, 20)
    
    -- WiFi switch styling
    lvgl.obj_set_style_bg_opa(wifi_switch, 255, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_bg_color(wifi_switch, 0xe6e2e6, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_bg_grad_dir(wifi_switch, lvgl.GRAD_DIR_NONE(), lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_border_width(wifi_switch, 0, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_radius(wifi_switch, 10, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_shadow_width(wifi_switch, 0, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    
    -- Switch indicator styling when checked
    lvgl.obj_set_style_bg_opa(wifi_switch, 255, lvgl.PART_INDICATOR() | lvgl.STATE_CHECKED())
    lvgl.obj_set_style_bg_color(wifi_switch, 0x2195f6, lvgl.PART_INDICATOR() | lvgl.STATE_CHECKED())
    lvgl.obj_set_style_bg_grad_dir(wifi_switch, lvgl.GRAD_DIR_NONE(), lvgl.PART_INDICATOR() | lvgl.STATE_CHECKED())
    lvgl.obj_set_style_border_width(wifi_switch, 0, lvgl.PART_INDICATOR() | lvgl.STATE_CHECKED())
    
    -- Switch knob styling
    lvgl.obj_set_style_bg_opa(wifi_switch, 255, lvgl.PART_KNOB() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_bg_color(wifi_switch, 0xffffff, lvgl.PART_KNOB() | lvgl.STATE_DEFAULT())
    
    local wifi_list = lvgl.list_create(cont)
    lvgl.obj_set_pos(wifi_list, 50, 140)
    lvgl.obj_set_size(wifi_list, 380, 100)
    
    local status_label = create_label(cont, "WiFi已禁用", 250, 99, 150, 20)
    
    local next_btn = create_button(cont, "下一步", 190, 260, 100, 50, function()
        print("Next button clicked, moving to screen 3")
        load_screen(3)
    end)
    
    ui.screen_2 = {
        screen = screen,
        container = cont,
        title = title,
        wifi_switch = wifi_switch,
        wifi_list = wifi_list,
        status_label = status_label,
        next_btn = next_btn
    }
    
    -- WiFi switch event handler
    lvgl.obj_add_event_cb(wifi_switch, function(e)
        -- Initialize WiFi when switch is turned on
        local success, msg = system.wifi_init()
        if success then
            lvgl.label_set_text(status_label, "正在扫描网络...")
            
            -- Scan for networks
            local networks = system.wifi_scan()
            if networks and #networks > 0 then
                -- Clear existing items
                lvgl.obj_clean(wifi_list)
                
                -- Add networks to list
                for i, network in ipairs(networks) do
                    if i <= 5 then -- Show only first 5 networks
                        local btn = lvgl.list_add_btn(wifi_list, nil, network.ssid)
                        lvgl.obj_add_event_cb(btn, function()
                            -- Only connect to open networks for demo
                            if network.authmode == 0 then -- Open network
                                local conn_success, conn_msg = system.wifi_connect(network.ssid, "")
                                if conn_success then
                                    oobe_state.wifi_connected = true
                                    lvgl.label_set_text(status_label, "已连接到 " .. network.ssid)
                                else
                                    lvgl.label_set_text(status_label, "连接失败: " .. (conn_msg or "未知错误"))
                                end
                            else
                                lvgl.label_set_text(status_label, "网络需要密码，暂不支持")
                            end
                        end)
                    end
                end
                lvgl.label_set_text(status_label, "找到 " .. #networks .. " 个网络")
            else
                lvgl.label_set_text(status_label, "未找到网络")
            end
        else
            lvgl.label_set_text(status_label, "WiFi初始化失败")
        end
    end)
    
    return screen
end

-- Screen 3: System Installation
local function setup_screen_3()
    print("Setting up screen 3: System Installation")
    
    local screen = lvgl.obj_create(nil)
    lvgl.obj_set_size(screen, 480, 320)
    lvgl.obj_set_scrollbar_mode(screen, lvgl.SCROLLBAR_MODE_OFF())
    lvgl.obj_set_style_bg_opa(screen, 0, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    
    local cont = create_container(screen)
    local title = create_title(cont, "安装系统至SD卡", 150, 30)
    
    local progress_bar = lvgl.bar_create(cont)
    lvgl.obj_set_pos(progress_bar, 116, 180)
    lvgl.obj_set_size(progress_bar, 238, 15)
    lvgl.obj_set_style_anim_time(progress_bar, 1000, 0)
    lvgl.bar_set_mode(progress_bar, lvgl.BAR_MODE_NORMAL())
    lvgl.bar_set_range(progress_bar, 0, 100)
    lvgl.bar_set_value(progress_bar, 0, lvgl.ANIM_OFF())
    
    -- Progress bar styling
    lvgl.obj_set_style_bg_opa(progress_bar, 60, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_bg_color(progress_bar, 0x2195f6, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_bg_grad_dir(progress_bar, lvgl.GRAD_DIR_NONE(), lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_radius(progress_bar, 10, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_shadow_width(progress_bar, 0, lvgl.PART_MAIN() | lvgl.STATE_DEFAULT())
    
    -- Progress bar indicator styling
    lvgl.obj_set_style_bg_opa(progress_bar, 255, lvgl.PART_INDICATOR() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_bg_color(progress_bar, 0x2195f6, lvgl.PART_INDICATOR() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_bg_grad_dir(progress_bar, lvgl.GRAD_DIR_NONE(), lvgl.PART_INDICATOR() | lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_radius(progress_bar, 10, lvgl.PART_INDICATOR() | lvgl.STATE_DEFAULT())
    
    local status_label = create_label(cont, "准备安装...", 150, 120, 180, 30)
    local progress_label = create_label(cont, "0%", 225, 210, 50, 30)
    local detail_label = create_label(cont, "正在初始化安装程序", 50, 250, 380, 30)
    
    local complete_btn = create_button(cont, "完成", 190, 260, 100, 50, function()
        print("Installation complete, restarting system")
        -- Save OOBE completion flag
        if oobe_state.sd_available then
            system.sd_write_file("oobe_complete.flag", "1")
        end
        
        -- Show completion message
        local msgbox = lvgl.msgbox_create(cont, "安装完成", "系统将重新启动", "确定", false)
        
        -- Restart after 3 seconds
        system.timer_create(3000, false, function()
            system.restart()
        end)
    end)
    
    -- Initially hide the complete button
    -- Note: LVGL object visibility functions not yet implemented
    
    ui.screen_3 = {
        screen = screen,
        container = cont,
        title = title,
        progress_bar = progress_bar,
        status_label = status_label,
        progress_label = progress_label,
        detail_label = detail_label,
        complete_btn = complete_btn
    }
    
    -- Start installation simulation
    start_installation()
    
    return screen
end

-- Installation simulation
function start_installation()
    local steps = {
        {progress = 10, status = "检查SD卡空间", detail = "验证存储空间..."},
        {progress = 25, status = "创建文件系统", detail = "正在格式化分区..."},
        {progress = 40, status = "复制系统文件", detail = "正在复制内核文件..."},
        {progress = 60, status = "安装应用程序", detail = "正在安装用户应用..."},
        {progress = 80, status = "配置系统", detail = "正在写入配置文件..."},
        {progress = 95, status = "验证安装", detail = "正在验证文件完整性..."},
        {progress = 100, status = "安装完成", detail = "系统安装成功完成！"}
    }
    
    local current_step = 1
    local timer = system.timer_create(2000, true, function()
        if current_step <= #steps then
            local step = steps[current_step]
            oobe_state.installation_progress = step.progress
            
            if ui.screen_3 then
                lvgl.bar_set_value(ui.screen_3.progress_bar, step.progress, lvgl.ANIM_ON())
                lvgl.label_set_text(ui.screen_3.status_label, step.status)
                lvgl.label_set_text(ui.screen_3.progress_label, step.progress .. "%")
                lvgl.label_set_text(ui.screen_3.detail_label, step.detail)
                
                if step.progress == 100 then
                    -- Show complete button (visibility functions not implemented yet)
                    print("Installation complete - showing complete button")
                end
            end
            
            current_step = current_step + 1
        else
            system.timer_stop(timer)
        end
    end)
    
    system.timer_start(timer)
end

-- Load screen function
function load_screen(screen_index)
    oobe_state.screen_index = screen_index
    
    local new_screen = nil
    if screen_index == 0 then
        new_screen = setup_screen_0()
    elseif screen_index == 1 then
        new_screen = setup_screen_1()
    elseif screen_index == 2 then
        new_screen = setup_screen_2()
    elseif screen_index == 3 then
        new_screen = setup_screen_3()
    end
    
    if new_screen then
        print("Loading screen " .. screen_index)
        -- Disable auto_del to prevent memory access errors
        lvgl.scr_load_anim(new_screen, lvgl.SCR_LOAD_ANIM_NONE(), 200, 200, false)
        
        -- Manually delete the old screen after a delay
        if current_screen then
            system.timer_create(500, false, function()
                -- Note: Manual deletion would be done here if needed
                -- For now, just clear the reference
                current_screen = nil
            end)
        end
        
        current_screen = new_screen
        
        -- Force refresh
        lvgl.refr_now()
    end
end

-- Main OOBE entry point
function main()
    print("LuaRTOS OOBE Started")
    print("Free heap: " .. system.get_free_heap() .. " bytes")
    print("PSRAM size: " .. system.get_psram_size() .. " bytes")
    
    -- Check if OOBE has already been completed
    if system.sd_is_mounted() then
        local oobe_flag = system.sd_read_file("oobe_complete.flag")
        if oobe_flag then
            print("OOBE already completed, skipping...")
            return
        end
    end
    
    -- Start with screen 0
    load_screen(0)
    
    print("OOBE initialization complete")
end

-- Start the OOBE
main()
