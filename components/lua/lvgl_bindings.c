#include "lvgl_bindings.h"
#include "esp_log.h"

static const char *TAG = "LVGL_BINDINGS";

// Metatable names for LVGL objects
#define LVGL_OBJ_METATABLE "lvgl.obj"

// Helper function to check if a value is a valid LVGL object pointer
static lv_obj_t* check_lvgl_obj(lua_State* L, int index) {
    void** obj_ptr = (void**)luaL_checkudata(L, index, LVGL_OBJ_METATABLE);
    luaL_argcheck(L, obj_ptr != NULL, index, "expected lvgl object");
    return (lv_obj_t*)*obj_ptr;
}

// Helper function to push an LVGL object to Lua stack
static void push_lvgl_obj(lua_State* L, lv_obj_t* obj) {
    if (obj == NULL) {
        lua_pushnil(L);
        return;
    }
    
    void** obj_ptr = (void**)lua_newuserdata(L, sizeof(void*));
    *obj_ptr = obj;
    luaL_getmetatable(L, LVGL_OBJ_METATABLE);
    lua_setmetatable(L, -2);
}

// Object creation and manipulation functions
int lvgl_scr_act(lua_State* L) {
    lv_obj_t* scr = lv_scr_act();
    push_lvgl_obj(L, scr);
    return 1;
}

int lvgl_obj_create(lua_State* L) {
    lv_obj_t* parent = NULL;
    if (lua_gettop(L) > 0 && !lua_isnil(L, 1)) {
        parent = check_lvgl_obj(L, 1);
    }
    
    lv_obj_t* obj = lv_obj_create(parent);
    push_lvgl_obj(L, obj);
    return 1;
}

int lvgl_obj_set_size(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_coord_t width = luaL_checkinteger(L, 2);
    lv_coord_t height = luaL_checkinteger(L, 3);
    
    lv_obj_set_size(obj, width, height);
    return 0;
}

int lvgl_obj_set_pos(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_coord_t x = luaL_checkinteger(L, 2);
    lv_coord_t y = luaL_checkinteger(L, 3);
    
    lv_obj_set_pos(obj, x, y);
    return 0;
}

int lvgl_obj_align(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_align_t align = luaL_checkinteger(L, 2);
    lv_coord_t x_ofs = luaL_checkinteger(L, 3);
    lv_coord_t y_ofs = luaL_checkinteger(L, 4);
    
    lv_obj_align(obj, align, x_ofs, y_ofs);
    return 0;
}

int lvgl_obj_align_to(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_obj_t* base = check_lvgl_obj(L, 2);
    lv_align_t align = luaL_checkinteger(L, 3);
    lv_coord_t x_ofs = luaL_checkinteger(L, 4);
    lv_coord_t y_ofs = luaL_checkinteger(L, 5);
    
    lv_obj_align_to(obj, base, align, x_ofs, y_ofs);
    return 0;
}

int lvgl_obj_center(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_obj_center(obj);
    return 0;
}

int lvgl_obj_clean(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_obj_clean(obj);
    return 0;
}

int lvgl_obj_invalidate(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_obj_invalidate(obj);
    return 0;
}

int lvgl_obj_set_style_bg_color(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    uint32_t color_value = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_color_t color = lv_color_hex(color_value);
    lv_obj_set_style_bg_color(obj, color, selector);
    return 0;
}

int lvgl_obj_set_style_text_color(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    uint32_t color_value = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_color_t color = lv_color_hex(color_value);
    lv_obj_set_style_text_color(obj, color, selector);
    return 0;
}

int lvgl_obj_set_style_text_font(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    // For simplicity, we'll use a pointer passed as light userdata
    const lv_font_t* font = (const lv_font_t*)lua_touserdata(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_text_font(obj, font, selector);
    return 0;
}

int lvgl_obj_set_style_border_width(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_coord_t width = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_border_width(obj, width, selector);
    return 0;
}

int lvgl_obj_set_style_border_color(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    uint32_t color_value = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_color_t color = lv_color_hex(color_value);
    lv_obj_set_style_border_color(obj, color, selector);
    return 0;
}

// Widget creation functions
int lvgl_label_create(lua_State* L) {
    lv_obj_t* parent = check_lvgl_obj(L, 1);
    lv_obj_t* label = lv_label_create(parent);
    push_lvgl_obj(L, label);
    return 1;
}

int lvgl_label_set_text(lua_State* L) {
    lv_obj_t* label = check_lvgl_obj(L, 1);
    const char* text = luaL_checkstring(L, 2);
    
    lv_label_set_text(label, text);
    return 0;
}

int lvgl_btn_create(lua_State* L) {
    lv_obj_t* parent = check_lvgl_obj(L, 1);
    lv_obj_t* btn = lv_btn_create(parent);
    push_lvgl_obj(L, btn);
    return 1;
}

int lvgl_slider_create(lua_State* L) {
    lv_obj_t* parent = check_lvgl_obj(L, 1);
    lv_obj_t* slider = lv_slider_create(parent);
    push_lvgl_obj(L, slider);
    return 1;
}

int lvgl_slider_set_value(lua_State* L) {
    lv_obj_t* slider = check_lvgl_obj(L, 1);
    int32_t value = luaL_checkinteger(L, 2);
    lv_anim_enable_t anim = luaL_checkinteger(L, 3);
    
    lv_slider_set_value(slider, value, anim);
    return 0;
}

int lvgl_switch_create(lua_State* L) {
    lv_obj_t* parent = check_lvgl_obj(L, 1);
    lv_obj_t* sw = lv_switch_create(parent);
    push_lvgl_obj(L, sw);
    return 1;
}

int lvgl_bar_create(lua_State* L) {
    lv_obj_t* parent = check_lvgl_obj(L, 1);
    lv_obj_t* bar = lv_bar_create(parent);
    push_lvgl_obj(L, bar);
    return 1;
}

int lvgl_bar_set_value(lua_State* L) {
    lv_obj_t* bar = check_lvgl_obj(L, 1);
    int32_t value = luaL_checkinteger(L, 2);
    lv_anim_enable_t anim = luaL_checkinteger(L, 3);
    
    lv_bar_set_value(bar, value, anim);
    return 0;
}

// Utility functions
int lvgl_color_hex(lua_State* L) {
    uint32_t hex_value = luaL_checkinteger(L, 1);
    lua_pushinteger(L, hex_value);
    return 1;
}

int lvgl_color_white(lua_State* L) {
    lua_pushinteger(L, 0xFFFFFF);
    return 1;
}

int lvgl_refr_now(lua_State* L) {
    lv_refr_now(NULL);
    return 0;
}

// Font constants
int lvgl_font_montserrat_14(lua_State* L) {
    lua_pushlightuserdata(L, (void*)&lv_font_montserrat_14);
    return 1;
}

// Animation constants
int lvgl_anim_off(lua_State* L) {
    lua_pushinteger(L, LV_ANIM_OFF);
    return 1;
}

// Alignment constants
int lvgl_align_center(lua_State* L) {
    lua_pushinteger(L, LV_ALIGN_CENTER);
    return 1;
}

int lvgl_align_top_mid(lua_State* L) {
    lua_pushinteger(L, LV_ALIGN_TOP_MID);
    return 1;
}

int lvgl_align_bottom_mid(lua_State* L) {
    lua_pushinteger(L, LV_ALIGN_BOTTOM_MID);
    return 1;
}

int lvgl_align_out_top_mid(lua_State* L) {
    lua_pushinteger(L, LV_ALIGN_OUT_TOP_MID);
    return 1;
}

// Part constants
int lvgl_part_main(lua_State* L) {
    lua_pushinteger(L, LV_PART_MAIN);
    return 1;
}

// Object metatable functions
static int lvgl_obj_gc(lua_State* L) {
    // LVGL objects are managed by LVGL itself, so we don't free them here
    return 0;
}

static int lvgl_obj_tostring(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lua_pushfstring(L, "lvgl.obj: %p", obj);
    return 1;
}

// Function registry
static const luaL_Reg lvgl_functions[] = {
    // Core functions
    {"scr_act", lvgl_scr_act},
    {"obj_create", lvgl_obj_create},
    {"obj_set_size", lvgl_obj_set_size},
    {"obj_set_pos", lvgl_obj_set_pos},
    {"obj_align", lvgl_obj_align},
    {"obj_align_to", lvgl_obj_align_to},
    {"obj_center", lvgl_obj_center},
    {"obj_clean", lvgl_obj_clean},
    {"obj_invalidate", lvgl_obj_invalidate},
    {"obj_set_style_bg_color", lvgl_obj_set_style_bg_color},
    {"obj_set_style_text_color", lvgl_obj_set_style_text_color},
    {"obj_set_style_text_font", lvgl_obj_set_style_text_font},
    {"obj_set_style_border_width", lvgl_obj_set_style_border_width},
    {"obj_set_style_border_color", lvgl_obj_set_style_border_color},
    
    // Widget functions
    {"label_create", lvgl_label_create},
    {"label_set_text", lvgl_label_set_text},
    {"btn_create", lvgl_btn_create},
    {"slider_create", lvgl_slider_create},
    {"slider_set_value", lvgl_slider_set_value},
    {"switch_create", lvgl_switch_create},
    {"bar_create", lvgl_bar_create},
    {"bar_set_value", lvgl_bar_set_value},
    
    // Utility functions
    {"color_hex", lvgl_color_hex},
    {"color_white", lvgl_color_white},
    {"refr_now", lvgl_refr_now},
    
    // Constants
    {"font_montserrat_14", lvgl_font_montserrat_14},
    {"ANIM_OFF", lvgl_anim_off},
    {"ALIGN_CENTER", lvgl_align_center},
    {"ALIGN_TOP_MID", lvgl_align_top_mid},
    {"ALIGN_BOTTOM_MID", lvgl_align_bottom_mid},
    {"ALIGN_OUT_TOP_MID", lvgl_align_out_top_mid},
    {"PART_MAIN", lvgl_part_main},
    
    {NULL, NULL}
};

static const luaL_Reg lvgl_obj_methods[] = {
    {"__gc", lvgl_obj_gc},
    {"__tostring", lvgl_obj_tostring},
    {NULL, NULL}
};

int luaopen_lvgl(lua_State* L) {
    ESP_LOGI(TAG, "Registering LVGL bindings...");
    
    // Create object metatable
    luaL_newmetatable(L, LVGL_OBJ_METATABLE);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, lvgl_obj_methods, 0);
    lua_pop(L, 1);
    
    // Create lvgl table
    luaL_newlib(L, lvgl_functions);
    
    // Set global lvgl table
    lua_setglobal(L, "lvgl");
    
    ESP_LOGI(TAG, "LVGL bindings registered successfully");
    return 0;
}
