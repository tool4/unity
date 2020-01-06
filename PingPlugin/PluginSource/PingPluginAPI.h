// Author: tomasz.olejniczak@gmail.com
// All rights reserved.

#ifndef UNITY_PINGPLUGIN_PINGPLUGINAPI_H_FILE
#define UNITY_PINGPLUGIN_PINGPLUGINAPI_H_FILE

#if _MSC_VER // this is defined when compiling with Visual Studio
#define EXPORT_API __declspec(dllexport) // Visual Studio needs annotating exported functions with this
#else
#define EXPORT_API // XCode does not need annotating exported functions, so define is empty
#endif

namespace pinger
{

enum PING_STATUS
{
    INVALID_STATUS = 0,
    PING_SUCCESSFUL,
    DEST_UNKNOWN,
    PING_TIMEOUT,
    DEST_UNREACHABLE,
    INVALID_HANDLE,
    INVALID_ADDRESS,
};

extern "C"
{
    // API functions:

    // Synchronous version of ping (blocking call)
    // Invokes ping call to given address provided as a c-string
    // (e.g. 127.0.0.1 or www.google.com)
    // params:
    //      - ip_address: destination address
    // return value:
    //      - the average ping time of in miliseconds, or -1 on failure.
    // note: average is calculated over multiple iterations
    //       number of iterations can be set by SetNumIterations() call
    EXPORT_API int PingSync(char const * const ip_address);

    // Asynchronous version of ping command
    // params:
    //      - ip_address: ping destination address
    // return value:
    //      - ping_handle that should be used for completing
    //        asynchronous operation (e.g. for polling the status).
    EXPORT_API unsigned int PingAsync(char const * const ip_address);

    // Check whether PingAsync already completed
    // params:
    //      - ping_handle: handle returned by PingAsync
    // return value:
    //      - true/false
    EXPORT_API bool PingIsDone(unsigned int const ping_handle);

    // Queries the ping time for given ping request
    // params:
    //      - ping_handle: handle returned by PingAsync
    // return value:
    //      - ping time in miliseconds or -1 on failure
    EXPORT_API int  PingTime(unsigned int const ping_handle);

    // Queries the ping status for given ping request
    // params:
    //      - ping_handle: handle returned by PingAsync
    // return value:
    //      - PING_SUCCESSFUL (1) if ping succeded
    //      - Error code (>1) if ping failed
    EXPORT_API int PingStatus(unsigned int const ping_handle);

    // Sets the debug log level for the plugin
    // note: takes effect only for debug builds
    // valid values: 0 = off, 1 = normal, 2+ = verbose
    EXPORT_API void SetLogLevel(unsigned int const log_level);

    // Set timeout value in miliseconds for single ping request
    // default value: 1000 (miliseconds)
    EXPORT_API void SetTimeout(unsigned int const timeout);

    // Set number of ping iterations to be made for single ping request
    // default value: 4
    EXPORT_API void SetNumIterations(unsigned int const num_iterations);

    // Set time to live for single ping request (after that time ping result 
    // status data is going to be removed and IsDone/Status/Time will fail
    // even if last ping succeded.
    // Not implemented yet - calls to PingTime are removing stored data for
    // given ping request at the moment.
    //void SetTimeToLive(unsigned int const num_ms);
}

}// namespace 

#endif // UNITY_PINGPLUGIN_PINGPLUGINAPI_H_FILE
