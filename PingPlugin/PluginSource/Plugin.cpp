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

// Synchronous version of ping (blocking call)
// Invokes ping request to given address provided as a c-string
// (e.g. 127.0.0.1 or www.google.com)
// params:
//      - ip_address: destination address
// return value:
//      - the average ping time of in miliseconds, or -1 on failure.
// note: average is calculated over multiple iterations
//       number of iterations can be set by SetNumIterations() call
EXPORT_API int PingSync(char const * const ip_address)
{
    unsigned int ip_key = 0;
    unsigned int ret_value = 0;
    CPinger *pinger = CPinger::GetInstance();
    ip_key = pinger->SetupDestAddr(ip_address);

    if (ip_key == 0)
    {
        return -1;
    }

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

// Asynchronous version of ping command
// params:
//      - ip_address: ping destination address
// return value:
//      - ip_key value (handle) that should be used for completing
//        asynchronous operation (e.g. for polling the status).
EXPORT_API unsigned int PingAsync(char const * const ip_address)
{
    CPinger *pinger = CPinger::GetInstance();

    unsigned int ip_key = pinger->SetupDestAddr(ip_address);
    if (ip_key != INVALID_ADDRESS)
    {
        return pinger->PingAsync(ip_key);
    }
    else
    {
        return INVALID_ADDRESS;
    }
}

// Check whether PingAsync already completed
// params:
//      - ping_handle: handle returned by PingAsync
// return value:
//      - true/false
EXPORT_API bool PingIsDone(unsigned int const ping_handle)
{
    if (ping_handle != 0)
    {
        CPinger *pinger = CPinger::GetInstance();
        return pinger->IsDone(ping_handle);
    }
    return true;
}

// Queries the ping time for given ping request
// params:
//      - ping_handle: handle returned by PingAsync
// return value:
//      - ping time in miliseconds or -1 on failure
EXPORT_API int PingTime(unsigned int const ping_handle)
{
    if (ping_handle != 0)
    {
        CPinger *pinger = CPinger::GetInstance();
        return pinger->Time(ping_handle);
    }
    return -1;
}

// Queries the ping status for given ping request
// params:
//      - ping_handle: handle returned by PingAsync
// return value:
//      - PING_SUCCESSFUL (1) if ping succeded
//      - Error code (>1) if ping failed
EXPORT_API int PingStatus(unsigned int const ping_handle)
{
    if (ping_handle == 0)
    {
        return INVALID_HANDLE;
    }

    CPinger *pinger = CPinger::GetInstance();
    return pinger->Status(ping_handle);
}

// Sets the debug log level for the plugin
// note: takes effect only for debug builds
// valid values: 0 = off, 1 = normal, 2+ = verbose
EXPORT_API void SetLogLevel(unsigned int const log_level)
{
    g_log_level = log_level;
}

// Set timeout value in miliseconds for single iteration of ping request
// default value: 1000 (miliseconds)
EXPORT_API void SetTimeout(unsigned int const timeout)
{
    CPinger *pinger = CPinger::GetInstance();
    pinger->SetTimeout(timeout);
}

// Set number of iterations to be made for ping request
// default value: 4
EXPORT_API void SetNumIterations(unsigned int const num_iterations)
{
    CPinger *pinger = CPinger::GetInstance();
    pinger->SetNumIterations(num_iterations);
}

} // namespace

} // end of export C block
