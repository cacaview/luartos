#include "lvgl_bindings.h"
#include "esp_log.h"

static const char *TAG = "LVGL_BINDINGS";

// Metatable names for LVGL objects
#define LVGL_OBJ_METATABLE "lvgl.obj"

// Helper function to check if a value is a valid LVGL object pointer
static lv_obj_t* check_lvgl_obj(lua_State* L, int index) {
    void** obj_ptr = (void**)luaL_checkudata(L, index, LVGL_OBJ_METATABLE);
    luaL_argcheck(L, obj_ptr != NULL, index, "expected lvgl object");
    lv_obj_t* obj = (lv_obj_t*)*obj_ptr;
    luaL_argcheck(L, obj != NULL, index, "invalid lvgl object (null pointer)");

    // Add a crucial check to ensure the object is still valid in LVGL's context
    if (!lv_obj_is_valid(obj)) {
        luaL_error(L, "attempt to use an invalid or deleted lvgl object");
    }
    
    return obj;
}

// Helper function to push an LVGL object to Lua stack
static void push_lvgl_obj(lua_State* L, lv_obj_t* obj) {
    if (obj == NULL) {
        lua_pushnil(L);
        return;
    }
    
    ESP_LOGD(TAG, "Pushing new LVGL object to Lua: %p", obj);
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
    if (obj == NULL) {
        lua_pushnil(L);
    } else {
        push_lvgl_obj(L, obj);
    }
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
    if (label == NULL) {
        lua_pushnil(L);
    } else {
        push_lvgl_obj(L, label);
    }
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
    if (btn == NULL) {
        lua_pushnil(L);
    } else {
        push_lvgl_obj(L, btn);
    }
    return 1;
}

int lvgl_slider_create(lua_State* L) {
    lv_obj_t* parent = check_lvgl_obj(L, 1);
    lv_obj_t* slider = lv_slider_create(parent);
    if (slider == NULL) {
        lua_pushnil(L);
    } else {
        push_lvgl_obj(L, slider);
    }
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
    if (sw == NULL) {
        lua_pushnil(L);
    } else {
        push_lvgl_obj(L, sw);
    }
    return 1;
}

int lvgl_bar_create(lua_State* L) {
    lv_obj_t* parent = check_lvgl_obj(L, 1);
    lv_obj_t* bar = lv_bar_create(parent);
    if (bar == NULL) {
        lua_pushnil(L);
    } else {
        push_lvgl_obj(L, bar);
    }
    return 1;
}

int lvgl_bar_set_value(lua_State* L) {
    lv_obj_t* bar = check_lvgl_obj(L, 1);
    int32_t value = luaL_checkinteger(L, 2);
    lv_anim_enable_t anim = luaL_checkinteger(L, 3);
    
    lv_bar_set_value(bar, value, anim);
    return 0;
}

int lvgl_bar_set_range(lua_State* L) {
    lv_obj_t* bar = check_lvgl_obj(L, 1);
    int32_t min = luaL_checkinteger(L, 2);
    int32_t max = luaL_checkinteger(L, 3);
    
    lv_bar_set_range(bar, min, max);
    return 0;
}

int lvgl_bar_set_mode(lua_State* L) {
    lv_obj_t* bar = check_lvgl_obj(L, 1);
    lv_bar_mode_t mode = luaL_checkinteger(L, 2);
    
    lv_bar_set_mode(bar, mode);
    return 0;
}

// Advanced widget functions
int lvgl_spangroup_create(lua_State* L) {
    lv_obj_t* parent = check_lvgl_obj(L, 1);
    lv_obj_t* spangroup = lv_spangroup_create(parent);
    if (spangroup == NULL) {
        lua_pushnil(L);
    } else {
        push_lvgl_obj(L, spangroup);
    }
    return 1;
}

int lvgl_spangroup_new_span(lua_State* L) {
    lv_obj_t* spangroup = check_lvgl_obj(L, 1);
    lv_span_t* span = lv_spangroup_new_span(spangroup);
    if (span == NULL) {
        lua_pushnil(L);
    } else {
        lua_pushlightuserdata(L, span);
    }
    return 1;
}

int lvgl_span_set_text(lua_State* L) {
    lv_span_t* span = (lv_span_t*)lua_touserdata(L, 1);
    const char* text = luaL_checkstring(L, 2);

    if (span == NULL) {
        return luaL_error(L, "attempt to use a NULL span object");
    }
    
    lv_span_set_text(span, text);
    return 0;
}

int lvgl_spangroup_set_align(lua_State* L) {
    lv_obj_t* spangroup = check_lvgl_obj(L, 1);
    lv_text_align_t align = luaL_checkinteger(L, 2);
    
    lv_spangroup_set_align(spangroup, align);
    return 0;
}

int lvgl_spangroup_set_overflow(lua_State* L) {
    lv_obj_t* spangroup = check_lvgl_obj(L, 1);
    lv_span_overflow_t overflow = luaL_checkinteger(L, 2);
    
    lv_spangroup_set_overflow(spangroup, overflow);
    return 0;
}

int lvgl_spangroup_set_mode(lua_State* L) {
    lv_obj_t* spangroup = check_lvgl_obj(L, 1);
    lv_span_mode_t mode = luaL_checkinteger(L, 2);
    
    lv_spangroup_set_mode(spangroup, mode);
    return 0;
}

int lvgl_spangroup_refr_mode(lua_State* L) {
    lv_obj_t* spangroup = check_lvgl_obj(L, 1);
    lv_spangroup_refr_mode(spangroup);
    return 0;
}

int lvgl_msgbox_create(lua_State* L) {
    lv_obj_t* parent = check_lvgl_obj(L, 1);
    const char* title = luaL_checkstring(L, 2);
    const char* txt = luaL_checkstring(L, 3);
    bool add_close_btn = lua_toboolean(L, 5);

    const char** btn_txts = NULL;
    int n_btns = 0;

    // Check if the 4th argument is a table
    if (lua_istable(L, 4)) {
        n_btns = luaL_len(L, 4);
        if (n_btns > 0) {
            // Allocate memory for the button text pointer array (+1 for NULL terminator)
            btn_txts = (const char**)malloc(sizeof(const char*) * (n_btns + 1));
            if (!btn_txts) {
                return luaL_error(L, "Failed to allocate memory for button texts");
            }

            for (int i = 0; i < n_btns; i++) {
                lua_rawgeti(L, 4, i + 1); // Lua tables are 1-based
                btn_txts[i] = luaL_checkstring(L, -1);
                lua_pop(L, 1);
            }
            btn_txts[n_btns] = NULL; // NULL-terminate the array
        }
    }
    // If the 4th argument is not a table (e.g., nil), btn_txts will remain NULL,
    // which is what lv_msgbox_create expects for no buttons.

    lv_obj_t* msgbox = lv_msgbox_create(parent, title, txt, btn_txts, add_close_btn);

    // Free the allocated memory for the button texts array
    if (btn_txts) {
        free(btn_txts);
    }

    if (msgbox == NULL) {
        lua_pushnil(L);
    } else {
        push_lvgl_obj(L, msgbox);
    }
    return 1;
}

int lvgl_list_create(lua_State* L) {
    lv_obj_t* parent = check_lvgl_obj(L, 1);
    lv_obj_t* list = lv_list_create(parent);
    if (list == NULL) {
        lua_pushnil(L);
    } else {
        push_lvgl_obj(L, list);
    }
    return 1;
}

int lvgl_list_add_text(lua_State* L) {
    lv_obj_t* list = check_lvgl_obj(L, 1);
    const char* txt = luaL_checkstring(L, 2);
    
    lv_obj_t* txt_obj = lv_list_add_text(list, txt);
    if (txt_obj == NULL) {
        lua_pushnil(L);
    } else {
        push_lvgl_obj(L, txt_obj);
    }
    return 1;
}

int lvgl_list_add_btn(lua_State* L) {
    lv_obj_t* list = check_lvgl_obj(L, 1);
    const void* icon = lua_touserdata(L, 2); // Can be NULL
    const char* txt = luaL_checkstring(L, 3);
    
    lv_obj_t* btn = lv_list_add_btn(list, icon, txt);
    if (btn == NULL) {
        lua_pushnil(L);
    } else {
        push_lvgl_obj(L, btn);
    }
    return 1;
}

// Style functions
int lvgl_obj_set_style_bg_opa(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_opa_t opa = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_bg_opa(obj, opa, selector);
    return 0;
}

int lvgl_obj_set_style_border_opa(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_opa_t opa = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_border_opa(obj, opa, selector);
    return 0;
}

int lvgl_obj_set_style_border_side(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_border_side_t side = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_border_side(obj, side, selector);
    return 0;
}

int lvgl_obj_set_style_radius(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_coord_t radius = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_radius(obj, radius, selector);
    return 0;
}

int lvgl_obj_set_style_bg_grad_dir(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_grad_dir_t grad_dir = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_bg_grad_dir(obj, grad_dir, selector);
    return 0;
}

int lvgl_obj_set_style_pad_all(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_coord_t pad = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_pad_all(obj, pad, selector);
    return 0;
}

int lvgl_obj_set_style_pad_top(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_coord_t pad = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_pad_top(obj, pad, selector);
    return 0;
}

int lvgl_obj_set_style_pad_bottom(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_coord_t pad = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_pad_bottom(obj, pad, selector);
    return 0;
}

int lvgl_obj_set_style_pad_left(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_coord_t pad = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_pad_left(obj, pad, selector);
    return 0;
}

int lvgl_obj_set_style_pad_right(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_coord_t pad = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_pad_right(obj, pad, selector);
    return 0;
}

int lvgl_obj_set_style_shadow_width(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_coord_t width = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_shadow_width(obj, width, selector);
    return 0;
}

int lvgl_obj_set_style_text_opa(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_opa_t opa = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_text_opa(obj, opa, selector);
    return 0;
}

int lvgl_obj_set_style_text_letter_space(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_coord_t space = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_text_letter_space(obj, space, selector);
    return 0;
}

int lvgl_obj_set_style_text_line_space(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_coord_t space = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_text_line_space(obj, space, selector);
    return 0;
}

int lvgl_obj_set_style_text_align(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_text_align_t align = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_text_align(obj, align, selector);
    return 0;
}

int lvgl_obj_set_style_text_decor(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_text_decor_t decor = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_text_decor(obj, decor, selector);
    return 0;
}

int lvgl_obj_set_style_anim_time(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    uint32_t time = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_anim_time(obj, time, selector);
    return 0;
}

// Object configuration functions
int lvgl_obj_set_scrollbar_mode(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_scrollbar_mode_t mode = luaL_checkinteger(L, 2);
    
    lv_obj_set_scrollbar_mode(obj, mode);
    return 0;
}

int lvgl_obj_set_width(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_coord_t width = luaL_checkinteger(L, 2);
    
    lv_obj_set_width(obj, width);
    return 0;
}

int lvgl_label_set_long_mode(lua_State* L) {
    lv_obj_t* label = check_lvgl_obj(L, 1);
    lv_label_long_mode_t mode = luaL_checkinteger(L, 2);
    
    lv_label_set_long_mode(label, mode);
    return 0;
}

// Event handling - Fixed: Per-object callback storage
typedef struct {
    lua_State* L;
    int callback_ref;
} lua_event_data_t;

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static void lua_event_callback(lv_event_t* e) {
    lua_event_data_t* event_data = (lua_event_data_t*)lv_event_get_user_data(e);
    if (!event_data || !event_data->L || event_data->callback_ref == LUA_NOREF) {
        return;
    }

    lua_State* L = event_data->L;
    lv_event_code_t code = lv_event_get_code(e);

    // Handle regular event callbacks
    if (code != LV_EVENT_DELETE) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, event_data->callback_ref);
        if (lua_isfunction(L, -1)) {
            lua_pushlightuserdata(L, e);
            int pcall_result = lua_pcall(L, 1, 0, 0);
            if (pcall_result != LUA_OK) {
                const char* error_msg = lua_tostring(L, -1);
                ESP_LOGE("LVGL_EVENT", "Lua callback error: %s", error_msg ? error_msg : "unknown error");
                lua_pop(L, 1);
            }
        } else {
            lua_pop(L, 1);
        }
    }
    // Handle the DELETE event to clean up resources
    else {
        // Unreference the Lua callback function
        luaL_unref(L, LUA_REGISTRYINDEX, event_data->callback_ref);
        // Free the container
        free(event_data);
    }
}

int lvgl_obj_add_event_cb(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    if (!lua_isfunction(L, 2)) {
        return luaL_argerror(L, 2, "expected a function");
    }

    // Allocate a container for the Lua state and callback reference
    lua_event_data_t* event_data = (lua_event_data_t*)malloc(sizeof(lua_event_data_t));
    if (!event_data) {
        return luaL_error(L, "Failed to allocate memory for event data");
    }

    // Store the Lua state and create a reference to the callback function
    event_data->L = L;
    lua_pushvalue(L, 2); // Duplicate function on stack for luaL_ref
    event_data->callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    // Add the event callback to the object
    // The lua_event_callback will be called for ALL events, including DELETE
    lv_obj_add_event_cb(obj, lua_event_callback, LV_EVENT_ALL, event_data);

    return 0;
}

// Screen loading
int lvgl_scr_load_anim(lua_State* L) {
    lv_obj_t* scr = check_lvgl_obj(L, 1);
    lv_scr_load_anim_t anim_type = luaL_checkinteger(L, 2);
    uint32_t time = luaL_checkinteger(L, 3);
    uint32_t delay = luaL_checkinteger(L, 4);
    bool auto_del = lua_toboolean(L, 5);
    
    lv_scr_load_anim(scr, anim_type, time, delay, auto_del);
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

int lvgl_color_black(lua_State* L) {
    lua_pushinteger(L, 0x000000);
    return 1;
}

int lvgl_refr_now(lua_State* L) {
    lv_refr_now(NULL);
    return 0;
}

// Part constants
int lvgl_part_main(lua_State* L) {
    lua_pushinteger(L, LV_PART_MAIN);
    return 1;
}

int lvgl_part_indicator(lua_State* L) {
    lua_pushinteger(L, LV_PART_INDICATOR);
    return 1;
}

int lvgl_part_knob(lua_State* L) {
    lua_pushinteger(L, LV_PART_KNOB);
    return 1;
}

// State constants
int lvgl_state_default(lua_State* L) {
    lua_pushinteger(L, LV_STATE_DEFAULT);
    return 1;
}

int lvgl_state_checked(lua_State* L) {
    lua_pushinteger(L, LV_STATE_CHECKED);
    return 1;
}

// Border constants
int lvgl_border_side_full(lua_State* L) {
    lua_pushinteger(L, LV_BORDER_SIDE_FULL);
    return 1;
}

// Text alignment constants
int lvgl_text_align_left(lua_State* L) {
    lua_pushinteger(L, LV_TEXT_ALIGN_LEFT);
    return 1;
}

int lvgl_text_align_center(lua_State* L) {
    lua_pushinteger(L, LV_TEXT_ALIGN_CENTER);
    return 1;
}

int lvgl_text_align_right(lua_State* L) {
    lua_pushinteger(L, LV_TEXT_ALIGN_RIGHT);
    return 1;
}

// Scrollbar mode constants
int lvgl_scrollbar_mode_off(lua_State* L) {
    lua_pushinteger(L, LV_SCROLLBAR_MODE_OFF);
    return 1;
}

int lvgl_scrollbar_mode_on(lua_State* L) {
    lua_pushinteger(L, LV_SCROLLBAR_MODE_ON);
    return 1;
}

// Label mode constants
int lvgl_label_long_wrap(lua_State* L) {
    lua_pushinteger(L, LV_LABEL_LONG_WRAP);
    return 1;
}

// Span constants
int lvgl_span_overflow_clip(lua_State* L) {
    lua_pushinteger(L, LV_SPAN_OVERFLOW_CLIP);
    return 1;
}

int lvgl_span_mode_break(lua_State* L) {
    lua_pushinteger(L, LV_SPAN_MODE_BREAK);
    return 1;
}

// Bar mode constants
int lvgl_bar_mode_normal(lua_State* L) {
    lua_pushinteger(L, LV_BAR_MODE_NORMAL);
    return 1;
}

// Grad dir constants
int lvgl_grad_dir_none(lua_State* L) {
    lua_pushinteger(L, LV_GRAD_DIR_NONE);
    return 1;
}

// Direction constants
int lvgl_dir_top(lua_State* L) {
    lua_pushinteger(L, LV_DIR_TOP);
    return 1;
}

// Text decoration constants
int lvgl_text_decor_none(lua_State* L) {
    lua_pushinteger(L, LV_TEXT_DECOR_NONE);
    return 1;
}

// Screen load animation constants
int lvgl_scr_load_anim_none(lua_State* L) {
    lua_pushinteger(L, LV_SCR_LOAD_ANIM_NONE);
    return 1;
}

// Event constants
int lvgl_event_clicked(lua_State* L) {
    lua_pushinteger(L, LV_EVENT_CLICKED);
    return 1;
}

int lvgl_event_value_changed(lua_State* L) {
    lua_pushinteger(L, LV_EVENT_VALUE_CHANGED);
    return 1;
}

int lvgl_event_ready(lua_State* L) {
    lua_pushinteger(L, LV_EVENT_READY);
    return 1;
}

int lvgl_event_cancel(lua_State* L) {
    lua_pushinteger(L, LV_EVENT_CANCEL);
    return 1;
}

int lvgl_event_focused(lua_State* L) {
    lua_pushinteger(L, LV_EVENT_FOCUSED);
    return 1;
}

int lvgl_event_defocused(lua_State* L) {
    lua_pushinteger(L, LV_EVENT_DEFOCUSED);
    return 1;
}

// Object flag constants
int lvgl_obj_flag_hidden(lua_State* L) {
    lua_pushinteger(L, LV_OBJ_FLAG_HIDDEN);
    return 1;
}

int lvgl_obj_flag_scrollable(lua_State* L) {
    lua_pushinteger(L, LV_OBJ_FLAG_SCROLLABLE);
    return 1;
}

int lvgl_obj_flag_clickable(lua_State* L) {
    lua_pushinteger(L, LV_OBJ_FLAG_CLICKABLE);
    return 1;
}

// Symbol constants
int lvgl_symbol_wifi(lua_State* L) {
    lua_pushstring(L, LV_SYMBOL_WIFI);
    return 1;
}

int lvgl_symbol_ok(lua_State* L) {
    lua_pushstring(L, LV_SYMBOL_OK);
    return 1;
}

int lvgl_symbol_close(lua_State* L) {
    lua_pushstring(L, LV_SYMBOL_CLOSE);
    return 1;
}

// Font constants
int lvgl_font_montserrat_12(lua_State* L) {
    // Fallback to montserrat_14 since montserrat_12 is not enabled
    lua_pushlightuserdata(L, (void*)&lv_font_montserrat_14);
    return 1;
}

int lvgl_font_montserrat_20(lua_State* L) {
    // Fallback to montserrat_16 since montserrat_20 is not enabled
    lua_pushlightuserdata(L, (void*)&lv_font_montserrat_16);
    return 1;
}

// Event handling helper
int lvgl_event_get_code(lua_State* L) {
    lv_event_t* event = (lv_event_t*)lua_touserdata(L, 1);
    lv_event_code_t code = lv_event_get_code(event);
    lua_pushinteger(L, code);
    return 1;
}

int lvgl_event_get_target(lua_State* L) {
    lv_event_t* event = (lv_event_t*)lua_touserdata(L, 1);
    lv_obj_t* target = lv_event_get_target(event);
    push_lvgl_obj(L, target);
    return 1;
}

int lvgl_event_send(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_event_code_t code = luaL_checkinteger(L, 2);
    void* user_data = lua_isnoneornil(L, 3) ? NULL : lua_touserdata(L, 3);
    lv_event_send(obj, code, user_data);
    return 0;
}

// Object state checking
int lvgl_obj_has_state(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_state_t state = luaL_checkinteger(L, 2);
    bool has_state = lv_obj_has_state(obj, state);
    lua_pushboolean(L, has_state);
    return 1;
}

// Object flag functions
int lvgl_obj_add_flag(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_obj_flag_t flag = luaL_checkinteger(L, 2);
    lv_obj_add_flag(obj, flag);
    return 0;
}

int lvgl_obj_clear_flag(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_obj_flag_t flag = luaL_checkinteger(L, 2);
    lv_obj_clear_flag(obj, flag);
    return 0;
}

int lvgl_obj_is_valid(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    bool is_valid = lv_obj_is_valid(obj);
    lua_pushboolean(L, is_valid);
    return 1;
}

int lvgl_obj_del(lua_State* L) {
    void** obj_ptr = (void**)luaL_checkudata(L, 1, LVGL_OBJ_METATABLE);
    if (obj_ptr && *obj_ptr) {
        lv_obj_t* obj = (lv_obj_t*)*obj_ptr;
        if (lv_obj_is_valid(obj)) {
            lv_obj_del(obj);
        }
        // Invalidate the Lua userdata by setting its pointer to NULL.
        *obj_ptr = NULL;
    }
    return 0;
}

// Functions to get object properties
int lvgl_obj_get_x(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lua_pushinteger(L, lv_obj_get_x(obj));
    return 1;
}

int lvgl_obj_get_y(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lua_pushinteger(L, lv_obj_get_y(obj));
    return 1;
}

int lvgl_obj_get_width(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lua_pushinteger(L, lv_obj_get_width(obj));
    return 1;
}

int lvgl_obj_get_height(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lua_pushinteger(L, lv_obj_get_height(obj));
    return 1;
}

// Flexbox layout functions
int lvgl_obj_set_layout(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    uint32_t layout = luaL_checkinteger(L, 2);
    lv_obj_set_layout(obj, layout);
    return 0;
}

int lvgl_obj_set_flex_flow(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_flex_flow_t flow = luaL_checkinteger(L, 2);
    lv_obj_set_flex_flow(obj, flow);
    return 0;
}

int lvgl_obj_set_flex_align(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_flex_align_t main_place = luaL_checkinteger(L, 2);
    lv_flex_align_t cross_place = luaL_checkinteger(L, 3);
    lv_flex_align_t track_cross_place = luaL_checkinteger(L, 4);
    lv_obj_set_flex_align(obj, main_place, cross_place, track_cross_place);
    return 0;
}

int lvgl_obj_set_style_pad_gap(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_coord_t value = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    lv_obj_set_style_pad_gap(obj, value, selector);
    return 0;
}

// Widget functions for missing entries
int lvgl_img_create(lua_State* L) {
    lv_obj_t* parent = check_lvgl_obj(L, 1);
    lv_obj_t* img = lv_img_create(parent);
    if (img == NULL) {
        lua_pushnil(L);
    } else {
        push_lvgl_obj(L, img);
    }
    return 1;
}

int lvgl_textarea_create(lua_State* L) {
    lv_obj_t* parent = check_lvgl_obj(L, 1);
    lv_obj_t* textarea = lv_textarea_create(parent);
    if (textarea == NULL) {
        lua_pushnil(L);
    } else {
        push_lvgl_obj(L, textarea);
    }
    return 1;
}

int lvgl_textarea_set_text(lua_State* L) {
    lv_obj_t* textarea = check_lvgl_obj(L, 1);
    const char* text = luaL_checkstring(L, 2);
    lv_textarea_set_text(textarea, text);
    return 0;
}

int lvgl_textarea_get_text(lua_State* L) {
    lv_obj_t* textarea = check_lvgl_obj(L, 1);
    const char* text = lv_textarea_get_text(textarea);
    lua_pushstring(L, text);
    return 1;
}

int lvgl_textarea_set_one_line(lua_State* L) {
    lv_obj_t* ta = check_lvgl_obj(L, 1);
    bool en = lua_toboolean(L, 2);
    lv_textarea_set_one_line(ta, en);
    return 0;
}

int lvgl_textarea_set_placeholder_text(lua_State* L) {
    lv_obj_t* ta = check_lvgl_obj(L, 1);
    const char* txt = luaL_checkstring(L, 2);
    lv_textarea_set_placeholder_text(ta, txt);
    return 0;
}

int lvgl_textarea_set_max_length(lua_State* L) {
    lv_obj_t* ta = check_lvgl_obj(L, 1);
    uint32_t len = luaL_checkinteger(L, 2);
    lv_textarea_set_max_length(ta, len);
    return 0;
}

int lvgl_textarea_set_password_mode(lua_State* L) {
    lv_obj_t* ta = check_lvgl_obj(L, 1);
    bool en = lua_toboolean(L, 2);
    lv_textarea_set_password_mode(ta, en);
    return 0;
}

int lvgl_keyboard_create(lua_State* L) {
    lv_obj_t* parent = check_lvgl_obj(L, 1);
    lv_obj_t* keyboard = lv_keyboard_create(parent);
    if (keyboard == NULL) {
        lua_pushnil(L);
    } else {
        push_lvgl_obj(L, keyboard);
    }
    return 1;
}

int lvgl_keyboard_set_textarea(lua_State* L) {
    lv_obj_t* kb = check_lvgl_obj(L, 1);
    lv_obj_t* ta = check_lvgl_obj(L, 2);
    lv_keyboard_set_textarea(kb, ta);
    return 0;
}

// Menu widget functions
int lvgl_menu_create(lua_State* L) {
    lv_obj_t* parent = check_lvgl_obj(L, 1);
    lv_obj_t* menu = lv_menu_create(parent);
    if (menu == NULL) {
        lua_pushnil(L);
    } else {
        push_lvgl_obj(L, menu);
    }
    return 1;
}

int lvgl_menu_page_create(lua_State* L) {
    lv_obj_t* menu = check_lvgl_obj(L, 1);
    const char* title = lua_isnoneornil(L, 2) ? NULL : luaL_checkstring(L, 2);
    lv_obj_t* page = lv_menu_page_create(menu, (char*)title);
    if (page == NULL) {
        lua_pushnil(L);
    } else {
        push_lvgl_obj(L, page);
    }
    return 1;
}

int lvgl_menu_cont_create(lua_State* L) {
    lv_obj_t* page = check_lvgl_obj(L, 1);
    lv_obj_t* cont = lv_menu_cont_create(page);
    if (cont == NULL) {
        lua_pushnil(L);
    } else {
        push_lvgl_obj(L, cont);
    }
    return 1;
}

// Tabview bindings
int lvgl_tabview_create(lua_State* L) {
    lv_obj_t* parent = check_lvgl_obj(L, 1);
    lv_dir_t dir = luaL_checkinteger(L, 2);
    lv_coord_t size = luaL_checkinteger(L, 3);

    lv_obj_t* tabview = lv_tabview_create(parent, dir, size);
    if (tabview == NULL) {
        lua_pushnil(L);
    } else {
        push_lvgl_obj(L, tabview);
    }
    return 1;
}

int lvgl_tabview_add_tab(lua_State* L) {
    lv_obj_t* tabview = check_lvgl_obj(L, 1);
    const char* name = luaL_checkstring(L, 2);

    lv_obj_t* tab = lv_tabview_add_tab(tabview, name);
    if (tab == NULL) {
        lua_pushnil(L);
    } else {
        push_lvgl_obj(L, tab);
    }
    return 1;
}

int lvgl_menu_set_sidebar_page(lua_State* L) {
    lv_obj_t* menu = check_lvgl_obj(L, 1);
    lv_obj_t* page = check_lvgl_obj(L, 2);
    lv_menu_set_sidebar_page(menu, page);
    return 0;
}

int lvgl_menu_set_load_page_event(lua_State* L) {
    lv_obj_t* menu = check_lvgl_obj(L, 1);
    lv_obj_t* obj = check_lvgl_obj(L, 2);
    lv_obj_t* page = check_lvgl_obj(L, 3);
    lv_menu_set_load_page_event(menu, obj, page);
    return 0;
}

int lvgl_menu_get_sidebar_header(lua_State* L) {
    lv_obj_t* menu = check_lvgl_obj(L, 1);
    lv_obj_t* header = lv_menu_get_sidebar_header(menu);
    push_lvgl_obj(L, header);
    return 1;
}

// Other missing functions
int lvgl_obj_update_layout(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_obj_update_layout(obj);
    return 0;
}

int lvgl_obj_move_foreground(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_obj_move_foreground(obj);
    return 0;
}

int lvgl_obj_move_background(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_obj_move_background(obj);
    return 0;
}

// Msgbox helper functions
int lvgl_msgbox_get_btns(lua_State* L) {
    lv_obj_t* msgbox = check_lvgl_obj(L, 1);
    lv_obj_t* btns = lv_msgbox_get_btns(msgbox);
    push_lvgl_obj(L, btns);
    return 1;
}

int lvgl_msgbox_get_title(lua_State* L) {
    lv_obj_t* msgbox = check_lvgl_obj(L, 1);
    lv_obj_t* title = lv_msgbox_get_title(msgbox);
    push_lvgl_obj(L, title);
    return 1;
}

int lvgl_msgbox_get_text(lua_State* L) {
    lv_obj_t* msgbox = check_lvgl_obj(L, 1);
    lv_obj_t* text = lv_msgbox_get_text(msgbox);
    push_lvgl_obj(L, text);
    return 1;
}

// Font constants
int lvgl_font_montserrat_14(lua_State* L) {
    lua_pushlightuserdata(L, (void*)&lv_font_montserrat_14);
    return 1;
}

int lvgl_font_montserrat_16(lua_State* L) {
    lua_pushlightuserdata(L, (void*)&lv_font_montserrat_16);
    return 1;
}

// Animation constants
int lvgl_anim_off(lua_State* L) {
    lua_pushinteger(L, LV_ANIM_OFF);
    return 1;
}

int lvgl_anim_on(lua_State* L) {
    lua_pushinteger(L, LV_ANIM_ON);
    return 1;
}

// Alignment constants
int lvgl_align_center(lua_State* L) {
    lua_pushinteger(L, LV_ALIGN_CENTER);
    return 1;
}

int lvgl_align_left_mid(lua_State* L) {
    lua_pushinteger(L, LV_ALIGN_LEFT_MID);
    return 1;
}

int lvgl_align_right_mid(lua_State* L) {
    lua_pushinteger(L, LV_ALIGN_RIGHT_MID);
    return 1;
}

int lvgl_align_top_left(lua_State* L) {
    lua_pushinteger(L, LV_ALIGN_TOP_LEFT);
    return 1;
}

int lvgl_align_top_mid(lua_State* L) {
    lua_pushinteger(L, LV_ALIGN_TOP_MID);
    return 1;
}

int lvgl_align_top_right(lua_State* L) {
    lua_pushinteger(L, LV_ALIGN_TOP_RIGHT);
    return 1;
}

int lvgl_align_bottom_left(lua_State* L) {
    lua_pushinteger(L, LV_ALIGN_BOTTOM_LEFT);
    return 1;
}

int lvgl_align_bottom_mid(lua_State* L) {
    lua_pushinteger(L, LV_ALIGN_BOTTOM_MID);
    return 1;
}

int lvgl_align_bottom_right(lua_State* L) {
    lua_pushinteger(L, LV_ALIGN_BOTTOM_RIGHT);
    return 1;
}

int lvgl_align_out_top_mid(lua_State* L) {
    lua_pushinteger(L, LV_ALIGN_OUT_TOP_MID);
    return 1;
}

int lvgl_align_out_bottom_left(lua_State* L) {
    lua_pushinteger(L, LV_ALIGN_OUT_BOTTOM_LEFT);
    return 1;
}

// Special value functions
int lvgl_pct(lua_State* L) {
    int32_t v = luaL_checkinteger(L, 1);
    lua_pushinteger(L, LV_PCT(v));
    return 1;
}

// Object metatable functions
static int lvgl_obj_gc(lua_State* L) {
    // This is called by Lua's garbage collector.
    // We need to ensure the underlying LVGL object is deleted.
    void** obj_ptr = (void**)luaL_checkudata(L, 1, LVGL_OBJ_METATABLE);
    if (obj_ptr && *obj_ptr) {
        lv_obj_t* obj = (lv_obj_t*)*obj_ptr;
        // Check if the object is still valid before deleting.
        // This prevents double-freeing if it was already deleted manually.
        if (lv_obj_is_valid(obj)) {
            lv_obj_del(obj);
        }
        // Set the pointer to NULL to prevent use-after-free.
        *obj_ptr = NULL;
    }
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
    {"obj_set_scrollbar_mode", lvgl_obj_set_scrollbar_mode},
    {"obj_set_width", lvgl_obj_set_width},
    {"obj_add_event_cb", lvgl_obj_add_event_cb},
    
    // Style functions
    {"obj_set_style_bg_color", lvgl_obj_set_style_bg_color},
    {"obj_set_style_text_color", lvgl_obj_set_style_text_color},
    {"obj_set_style_text_font", lvgl_obj_set_style_text_font},
    {"obj_set_style_border_width", lvgl_obj_set_style_border_width},
    {"obj_set_style_border_color", lvgl_obj_set_style_border_color},
    {"obj_set_style_bg_opa", lvgl_obj_set_style_bg_opa},
    {"obj_set_style_border_opa", lvgl_obj_set_style_border_opa},
    {"obj_set_style_border_side", lvgl_obj_set_style_border_side},
    {"obj_set_style_radius", lvgl_obj_set_style_radius},
    {"obj_set_style_bg_grad_dir", lvgl_obj_set_style_bg_grad_dir},
    {"obj_set_style_pad_all", lvgl_obj_set_style_pad_all},
    {"obj_set_style_pad_top", lvgl_obj_set_style_pad_top},
    {"obj_set_style_pad_bottom", lvgl_obj_set_style_pad_bottom},
    {"obj_set_style_pad_left", lvgl_obj_set_style_pad_left},
    {"obj_set_style_pad_right", lvgl_obj_set_style_pad_right},
    {"obj_set_style_shadow_width", lvgl_obj_set_style_shadow_width},
    {"obj_set_style_text_opa", lvgl_obj_set_style_text_opa},
    {"obj_set_style_text_letter_space", lvgl_obj_set_style_text_letter_space},
    {"obj_set_style_text_line_space", lvgl_obj_set_style_text_line_space},
    {"obj_set_style_text_align", lvgl_obj_set_style_text_align},
    {"obj_set_style_text_decor", lvgl_obj_set_style_text_decor},
    {"obj_set_style_anim_time", lvgl_obj_set_style_anim_time},
    
    // Widget functions
    {"label_create", lvgl_label_create},
    {"label_set_text", lvgl_label_set_text},
    {"label_set_long_mode", lvgl_label_set_long_mode},
    {"btn_create", lvgl_btn_create},
    {"slider_create", lvgl_slider_create},
    {"slider_set_value", lvgl_slider_set_value},
    {"switch_create", lvgl_switch_create},
    {"bar_create", lvgl_bar_create},
    {"bar_set_value", lvgl_bar_set_value},
    {"bar_set_range", lvgl_bar_set_range},
    {"bar_set_mode", lvgl_bar_set_mode},
    {"spangroup_create", lvgl_spangroup_create},
    {"spangroup_new_span", lvgl_spangroup_new_span},
    {"span_set_text", lvgl_span_set_text},
    {"spangroup_set_align", lvgl_spangroup_set_align},
    {"spangroup_set_overflow", lvgl_spangroup_set_overflow},
    {"spangroup_set_mode", lvgl_spangroup_set_mode},
    {"spangroup_refr_mode", lvgl_spangroup_refr_mode},
    {"msgbox_create", lvgl_msgbox_create},
    {"msgbox_get_btns", lvgl_msgbox_get_btns},
    {"msgbox_get_title", lvgl_msgbox_get_title},
    {"msgbox_get_text", lvgl_msgbox_get_text},
    {"list_create", lvgl_list_create},
    {"list_add_text", lvgl_list_add_text},
    {"list_add_btn", lvgl_list_add_btn},
    {"img_create", lvgl_img_create},
    {"textarea_create", lvgl_textarea_create},
    {"textarea_set_text", lvgl_textarea_set_text},
    {"textarea_get_text", lvgl_textarea_get_text},
    {"textarea_set_one_line", lvgl_textarea_set_one_line},
    {"textarea_set_placeholder_text", lvgl_textarea_set_placeholder_text},
    {"textarea_set_max_length", lvgl_textarea_set_max_length},
    {"textarea_set_password_mode", lvgl_textarea_set_password_mode},
    {"keyboard_create", lvgl_keyboard_create},
    {"keyboard_set_textarea", lvgl_keyboard_set_textarea},

    // Tabview functions
    {"tabview_create", lvgl_tabview_create},
    {"tabview_add_tab", lvgl_tabview_add_tab},

    // Menu functions
    {"menu_create", lvgl_menu_create},
    {"menu_page_create", lvgl_menu_page_create},
    {"menu_cont_create", lvgl_menu_cont_create},
    {"menu_set_sidebar_page", lvgl_menu_set_sidebar_page},
    {"menu_set_load_page_event", lvgl_menu_set_load_page_event},
    {"menu_get_sidebar_header", lvgl_menu_get_sidebar_header},

    // Other functions
    {"obj_update_layout", lvgl_obj_update_layout},
    {"obj_move_foreground", lvgl_obj_move_foreground},
    {"obj_move_background", lvgl_obj_move_background},

    {"obj_add_flag", lvgl_obj_add_flag},
    {"obj_clear_flag", lvgl_obj_clear_flag},
    {"obj_has_state", lvgl_obj_has_state},
    {"obj_is_valid", lvgl_obj_is_valid},
    {"obj_del", lvgl_obj_del},
    {"obj_get_x", lvgl_obj_get_x},
    {"obj_get_y", lvgl_obj_get_y},
    {"obj_get_width", lvgl_obj_get_width},
    {"obj_get_height", lvgl_obj_get_height},
    {"pct", lvgl_pct},
    
    // Event functions
    {"event_get_code", lvgl_event_get_code},
    {"event_get_target", lvgl_event_get_target},
    {"event_send", lvgl_event_send},
    
    // Utility functions
    {"color_hex", lvgl_color_hex},
    {"color_white", lvgl_color_white},
    {"color_black", lvgl_color_black},
    {"refr_now", lvgl_refr_now},
    {"scr_load_anim", lvgl_scr_load_anim},
    
    // Font constants
    {"font_montserrat_14", lvgl_font_montserrat_14},
    {"font_montserrat_16", lvgl_font_montserrat_16},
    {"font_montserrat_20", lvgl_font_montserrat_20},
    {"font_montserrat_12", lvgl_font_montserrat_12},
    
    // Constants functions
    {"PART_MAIN", lvgl_part_main},
    {"PART_INDICATOR", lvgl_part_indicator},
    {"PART_KNOB", lvgl_part_knob},
    {"STATE_DEFAULT", lvgl_state_default},
    {"STATE_CHECKED", lvgl_state_checked},
    {"ALIGN_CENTER", lvgl_align_center},
    {"ALIGN_TOP_MID", lvgl_align_top_mid},
    {"ALIGN_LEFT_MID", lvgl_align_left_mid},
    {"ALIGN_RIGHT_MID", lvgl_align_right_mid},
    {"ALIGN_BOTTOM_MID", lvgl_align_bottom_mid},
    {"TEXT_ALIGN_LEFT", lvgl_text_align_left},
    {"TEXT_ALIGN_CENTER", lvgl_text_align_center},
    {"TEXT_ALIGN_RIGHT", lvgl_text_align_right},
    {"SPAN_MODE_BREAK", lvgl_span_mode_break},
    {"SPAN_OVERFLOW_CLIP", lvgl_span_overflow_clip},
    {"TEXT_DECOR_NONE", lvgl_text_decor_none},
    {"BORDER_SIDE_FULL", lvgl_border_side_full},
    {"GRAD_DIR_NONE", lvgl_grad_dir_none},
    {"DIR_TOP", lvgl_dir_top},
    {"SCROLLBAR_MODE_OFF", lvgl_scrollbar_mode_off},
    {"SCROLLBAR_MODE_ON", lvgl_scrollbar_mode_on},
    {"LABEL_LONG_WRAP", lvgl_label_long_wrap},
    {"BAR_MODE_NORMAL", lvgl_bar_mode_normal},
    {"ANIM_OFF", lvgl_anim_off},
    {"SCR_LOAD_ANIM_NONE", lvgl_scr_load_anim_none},
    {"EVENT_CLICKED", lvgl_event_clicked},
    {"EVENT_VALUE_CHANGED", lvgl_event_value_changed},
    {"EVENT_READY", lvgl_event_ready},
    {"EVENT_CANCEL", lvgl_event_cancel},
    {"EVENT_FOCUSED", lvgl_event_focused},
    {"EVENT_DEFOCUSED", lvgl_event_defocused},
    {"OBJ_FLAG_HIDDEN", lvgl_obj_flag_hidden},
    {"OBJ_FLAG_SCROLLABLE", lvgl_obj_flag_scrollable},
    {"OBJ_FLAG_CLICKABLE", lvgl_obj_flag_clickable},
    {"SYMBOL_WIFI", lvgl_symbol_wifi},
    {"SYMBOL_OK", lvgl_symbol_ok},
    {"SYMBOL_CLOSE", lvgl_symbol_close},
    
    // Additional alignment constants
    {"ALIGN_TOP_LEFT", lvgl_align_top_left},
    {"ALIGN_TOP_RIGHT", lvgl_align_top_right},
    {"ALIGN_BOTTOM_LEFT", lvgl_align_bottom_left},
    {"ALIGN_BOTTOM_RIGHT", lvgl_align_bottom_right},
    {"ALIGN_OUT_TOP_MID", lvgl_align_out_top_mid},
    {"ALIGN_OUT_BOTTOM_LEFT", lvgl_align_out_bottom_left},
    
    // Animation constants
    {"ANIM_ON", lvgl_anim_on},

    // Layout and Flexbox bindings
    {"obj_set_layout", lvgl_obj_set_layout},
    {"obj_set_flex_flow", lvgl_obj_set_flex_flow},
    {"obj_set_flex_align", lvgl_obj_set_flex_align},
    {"obj_set_style_pad_gap", lvgl_obj_set_style_pad_gap},
    
    {NULL, NULL}
};

// Helper macro to register a constant value
#define LUA_REG_CONST_INT(L, name, value) \
    lua_pushinteger(L, value); \
    lua_setfield(L, -2, name);

// This function will be called from luaopen_lvgl to register all constants
static void register_lvgl_constants(lua_State* L) {
    // Flex Flow
    LUA_REG_CONST_INT(L, "FLEX_FLOW_ROW", LV_FLEX_FLOW_ROW);
    LUA_REG_CONST_INT(L, "FLEX_FLOW_COLUMN", LV_FLEX_FLOW_COLUMN);
    LUA_REG_CONST_INT(L, "FLEX_FLOW_ROW_WRAP", LV_FLEX_FLOW_ROW_WRAP);
    LUA_REG_CONST_INT(L, "FLEX_FLOW_COLUMN_WRAP", LV_FLEX_FLOW_COLUMN_WRAP);
    LUA_REG_CONST_INT(L, "FLEX_FLOW_ROW_REVERSE", LV_FLEX_FLOW_ROW_REVERSE);
    LUA_REG_CONST_INT(L, "FLEX_FLOW_COLUMN_REVERSE", LV_FLEX_FLOW_COLUMN_REVERSE);
    LUA_REG_CONST_INT(L, "FLEX_FLOW_ROW_WRAP_REVERSE", LV_FLEX_FLOW_ROW_WRAP_REVERSE);
    LUA_REG_CONST_INT(L, "FLEX_FLOW_COLUMN_WRAP_REVERSE", LV_FLEX_FLOW_COLUMN_WRAP_REVERSE);

    // Flex Align
    LUA_REG_CONST_INT(L, "FLEX_ALIGN_START", LV_FLEX_ALIGN_START);
    LUA_REG_CONST_INT(L, "FLEX_ALIGN_END", LV_FLEX_ALIGN_END);
    LUA_REG_CONST_INT(L, "FLEX_ALIGN_CENTER", LV_FLEX_ALIGN_CENTER);
    LUA_REG_CONST_INT(L, "FLEX_ALIGN_SPACE_EVENLY", LV_FLEX_ALIGN_SPACE_EVENLY);
    LUA_REG_CONST_INT(L, "FLEX_ALIGN_SPACE_AROUND", LV_FLEX_ALIGN_SPACE_AROUND);
    LUA_REG_CONST_INT(L, "FLEX_ALIGN_SPACE_BETWEEN", LV_FLEX_ALIGN_SPACE_BETWEEN);

    // Layouts
    LUA_REG_CONST_INT(L, "LAYOUT_FLEX", LV_LAYOUT_FLEX);
    LUA_REG_CONST_INT(L, "LAYOUT_GRID", LV_LAYOUT_GRID);
}

int luaopen_lvgl(lua_State* L) {
    ESP_LOGI(TAG, "Registering LVGL bindings...");

    // Create the main lvgl library table and register functions
    luaL_newlib(L, lvgl_functions);
    int lib_idx = lua_gettop(L);

    // Create the metatable for LVGL objects
    luaL_newmetatable(L, LVGL_OBJ_METATABLE);
    int meta_idx = lua_gettop(L);

    // Define metatable methods locally to avoid static analysis issues.
    const luaL_Reg obj_methods[] = {
        {"__gc", lvgl_obj_gc},
        {"__tostring", lvgl_obj_tostring},
        {NULL, NULL}
    };

    // Set __index = lvgl_table
    // This makes obj:method() work by looking up methods in the main lvgl table
    lua_pushvalue(L, lib_idx);
    lua_setfield(L, meta_idx, "__index");

    // Register the __gc and __tostring methods to the metatable
    luaL_setfuncs(L, obj_methods, 0);

    lua_pop(L, 1); // Pop the metatable

    // Register all constants into the main lvgl library table
    lua_pushvalue(L, lib_idx); // Push library table to top
    register_lvgl_constants(L);
    lua_pop(L, 1); // Pop library table

    ESP_LOGI(TAG, "LVGL bindings registered successfully");
    
    // Return the main library table, which is already on the stack
    return 1;
}
