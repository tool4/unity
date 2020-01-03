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

    if (hinstLib != NULL)
    {
        HELLOPROC hello_proc = (HELLOPROC)GetProcAddress(hinstLib, "PrintHello");
        // If the function address is valid, call the function.
        if (NULL != hello_proc) {
            ret = TRUE;
            char* str = (hello_proc)();
            printf("response from dll: [%s]\n", str);
        }

        for (int i = 0; i < 25; i++)
        {
            pinger::SetLogLevel(0);
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
                ip = "www.unity22.com";
            }

            int handle = pinger::Ping(ip.c_str(), 1000, 41, 10000);
            while (!pinger::PingIsDone(handle))
            {
                Sleep(1);
            }
            if (pinger::PingStatus(handle) == pinger::PING_SUCCESSFUL)
            {
                printf("%s is alive - ping time: %d ms\n", ip.c_str(), pinger::PingTime(handle));
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
