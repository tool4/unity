// Author: tomasz.olejniczak@gmail.com
// All rights reserved.

#include "Pinger.h"
#include "ICMP.h"
#include "PingPluginAPI.h"
#include <thread>

// TODO: clean this up! (remove?)
// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma warning(disable:4996)

using namespace icmp;
namespace pinger
{

CPinger::CPinger() :
    packets_received_(0),
    packets_sent_(0),
    raw_socket_(INVALID_SOCKET),
    worker_thread_([=] { PingWorkerThread(); }),
    num_tries_(4)
{
}

CPinger::~CPinger() 
{
    terminate_ = true;
    worker_thread_.join();
}

CPinger* CPinger::GetInstance()
{
    // Singleton object - allow only one instance of this class to be
    // generated (since we want to reuse already opened socket).
    if (!g_s_instance) {
        g_s_instance = new CPinger();
        g_s_instance->Initialize();
    }
    return g_s_instance;
}

int CPinger::Ping(
    unsigned int const ip_key,
    unsigned int const timeout = 1000,
    unsigned int const num_iterations = 4,
    unsigned int const time_to_live = 10000)
{
    queue_.push(ip_key);
    status_map_[ip_key] = SPingStatus(num_tries_, timeout, time_to_live);
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

int CPinger::Time(unsigned int const ip_key) const
{
    return 1;
}

PING_STATUS CPinger::Status(unsigned int const ip_key) const
{
    auto it = status_map_.find(ip_key);
    if (it != status_map_.end())
    {
        return static_cast<PING_STATUS>(it->second.status);
    }
    return PING_SUCCESSFUL;
}

int CPinger::Initialize()
{
    WSADATA wsaData;
    int timeout = 100;
    unsigned int addr = 0;
    struct hostent *hp = NULL;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        LOG(LL_NORMAL, "WSAStartup() failed: %d\n", GetLastError());
        return -1;
    }

    raw_socket_ = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    if (raw_socket_ == INVALID_SOCKET) {
        LOG(LL_NORMAL, "WSASocket() failed: %d\n", WSAGetLastError());
        // This may fail if process has no super user rights
        // fall back TCP connection to workaround this?
        return -1;
    }
    // Set the send/recv timeout values
    int ret = setsockopt(raw_socket_, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    if (ret == SOCKET_ERROR) {
        LOG(LL_NORMAL, "setsockopt(SO_RCVTIMEO) failed: %d\n",
            WSAGetLastError());
        return -1;
    }

    ret = setsockopt(raw_socket_, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));

    if (ret == SOCKET_ERROR) {
        LOG(LL_NORMAL, "setsockopt(SO_SNDTIMEO) failed: %d\n",
            WSAGetLastError());
        return -1;
    }

    memset(&dest_, 0, sizeof(dest_));

    return 0;
}

int CPinger::PingWorkerThread()
{
    LOG(LL_NORMAL, "%s has started\n", __FUNCTION__);

    while (!terminate_)
    {
        while (!queue_.empty())
        {
            int read_status = 0;
            int write_status = 0;
            unsigned int seq_no = 0;
            int ip_key = queue_.front();
            queue_.pop();

            SetupDestAddr(ip_key);

            for (int p = 0; p < num_tries_; p++) {
                write_status = WriteRawSocket(seq_no, ip_key);
                if (write_status == sizeof(dest_))
                {
                    ++packets_sent_;
                    // todo: clean me (read status will be overwritten)
                    read_status = ReadRawSocket();
                }
            }
            auto it = status_map_.find(ip_key);
            if (it != status_map_.end())
            {
                it->second.done = true;
                it->second.status = read_status;
                it->second.time = 2;
            }
        }
        std::this_thread::yield();
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
            LOG(LL_NORMAL, "gethostbyname() failed: %d\n", WSAGetLastError());
            return 0;
        }
    }
    return dest_.sin_addr.S_un.S_addr;
}

int CPinger::SetupDestAddr(unsigned int const ip_key)
{
    dest_.sin_addr.S_un.S_addr = ip_key;
    return ip_key;
}

int CPinger::WriteRawSocket(unsigned int &seq_no, unsigned int const data)
{
    ICMPPacket icmp_packet = ICMPPacket(
        GetTickCount(),
        (uint16_t)GetCurrentProcessId(),
        seq_no++,
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
        LOG(LL_NORMAL, "socket: %08X, dest: %s - sendto() failed: %d\n",
            raw_socket_, inet_ntoa(dest_.sin_addr), WSAGetLastError());
    }
    else if (ret < sizeof(ICMPPacket)) {
        LOG(LL_NORMAL, "%s Wrote %d/%d bytes\n", inet_ntoa(dest_.sin_addr), ret, sizeof(ICMPPacket));
    }
    return ret;
}

PING_STATUS CPinger::ReadRawSocket()
{
    struct sockaddr_in from;
    int from_len = sizeof(from);
    char buf[sizeof(IPHeader) + sizeof(ICMPPacket)];

    int ret_value = recvfrom(
        raw_socket_,
        buf,
        sizeof(buf),
        0,
        (struct sockaddr*)&from,
        &from_len);

    if (ret_value == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAETIMEDOUT) {
            LOG(LL_NORMAL, "%s timed out\n", inet_ntoa(from.sin_addr));
            return PING_TIMEOUT;
        }
        return DEST_UNREACHABLE;
    }

    uint32_t tick = GetTickCount();
    ++packets_received_;
    uint32_t data = 0xDEADBEEF;
    IPHeader *iphdr = reinterpret_cast<IPHeader *>(buf);
    ICMPPacket *icmp_pkt = reinterpret_cast<ICMPPacket *>(buf + iphdr->h_len * sizeof(uint32_t));

    if (icmp_pkt->header.i_type == ICMP_ECHOREPLY) {
        if (icmp_pkt->header.i_id != (uint16_t)GetCurrentProcessId()) {
            LOG(LL_NORMAL, "received someone else's packet!\n");
            return DEST_UNKNOWN;
        }
        else {
            LOG(LL_NORMAL, "received %3d bytes from %12s: icmp_seq = %d. time: %d ms\n",
                ret_value,
                inet_ntoa(from.sin_addr),
                icmp_pkt->header.i_seq,
                tick - icmp_pkt->timestamp);
            return PING_SUCCESSFUL;
        }
    }
    else {
        if (icmp_pkt->header.i_type == 3) {
            LOG(LL_NORMAL, "%d.%d.%d.%d - destination host Unreachable (data: 0x%08X from: %08X)\n",
                data >> 0 & 0xFF,
                data >> 8 & 0xFF,
                data >> 16 & 0xFF,
                data >> 24 & 0xFF,
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
    return INVALID_STATUS;
}

} // namespace
