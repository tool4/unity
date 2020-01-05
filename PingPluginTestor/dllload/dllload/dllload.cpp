// dllload.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

// A simple program that uses LoadLibrary and 
// GetProcAddress to access myPuts from Myputs.dll. 
// as well as direct calls to statically linked library

#include "../../../PingPlugin/PluginSource/PingPluginAPI.h"
#include <stdio.h> 
#include <string> 
#include <windows.h> 

typedef char* (__cdecl *HELLOPROC)();
typedef bool(__cdecl *PINGPROC)(int, int, int, int);

int main(int argc, char*argv[])
{
    BOOL ret = FALSE;
    HINSTANCE hinstLib = LoadLibrary(TEXT("PingPlugin.dll"));
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

    }

    if (hinstLib != NULL)
    {
        HELLOPROC hello_proc = (HELLOPROC)GetProcAddress(hinstLib, "PrintHello");
        // If the function address is valid, call the function.
        if (NULL != hello_proc) {
            ret = TRUE;
            char* str = (hello_proc)();
            printf("response from dll: [%s]\n", str);
        }

        pinger::SetNumIterations(100);
        pinger::SetTimeout(100);

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
        ret = FreeLibrary(hinstLib);
    }

    return ret;

}
