/*----------------------------------------------------------------------------
| File:
|   udp.c
|
| Description:
|   Vector XL-API for Ethernet V3 UDP stack
|
| Copyright (c) Vector Informatik GmbH. All rights reserved.
| Licensed under the MIT license. See LICENSE file in the project root for details.
|
 ----------------------------------------------------------------------------*/

#include "main.h"

#ifdef _WIN 
#ifdef APP_ENABLE_XLAPI_V3

#include "udp.h"

#define IPV4 0x0800
#define ARP  0x0806
#define IPV6 0x86dd
#define UDP  17

#ifdef _WIN
#include <pshpack1.h>
#else
#pragma pack(1) 
#endif

// 8 bytes
struct udphdr
{
    vuint16 source;
    vuint16 dest;
    vuint16 len;
    vuint16 check;
};

// 14 bytes
struct iphdr
{
    vuint16 ver_ihl_dscp_ecn; // unsigned int version : 4;  unsigned int ihl : 4; unsigned int dscp : 6; unsigned int ecn : 2;
    vuint16 tot_len;
    vuint16 id;
    vuint16 frag_off;
    vuint8 ttl;
    vuint8 protocol;
    vuint16 check;
    vuint8 saddr[4]; // src addr, high byte first 
    vuint8 daddr[4]; // dst addr 
    /*The options start here. */
};

struct arp
{
    vuint16	hrd;		/* format of hardware address */
#define ARPHRD_ETHER 1	/* ethernet hardware format */
    vuint16	pro;		/* format of protocol address */
    vuint8	hln;		/* length of hardware address */
    vuint8	pln;		/* length of protocol address */
    vuint16	op;		/* one of: */
#define	ARPOP_REQUEST 1	/* request to resolve address */
#define	ARPOP_REPLY	2	/* response to previous request */
    vuint8 sha[6];	/* sender hardware address */
    vuint8 spa[4];	/* sender protocol address */
    vuint8 tha[6];	/* target hardware address */
    vuint8 tpa[4];	/* target protocol address */
};

#ifdef _WIN
#include <poppack.h>
#else
#pragma pack() 
#endif


#ifdef XCP_ENABLE_TESTMODE

static int printFrame(const unsigned char* d, const unsigned char* s, const unsigned char* data, uint16_t dataLen, unsigned int fcs, XLuint64 timeStamp) {
    printf("%llu: dest=%02X:%02X:%02X:%02X:%02X:%02X\n", timeStamp, d[0], d[1], d[2], d[3], d[4], d[5]);
    return 1;
}

static int printIPV4Frame(const char* dir, const T_XL_ETH_FRAMEDATA* frameData, unsigned int frameLen) {

    // IPv4
    if (frameData->ethFrame.etherType == HTONS(IPV4)) { 

        struct iphdr* ip = (struct iphdr*) & frameData->ethFrame.payload[0];
        printf("%s l=%u: IPv4 l=%u ", dir, frameLen, HTONS(ip->tot_len));
        printf("%u.%u.%u.%u->", ip->saddr[0], ip->saddr[1], ip->saddr[2], ip->saddr[3]);
        printf("%u.%u.%u.%u ", ip->daddr[0], ip->daddr[1], ip->daddr[2], ip->daddr[3]);
        
        // UDP
        if (ip->protocol == UDP) { 
            struct udphdr* udp = (struct udphdr*) & frameData->ethFrame.payload[sizeof(struct iphdr)];
            printf("UDP udpl=%u %u->%u s=%u ", (vuint32)(HTONS(udp->len) - sizeof(struct udphdr)), HTONS(udp->source), HTONS(udp->dest), HTONS(udp->check));
            
            // UDP MULTICAST
            if ((ip->daddr[0] >> 4) == 0x0E) {
                printf("MULTICAST ");
            }

            if (gDebugLevel >= 3) {
                for (unsigned int byte = 0; byte < HTONS(udp->len) - sizeof(struct udphdr); byte++) {
                    printf("%02X ", frameData->ethFrame.payload[sizeof(struct iphdr) + sizeof(struct udphdr) + byte]);
                }
            }
        }
        printf("\n");
        return 1;
    }

    return 0;
}

static int printARPFrame(const char* d, const T_XL_ETH_FRAMEDATA* frameData, unsigned int frameLen) {

    if (frameData->ethFrame.etherType == HTONS(ARP)) { // ARP
        struct arp* arp = (struct arp*) & frameData->ethFrame.payload[0];
        printf("%s: ARP %u 0x%04X %u/%u %s", d, HTONS(arp->hrd), HTONS(arp->pro), arp->hln, arp->pln, (1 == arp->op) ? "Req " : "Res ");
        printf("sha 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X ", arp->sha[0], arp->sha[1], arp->sha[2], arp->sha[3], arp->sha[4], arp->sha[5]);
        printf("spa %u.%u.%u.%u ", arp->spa[0], arp->spa[1], arp->spa[2], arp->spa[3]);
        printf("tha 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X ", arp->tha[0], arp->tha[1], arp->tha[2], arp->tha[3], arp->tha[4], arp->tha[5]);
        printf("tpa %u.%u.%u.%u\n", arp->tpa[0], arp->tpa[1], arp->tpa[2], arp->tpa[3]);
        return 1;
    }
    return 0;
}


static void printRxFrame(const char* d, XLuint64 timestamp, const T_XL_NET_ETH_DATAFRAME_RX* frame) {


    if (!printARPFrame(d, &frame->frameData, frame->dataLen)) {
        if (!printIPV4Frame(d, &frame->frameData, frame->dataLen)) {
            printFrame(frame->destMAC, frame->sourceMAC, frame->frameData.rawData, frame->dataLen, frame->fcs, timestamp);
        }
    }
}


static int printEvent(const T_XL_NET_ETH_EVENT* pRxEvent) {

    switch (pRxEvent->tag) {
    case XL_ETH_EVENT_TAG_FRAMERX_ERROR_MEASUREMENT:
        printf("XL_ETH_EVENT_TAG_FRAMERX_ERROR_MEASUREMENT \n");
        break;
    case XL_ETH_EVENT_TAG_FRAMETX_ERROR_MEASUREMENT:
        printf("XL_ETH_EVENT_TAG_FRAMETX_ERROR_MEASUREMENT \n");
        break;
    case XL_ETH_EVENT_TAG_FRAMERX_MEASUREMENT:
        printf("XL_ETH_EVENT_TAG_FRAMERX_MEASUREMENT \n");
        break;
    case XL_ETH_EVENT_TAG_FRAMETX_MEASUREMENT:
        printf("XL_ETH_EVENT_TAG_FRAMETX_MEASUREMENT \n");
        break; 
    case XL_ETH_EVENT_TAG_FRAMETX_ACK_SIMULATION:
        printf("XL_ETH_EVENT_TAG_FRAMETX_ACK_SIMULATION \n");
        break;
    case XL_ETH_EVENT_TAG_LOSTEVENT:
        printf("XL_ETH_EVENT_TAG_LOSTEVENT \n");
        break;
    case XL_ETH_EVENT_TAG_ERROR:
        printf("XL_ETH_EVENT_TAG_ERROR \n");
        break;
    case XL_ETH_EVENT_TAG_CHANNEL_STATUS:
        printf("LINK %s\n", (unsigned int)pRxEvent->tagData.channelStatus.link == XL_ETH_STATUS_LINK_UP ? "UP" : "DOWN");
        return 1;
    }
    return 0;
}

#endif // TESTMODE


static uint16_t ipChecksum(uint16_t* buf, int nwords) {

    uint32_t sum;
    for (sum = 0; nwords > 0; nwords--) sum += *buf++; // No HTONS here, one complement sum is independant from Motorola or Intel format
    while (sum >> 16) sum = (sum & 0xffff) + (sum >> 16);
    return (uint16_t)(~sum);
}


static void udpInitIpHdr(struct iphdr* ip, vuint8* src, vuint8* dst) {

    // IP header(20 byte)
    ip->ver_ihl_dscp_ecn = HTONS(0x4500); // Version 4, header length 5
    // ip->tot_len = 0; // Total Length 
    ip->id = HTONS(54321);              // Identification: 0xd431 (45109???)
    ip->frag_off = 0;                   // Fragment offset : 0
    ip->ttl = 64;                       // Time to live: 64
    ip->protocol = UDP;                  // Protocol: UDP (17)
    memcpy(ip->saddr, src, 4);   // Source
    memcpy(ip->daddr, dst, 4);   // Destination
    // ip->check = 0; // Checksum has to be calculated later depending on final tot_len
}


static void udpInitUdpHdr(struct udphdr* udp, vuint16 src, vuint16 dst) {

    // UDP header (8 byte)
    udp->source = src;         // Source Port
    udp->dest = dst;           // Destination Port 
    udp->len = HTONS(sizeof(struct udphdr));    // Length: 8
    udp->check = 0; // Checsum not calculated                         

}


static int udpSendARPResponse(tUdpSockXl* sock, unsigned char sha[], unsigned char spa[]) {

    XLstatus err;
    T_XL_NET_ETH_DATAFRAME_TX frame;

    memset(&frame, 0, sizeof(T_XL_NET_ETH_DATAFRAME_TX));
    // header
    frame.dataLen = XL_ETH_PAYLOAD_SIZE_MIN + 2; // payload length +2 (with ethertype) 28 or XL_ETH_PAYLOAD_SIZE_MIN ???
    frame.flags |= XL_ETH_DATAFRAME_FLAGS_USE_SOURCE_MAC | XL_ETH_DATAFRAME_FLAGS_NO_TX_EVENT_GEN;
    memcpy(&frame.sourceMAC, sock->localAddr.sin_mac, sizeof(frame.sourceMAC));
    memcpy(&frame.destMAC, sha, sizeof(frame.destMAC));
    frame.frameData.ethFrame.etherType = HTONS(ARP);
    // payload
    struct arp* arp = (struct arp*)&frame.frameData.ethFrame.payload[0];
    arp->hrd = HTONS(ARPHRD_ETHER);		/* format of hardware address */
    arp->pro = HTONS(0x0800);		/* format of protocol address */
    arp->hln = 6;		/* length of hardware address */
    arp->pln = 4;		/* length of protocol address */
    arp->op = HTONS(ARPOP_REPLY);		/* one of: */
    memcpy(arp->sha, sock->localAddr.sin_mac, 6);
    memcpy(arp->spa, sock->localAddr.sin_addr, 4);
    memcpy(arp->tha, sha, 6);
    memcpy(arp->tpa, spa, 4);

    if (gDebugLevel >= 3) {
        printARPFrame("SendARPResponse:", &frame.frameData, frame.dataLen); //T_XL_ETH_FRAMEDATA
    }
    if (XL_SUCCESS != (err = xlNetEthSend(sock->networkHandle, sock->portHandle, (XLuserHandle)1, &frame))) {
        printf("ERROR: xlNetEthSend failed with error: %s (%d)!\n", xlGetErrorString(err), err);
        return 0;
    }

    return 1;
}


int udpRecvFrom(tUdpSockXl* sock, unsigned char* data, unsigned int size, tUdpSockAddrXl* addr, unsigned int *flags) {

    XLstatus err;
    T_XL_NET_ETH_EVENT rxEvent;
    XLrxHandle rxHandles[128];
    unsigned int rxCount = 128;
    *flags = 0;

    // Blocking
    HANDLE event_array[1];
    event_array[0] = sock->event;
    WaitForMultipleObjects(1, event_array, FALSE, 100);

    // Read queue until empty
    for (;;) {

        err = xlNetEthReceive(sock->networkHandle, &rxEvent, &rxCount, rxHandles);
        switch (err) {
        case XL_SUCCESS:
            break;
        case XL_ERR_QUEUE_IS_EMPTY:
            return 0;
        case XL_ERR_INSUFFICIENT_BUFFER:
            printf("ERROR: insufficient receive buffer!\n");
            return -1;
        default:
            printf("ERROR: xlNetEthReceive failed with error: x%x\n", err);
            return -1;
        }

        if (rxEvent.flagsChip & XL_ETH_QUEUE_OVERFLOW) {
            printf("ERROR: receive buffer overflow!\n");
        }


        if (rxEvent.tag == XL_ETH_EVENT_TAG_FRAMERX_SIMULATION) {

            T_XL_NET_ETH_DATAFRAME_RX* frameRx = &rxEvent.tagData.frameSimRx;

            if (gDebugLevel >= 5) printRxFrame("xlNetRecv", rxEvent.timeStampSync, frameRx); // Print all

            // ARP
            if (frameRx->frameData.ethFrame.etherType == HTONS(ARP)) {

                struct arp* arp = (struct arp*) & frameRx->frameData.ethFrame.payload[0];
                if (arp->hrd == HTONS(ARPHRD_ETHER) && arp->pro == HTONS(0x0800) && arp->op == HTONS(ARPOP_REQUEST)) { // Request
                    if (memcmp(arp->tpa, sock->localAddr.sin_addr, 4) == 0) {

                        udpSendARPResponse(sock, arp->sha, arp->spa);
                        return 0;
                    }
                }
            } // ARP

            // IPV4
            else if (frameRx->frameData.ethFrame.etherType == HTONS(IPV4))
            {
                if (gDebugLevel == 4) printRxFrame("xlNetRecv", rxEvent.timeStampSync, frameRx); // Print all IPv4

                struct iphdr* ip = (struct iphdr*) & frameRx->frameData.ethFrame.payload[0];
                if (ip->protocol == UDP) {

                    struct udphdr* udp = (struct udphdr*) & frameRx->frameData.ethFrame.payload[sizeof(struct iphdr)];
                    uint16_t l = HTONS(udp->len);
                    if (l <= sizeof(struct udphdr)) return 0;
                    l -= sizeof(struct udphdr);
                    if (l < size) size = l;

                    // Multicast
#ifdef APP_ENABLE_MULTICAST
                    if ((ip->daddr[0] >> 4) == 0x0E) {
                        //  XCP multicast addr
                        if (memcmp(ip->daddr, sock->multicastAddr.sin_addr, 2) == 0 && udp->dest == sock->multicastAddr.sin_port) {  
                            *flags |= RECV_FLAGS_MULTICAST;
                        }
                    }
#endif

                    // Unicast
                    if (memcmp(ip->daddr, sock->localAddr.sin_addr, 4)==0 && udp->dest==sock->localAddr.sin_port) { // XCP slave addr
                        *flags |= RECV_FLAGS_UNICAST;
                    }

                    if (*flags) {
                        memcpy(data, &frameRx->frameData.ethFrame.payload[sizeof(struct iphdr) + sizeof(struct udphdr)], size); // return payload
                        memset(addr, 0, sizeof(tUdpSockAddrXl)); // return remote addr
                        addr->sin_family = AF_INET;
                        memcpy(addr->sin_mac, frameRx->sourceMAC, 6);
                        memcpy(addr->sin_addr, ip->saddr, 4);
                        addr->sin_port = udp->source;
                        return size; // Receive size bytes
                    }
                } // UDP

                if (gDebugLevel == 3) printRxFrame("xlNetRecv", rxEvent.timeStampSync, frameRx); // Print ignored IPv4

            } // IPv4

        }

        else { // Other XL-API events 
            if (!printEvent(&rxEvent)) printf("ERROR: xlNetEthReceive unexpected event tag %u!\n", rxEvent.tag);
        }

    }

    return 0;
}

// Returns <0 on error, 0 on would block, size when ok
int udpSendTo(tUdpSockXl *sock, const unsigned char* data, unsigned int size, int mode, tUdpSockAddrXl* addr, int addr_size) {

        XLstatus err;
        T_XL_NET_ETH_DATAFRAME_TX frame;
        uint16_t iplen,udplen;

        assert(size <= gXcpTl.SlaveMTU);
        memset(&frame, 0, sizeof(T_XL_NET_ETH_DATAFRAME_TX));
        
        // header
        frame.flags |= XL_ETH_DATAFRAME_FLAGS_USE_SOURCE_MAC | XL_ETH_DATAFRAME_FLAGS_NO_TX_EVENT_GEN;
        memcpy(&frame.sourceMAC, sock->localAddr.sin_mac, sizeof(frame.sourceMAC));
        memcpy(&frame.destMAC, addr->sin_mac, sizeof(frame.destMAC));
        frame.frameData.ethFrame.etherType = HTONS(IPV4);
        
        // payload
        struct iphdr* ip = (struct iphdr*)&frame.frameData.ethFrame.payload[0];
        udpInitIpHdr(ip, sock->localAddr.sin_addr,addr->sin_addr);
        struct udphdr* udp = (struct udphdr*)&frame.frameData.ethFrame.payload[sizeof(struct iphdr)];
        udpInitUdpHdr(udp, sock->localAddr.sin_port,addr->sin_port);

        // IP header
        iplen = (uint16_t)(sizeof(struct iphdr) + sizeof(struct udphdr) + size); // Total Length
        ip->tot_len = HTONS(iplen);
        ip->check = 0;
        uint16_t s = ipChecksum((uint16_t*)ip, 10);
        ip->check = s; 

        // UDP header
        udplen = (uint16_t)(sizeof(struct udphdr) + size); 
        udp->len = HTONS(udplen);
        udp->check = 0;

        // Data
        memcpy(&frame.frameData.ethFrame.payload[sizeof(struct iphdr) + sizeof(struct udphdr)], data, size);

        frame.dataLen = 2 + sizeof(struct iphdr) + udplen; // XL-API frame len = payload length + 2 for ethertype 
        if (frame.dataLen < XL_ETH_PAYLOAD_SIZE_MIN + 2) frame.dataLen = XL_ETH_PAYLOAD_SIZE_MIN + 2; 
        if (gDebugLevel >= 3) {
            printIPV4Frame("udpSendTo", &frame.frameData, frame.dataLen); //T_XL_ETH_FRAMEDATA
        }

        if (XL_SUCCESS != (err = xlNetEthSend(sock->networkHandle, sock->portHandle, (XLuserHandle)1, &frame))) {
            if (err == XL_ERR_QUEUE_IS_FULL) {
                return 0;
            }
            printf("ERROR: xlNetEthSend failed with ERROR: %s (%d)!\n", xlGetErrorString(err), err);
            return -err;
        }

        return size;
}


// appname, net, seg, ipaddr, port -> netHandle, vpHandle, event
int udpInit(tUdpSockXl** pSock, tUdpSockAddrXl* addr, tUdpSockAddrXl* multicastAddr) {

    XLstatus err = XL_SUCCESS;
    
    tUdpSockXl* sock;
    sock = (tUdpSockXl *)malloc(sizeof(tUdpSockXl));
    if (sock == NULL) return 0;
    memset(sock, 0, sizeof(tUdpSockXl));
    sock->localAddr = *addr;
    if (multicastAddr!=NULL) sock->multicastAddr = *multicastAddr;

    if (XL_SUCCESS != (err = xlOpenDriver())) {
        printf("ERROR: xlOpenDriver failed with ERROR: %s (%d)!\n", xlGetErrorString(err), err);
        return 0;
    }
    
    if (XL_SUCCESS != (err = xlNetEthOpenNetwork(gOptionXlSlaveNet, &sock->networkHandle, APP_SLAVE_ID, XL_ACCESS_TYPE_RELIABLE, 8 * 1024 * 1024))) {
        printf("ERROR: xlNetEthOpenNetwork(NET1) failed with ERROR: %s (%d)!\n", xlGetErrorString(err), err);
        return 0;
    }

    if (XL_SUCCESS != (err = xlNetAddVirtualPort(sock->networkHandle, gOptionXlSlaveSeg, APP_SLAVE_ID, &sock->portHandle, 1))) {
        printf("ERROR: xlNetAddVirtualPort SEG1 failed with ERROR: %s (%d)!\n", xlGetErrorString(err), err);
        return 0;
    }

    // Set notification handle (required to use event handling via WaitForMultipleObjects)
    sock->event = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (XL_SUCCESS != (err = xlNetSetNotification(sock->networkHandle, &sock->event, 1))) { // (1) - Notify for each single event in queue
        printf("ERROR: xlNetSetNotification failed with ERROR: %s (%d)!\n", xlGetErrorString(err), err);
        return 0;
    }

    if (XL_SUCCESS != (err = xlNetActivateNetwork(sock->networkHandle))) {
        printf("ERROR: xlNetActivateNetwork failed: %s (%d)\n", xlGetErrorString(err), err);
        return 0;
    }

    printf("Init socket on virtual port %s-%s with IP %u.%u.%u.%u on UDP port %u \n", gOptionXlSlaveNet, gOptionXlSlaveSeg, addr->sin_addr[0], addr->sin_addr[1], addr->sin_addr[2], addr->sin_addr[3], htons(addr->sin_port));

    *pSock = sock;
    return 1;
}


void udpShutdown(tUdpSockXl* sock) {

    XLstatus err = XL_SUCCESS;

    if (sock != NULL) {
        if (sock->networkHandle) {
            if (XL_SUCCESS != (err = xlNetCloseNetwork(sock->networkHandle))) {
                printf("ERROR: xlNetCloseNetwork failed: %s (%d)\n", xlGetErrorString(err), err);
            }
        }
        free(sock);
    }
    if (XL_SUCCESS != (err = xlCloseDriver())) {
        printf("ERROR: xlCloseDriver failed: %s (%d)\n", xlGetErrorString(err), err);
    }
}


#endif
#endif


