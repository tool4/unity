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
    DEST_UNKNOWN,
    PING_SUCCESSFUL,
    PING_FAILED,
    PING_TIMEOUT,
    DEST_UNREACHABLE,
    ENV_ERROR,
    INVALID_HANDLE,
    INVALID_ADDRESS,
};

extern "C"
{
    // API functions:

    // TODO: update comments

    // invoke ping call to given ip v4 address provided as a c-string
    // (e.g. 127.0.0.1 or www.google.com)
    // params:
    //      - ip_address: ping destination address
    //      - async - if true asynchronous ping will be executed, synchronous otherwise
    // return value:
    //      - for synchronous (blocking) operation the average time of ping will be 
    //        returned in miliseconds, or -1 on failure.
    EXPORT_API unsigned int PingSync(char const * const ip_address);
    //      - for asynchronous the ip_key value (handle) will be returned that should be
    //         used for completing asynchronous operation (e.g. for polling the status).
    EXPORT_API unsigned int PingAsync(char const * const ip_address);

    EXPORT_API bool PingIsDone(unsigned int const ping_handle);

    EXPORT_API int  PingTime(unsigned int const ping_handle);

    EXPORT_API PING_STATUS PingStatus(unsigned int const ping_handle);

    EXPORT_API void SetLogLevel(unsigned int const log_level);

    // Set timeout value in miliseconds for single ping request
    EXPORT_API void SetTimeout(unsigned int const timeout);

    // Set number of ping iterations to be made for single ping request
    EXPORT_API void SetNumIterations(unsigned int const num_iterations);

    // Set time to live for single ping request (after that time ping result 
    // history is going to be removed and IsDone/Status will return false
    // even if last ping succeded (TBD: or maybe status will return some special value)
    //void SetTimeToLive(unsigned int const num_ms);
}

}// namespace 

#endif // UNITY_PINGPLUGIN_PINGPLUGINAPI_H_FILE
