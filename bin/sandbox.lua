hive.file_list = hive.file_list or {};
hive.file_meta = {__index=function(t, k) return _G[k]; end};

function import(filename)
    local file_module = hive.file_list[filename];
    if file_module then
        return file_module.env;
    end

    local env = {};
    setmetatable(env, luna.meta);

    local trunk, msg = loadfile(filename, "bt", env);
    if not trunk then
        luna.print(string.format("load file: %s ... ... [failed]", filename));
        luna.print(msg);
        return nil;
    end

    local file_time = luna.get_file_time(filename);
    hive.file_list[filename] = {filename=filename, time=file_time, env=env };

    local ok, err = pcall(trunk);
    if not ok then
        luna.print(string.format("load file: %s ... ... [failed]", filename));
        luna.print(err);
        return nil;
    end

    luna.print(string.format("load file: %s ... ... [ok]", filename));
    return env;
end

local reload_script = function(filename, env)
    local trunk, msg = loadfile(filename, "bt", env);
    if not trunk then
        return false, msg;
    end

    local ok, err = pcall(trunk);
    if not ok then
        return false, err;
    end
    return true, nil;
end

luna.try_reload = function()
    for filename, filenode in pairs(hive.file_list) do
        local filetime = luna.get_file_time(filename);
        if filetime ~= 0 and filetime ~= filenode.time then
            filenode.time = filetime;
            local ok, err = reload_script(filename, filenode.env);
            luna.print(string.format("load file: %s ... ... [%s]", filename, ok and "ok" or "failed"));
            if not ok then
                luna.print(err);
            end
        end
    end
end
