/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for lua: core API; calls luaL_newstate(), luaL_openlibs(), luaL_dostring(), lua_tostring().
 */

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include <stdio.h>
#include <string.h>

int main(void) {
    lua_State *state = luaL_newstate();
    if (state == NULL) {
        fprintf(stderr, "luaL_newstate failed\n");
        return 1;
    }
    luaL_openlibs(state);
    const char *chunk =
        "local t = {} for i = 1, 10 do t[#t + 1] = i * i end "
        "return string.format('%s|%d|%s|%s', _VERSION, t[10] // 3, math.type(2.5), utf8.char(72, 105))";
    if (luaL_dostring(state, chunk) != LUA_OK) {
        fprintf(stderr, "lua error: %s\n", lua_tostring(state, -1));
        lua_close(state);
        return 1;
    }
    const char *result = lua_tostring(state, -1);
    /* _VERSION is "Lua 5.x"; everything after the first '|' is the same on every 5.x release. */
    const char *rest = result != NULL ? strchr(result, '|') : NULL;
    int ok = result != NULL && strncmp(result, "Lua 5.", 6) == 0 && rest != NULL && strcmp(rest, "|33|float|Hi") == 0;
    printf("lua probe: %s\n", result != NULL ? result : "(null)");
    lua_close(state);
    if (!ok) {
        fprintf(stderr, "expected 'Lua 5.x|33|float|Hi'\n");
        return 1;
    }
    return 0;
}
