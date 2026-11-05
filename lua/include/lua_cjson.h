//
//  lua_cjson.h
//  luademo
//
//  Created by h7ing on 06/07/2017.
//  Copyright © 2017 h7ing. All rights reserved.
//

#ifndef lua_cjson_h
#define lua_cjson_h

#include "lua.h"

#ifndef CJSON_EXPORT
#define CJSON_EXPORT extern
#endif

CJSON_EXPORT int luaopen_cjson(lua_State *l);
CJSON_EXPORT int luaopen_cjson_safe(lua_State *l);


#endif /* lua_cjson_h */
