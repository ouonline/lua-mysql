#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

extern int luaopen_luamysql(lua_State* l);

int main(int argc, char* argv[])
{
    lua_State* l;

    if (argc != 2) {
        fprintf(stderr, "usage: %s luafile\n", argv[0]);
        return -1;
    }

    l = luaL_newstate();
    luaL_openlibs(l);

    luaopen_luamysql(l); /* this will put the function table on top of the stack */
    lua_setglobal(l, "mysql");

    if (luaL_dofile(l, argv[1]) != 0)
        fprintf(stderr, "luaL_dofile() error: %s.\n", lua_tostring(l, -1));

    lua_close(l);

    return 0;
}
