// Author: tomasz.olejniczak@gmail.com
// All rights reserved.

#include "Debug.h"
#include "Pinger.h"
#include "Plugin.h"

namespace pinger
{

using namespace std;
int g_log_level = 0;

// ------------------------------------------------------------------------
// Ping Plugin:

// Link following functions C-style (required for plugins)
extern "C"
{

// The functions we will call from Unity.

EXPORT_API unsigned int PingSync(char const * const ip_address)
{
    unsigned int ip_key = 0;
    unsigned int ret_value = 0;
    CPinger *pinger = CPinger::GetInstance();
    ip_key = pinger->SetupDestAddr(ip_address);

    unsigned int total_time = 0;
    unsigned int num_iterations = pinger->GetNumIterations();

    for (unsigned int i = 0; i < num_iterations; i++)
    {
        unsigned int time = 0;
        ret_value = pinger->SendPing(ip_key, i, time);
        total_time += time;
    }
    if (ret_value == PING_SUCCESSFUL)
    {
        return total_time / num_iterations;
    }
    else
    {
        return -1;
    }
}


EXPORT_API unsigned int PingAsync(char const * const ip_address)
{
    unsigned int ip_key = 0;
    unsigned int ret_value = 0;
    CPinger *pinger = CPinger::GetInstance();
    ip_key = pinger->SetupDestAddr(ip_address);

    ret_value = pinger->PingAsync(ip_key);

    return ret_value;
}

// Returns true if given ping request has completed.
// input: ping_handle - handle returned by Ping method
EXPORT_API bool PingIsDone(unsigned int const ping_handle)
{
    if (ping_handle != 0)
    {
        CPinger *pinger = CPinger::GetInstance();
        return pinger->IsDone(ping_handle);
    }

    return true;
}

// Returns average time of given ping request in miliseconds.
// input: ping_handle - handle returned by Ping method
EXPORT_API int PingTime(unsigned int const ping_handle)
{
    if (ping_handle != 0)
    {
        CPinger *pinger = CPinger::GetInstance();
        return pinger->Time(ping_handle);
    }

    return -1;
}

// Returns status of given ping request.
// input: ping_handle - handle returned by Ping method
EXPORT_API PING_STATUS PingStatus(unsigned int const ping_handle)
{
    if (ping_handle == 0)
    {
        return INVALID_HANDLE;
    }

    CPinger *pinger = CPinger::GetInstance();
    return pinger->Status(ping_handle);
}

EXPORT_API void SetLogLevel(unsigned int const log_level)
{
    g_log_level = log_level;
}

EXPORT_API void SetTimeout(unsigned int const timeout)
{
    CPinger *pinger = CPinger::GetInstance();
    pinger->SetTimeout(timeout);
}

EXPORT_API void SetNumIterations(unsigned int const num_iterations)
{
    CPinger *pinger = CPinger::GetInstance();
    pinger->SetNumIterations(num_iterations);
}

} // namespace

} // end of export C block
