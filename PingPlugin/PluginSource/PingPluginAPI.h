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
    // Set number of ping iterations to be made for single ping request
    //void SetNumIterations();
    // Set timeout value in miliseconds for single ping request
    //void SetTimeout(unsigned int const num_ms);
    // Set time to live for single ping request (after that time ping result 
    // history is going to be removed and IsDone/Status will return false
    // even if last ping succeded (TBD: or maybe status will return some special value)
    //void SetTimeToLive(unsigned int const num_ms);
    // invoke asynchronous ping call to given ip v4 address provided as a c-string
    // in . notation (e.g. 127.0.0.1), other notations are not supported
    EXPORT_API unsigned int Ping(
        char const * const ip_address,
        unsigned int const timeout = 1000,
        unsigned int const num_iterations = 4,
        unsigned int const time_to_live = 10000);

    EXPORT_API bool PingIsDone(unsigned int const ping_handle);

    EXPORT_API int  PingTime(unsigned int const ping_handle);

    EXPORT_API PING_STATUS PingStatus(unsigned int const ping_handle);

    EXPORT_API void SetLogLevel(unsigned int const log_level);
}

}// namespace 

#endif // UNITY_PINGPLUGIN_PINGPLUGINAPI_H_FILE
