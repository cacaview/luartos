idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES lvgl lvgl_esp32_drivers lua sdcard)

# Embed the OOBE Lua script
target_add_binary_data(${COMPONENT_LIB} "oobe_lua.lua" TEXT)
