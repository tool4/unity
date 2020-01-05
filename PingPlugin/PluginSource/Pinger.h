// Author: tomasz.olejniczak@gmail.com
// All rights reserved.

#ifndef UNITY_PINGPLUGIN_PING_H_FILE
#define UNITY_PINGPLUGIN_PING_H_FILE

#include <map>
#include <queue>
#include <thread>
#include "Debug.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "PingPluginAPI.h"

#define INVALID_SOCKET  (~0)
#define SOCKET_ERROR    (-1)
#endif

namespace pinger
{

enum PING_STATUS;
class CPinger;

class CPinger
{
    // TODO: compress this struct??
    struct SPingStatus
    {
        SPingStatus() :
            SPingStatus(0)
        {
        }
        SPingStatus(int destroy_at_arg) :
            done(0),
            status(0),
            time(0),
            destroy_at(destroy_at_arg)
        {
        }

        int done;
        int status;
        int time;
        int num_tries;
        int timeout;
        int destroy_at;
    };

public:
    ~CPinger();
    int Initialize();
    int PingAsync(
        unsigned int const ip_key,
        unsigned int const time_to_live = 10000);
    bool IsDone(unsigned int const ip_key) const;
    int Time(unsigned int const ip_key);
    PING_STATUS Status(unsigned int const ip_key) const;
    static CPinger* GetInstance();
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

    bool terminate_ = false;
    unsigned int packets_received_;
    unsigned int packets_sent_;

    std::thread worker_thread_;
    int raw_socket_;
    struct sockaddr_in dest_;
    std::queue<int> queue_;
    std::map<unsigned int, SPingStatus> status_map_;
    unsigned int timeout_;
    unsigned int num_iterations_;

}; // class Pinger

}

#endif // UNITY_PINGPLUGIN_PING_H_FILE
