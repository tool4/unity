// Author: tomasz.olejniczak@gmail.com
// All rights reserved.

#include "Debug.h"
#include "Pinger.h"
#include "Plugin.h"

namespace pinger
{
// Are c++ exceptions always allowed in unity plugins?
// May not need this if yes.
// Left here to demonstrate how that can be handled. However
// this may be not sufficient, as if exceptions are not supported
// e.g. on android, then we still can run into problems, as the
// code within plugin assumes that exception can be thrown (e.g.
// in cale of out of memory error). So, if we need to support
// platform without exceptions some parts of the plugin may need
// to be rewritten. So far, it's fine as this plugin is supported
// only on windows and linux and they both have exceptions support.
#define EXCEPTIONS_ALLOWED 1
#if EXCEPTIONS_ALLOWED 
    #define TRY             try
    #define CATCH(x)        catch (x)
    #define EXCEPTION_STR   e.what()
#else
    #define TRY             if(1)
    #define CATCH(x)        else if(0)
    #define EXCEPTION_STR   "exceptions disabled"
#endif

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
	return "Hello, hello!";
}

EXPORT_API unsigned int Ping(
    char const * const ip_address,
    unsigned int const timeout,
    unsigned int const num_iterations,
    unsigned int const time_to_live)
{
    unsigned int ip_key = 0;
    TRY {
        CPinger *pinger = CPinger::GetInstance();
        ip_key = pinger->SetupDestAddr(ip_address);
        // GetTickCount may overlap, TODO: GetTickCount64() ??
        int destroy_at = time_to_live + GetTickCount();
        ip_key = pinger->Ping(ip_key, timeout, num_iterations, time_to_live);
    }
    CATCH (std::runtime_error &e) {
        const char *exception_str = EXCEPTION_STR;
        LOG(LL_NORMAL, "%s - Exception caught!:\n%s\n", __FUNCTION__, exception_str);
    }
    CATCH(...) {
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
        TRY {
            CPinger *pinger = CPinger::GetInstance();
            return pinger->IsDone(ping_handle);
        }
        CATCH (std::runtime_error &e) {
            const char *exception_str = EXCEPTION_STR;
            LOG(LL_NORMAL, "%s - Exception caught!:\n%s\n", __FUNCTION__, exception_str);
        }
        CATCH (...) {
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
        TRY {
            CPinger *pinger = CPinger::GetInstance();
            return pinger->Time(ping_handle);
        }
        CATCH (std::runtime_error &e) {
            const char *exception_str = EXCEPTION_STR;
            LOG(LL_NORMAL, "%s - Exception caught!:\n%s\n", __FUNCTION__, exception_str);
        }
        CATCH (...) {
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

    TRY {
        CPinger *pinger = CPinger::GetInstance();
        return pinger->Status(ping_handle);
    }
    CATCH (std::runtime_error &e) {
        const char *exception_str = EXCEPTION_STR;
        LOG(LL_NORMAL, "%s - Exception caught!:\n%s\n", __FUNCTION__, exception_str);
    }
    CATCH (...) {
        LOG(LL_NORMAL, "%s: Unknown exception caught!\n", __FUNCTION__);
    }

    return INVALID_STATUS;
}

EXPORT_API void SetLogLevel(unsigned int const log_level)
{
    g_log_level = log_level;
}

} // namespace

} // end of export C block
