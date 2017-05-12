/*
** repository: https://github.com/trumanzhao/luna
** trumanzhao, 2017-05-13, trumanzhao@foxmail.com
*/

#include <stdio.h>
#include <locale>
#include "tools.h"
#include "hive.h"

int main(int argc, const char* argv[])
{
    tzset();
    setlocale(LC_ALL, "");

    if (argc < 2)
    {
        puts("hive your_entry.lua ...");
        return 1;
    }

    g_app = new hive_app();
    g_app->run(argv[1]);
    delete g_app;
    return 0;
}
