// Author: tomasz.olejniczak@gmail.com
// All rights reserved.

#ifndef UNITY_PINGPLUGIN_PINGER_H_FILE
#define UNITY_PINGPLUGIN_PINGER_H_FILE

#include <map>
#include <queue>
#include <thread>
#include "Debug.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#elif __linux__
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "PingPluginAPI.h"
#define INVALID_SOCKET  (~0)
#define SOCKET_ERROR    (-1)
namespace pinger
{
inline unsigned int GetTickCount()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

inline unsigned int GetCurrentProcessId()
{
    return static_cast<unsigned int>(getpid());
}

} //namespace
#else
static_assert(0, "Unsupported platform");
#endif // WIN32/__linux__

namespace pinger
{

inline unsigned int GetLastError()
{
#ifdef WIN32
    return WSAGetLastError();
#elif __linux__
    return errno;
#else
    static_assert(0, "Unsupported platform");
#endif
}

enum PING_STATUS;
class CPinger;

class CPinger
{
    struct SPingStatus
    {
        SPingStatus() :
            done(0),
            status(0),
            time(0)
        {
        }
        int done;
        int status;
        int time;
    };

public:
    ~CPinger();

    static CPinger* GetInstance();
    static void RemoveInstance();

    int Initialize();
    int PingAsync(unsigned int const ip_key);
    bool IsDone(unsigned int const ip_key) const;
    int Time(unsigned int const ip_key);
    PING_STATUS Status(unsigned int const ip_key) const;
    int SetupDestAddr(char const * const addr_str);
    int SendPing(
        unsigned int const ip_key,
        unsigned int const seq_no,
        unsigned int &time);
    void SetTimeout(unsigned int const timeout);
    void SetNumIterations(unsigned int const num_iterations);
    unsigned int GetNumIterations() const;

private:
    CPinger();
    CPinger(CPinger const&) = delete;
    CPinger& operator=(CPinger const&) = delete;
    int SetupDestAddr(unsigned int const ip_key);
    int WriteRawSocket(
        unsigned int const seq_no,
        unsigned int const data);
    PING_STATUS ReadRawSocket(unsigned int &time);
    int PingWorkerThread();
    int UpdateTimeouts();

    bool terminate_ = false;
    std::thread worker_thread_;
    int raw_socket_;
    struct sockaddr_in dest_;
    std::queue<int> queue_;
    std::map<unsigned int, SPingStatus> status_map_;
    unsigned int timeout_;
    unsigned int num_iterations_;

}; // class Pinger

}

#endif // UNITY_PINGPLUGIN_PINGER_H_FILE
