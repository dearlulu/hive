/*
** repository: https://github.com/trumanzhao/luna
** trumanzhao, 2017-05-13, trumanzhao@foxmail.com
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <locale>
#include <stdint.h>
#include <signal.h>
#include "tools.h"
#include "socket_mgr.h"
#include "socket_wapper.h"
#include "hive.h"

#ifdef _MSC_VER
void daemon() {  } // do nothing !
#endif

hive_app* g_app = nullptr;

static void on_signal(int signo)
{
    if (g_app)
    {
        g_app->set_signal(signo);
    }
}

EXPORT_CLASS_BEGIN(hive_app)
EXPORT_LUA_FUNCTION(get_file_time)
EXPORT_LUA_FUNCTION(get_time_ms)
EXPORT_LUA_FUNCTION(get_time_ns)
EXPORT_LUA_FUNCTION(sleep_ms)
EXPORT_LUA_FUNCTION(daemon)
EXPORT_LUA_FUNCTION(register_signal)
EXPORT_LUA_FUNCTION(default_signal)
EXPORT_LUA_FUNCTION(ignore_signal)
EXPORT_LUA_FUNCTION(create_socket_mgr)
EXPORT_LUA_INT64(m_signal)
EXPORT_LUA_INT(m_reload_time)
EXPORT_CLASS_END()

time_t hive_app::get_file_time(const char* file_name)
{
    return ::get_file_time(file_name);
}

int64_t hive_app::get_time_ms()
{
    return ::get_time_ms();
}

int64_t hive_app::get_time_ns()
{
    return ::get_time_ns();
}

void hive_app::sleep_ms(int ms)
{
    ::sleep_ms(ms);
}

void hive_app::daemon()
{
    ::daemon();
}

void hive_app::register_signal(int n)
{
    signal(n, SIG_DFL);
}

void hive_app::default_signal(int n)
{
    signal(n, SIG_DFL);
}

void hive_app::ignore_signal(int n)
{
    signal(n, SIG_IGN);
}

int hive_app::create_socket_mgr(lua_State* L)
{
    return ::create_socket_mgr(L);
}

void hive_app::set_signal(int n)
{
    uint64_t mask = 1;
    mask <<= n;
    m_signal |= mask;
}

static const char* g_sandbox = u8R"__(
hive.files = {};
hive.meta = {__index=function(t, k) return _G[k]; end};
hive.print = print;

local do_load = function(filename, env)
    local trunk, msg = loadfile(filename, "bt", env);
    if not trunk then
        hive.print(string.format("load file: %s ... ... [failed]", filename));
        hive.print(msg);
        return nil;
    end

    local ok, err = pcall(trunk);
    if not ok then
        hive.print(string.format("exec file: %s ... ... [failed]", filename));
        hive.print(err);
        return nil;
    end

    hive.print(string.format("load file: %s ... ... [ok]", filename));
    return env;
end

function import(filename)
    local file_module = hive.files[filename];
    if file_module then
        return file_module.env;
    end

    local env = {};
    setmetatable(env, hive.meta);
    hive.files[filename] = {time=hive.get_file_time(filename), env=env };

    return do_load(filename, env);
end

hive.reload = function()
    for filename, filenode in pairs(hive.files) do
        local filetime = hive.get_file_time(filename);
        if filetime ~= filenode.time then
            filenode.time = filetime;
            if filetime ~= 0 then
                do_load(filename, filenode.env);
            end
        end
    end
end
)__";

void hive_app::run(const char filename[])
{
    lua_State* L = luaL_newstate();
    int64_t last_check = ::get_time_ms();

    luaL_openlibs(L);
    lua_push_object(L, this);
    lua_setglobal(L, "hive");
    luaL_dostring(L, g_sandbox);

    lua_call_global_function(L, "import", std::tie(), filename);

    while (lua_call_object_function(L, this, "run"))
    {
        int64_t now = ::get_time_ms();
        if (now > last_check + m_reload_time)
        {
            lua_call_object_function(L, this, "reload");
            last_check = now;
        }
    }

    lua_close(L);
}
