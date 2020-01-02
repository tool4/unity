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

EXPORT_API const char* PrintHello()
{
    LOG(LL_NORMAL, "PrintHello called!\n");
	return "Hello aloha, hola!";
}

EXPORT_API unsigned int Ping(
    char const * const ip_address,
    unsigned int const timeout,
    unsigned int const num_iterations,
    unsigned int const time_to_live)
{
    unsigned int ip_key = 0;
    try {
        CPinger *pinger = CPinger::GetInstance();
        ip_key = CPinger::make_key(ip_address);
        int destroy_at = time_to_live + GetTickCount(); //TODO: GetTickCount64() ??
        ip_key = pinger->Ping(ip_key, timeout, num_iterations, time_to_live);
    }
    catch (std::runtime_error &e) {
        LOG(LL_NORMAL, "%s - Exception caught!:\n%s\n", __FUNCTION__, e.what());
    }
    catch (...) {
        LOG(LL_NORMAL, "%s: Unknown exception caught!\n", __FUNCTION__);
    }

    return ip_key;
}

// Returns true if given ping request has completed.
// input: ping_handle - handle returned by Ping method
EXPORT_API bool PingIsDone(unsigned int const ping_handle)
{
    if (ping_handle != 0)
    {
        try {
            CPinger *pinger = CPinger::GetInstance();
            return pinger->IsDone(ping_handle);
        }
        catch (std::runtime_error &e) {
            LOG(LL_NORMAL, "%s - Exception caught!:\n%s\n", __FUNCTION__, e.what());
        }
        catch (...) {
            LOG(LL_NORMAL, "%s: Unknown exception caught!\n", __FUNCTION__);
        }
    }

    return true;
}

// Returns average time of given ping request in miliseconds.
// input: ping_handle - handle returned by Ping method
EXPORT_API int PingTime(unsigned int const ping_handle)
{
    if (ping_handle != 0)
    {
        try {
            CPinger *pinger = CPinger::GetInstance();
            return pinger->Time(ping_handle);
        }
        catch (std::runtime_error &e) {
            LOG(LL_NORMAL, "%s - Exception caught!:\n%s\n", __FUNCTION__, e.what());
        }
        catch (...) {
            LOG(LL_NORMAL, "%s: Unknown exception caught!\n", __FUNCTION__);
        }
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

    try {
        CPinger *pinger = CPinger::GetInstance();
        return pinger->Status(ping_handle);
    }
    catch (std::runtime_error &e) {
        LOG(LL_NORMAL, "Exception caught!:\n%s\n", e.what());
    }
    catch (...) {
        LOG(LL_NORMAL, "%s: Unknown exception caught!\n", __FUNCTION__);
    }

    return INVALID_STATUS;
}

EXPORT_API void SetLogLevel(unsigned int const log_level)
{
    g_log_level = log_level;
}

}
} // end of export C block
