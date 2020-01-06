#ifndef UNITY_PINGPLUGIN_ICMP_H_FILE
#define UNITY_PINGPLUGIN_ICMP_H_FILE

#include <stdint.h>

namespace icmp
{
int const ICMP_ECHO = 8;
int const ICMP_ECHOREPLY = 0;

inline uint16_t CheckSum(const uint16_t *buffer, const int size)
{
    uint32_t checksum = 0;
    int s = size;
    while (s > 1) {
        checksum += *buffer++;
        s -= sizeof(uint16_t);
    }
    if (s == 1) {
        checksum += *reinterpret_cast<const uint8_t*>(buffer);
    }
    checksum = (checksum >> 16) + (checksum & 0xffff);
    checksum += (checksum >> 16);
    return static_cast<uint16_t>(~checksum);
}

// IP header
typedef struct S_IPHeader
{
    uint32_t h_len : 4;         // Header length
    uint32_t version : 4;       // IP Version
    uint8_t  tos;               // Type of service
    uint16_t total_len;         // Total length of the packet
    uint16_t ident;             // Unique identifier
    uint16_t flags_and_frag;    // Flags and fragmented offset
    uint8_t  ttl;               // Time to live
    uint8_t  proto;             // Protocol (TCP, UDP etc)
    uint16_t checksum;          // IP checksum
    uint32_t sourceIP;          // Source IP addres
    uint32_t destIP;            // Destination IP address
} IPHeader;

// ICMP header
typedef struct S_ICMPHeader
{
    uint8_t     i_type;
    uint8_t     i_code;
    uint16_t    i_cksum;
    uint16_t    i_id;
    uint16_t    i_seq;
} ICMPHeader;

// ICMP packet
typedef struct S_ICMPPacket
{
    S_ICMPPacket(uint32_t t_stamp, uint16_t id, uint16_t seq, uint32_t dat)
    {
        header.i_type = ICMP_ECHO;
        header.i_code = 0;
        header.i_id = id;
        header.i_cksum = 0;
        header.i_seq = seq;
        this->timestamp = t_stamp;
        this->data = dat;
        header.i_cksum = CheckSum(reinterpret_cast<uint16_t*>(this), sizeof(S_ICMPPacket));
    }
    ICMPHeader  header;
    uint32_t    timestamp;
    uint32_t    data;               // that's dest addr encoded on 32 bit
} ICMPPacket;

} // namespace

#endif //UNITY_PINGPLUGIN_ICMP_H_FILE