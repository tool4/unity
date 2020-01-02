// Author: tomasz.olejniczak@gmail.com
// All rights reserved.

#ifndef UNITY_PINGPLUGIN_PING_H_FILE
#define UNITY_PINGPLUGIN_PING_H_FILE

#include "debug.h"

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <map>
#include <queue>
#include <thread>

namespace pinger
{

enum PING_STATUS;
class CPinger;

static CPinger* g_s_instance;

extern std::queue<int> queue_;
extern int num_tries_;


class CPinger
{
    // TODO: compress this struct??
    struct SPingStatus
    {
        SPingStatus() :
            SPingStatus(0, 0, 0)
        {
        }
        SPingStatus(int num_tries_arg, int timeout_arg, int destroy_at_arg) :
            done(0),
            status(0),
            time(0),
            num_tries(num_tries_arg),
            timeout(timeout_arg),
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
    int Ping(
        unsigned int const ip_key,
        unsigned int const timeout,
        unsigned int const num_iterations,
        unsigned int const time_to_live);
    bool IsDone(unsigned int const ip_key) const;
    int Time(unsigned int const ip_key) const;
    PING_STATUS Status(unsigned int const ip_key) const;
    static uint32_t make_key(const char * ip_address);
    static CPinger* GetInstance();

private:
    CPinger();
    CPinger(CPinger const&){};
    CPinger& operator=(CPinger const&){};

    int SetupDestAddr(struct sockaddr_in &dest, unsigned int const ip_key) const;
    int WriteRawSocket(struct sockaddr_in &dest, unsigned int &seq_no, unsigned int const data);
    PING_STATUS ReadRawSocket(struct sockaddr_in &dest);
    int PingWorkerThread();

    bool terminate_ = false;
    unsigned int packets_received_;
    unsigned int packets_sent_;

    std::thread worker_thread_;
    SOCKET raw_socket_;
    std::queue<int> queue_;
    int num_tries_;
    std::map<unsigned int, SPingStatus> status_map_;

}; // class Pinger

}

#endif // UNITY_PINGPLUGIN_PING_H_FILE
