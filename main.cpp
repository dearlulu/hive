#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <locale>
#include <stdint.h>
#include <signal.h>
#include "luna.h"
#include "tools.h"
#include "socket_mgr.h"
#include "socket_wapper.h"

#ifdef _MSC_VER
#define tzset _tzset
void daemon() {  } // do nothing !
#endif

static bool g_quit_signal = false;
static void on_quit_signal(int signo) { g_quit_signal = true; }
static bool get_guit_signal() { return g_quit_signal; }

class hive_app final
{
public:
	hive_app() { }
	~hive_app() { }
	time_t get_file_time(const char* file_name) { return ::get_file_time(file_name); }
	int64_t get_time_ms() { return ::get_time_ms(); }
	int64_t get_time_ns() { return ::get_time_ns(); }
	void sleep_ms(int ms) { ::sleep_ms(ms); }
	void daemon() { ::daemon(); }
	int create_socket_mgr(lua_State* L) { return ::create_socket_mgr(L); }

public:
	DECLARE_LUA_CLASS(hive_app);
};

EXPORT_CLASS_BEGIN(hive_app)
EXPORT_LUA_FUNCTION(get_file_time)
EXPORT_LUA_FUNCTION(get_time_ms)
EXPORT_LUA_FUNCTION(get_time_ns)
EXPORT_LUA_FUNCTION(sleep_ms)
EXPORT_LUA_FUNCTION(daemon)
EXPORT_LUA_FUNCTION(create_socket_mgr)
EXPORT_CLASS_END()

int main(int argc, const char* argv[])
{
    tzset();
    setlocale(LC_ALL, "");

#if defined(__linux) || defined(__APPLE__)
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
    //signal(SIGINT, on_quit_signal);
    //signal(SIGTERM, on_quit_signal);
#endif
	
	if (argc > 1)
	{
		lua_State* L = luaL_newstate();
		luaL_openlibs(L);
		lua_push_object(L, new hive_app());
		lua_setglobal(L, "hive");
		luaL_dofile(L, argv[1]);
		lua_close(L);
	}

    return 0;
}
