#ifndef AIScriptBindExternalDelegate_hpp
#define AIScriptBindExternalDelegate_hpp

#include <string>

struct lua_State;

namespace OnePiece {
    extern int luaopen_externaldelegate(lua_State *L);

    extern bool luahelper_uiunit_get_adjust_shadowscale_by_deltay(lua_State *L, float oldscale, int deltay, float *out_adjust_scale, unsigned char *out_adjust_opacity);
    extern void luahelper_uiunit_get_armorbar_config(lua_State *L, std::string *out_icon_path, std::string *out_bar_path, float *out_icon_pos_x, float *out_icon_pos_y, float *out_bar_pos_x, float *out_bar_pos_y);
}

#endif /* AIScriptBindExternalDelegate_hpp */