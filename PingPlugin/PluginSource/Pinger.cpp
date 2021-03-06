// Author: tomasz.olejniczak@gmail.com
// All rights reserved.

#include "Pinger.h"
#include "ICMP.h"
#include "PingPluginAPI.h"
#include <thread>
#include <string.h>

#ifdef WIN32
// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma warning(disable:4996)
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#endif

using namespace icmp;
namespace pinger
{
static CPinger* g_s_instance;

CPinger::CPinger() :
    worker_thread_([=] { PingWorkerThread(); }),
    raw_socket_(INVALID_SOCKET),
    timeout_(1000),
    num_iterations_(4)
{
}

CPinger::~CPinger() 
{
    terminate_ = true;
    worker_thread_.join();
}

CPinger* CPinger::GetInstance()
{
    TRY
    {
        // Singleton object - allow only one instance of this class to be
        // generated (since we want to reuse already opened socket).
        if (!g_s_instance)
        {
            g_s_instance = new CPinger();
            g_s_instance->Initialize();
        }
    }
    CATCH(std::runtime_error &e)
    {
        const char *exception_str = EXCEPTION_STR;
        LOG(LL_NORMAL, "%s - Exception caught!:\n%s\n",
            __FUNCTION__, exception_str);
    }
    CATCH(...)
    {
        LOG(LL_NORMAL, "%s: Unknown exception caught!\n", __FUNCTION__);
    }

    return g_s_instance;
}

void CPinger::RemoveInstance()
{
    if (g_s_instance)
    {
        delete g_s_instance;
        g_s_instance = NULL;
    }
}

int CPinger::PingAsync(unsigned int const ip_key)
{
    TRY
    {
        queue_.push(ip_key);
        status_map_[ip_key] = SPingStatus();
    }
    CATCH(std::runtime_error &e)
    {
        const char *exception_str = EXCEPTION_STR;
        LOG(LL_NORMAL, "%s - Exception caught!:\n%s\n",
            __FUNCTION__, exception_str);
    }
    CATCH(...)
    {
        LOG(LL_NORMAL, "%s: Unknown exception caught!\n", __FUNCTION__);
    }
    return ip_key;
}

bool CPinger::IsDone(unsigned int const ip_key) const
{
    auto it = status_map_.find(ip_key);
    if (it != status_map_.end())
    {
        return (it->second.done == 1 ? true : false);
    }
    return true;
}

// Successful calls to this function erase the stored status
// data (another call with the same ip_key is going to fail).
// This is not best choice as with improper usage may lead to
// leaking the memory, used here only temporarily.
// TODO: add a thread responsible for cleaning old stored data,
// it should remove data unused for given period of time.
int CPinger::Time(unsigned int const ip_key)
{
    auto it = status_map_.find(ip_key);
    if (it != status_map_.end())
    {
        int time = -1;
        if (it->second.done)
        {
            if (it->second.status == PING_SUCCESSFUL)
            {
                time = it->second.time;
            }
            status_map_.erase(it);
        }
        return time;
    }
    return -1;
}

PING_STATUS CPinger::Status(unsigned int const ip_key) const
{
    auto it = status_map_.find(ip_key);
    if (it != status_map_.end())
    {
        return static_cast<PING_STATUS>(it->second.status);
    }
    return INVALID_HANDLE;
}

int CPinger::UpdateTimeouts()
{
    int ret = 0;

#ifdef WIN32
    unsigned int timeout = timeout_;
#else
    struct timeval timeout =
    {
        static_cast<long int>(timeout_) / 1000,
        (static_cast<long int>(timeout_) % 1000) * 1000
    };
#endif

    if (raw_socket_)
    {
        // Set the recv timeout value
        ret = setsockopt(
            raw_socket_,
            SOL_SOCKET,
            SO_RCVTIMEO,
            reinterpret_cast<char*>(&timeout),
            sizeof(timeout));

        if (ret == SOCKET_ERROR) {
            LOG(LL_NORMAL, "setsockopt(SO_RCVTIMEO) failed: %d\n", GetLastError());
            return -1;
        }

        // Set the send timeout value
        ret = setsockopt(
            raw_socket_,
            SOL_SOCKET,
            SO_SNDTIMEO,
            reinterpret_cast<char*>(&timeout),
            sizeof(timeout));

        if (ret == SOCKET_ERROR) {
            LOG(LL_NORMAL, "setsockopt(SO_SNDTIMEO) failed: %d\n", GetLastError());
            return -1;
        }
    }

    return ret;
}

int CPinger::Initialize()
{
#ifdef WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        LOG(LL_NORMAL, "WSAStartup() failed: %d\n", GetLastError());
        return -1;
    }
#endif
    memset(&dest_, 0, sizeof(dest_));

    raw_socket_ = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    if (raw_socket_ == INVALID_SOCKET) {
        LOG(LL_NORMAL, "Raw socket creation failed (insufficient access rights?)\n");
        LOG(LL_NORMAL, "Error code returned: %d \n", GetLastError());
        // This may fail if process has no super user rights
        // TODO: Fallback to TCP connection to workaround this?
        return -1;
    }

    return UpdateTimeouts();;
}

int CPinger::SendPing(
    unsigned int const ip_key,
    unsigned int const seq_no,
    unsigned int &time)
{
    int ret_value = WriteRawSocket(seq_no, ip_key);
    if (ret_value != SOCKET_ERROR)
    {
        ret_value = ReadRawSocket(time);
    }
    return ret_value;
}

int CPinger::PingWorkerThread()
{
    TRY
    {
        LOG(LL_NORMAL, "%s has started\n", __FUNCTION__);
        while (!terminate_)
        {
            while (!queue_.empty())
            {
                int ret_value = 0;
                unsigned int seq_no = 0;
                unsigned int total_time = 0;
                int ip_key = queue_.front();
                queue_.pop();

                SetupDestAddr(ip_key);

                for (unsigned int p = 0; p < num_iterations_; p++) {
                    unsigned int time = 0;
                    ret_value = SendPing(ip_key, seq_no, time);
                    total_time += time;
                    seq_no++;
                }
                auto it = status_map_.find(ip_key);
                if (it != status_map_.end())
                {
                    it->second.done = true;
                    it->second.status = ret_value;
                    it->second.time = total_time / num_iterations_;
                }
            }
            std::this_thread::yield();
        }
    }
    CATCH(std::runtime_error &e)
    {
        const char *exception_str = EXCEPTION_STR;
        LOG(LL_NORMAL, "%s - Exception caught!:\n%s\n",
            __FUNCTION__, exception_str);
    }
    CATCH(...)
    {
        LOG(LL_NORMAL, "%s: Unknown exception caught!\n", __FUNCTION__);
    }
    return 0;
}

// Setups the dest_ struct based on address passed as string
// returns ipv4 addr encoded on 32bit var
// e.g. 0x0A00000A for 10.0.0.10 or 0 on failure
int CPinger::SetupDestAddr(char const * const addr_str)
{
    dest_.sin_family = AF_INET;
    if ((dest_.sin_addr.s_addr = inet_addr(addr_str)) == INADDR_NONE)
    {
        struct hostent *hp = NULL;
        if ((hp = gethostbyname(addr_str)) != NULL)
        {
            memcpy(&(dest_.sin_addr), hp->h_addr, hp->h_length);
            dest_.sin_family = hp->h_addrtype;
        }
        else
        {
            LOG(LL_NORMAL, "gethostbyname() failed: %d\n", GetLastError());
            // 0.0.0.0 is an invalid ip address, 0 is fine for error signaling
            return 0;
        }
    }
    return dest_.sin_addr.s_addr;
}

int CPinger::SetupDestAddr(unsigned int const ip_key)
{
    dest_.sin_addr.s_addr = ip_key;
    return ip_key;
}

int CPinger::WriteRawSocket(
    unsigned int const seq_no,
    unsigned int const data)
{
    ICMPPacket icmp_packet = ICMPPacket(
        GetTickCount(),
        static_cast<uint16_t>(GetCurrentProcessId()),
        static_cast<uint16_t>(seq_no),
        data);

    LOG(LL_NORMAL, "sending: %d bytes\n", sizeof(ICMPPacket));

    int ret = sendto(
        raw_socket_,
        (char*)(&icmp_packet),
        sizeof(ICMPPacket),
        0,
        (struct sockaddr*)&dest_,
        sizeof(dest_));

    if (ret == SOCKET_ERROR) {
        LOG(LL_NORMAL,
            "socket: %08X, dest: %s - sendto() failed, error code: %d\n",
            raw_socket_, inet_ntoa(dest_.sin_addr), GetLastError());
    }
    else if (ret < static_cast<int>(sizeof(ICMPPacket))) {
        LOG(LL_NORMAL,
            "%s Wrote %d/%d bytes\n",
            inet_ntoa(dest_.sin_addr), ret, sizeof(ICMPPacket));
    }
    return ret;
}

PING_STATUS CPinger::ReadRawSocket(unsigned int &time)
{
    struct sockaddr_in from;
    int from_len = sizeof(from);
    char buf[sizeof(IPHeader) + sizeof(ICMPPacket)];
    time = 0;

    int ret_value = recvfrom(
        raw_socket_,
        buf,
        sizeof(buf),
        0,
        (struct sockaddr*)&from,
        (socklen_t*)&from_len);

    if (ret_value == SOCKET_ERROR) {
#ifdef WIN32
        if (GetLastError() == WSAETIMEDOUT){
            LOG(LL_NORMAL, "%s timed out\n", inet_ntoa(from.sin_addr));
            return PING_TIMEOUT;
        }
#endif
        LOG(LL_NORMAL, "recvfrom %s - SOCKET_ERROR, error code: %d\n",
            inet_ntoa(from.sin_addr), GetLastError());
            return DEST_UNREACHABLE;
    }

    uint32_t tick = GetTickCount();
    uint32_t data = 0xDEADBEEF;
    IPHeader *iphdr = reinterpret_cast<IPHeader *>(buf);
    ICMPPacket *icmp_pkt = reinterpret_cast<ICMPPacket *>(
    buf + iphdr->h_len * sizeof(uint32_t));

    if (icmp_pkt->header.i_type == ICMP_ECHOREPLY) {
        if (icmp_pkt->header.i_id != (uint16_t)GetCurrentProcessId()) {
            LOG(LL_NORMAL, "received someone else's packet!\n");
            return DEST_UNKNOWN;
        }
        else {
            time = tick - icmp_pkt->timestamp;
            LOG(LL_NORMAL,
                "received %3d bytes from %12s: icmp_seq = %d. time: %d ms\n",
                ret_value,
                inet_ntoa(from.sin_addr),
                icmp_pkt->header.i_seq,
                tick - icmp_pkt->timestamp);
            return PING_SUCCESSFUL;
        }
    }
    else {
        if (icmp_pkt->header.i_type == 3) {
            LOG(LL_NORMAL, "%08X Destination unreachable (from: %08X)\n",
                data,
                from.sin_addr);
            return DEST_UNREACHABLE;
        }
        else {
            LOG(LL_NORMAL, "%s - received IMCP packet of unexpected type: %d\n",
                inet_ntoa(from.sin_addr),
                icmp_pkt->header.i_type);
            return DEST_UNKNOWN;
        }
    }
}

void CPinger::SetTimeout(unsigned int const timeout)
{
    timeout_ = timeout;
    UpdateTimeouts();
}

void CPinger::SetNumIterations(unsigned int const num_iterations)
{
    num_iterations_ = num_iterations;
}

unsigned int CPinger::GetNumIterations() const
{
    return num_iterations_;
}

} // namespace
