// unit test program for pingplugin dll/so

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
inline void Sleep(unsigned int miliseconds)
{
    usleep(miliseconds * 1000);
}
#endif

#include "../../../PingPlugin/PluginSource/PingPluginAPI.h"
#include <stdio.h>
#include <string.h>
#include <string>

int main(int argc, char*argv[])
{
    int ret = 0;
    int num_iterations = 1;
    int timeout = 1000;
    bool async = true;
    pinger::SetLogLevel(0);
    for (int i = 1; i< argc; i++)
    {
        if (strcmp(argv[i], "sync") == 0)
        {
            printf("synchronous operation enabled\n");
            async = false;
        }
        else if (strcmp(argv[i], "debug") == 0)
        {
            printf("debug mode enabled\n");
            pinger::SetLogLevel(4);
        }
        else if (argc > (i + 1) &&
                 strcmp(argv[i], "-num_iter") == 0)
        {
            num_iterations = atoi(argv[i+1]);
            ++i;
            printf("num iterations set to %d\n", num_iterations);
        }
        else if (argc > (i + 1) &&
                 strcmp(argv[i], "-timeout") == 0)
        {
            timeout = atoi(argv[i+1]);
            ++i;
            printf("timeout set to %d\n", num_iterations);
        }
    }

    {
        pinger::SetNumIterations(num_iterations);
        pinger::SetTimeout(timeout);

        for (int i = 0; i < 25; i++)
        {
            std::string ip = "192.168.0.";
            ip += std::to_string(i);

            if (i == 19)
            {
                ip = "bad string";
            }
            else if (i == 20)
            {
                ip = "www.google.com";
            }
            else if (i == 21)
            {
                ip = "www.unity.com";
            }
            else if (i == 22)
            {
                ip = "www.not.existing.address.com";
            }
            else if (i == 23)
            {
                ip = "www.wp.pl";
            }

            int time = 0;
            int status = pinger::PING_SUCCESSFUL;

            if (async)
            {
                int handle = pinger::PingAsync(ip.c_str());
                while (!pinger::PingIsDone(handle))
                {
                    Sleep(1);
                }
                status = pinger::PingStatus(handle);
                time = pinger::PingTime(handle);
            }
            else
            {
                time = pinger::PingSync(ip.c_str());
                if (time == -1)
                {
                    status = pinger::DEST_UNREACHABLE;
                }
            }

            if (status == pinger::PING_SUCCESSFUL)
            {
                printf("%s is alive - ping time: %d ms\n", ip.c_str(), time);
            }
            else
            {
                printf("%s is not reachable\n", ip.c_str());
            }
        }
    }

    return ret;
}
