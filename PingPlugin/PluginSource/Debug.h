// Author: tomasz.olejniczak@gmail.com
// All rights reserved.

#ifndef UNITY_PING_PLUGIN_DEBUG_H_FILE
#define UNITY_PING_PLUGIN_DEBUG_H_FILE

#if _MSC_VER
#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#define LOG
#endif

#include "assert.h"

namespace pinger
{

#ifdef _DEBUG
// will simple printf just work when called from dll?
// seems to work fine. TODO: remove debug printf
#define LOG debug_printf
#else
#define LOG
#endif

extern int g_log_level;
enum LOG_LEVEL
{
    LL_OFF = 0,
    LL_NORMAL,
    LL_VERBOSE,
};

#if _MSC_VER
//TODO; make non inline?
inline void debug_printf(LOG_LEVEL ll, const char *fmt, ...)
{
    if (ll <= g_log_level)
    {
        va_list args;
        va_start(args, fmt);
#if _MSC_VER
        int len = _vscprintf(fmt, args) + 1;
#else
        int len = vsnprintf(NULL, 0, fmt, args);
#endif
        va_end(args);

        char* str = new char[len];
        if (str) {
            va_start(args, fmt);
            vsprintf_s(str, len, fmt, args);
            va_end(args);
#if _MSC_VER
            // Get a handle to the console output device.
            void* console = CreateFileW(L"CONOUT$",
                GENERIC_WRITE,
                FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
            if (console != INVALID_HANDLE_VALUE) {
                DWORD bytes_written = 0;
                WriteConsoleA(console, str, len, &bytes_written, NULL);
            }
#else
            printf("%s", str);
#endif
            delete[] str;
        }
    }
}
#endif // _MSC_VER

} // namespace

#endif // UNITY_PING_PLUGIN_DEBUG_H_FILE