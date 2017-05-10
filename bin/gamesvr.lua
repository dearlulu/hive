#!/usr/bin/luna
--gamesvr示例

router_helper = import("router_helper.lua");

_G.s2s = s2s or {}; --所有server间的rpc定义在s2s中

function s2s.test(param)
    print("param");
end

last_test = 0;
function test_msg(now)
    if now - last_test > 2000 then
        call_dbagent("some key", "test", now);
        last_test = now;
    end
end

function main()
	socket_mgr = luna.create_socket_mgr(100);

    router_helper.setup(socket_mgr, "gamesvr", 1);

    local next_reload_time = 0;
	local quit_flag = false;
    while not quit_flag do
		socket_mgr.wait(50);

        local now = luna.get_time_ms();
        if now >= next_reload_time then
            luna.try_reload();
            next_reload_time = now + 3000;
        end

        router_helper.update(now);

        test_msg(now);

        quit_flag = luna.get_guit_signal();
    end
end
