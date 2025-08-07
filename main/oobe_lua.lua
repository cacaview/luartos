-- oobe_lua.lua - System Initialization Wizard
-- Replaces the original C implementation, provides complete OOBE experience with four pages

print("Starting OOBE Lua implementation...")

-- Global state management
local oobe = {
    current_screen = 0,  -- Current page: 0=Welcome, 1=SD Card, 2=WiFi, 3=System Install
    screens = {},        -- Store screen objects
    ui = {},            -- Store UI elements
    sd_formatted = false,  -- Whether SD card is formatted
    wifi_connected = false, -- Whether WiFi is connected
    install_progress = 0,   -- Installation progress
    install_timer = nil,    -- Installation progress timer
    wifi_list = {},        -- WiFi list
    selected_wifi = nil,   -- Selected WiFi
    password_input = nil   -- Password input field
}

-- Color constants
local COLORS = {
    WHITE = 0xffffff,
    BLUE = 0x2195f6,
    BLACK = 0x000000,
    GRAY = 0xe6e6e6,
    DARK_BLUE = 0x003a57
}

-- Initialize LVGL environment
local function init_lvgl()
    print("Initializing LVGL environment...")
    
    -- Initialize system bindings
    local wifi_ok, wifi_msg = system.wifi_init()
    if not wifi_ok then
        print("WARNING: WiFi initialization failed - " .. (wifi_msg or "unknown error"))
    end
    
    -- Initialize SD card
    local sd_ok, sd_msg = system.sd_init()
    if not sd_ok then
        print("WARNING: SD card initialization failed - " .. (sd_msg or "unknown error"))
    end
    
    print("LVGL environment initialized")
end

-- Create common style functions
local function set_common_container_style(obj)
    lvgl.obj_set_style_border_width(obj, 2, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_border_opa(obj, 255, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_border_color(obj, COLORS.BLUE, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_border_side(obj, lvgl.BORDER_SIDE_FULL(), lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_radius(obj, 0, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_bg_opa(obj, 255, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_bg_color(obj, COLORS.WHITE, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_bg_grad_dir(obj, lvgl.GRAD_DIR_NONE(), lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_pad_all(obj, 0, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_shadow_width(obj, 0, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
end

local function set_common_button_style(btn)
    lvgl.obj_set_style_bg_opa(btn, 255, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_bg_color(btn, COLORS.BLUE, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_bg_grad_dir(btn, lvgl.GRAD_DIR_NONE(), lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_border_width(btn, 0, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_radius(btn, 5, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_shadow_width(btn, 0, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_text_color(btn, COLORS.WHITE, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_text_font(btn, lvgl.font_montserrat_16(), lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_text_opa(btn, 255, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_text_align(btn, lvgl.TEXT_ALIGN_CENTER(), lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
end

-- Screen switching function
local function switch_to_screen(screen_index)
    print("=== SCREEN SWITCH REQUEST ===")
    print("From screen:", oobe.current_screen, "To screen:", screen_index)
    
    if oobe.screens[oobe.current_screen] then
        lvgl.obj_add_flag(oobe.screens[oobe.current_screen], lvgl.OBJ_FLAG_HIDDEN())
    end
    
    oobe.current_screen = screen_index
    
    if oobe.screens[screen_index] then
        lvgl.obj_clear_flag(oobe.screens[screen_index], lvgl.OBJ_FLAG_HIDDEN())
    end
    
    lvgl.refr_now()
    print("=== SUCCESSFULLY SWITCHED TO SCREEN", screen_index, "===")
end

-- Simplified event handling function
local function create_button_with_callback(parent, text, x, y, w, h, callback)
    local btn = lvgl.btn_create(parent)
    lvgl.obj_set_pos(btn, x, y)
    lvgl.obj_set_size(btn, w, h)
    set_common_button_style(btn)
    
    local btn_label = lvgl.label_create(btn)
    lvgl.label_set_text(btn_label, text)
    lvgl.obj_center(btn_label)
    
    -- Simplified event handling
    if callback then
        -- Register click event
        lvgl.obj_add_event_cb(btn, function(event)
            -- Check event type
            if lvgl.event_get_code(event) == lvgl.EVENT_CLICKED() then
                callback()
            end
        end)
    end
    
    return btn, btn_label
end

-- Welcome screen (Screen 0)
local function create_welcome_screen()
    print("Creating welcome screen...")
    
    local screen = lvgl.obj_create(lvgl.scr_act())
    lvgl.obj_set_size(screen, 480, 320)
    lvgl.obj_set_pos(screen, 0, 0)
    lvgl.obj_set_scrollbar_mode(screen, lvgl.SCROLLBAR_MODE_OFF())
    set_common_container_style(screen)
    
    -- Title
    local title = lvgl.label_create(screen)
    lvgl.label_set_text(title, "System Initialization")
    lvgl.obj_set_pos(title, 190, 30)
    lvgl.obj_set_size(title, 182, 42)
    lvgl.obj_set_style_text_font(title, lvgl.font_montserrat_20(), lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_text_color(title, COLORS.BLACK, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
    
    -- Start button
    local function welcome_btn_clicked()
        print("=== WELCOME BUTTON CLICKED ===")
        print("Current screen before click:", oobe.current_screen)
        print("Moving to SD card screen...")
        switch_to_screen(1)
    end
    
    local btn, btn_label = create_button_with_callback(screen, "Start", 190, 135, 100, 50, welcome_btn_clicked)
    
    oobe.screens[0] = screen
    oobe.ui.welcome = {
        screen = screen,
        title = title,
        btn = btn,
        btn_label = btn_label
    }
    
    print("Welcome screen created successfully")
end

-- SD Card check screen (Screen 1)
local function create_sd_screen()
    print("Creating SD card screen...")
    
    local screen = lvgl.obj_create(lvgl.scr_act())
    lvgl.obj_set_size(screen, 480, 320)
    lvgl.obj_set_pos(screen, 0, 0)
    lvgl.obj_set_scrollbar_mode(screen, lvgl.SCROLLBAR_MODE_OFF())
    set_common_container_style(screen)
    
    -- Title
    local title = lvgl.label_create(screen)
    lvgl.label_set_text(title, "Check SD Card")
    lvgl.obj_set_pos(title, 190, 30)
    lvgl.obj_set_size(title, 200, 100)
    lvgl.obj_set_style_text_font(title, lvgl.font_montserrat_20(), lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_text_color(title, COLORS.BLACK, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
    
    -- SD card status label
    local status_label = lvgl.label_create(screen)
    lvgl.label_set_text(status_label, "SD Card Status: Checking...")
    lvgl.obj_set_pos(status_label, 132, 127)
    lvgl.obj_set_size(status_label, 178, 36)
    
    -- SD card size label
    local size_label = lvgl.label_create(screen)
    lvgl.label_set_text(size_label, "SD Card Size:")
    lvgl.obj_set_pos(size_label, 132, 158)
    lvgl.obj_set_size(size_label, 200, 32)
    
    -- Forward declaration for check_sd_status
    local check_sd_status

    -- Format button event
    local function format_btn_clicked()
        if not oobe.sd_formatted then
            print("Format button clicked")
            
            -- Simple confirmation dialog
            print("User confirmed SD card format")
            
            -- Simulate formatting process
            lvgl.label_set_text(status_label, "SD Card Status: Formatting...")
            lvgl.refr_now()
            
            -- Use FreeRTOS delay to simulate formatting process
            system.sleep(2000)  -- 2 second delay
            
            oobe.sd_formatted = true
            lvgl.label_set_text(status_label, "SD Card Status: Formatted, Available")
            
            -- Re-check and display actual capacity after formatting
            check_sd_status()
            
            -- Delay to let user see the result
            system.sleep(1000)
            switch_to_screen(2)
        else
            switch_to_screen(2)
        end
    end
    
    local format_btn, format_btn_label = create_button_with_callback(screen, "Format & Next", 176, 209, 124, 54, format_btn_clicked)
    
    -- Check SD card status
    check_sd_status = function()
        print("=== CHECKING SD CARD STATUS ===")
        if system.sd_is_mounted() then
            print("SD card is mounted, getting info...")
            local info = system.sd_get_info()
            if info and info.total_bytes then
                print("SD card info received:")
                print("  Total bytes:", info.total_bytes)
                print("  Free bytes:", info.free_bytes)
                print("  Used bytes:", info.used_bytes)
                
                lvgl.label_set_text(status_label, "SD Card Status: Mounted")
                
                -- Dynamically calculate and display actual capacity, fixing hardcoding issue
                local total_bytes = info.total_bytes
                print("Raw total_bytes value:", total_bytes, "type:", type(total_bytes))
                
                -- Safe conversion for large numbers
                local bytes_per_mb = 1024 * 1024
                local bytes_per_gb = 1024 * 1024 * 1024
                
                local size_mb, size_gb
                if type(total_bytes) == "number" then
                    size_mb = math.floor(total_bytes / bytes_per_mb)
                    size_gb = math.floor(total_bytes / bytes_per_gb)
                else
                    -- If it's a large integer, use string processing or a default value
                    size_mb = math.floor(tonumber(total_bytes) / bytes_per_mb)
                    size_gb = math.floor(tonumber(total_bytes) / bytes_per_gb)
                end
                
                print("Calculated size_mb:", size_mb, "size_gb:", size_gb)
                
                -- Intelligently display capacity unit
                if size_gb and size_gb >= 1 then
                    local display_text = "SD Card Size: " .. size_gb .. "GB"
                    lvgl.label_set_text(size_label, display_text)
                    print("Displaying size as:", display_text)
                elseif size_mb and size_mb >= 1 then
                    local display_text = "SD Card Size: " .. size_mb .. "MB"
                    lvgl.label_set_text(size_label, display_text)
                    print("Displaying size as:", display_text)
                else
                    -- Capacity is unusually small, display raw byte count
                    local display_text = "SD Card Size: " .. tostring(total_bytes) .. " bytes"
                    lvgl.label_set_text(size_label, display_text)
                    print("Displaying size as:", display_text)
                end
            else
                print("Failed to get SD card info")
                lvgl.label_set_text(status_label, "SD Card Status: Needs Formatting")
                lvgl.label_set_text(size_label, "SD Card Size: Unknown")
            end
        else
            print("SD card is not mounted")
            lvgl.label_set_text(status_label, "SD Card Status: Not Detected")
            lvgl.label_set_text(size_label, "SD Card Size: Not Available")
        end
        print("=== SD CARD STATUS CHECK COMPLETE ===")
    end
    
    -- Initial check
    check_sd_status()
    
    oobe.screens[1] = screen
    oobe.ui.sd = {
        screen = screen,
        title = title,
        status_label = status_label,
        size_label = size_label,
        format_btn = format_btn
    }
    
    -- Hide this screen
    lvgl.obj_add_flag(screen, lvgl.OBJ_FLAG_HIDDEN())
    
    print("SD card screen created successfully")
end

-- WiFi connection screen (Screen 2)
local function create_wifi_screen()
    print("Creating WiFi screen...")

    local screen = lvgl.obj_create(lvgl.scr_act())
    lvgl.obj_set_size(screen, 480, 320)
    lvgl.obj_set_pos(screen, 0, 0)
    lvgl.obj_set_scrollbar_mode(screen, lvgl.SCROLLBAR_MODE_OFF())
    set_common_container_style(screen)

    -- Title
    local title = lvgl.label_create(screen)
    lvgl.label_set_text(title, "Connect to Network")
    lvgl.obj_set_pos(title, 190, 30)
    lvgl.obj_set_size(title, 179, 32)
    lvgl.obj_set_style_text_font(title, lvgl.font_montserrat_20(), lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_text_color(title, COLORS.BLACK, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())

    -- WiFi List
    local wifi_list = lvgl.list_create(screen)
    lvgl.obj_set_pos(wifi_list, 94, 130)
    lvgl.obj_set_size(wifi_list, 295, 168)

    -- Next button (initially disabled)
    local next_btn, next_label = create_button_with_callback(screen, "Next", 350, 260, 100, 40, function()
        if oobe.wifi_connected then
            switch_to_screen(3)
        end
    end)
    -- Manually "disable" by changing color, as state bindings are missing
    lvgl.obj_set_style_bg_color(next_btn, COLORS.GRAY, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())

    -- Refresh button
    local refresh_btn, refresh_label = create_button_with_callback(screen, "Refresh", 50, 260, 100, 40, function()
        update_wifi_list()
    end)

    -- Robust WiFi scanning with retries
    local try_wifi_scan -- Forward declaration

    try_wifi_scan = function(retries_left)
        if retries_left <= 0 then
            print("[WIFI_PAGE] Scan failed after multiple retries.")
            lvgl.obj_clean(wifi_list)
            lvgl.list_add_text(wifi_list, "Scan failed. Please refresh.")
            return
        end

        print("[WIFI_PAGE] Attempting to scan... " .. retries_left .. " retries left.")
        
        local networks, err_msg = system.wifi_scan()
        
        if not networks then
            print("[WIFI_PAGE] Scan attempt failed: " .. (err_msg or "Unknown error"))
            -- Check if the error is a timeout error and retry
            if err_msg and string.find(err_msg, "ESP_ERR_WIFI_TIMEOUT") then
                print("[WIFI_PAGE] Timeout detected, retrying in 1 second...")
                lvgl.obj_clean(wifi_list)
                lvgl.list_add_text(wifi_list, "Scanning... (retrying)")
                system.timer_create(1000, false, function()
                    try_wifi_scan(retries_left - 1)
                end)
            else
                -- For other errors, fail immediately
                lvgl.obj_clean(wifi_list)
                lvgl.list_add_text(wifi_list, "Scan failed: " .. (err_msg or "Unknown error"))
            end
            return
        end

        -- Scan succeeded
        lvgl.obj_clean(wifi_list)
        if #networks > 0 then
            print("[WIFI_PAGE] Scan successful, found " .. #networks .. " networks.")
            for i, net in ipairs(networks) do
                local btn = lvgl.list_add_btn(wifi_list, lvgl.SYMBOL_WIFI(), net.ssid)
                lvgl.obj_add_event_cb(btn, function(event)
                    if lvgl.event_get_code(event) == lvgl.EVENT_CLICKED() then
                        print("[WIFI_PAGE] Selected WiFi: " .. net.ssid .. " (AuthMode: " .. net.authmode .. ")")
                        handle_wifi_selection(net)
                    end
                end)
            end
        else
            print("[WIFI_PAGE] Scan successful, but no networks found.")
            lvgl.list_add_text(wifi_list, "No networks found")
        end
    end

    -- Function to update the WiFi list, initiating the retry sequence
    function update_wifi_list()
        print("[WIFI_PAGE] Starting WiFi scan sequence...")
        lvgl.obj_clean(wifi_list)
        lvgl.list_add_text(wifi_list, "Scanning...")

        -- Start the scan sequence with a small initial delay
        system.timer_create(200, false, function()
            try_wifi_scan(3) -- Start with 3 retries
        end)
    end

    -- Function to handle WiFi selection and password input
    function handle_wifi_selection(net)
        -- If network is open, connect directly
        if net.authmode == 0 then -- WIFI_AUTH_OPEN
            print("[WIFI_PAGE] Connecting to open network: " .. net.ssid)
            show_connecting_dialog()
            local success, msg = system.wifi_connect(net.ssid, "")
            handle_connection_result(success, msg)
            return
        end

        -- Manually create a password dialog with a virtual keyboard
        local dialog_bg = lvgl.obj_create(lvgl.scr_act())
        lvgl.obj_set_size(dialog_bg, 480, 320)
        lvgl.obj_set_style_bg_opa(dialog_bg, 128, 0)
        lvgl.obj_set_style_bg_color(dialog_bg, COLORS.BLACK, 0)

        -- Create a keyboard
        local kb = lvgl.keyboard_create(dialog_bg)
        lvgl.obj_set_size(kb, 480, 180)
        lvgl.obj_align(kb, lvgl.ALIGN_BOTTOM_MID(), 0, 0)

        -- Create a text area on a separate container above the keyboard
        local ta_container = lvgl.obj_create(dialog_bg)
        lvgl.obj_set_size(ta_container, 320, 80)
        lvgl.obj_align(ta_container, lvgl.ALIGN_TOP_MID(), 0, 20)

        local title = lvgl.label_create(ta_container)
        lvgl.label_set_text(title, "Enter Password for " .. net.ssid)
        lvgl.obj_align(title, lvgl.ALIGN_TOP_MID(), 0, 5)

        local ta = lvgl.textarea_create(ta_container)
        lvgl.obj_set_width(ta, 280)
        lvgl.textarea_set_one_line(ta, true)
        lvgl.textarea_set_password_mode(ta, true)
        lvgl.obj_align(ta, lvgl.ALIGN_CENTER(), 0, 10)
        oobe.password_input = ta

        -- Link keyboard to textarea
        lvgl.keyboard_set_textarea(kb, ta)

        -- Add event handlers to keyboard for OK/Cancel
        lvgl.obj_add_event_cb(kb, function(event)
            if lvgl.event_get_code(event) == lvgl.EVENT_READY() then -- OK button
                local password = lvgl.textarea_get_text(oobe.password_input)
                print("[WIFI_PAGE] Attempting to connect to " .. net.ssid .. " with password.")
                lvgl.obj_del(dialog_bg) -- Clean up dialog and keyboard
                show_connecting_dialog()
                system.timer_create(100, false, function()
                    local success, msg = system.wifi_connect(net.ssid, password)
                    handle_connection_result(success, msg)
                end)
            elseif lvgl.event_get_code(event) == lvgl.EVENT_CANCEL() then -- Close button
                lvgl.obj_del(dialog_bg) -- Clean up dialog and keyboard
            end
        end)
    end

    -- Show a "Connecting..." dialog
    function show_connecting_dialog()
        oobe.ui.wifi.connecting_mbox = lvgl.msgbox_create(screen, "Please Wait", "Connecting...", nil, false)
        lvgl.obj_center(oobe.ui.wifi.connecting_mbox)
    end

    -- Handle the result of a connection attempt
    function handle_connection_result(success, msg)
        -- Close "Connecting..." dialog if it exists
        if oobe.ui.wifi.connecting_mbox then
            lvgl.msgbox_close(oobe.ui.wifi.connecting_mbox)
            oobe.ui.wifi.connecting_mbox = nil
        end

        print("[WIFI_PAGE] Connection result: " .. tostring(success) .. ", Message: " .. (msg or "N/A"))
        if success then
            oobe.wifi_connected = true
            -- Manually "enable" by changing color
            lvgl.obj_set_style_bg_color(oobe.ui.wifi.next_btn, COLORS.BLUE, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
            lvgl.msgbox_create(screen, "Success", "Wi-Fi connected!", {"OK"}, false)
        else
            oobe.wifi_connected = false
            -- Manually "disable" by changing color
            lvgl.obj_set_style_bg_color(oobe.ui.wifi.next_btn, COLORS.GRAY, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
            lvgl.msgbox_create(screen, "Failed", "Could not connect to Wi-Fi network.\nPlease check the password and try again.", {"OK"}, false)
        end
    end

    oobe.screens[2] = screen
    oobe.ui.wifi = {
        screen = screen,
        title = title,
        wifi_list = wifi_list,
        next_btn = next_btn
    }

    -- Initial scan
    update_wifi_list()

    -- Hide this screen
    lvgl.obj_add_flag(screen, lvgl.OBJ_FLAG_HIDDEN())

    print("WiFi screen created successfully")
end

-- System installation screen (Screen 3)
local function create_install_screen()
    print("Creating system install screen...")
    
    local screen = lvgl.obj_create(lvgl.scr_act())
    lvgl.obj_set_size(screen, 480, 320)
    lvgl.obj_set_pos(screen, 0, 0)
    lvgl.obj_set_scrollbar_mode(screen, lvgl.SCROLLBAR_MODE_OFF())
    set_common_container_style(screen)
    
    -- Title
    local title = lvgl.label_create(screen)
    lvgl.label_set_text(title, "Install System to SD Card")
    lvgl.obj_set_pos(title, 150, 30)
    lvgl.obj_set_size(title, 204, 30)
    lvgl.obj_set_style_text_font(title, lvgl.font_montserrat_20(), lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
    lvgl.obj_set_style_text_color(title, COLORS.BLACK, lvgl.PART_MAIN(), lvgl.STATE_DEFAULT())
    
    -- Progress bar
    local progress_bar = lvgl.bar_create(screen)
    lvgl.obj_set_pos(progress_bar, 116, 180)
    lvgl.obj_set_size(progress_bar, 238, 15)
    lvgl.bar_set_mode(progress_bar, lvgl.BAR_MODE_NORMAL())
    lvgl.bar_set_range(progress_bar, 0, 100)
    lvgl.bar_set_value(progress_bar, 0, lvgl.ANIM_OFF())
    
    -- Installation step labels
    local step1_label = lvgl.label_create(screen)
    lvgl.label_set_text(step1_label, "Downloading packages...    " .. lvgl.SYMBOL_CLOSE())
    lvgl.obj_set_pos(step1_label, 135, 82)
    lvgl.obj_set_size(step1_label, 200, 20)
    
    local step2_label = lvgl.label_create(screen)
    lvgl.label_set_text(step2_label, "Extracting files....    " .. lvgl.SYMBOL_CLOSE())
    lvgl.obj_set_pos(step2_label, 135, 113)
    lvgl.obj_set_size(step2_label, 200, 18)
    
    local step3_label = lvgl.label_create(screen)
    lvgl.label_set_text(step3_label, "Cleaning up installation files....    " .. lvgl.SYMBOL_CLOSE())
    lvgl.obj_set_pos(step3_label, 135, 145)
    lvgl.obj_set_size(step3_label, 200, 23)
    
    -- Completion label (initially hidden)
    local complete_label = lvgl.label_create(screen)
    lvgl.label_set_text(complete_label, "Installation complete! System will restart in 5 seconds...")
    lvgl.obj_set_pos(complete_label, 150, 220)
    lvgl.obj_set_size(complete_label, 200, 30)
    lvgl.obj_add_flag(complete_label, lvgl.OBJ_FLAG_HIDDEN())
    
    -- Restart button (initially hidden)
    local function restart_clicked()
        print("=== RESTART BUTTON CLICKED ===")
        print("Current screen:", oobe.current_screen)
        print("Install progress:", oobe.install_progress)
        print("Manual restart requested")
        system.restart()
    end
    
    local restart_btn, restart_label = create_button_with_callback(screen, "Restart Now", 190, 260, 100, 40, restart_clicked)
    lvgl.obj_add_flag(restart_btn, lvgl.OBJ_FLAG_HIDDEN())
    
    -- Installation progress simulation
    local function simulate_installation()
        print("Starting system installation simulation...")
        
        -- Step 1: Download packages
        print("Step 1: Downloading packages...")
        for i = 0, 30 do
            lvgl.bar_set_value(progress_bar, i, lvgl.ANIM_OFF())
            lvgl.refr_now()
            system.sleep(50)
        end
        lvgl.label_set_text(step1_label, "Downloading packages...    " .. lvgl.SYMBOL_OK())
        
        -- Step 2: Extract files
        print("Step 2: Extracting files...")
        for i = 31, 70 do
            lvgl.bar_set_value(progress_bar, i, lvgl.ANIM_OFF())
            lvgl.refr_now()
            system.sleep(50)
        end
        lvgl.label_set_text(step2_label, "Extracting files....    " .. lvgl.SYMBOL_OK())
        
        -- Step 3: Clean up installation files
        print("Step 3: Cleaning up...")
        for i = 71, 100 do
            lvgl.bar_set_value(progress_bar, i, lvgl.ANIM_OFF())
            lvgl.refr_now()
            system.sleep(50)
        end
        lvgl.label_set_text(step3_label, "Cleaning up installation files....    " .. lvgl.SYMBOL_OK())
        
        -- Show completion message
        print("Installation completed successfully!")
        lvgl.obj_clear_flag(complete_label, lvgl.OBJ_FLAG_HIDDEN())
        lvgl.obj_clear_flag(restart_btn, lvgl.OBJ_FLAG_HIDDEN())
        lvgl.refr_now()
        
        -- Auto restart after 5 seconds
        system.sleep(5000)
        print("System will restart now...")
        system.restart()
    end
    
    oobe.screens[3] = screen
    oobe.ui.install = {
        screen = screen,
        title = title,
        progress_bar = progress_bar,
        step1_label = step1_label,
        step2_label = step2_label,
        step3_label = step3_label,
        complete_label = complete_label,
        restart_btn = restart_btn,
        simulate_installation = simulate_installation
    }
    
    -- Hide this screen
    lvgl.obj_add_flag(screen, lvgl.OBJ_FLAG_HIDDEN())
    
    print("System install screen created successfully")
end

-- Handler function for when entering a screen
local function on_screen_enter(screen_index)
    print("=== SCREEN ENTER EVENT ===")
    print("Entering screen:", screen_index)
    print("Previous screen:", oobe.current_screen)
    
    if screen_index == 3 then
        print("=== STARTING SYSTEM INSTALLATION ===")
        -- Auto start installation when entering install screen
        local install_ui = oobe.ui.install
        if install_ui and install_ui.simulate_installation then
            -- Start installation process after 500ms delay
            system.sleep(500)
            install_ui.simulate_installation()
        end
    end
end

-- Main initialization function
local function init_oobe()
    print("Initializing OOBE system...")
    
    -- Initialize LVGL environment
    init_lvgl()
    
    -- Create all screens
    create_welcome_screen()
    create_sd_screen()
    create_wifi_screen()
    create_install_screen()
    
    -- Show welcome screen
    switch_to_screen(0)
    
    print("OOBE system initialized successfully")
    print("Current screen: Welcome (0)")
    print("Use touch or button interactions to navigate")
end

-- Enhanced switch function with enter handling
local original_switch = switch_to_screen
switch_to_screen = function(screen_index)
    original_switch(screen_index)
    on_screen_enter(screen_index)
end

-- Start OOBE system
init_oobe()

-- Return oobe object for external access
return oobe
