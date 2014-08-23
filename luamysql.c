#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <mysql/mysql.h>

static const char* MySQLLib = "MySQL.lib";
static const char* MySQLResult = "MySQLResult";

/* ------------------------------------------------------------------------- */

#define mysql_init_error(l, fmt, ...) \
    do { \
        lua_pushnil(l); /* empty result */ \
        lua_pushfstring(l, fmt, ##__VA_ARGS__); /* error message */ \
    } while (0)

/* return 0 if ok */
static inline int get_args(lua_State* l, const char** host, unsigned int* port,
                           const char** user, const char** password,
                           const char** db)
{
    int argtype;

    if (lua_gettop(l) != 1) {
        mysql_init_error(l, "1 argument required, but %d is provided.",
                         lua_gettop(l));
        return 2;
    }

    if (!lua_istable(l, 1)) {
        mysql_init_error(l, "argument #1 expects a table, but a %s is provided.",
                         lua_typename(l, lua_type(l, 1)));
        return 2;
    }

    lua_getfield(l, 1, "host");
    if (!lua_isstring(l, -1)) {
        argtype = lua_type(l, -1);
        mysql_init_error(l, "argument#1::`host' expects a string, but a %s is provided.",
                         lua_typename(l, argtype));
        return 2;
    }
    *host = lua_tostring(l, -1);

    lua_getfield(l, 1, "port");
    if (!lua_isnumber(l, -1)) {
        argtype = lua_type(l, -1);
        mysql_init_error(l, "argument#1::`port' expects a number, but a %s is provided.",
                         lua_typename(l, argtype));
        return 2;
    }
    *port = lua_tointeger(l, -1);

    lua_getfield(l, 1, "user");
    if (!lua_isnil(l, -1)) {
        if (!lua_isstring(l, -1)) {
            argtype = lua_type(l, -1);
            mysql_init_error(l, "argument#1::`user' expects a string, but a %s is provided.",
                             lua_typename(l, argtype));
            return 2;
        }
        *user = lua_tostring(l, -1);
    }

    lua_getfield(l, 1, "password");
    if (!lua_isnil(l, -1)) {
        if (!lua_isstring(l, -1)) {
            argtype = lua_type(l, -1);
            mysql_init_error(l, "argument#1::`password' expects a string, but a %s is provided.",
                             lua_typename(l, argtype));
            return 2;
        }
        *password = lua_tostring(l, -1);
    }

    lua_getfield(l, 1, "db");
    if (!lua_isnil(l, -1)) {
        if (!lua_isstring(l, -1)) {
            argtype = lua_type(l, -1);
            mysql_init_error(l, "argument#1::`db' expects a string, but a %s is provided.",
                             lua_typename(l, argtype));
            return 2;
        }
        *db = lua_tostring(l, -1);
    }

    return 0;
}

/* return a connection and an error message */
static int l_new_mysqlclient(lua_State* l)
{
    MYSQL* conn;
    unsigned int port;
    const char *host, *user = NULL, *password = NULL, *db = NULL;
    const char* errmsg;

    if (get_args(l, &host, &port, &user, &password, &db) != 0)
        return 2;

    conn = lua_newuserdata(l, sizeof(MYSQL));
    luaL_setmetatable(l, MySQLLib);

    if (!mysql_init(conn)) {
        errmsg = "mysql_init() failed.";
        goto conn_err;
    }

    if (!mysql_real_connect(conn, host, user, password, db, port, NULL, 0)) {
        errmsg = mysql_error(conn);
        goto conn_err;
    }

    lua_pushnil(l); /* errmsg */
    return 2;

conn_err:
    lua_pop(l, 1); /* pop the newuserdata */
    mysql_init_error(l, errmsg);
    return 2;
}

/* ------------------------------------------------------------------------- */

/* return an error message if fails, otherwise nil */
static int l_mysqlclient_ping(lua_State* l)
{
    MYSQL* conn;

    conn = luaL_testudata(l, 1, MySQLLib);
    if (!conn) {
        lua_pushstring(l, "argument #1 is not a mysql client.");
        return 1;
    }

    if (mysql_ping(conn) == 0)
        lua_pushnil(l);
    else
        lua_pushstring(l, mysql_error(conn));

    return 1;
}

/* return an error message if fails, otherwise nil */
static int l_mysqlclient_selectdb(lua_State* l)
{
    MYSQL* conn;
    const char* db;

    conn = luaL_testudata(l, 1, MySQLLib);
    if (!conn) {
        lua_pushstring(l, "argument #1 is not a mysql client.");
        return 1;
    }

    if (!lua_isstring(l, 2)) {
        lua_pushfstring(l, "argument #2 expects a db name, but given a %s.",
                        lua_typename(l, lua_type(l, 2)));
        return 1;
    }
    db = lua_tostring(l, 2);

    if (mysql_select_db(conn, db) == 0)
        lua_pushnil(l);
    else
        lua_pushstring(l, mysql_error(conn));

    return 1;
}

/* return an error message if fails, otherwise nil */
static int l_mysqlclient_setcharset(lua_State* l)
{
    MYSQL* conn;
    const char* charset;

    conn = luaL_testudata(l, 1, MySQLLib);
    if (!conn) {
        lua_pushstring(l, "argument #1 is not a mysql client.");
        return 1;
    }

    if (!lua_isstring(l, 2)) {
        lua_pushfstring(l, "argument #2 expects a charset string, but given a %s.",
                        lua_typename(l, lua_type(l, 2)));
        return 1;
    }
    charset = lua_tostring(l, 2);

    if (mysql_set_character_set(conn, charset) == 0)
        lua_pushnil(l);
    else
        lua_pushstring(l, mysql_error(conn));

    return 1;
}

/* return the escaped string and the error message */
static int l_mysqlclient_escape(lua_State* l)
{
    MYSQL* conn;
    const char* content;
    char* buf;
    unsigned long len;

    conn = luaL_testudata(l, 1, MySQLLib);
    if (!conn) {
        lua_pushnil(l);
        lua_pushstring(l, "argument #1 is not a mysql client.");
        return 2;
    }

    if (!lua_isstring(l, 2)) {
        int type = lua_type(l, 2);
        lua_pushnil(l);
        lua_pushfstring(l, "argument #2 expects a sql string, but given a %s.",
                        lua_typename(l, type));
        return 2;
    }
    content = lua_tolstring(l, 2, &len);

    if (len == 0) {
        lua_pushstring(l, "");
        lua_pushnil(l);
        return 2;
    }

    buf = malloc(len * 2 + 1);
    if (!buf) {
        lua_pushnil(l);
        lua_pushstring(l, "allocating buffer failed.");
        return 2;
    }

    len = mysql_real_escape_string(conn, buf, content, len);

    lua_pushlstring(l, buf, len);
    lua_pushnil(l);

    free(buf);
    return 2;
}

/* return the result set and the error message */
static int l_mysqlclient_query(lua_State* l)
{
    int err;
    MYSQL* conn;
    const char* sqlstr;
    unsigned long sqllen;

    conn = luaL_testudata(l, 1, MySQLLib);
    if (!conn) {
        lua_pushnil(l);
        lua_pushstring(l, "argument #1 is not a mysql client.");
        return 2;
    }

    if (!lua_isstring(l, 2)) {
        int type = lua_type(l, 2);
        lua_pushnil(l);
        lua_pushfstring(l, "argument #2 expects a sql string, but given a %s.",
                        lua_typename(l, type));
        return 2;
    }
    sqlstr = lua_tolstring(l, 2, &sqllen);

    if (sqllen == 0) {
        lua_pushnil(l);
        lua_pushstring(l, "invalid SQL statement.");
        return 2;
    }

    err = mysql_real_query(conn, sqlstr, sqllen);
    if (err) {
        lua_pushnil(l);
        lua_pushstring(l, mysql_error(conn));
    } else {
        MYSQL_RES** result = lua_newuserdata(l, sizeof(MYSQL_RES*));
        luaL_setmetatable(l, MySQLResult);

        *result = mysql_store_result(conn);
        lua_pushnil(l);
    }

    return 2;
}

static int l_mysqlclient_gc(lua_State* l)
{
    MYSQL* conn = luaL_testudata(l, 1, MySQLLib);
    if (conn)
        mysql_close(conn);

    return 0;
}

/* ------------------------------------------------------------------------- */

/* return the fieldnamelist, or nil if fails. */
static int l_mysqlresult_fieldnamelist(lua_State* l)
{
    MYSQL_RES** result = luaL_testudata(l, 1, MySQLResult);

    if (!result)
        lua_pushnil(l);
    else {
        int i;
        int nr_field = mysql_num_fields(*result);
        MYSQL_FIELD* fieldlist = mysql_fetch_fields(*result);

        lua_newtable(l);
        for (i = 0; i < nr_field; ++i) {
            lua_pushstring(l, fieldlist[i].name);
            lua_rawseti(l, -2, i + 1);
        }
    }

    return 1;
}

/* return the number of record(s), or nil if fails. */
static int l_mysqlresult_count(lua_State* l)
{
    MYSQL_RES** result = luaL_testudata(l, 1, MySQLResult);

    if (!result)
        lua_pushnil(l);
    else
        lua_pushinteger(l, mysql_num_rows(*result));

    return 1;
}

static int l_mysqlresult_record_iter(lua_State* l)
{
    MYSQL_ROW row;
    MYSQL_RES** result = lua_touserdata(l, lua_upvalueindex(1));
    int nr_field = lua_tointeger(l, lua_upvalueindex(2));

    row = mysql_fetch_row(*result);
    if (row) {
        int i;

        lua_newtable(l);
        for (i = 0; i < nr_field; ++i) {
            lua_pushstring(l, row[i]);
            lua_rawseti(l, -2, i + 1);
        }

        return 1;
    }

    return 0;
}

/* return a record iterator, or nil if fails. */
static int l_mysqlresult_recordlist(lua_State* l)
{
    MYSQL_RES** result = luaL_testudata(l, 1, MySQLResult);

    if (!result)
        lua_pushnil(l);
    else {
        lua_pushvalue(l, -1); /* duplicate the result */
        lua_pushinteger(l, mysql_num_fields(*result)); /* number of fields */
        lua_pushcclosure(l, l_mysqlresult_record_iter, 2);
    }

    return 1;
}

static int l_mysqlresult_gc(lua_State* l)
{
    MYSQL_RES** result = luaL_testudata(l, 1, MySQLResult);

    if (*result)
        mysql_free_result(*result);

    return 0;
}

/* ------------------------------------------------------------------------- */

static const struct luaL_Reg mysqlclient_f[] = {
    {"newclient", l_new_mysqlclient},
    {NULL, NULL},
};

static const struct luaL_Reg mysqlclient_m[] = {
    {"ping", l_mysqlclient_ping},
    {"selectdb", l_mysqlclient_selectdb},
    {"setcharset", l_mysqlclient_setcharset},
    {"escape", l_mysqlclient_escape},
    {"query", l_mysqlclient_query},
    {"__gc", l_mysqlclient_gc},
    {NULL, NULL},
};

static const struct luaL_Reg mysqlresult_lib[] = {
    {"count", l_mysqlresult_count},
    {"fieldnamelist", l_mysqlresult_fieldnamelist},
    {"recordlist", l_mysqlresult_recordlist},
    {"__gc", l_mysqlresult_gc},
    {NULL, NULL},
};

int luaopen_luamysql(lua_State* l)
{
    /* meta table for mysql client */
    luaL_newmetatable(l, MySQLLib);
    lua_pushvalue(l, -1);
    lua_setfield(l, -2, "__index");
    luaL_setfuncs(l, mysqlclient_m, 0);

    /* meta table for mysql result */
    luaL_newmetatable(l, MySQLResult);
    lua_pushvalue(l, -1);
    lua_setfield(l, -2, "__index");
    luaL_setfuncs(l, mysqlresult_lib, 0);

    luaL_newlib(l, mysqlclient_f);
    return 1;
}
