
#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include "utils/crypto_toolbox/endian.h"
#include "sys/time.h"

#define LWIP_DONT_PROVIDE_BYTEORDER_FUNCTIONS
#define LWIP_NO_CTYPE_H 1

//开启支持IPV6
#define CONFIG_LWIP_IPV6_ENABLE

#define LWIP_TIMEVAL_PRIVATE 0

#define MULTI_NETIF_SUPPORT             0

#define JL_LWIP                         1

/**
 * LWIP_SOCKET==1: Enable Socket API (require to use sockets.c)
 */
#define LWIP_SOCKET                     1

/**
 * LWIP_COMPAT_SOCKETS==1: Enable BSD-style sockets functions names through defines.
 * LWIP_COMPAT_SOCKETS==2: Same as ==1 but correctly named functions are created.
 * While this helps code completion, it might conflict with existing libraries.
 * (only used if you use sockets.c)
 */
#define LWIP_COMPAT_SOCKETS             1

/**
 * LWIP_NETCONN==1: Enable Netconn API (require to use api_lib.c)
 */
#define LWIP_NETCONN                    0

/**
 * LWIP_SO_RCVTIMEO==1: Enable receive timeout for sockets/netconns and
 * SO_RCVTIMEO processing.
 */
#define LWIP_SO_RCVTIMEO                1

/**
 * LWIP_SO_SNDTIMEO==1: Enable send timeout for sockets/netconns and
 * SO_SNDTIMEO processing.
 */
#define LWIP_SO_SNDTIMEO                1

/**
 * LWIP_SO_SNDRCVTIMEO_NONSTANDARD==1: SO_RCVTIMEO/SO_SNDTIMEO take an int
 * (milliseconds, much like winsock does) instead of a struct timeval (default).
 */
#define LWIP_SO_SNDRCVTIMEO_NONSTANDARD 1

/**
 * LWIP_WND_SCALE and TCP_RCV_SCALE:
 * Set LWIP_WND_SCALE to 1 to enable window scaling.
 * Set TCP_RCV_SCALE to the desired scaling factor (shift count in the
 * range of [0..14]).
 * When LWIP_WND_SCALE is enabled but TCP_RCV_SCALE is 0, we can use a large
 * send window while having a small receive window only.
 */
#define LWIP_WND_SCALE                  0
#define TCP_RCV_SCALE                   0

#define DHCPS_UNICAST_ENABLE            0
#if DHCPS_UNICAST_ENABLE
// #define ETHARP_SUPPORT_STATIC_ENTRIES   1
#endif
#define ETHARP_SUPPORT_STATIC_ENTRIES   1

/*
   -----------------------------------------------
   ---------- Platform specific locking ----------
   -----------------------------------------------
*/
/**
 * DEFAULT_ACCEPTMBOX_SIZE: The mailbox size for the incoming connections.
 * The queue size value itself is platform-dependent, but is passed to
 * sys_mbox_new() when the acceptmbox is created.
 */
#define DEFAULT_ACCEPTMBOX_SIZE   60

/**
 * DEFAULT_RAW_RECVMBOX_SIZE: The mailbox size for the incoming packets on a
 * NETCONN_RAW. The queue size value itself is platform-dependent, but is passed
 * to sys_mbox_new() when the recvmbox is created.
 */
#define DEFAULT_RAW_RECVMBOX_SIZE 61

/**
 * DEFAULT_UDP_RECVMBOX_SIZE: The mailbox size for the incoming packets on a
 * NETCONN_UDP. The queue size value itself is platform-dependent, but is passed
 * to sys_mbox_new() when the recvmbox is created.
 */
#define DEFAULT_UDP_RECVMBOX_SIZE 62

/**
 * DEFAULT_TCP_RECVMBOX_SIZE: The mailbox size for the incoming packets on a
 * NETCONN_TCP. The queue size value itself is platform-dependent, but is passed
 * to sys_mbox_new() when the recvmbox is created.
 */
#define DEFAULT_TCP_RECVMBOX_SIZE 63

/**
 * TCPIP_MBOX_SIZE: The mailbox size for the tcpip thread messages
 * The queue size value itself is platform-dependent, but is passed to
 * sys_mbox_new() when tcpip_init is called.
 */
#define TCPIP_MBOX_SIZE          128

/**
 * MEMP_NUM_TCPIP_MSG_INPKT: the number of struct tcpip_msg, which are used
 * for incoming packets.
 * (only needed if you use tcpip.c)
 */
#define MEMP_NUM_TCPIP_MSG_INPKT  120

/**
 * MEMP_NUM_TCPIP_MSG_API: the number of struct tcpip_msg, which are used
 * for callback/timeout API communication.
 * (only needed if you use tcpip.c)
 */
#define MEMP_NUM_TCPIP_MSG_API    8

/**
 * MEMP_NUM_NETCONN: the number of struct netconns.
 * (only needed if you use the sequential API, like api_lib.c)
 */
#define MEMP_NUM_NETCONN          (MEMP_NUM_TCP_PCB+MEMP_NUM_TCP_PCB_LISTEN+MEMP_NUM_UDP_PCB+MEMP_NUM_RAW_PCB)

/**
 * MEMP_NUM_NETBUF: the number of struct netbufs.
 * (only needed if you use the sequential API, like api_lib.c)
 */
#define MEMP_NUM_NETBUF           64

/**
 * SYS_LIGHTWEIGHT_PROT==1: enable inter-task protection (and task-vs-interrupt
 * protection) for certain critical regions during buffer allocation, deallocation
 * and memory allocation and deallocation.
 * ATTENTION: This is required when using lwIP from more than one context! If
 * you disable this, you must be sure what you are doing!
 */
#define SYS_LIGHTWEIGHT_PROT            1

/**
 * NO_SYS==1: Use lwIP without OS-awareness (no thread, semaphores, mutexes or
 * mboxes). This means threaded APIs cannot be used (socket, netconn,
 * i.e. everything in the 'api' folder), only the callback-style raw API is
 * available (and you have to watch out for yourself that you don't access
 * lwIP functions/structures from more than one context at a time!)
 */
#define NO_SYS                          0

/**
 * LWIP_TIMERS==0: Drop support for sys_timeout and lwip-internal cyclic timers.
 * (the array of lwip-internal cyclic timers is still provided)
 * (check NO_SYS_NO_TIMERS for compatibility to old versions)
 */
#define NO_SYS_NO_TIMERS                1

/** LWIP_TCPIP_TIMEOUT==1: Enable tcpip_timeout/tcpip_untimeout to create
 * timers running in tcpip_thread from another thread.
 */
#define LWIP_TCPIP_TIMEOUT              1


/*---------- netif options ----------*/
/**
 * LWIP_NETIF_REMOVE_CALLBACK==1: Support a callback function that is called
 * when a netif has been removed
 */
#define LWIP_NETIF_REMOVE_CALLBACK      1


/*---------- ARP options ----------*/
/**
 * ARP_TABLE_SIZE: Number of active MAC-IP address pairs cached.
 */
#define ARP_TABLE_SIZE          8

/**
 * ARP_QUEUEING==1: Multiple outgoing packets are queued during hardware address
 * resolution. By default, only the most recent packet is queued per IP address.
 * This is sufficient for most protocols and mainly reduces TCP connection
 * startup time. Set this to 1 if you know your application sends more than one
 * packet in a row to an IP address that is not in the ARP cache.
 */
#define ARP_QUEUEING            0

/**
 * MEMP_NUM_ARP_QUEUE: the number of simultaneously queued outgoing
 * packets (pbufs) that are waiting for an ARP request (to resolve
 * their destination address) to finish.
 * (requires the ARP_QUEUEING option)
 */
#define MEMP_NUM_ARP_QUEUE      32

/*
   --------------------------------
   ---------- IP options ----------
   --------------------------------
*/
/**
 * IP_NAPT==1: Enables IPv4 Network Address and Port Translation
 * Note that IP_FORWARD needs to be enabled for NAPT to work
 */
#define IP_NAPT                         0

/**
 * IP_FORWARD==1: Enables the ability to forward IP packets across network
 * interfaces. If you are going to run lwIP on a device with only one network
 * interface, define this to 0.
 */
#define IP_FORWARD                      IP_NAPT

#define IP_NAPT_EXT                     IP_NAPT

/**
 * IP_REASSEMBLY==1: Reassemble incoming fragmented IP packets. Note that
 * this option does not affect outgoing packet sizes, which can be controlled
 * via IP_FRAG.
 */
#define IP_REASSEMBLY                   1

/**
 * IP_FRAG==1: Fragment outgoing IP packets if their size exceeds MTU. Note
 * that this option does not affect incoming packet sizes, which can be
 * controlled via IP_REASSEMBLY.
 */
#define IP_FRAG                         1

/**
 * IP_REASS_MAX_PBUFS: Total maximum amount of pbufs waiting to be reassembled.
 * Since the received pbufs are enqueued, be sure to configure
 * PBUF_POOL_SIZE > IP_REASS_MAX_PBUFS so that the stack is still able to receive
 * packets even if the maximum amount of fragments is enqueued for reassembly!
 * When IPv4 *and* IPv6 are enabled, this even changes to
 * (PBUF_POOL_SIZE > 2 * IP_REASS_MAX_PBUFS)!
 */
#define IP_REASS_MAX_PBUFS             44

/**
 * IP_REASS_MAXAGE: Maximum time (in multiples of IP_TMR_INTERVAL - so seconds, normally)
 * a fragmented IP packet waits for all fragments to arrive. If not all fragments arrived
 * in this time, the whole packet is discarded.
 */
#define IP_REASS_MAXAGE                15

//TODO
//#define LWIP_HOOK_IP4_ROUTE_SRC(dest,src) \
ip4_route2(src, dest)
#if IP_NAPT
#define LWIP_HOOK_IP4_ROUTE_SRC         ip4_route_src_hook
#else
/* #define LWIP_HOOK_IP4_ROUTE_SRC         user_ip4_route_src_hook */
/* #define LWIP_HOOK_IP6_ROUTE             user_ip6_route_src_hook */
#endif

#define LWIP_TCPIP_CORE_LOCKING         0
#define LWIP_NETCONN_FULLDUPLEX         1
#define LWIP_NETCONN_SEM_PER_THREAD     1

#define LWIP_RANDOMIZE_INITIAL_LOCAL_PORTS 1
#define LWIP_RAND()             (random32(0)+0xc000)

//#define LWIP_DHCP_MAX_NTP_SERVERS       8
//#define LWIP_DHCP_GET_NTP_SRV           1

#if IP_NAPT
#define NETIF_MTU 1500
#else
#define NETIF_MTU 1500 //1240 虽然以太网标准是1500 ,但是会导致某些网关丢弃数据包,当发现数据包发不出去网关就降低该值测试
#endif

/*---------- TCP options ----------*/
/**
 * TCP_QUEUE_OOSEQ==1: TCP will queue segments that arrive out of order.
 * Define to 0 if your device is low on memory.
 */
#define TCP_QUEUE_OOSEQ         1

/**
 * TCP_MSS: TCP Maximum segment size. (default is 536, a conservative default,
 * you might want to increase this.)
 * For the receive side, this MSS is advertised to the remote side
 * when opening a connection. For the transmit size, this MSS sets
 * an upper limit on the MSS advertised by the remote host.
 */
#define TCP_MSS                 (NETIF_MTU - 40)	/* TCP_MSS = (Ethernet MTU - IP header size - TCP header size) */

/**
 * TCP_WND: The size of a TCP window.  This must be at least
 * (2 * TCP_MSS) for things to work well.
 * ATTENTION: when using TCP_RCV_SCALE, TCP_WND is the total size
 * with scaling applied. Maximum window value in the TCP header
 * will be TCP_WND >> TCP_RCV_SCALE
 */
#define TCP_WND                 (65535) 		//最大可以设置为65535

/**
 * TCP_SND_BUF: TCP sender buffer space (bytes).
 * To achieve good performance, this should be at least 2 * TCP_MSS.
 */
#define TCP_SND_BUF             (32*TCP_MSS)

/**
 * TCP_RTO_TIME: TCP retransmission timeout (ms). (default is 3000, for faster recovery,
 * this value can be lowered to 1 second.(RFC 6298))
 */
#define TCP_RTO_TIME      		3000

/**
 * TCP_SND_QUEUELEN: TCP sender buffer space (pbufs). This must be at least
 * as much as (2 * TCP_SND_BUF/TCP_MSS) for things to work.
 */
#define TCP_SND_QUEUELEN        ((4 * (TCP_SND_BUF) + (TCP_MSS - 1))/(TCP_MSS))

/**
 * TCP_LISTEN_BACKLOG: Enable the backlog option for tcp listen pcb.
 */
#define TCP_LISTEN_BACKLOG      0

#define TCP_QUICKACK_ENABLE     1

/**
 * The maximum allowed backlog for TCP listen netconns.
 * This backlog is used unless another is explicitly specified.
 * 0xff is the maximum (u8_t).
 */
#define TCP_DEFAULT_LISTEN_BACKLOG      0xff

/**
 * TCP_MAXRTX: Maximum number of retransmissions of data segments.
 */
#define TCP_MAXRTX                      12

/**
 * TCP_SYNMAXRTX: Maximum number of retransmissions of SYN segments.
 */
#define TCP_SYNMAXRTX                   6

/*---------- RAW options ----------*/
/**
 * LWIP_RAW==1: Enable application layer to hook into the IP layer itself.
 */
#define LWIP_RAW                1

/**
 * LWIP_IPV6==1: Enable IPv6
 */
#ifdef CONFIG_LWIP_IPV6_ENABLE
#define LWIP_IPV6               1
#define LWIP_IPV6_DHCP6         1
#define LWIP_IPV6_DUP_DETECT_ATTEMPTS   0
#define LWIP_NETIF_EXT_STATUS_CALLBACK  1
#define LWIP_ND6_RDNSS_MAX_DNS_SERVERS  3
#else
#define LWIP_IPV6               0
#define LWIP_IPV6_DHCP6         0
#endif

/* ---------- Memory options ---------- */
/**
 * MEM_LIBC_MALLOC==1: Use malloc/free/realloc provided by your C-library
 * instead of the lwip internal allocator. Can save code size if you
 * already use it.
 */
#define MEM_LIBC_MALLOC                 1

/**
 * MEM_ALIGNMENT: should be set to the alignment of the CPU
 *    4 byte alignment -> \#define MEM_ALIGNMENT 4
 *    2 byte alignment -> \#define MEM_ALIGNMENT 2
 */
#define MEM_ALIGNMENT                   4

/**
 * MEMCPY: override this if you have a faster implementation at hand than the
 * one included in your C library
 */
#define MEMCPY(dst,src,len)             memcpy((void *)dst,(void *)src,len)

/**
 * SMEMCPY: override this with care! Some compilers (e.g. gcc) can inline a
 * call to memcpy() if the length is known at compile time and is small.
 */
#define SMEMCPY(dst,src,len)            memcpy((void *)dst,(void *)src,len)

/**
 * MEMP_NUM_PBUF: the number of memp struct pbufs (used for PBUF_ROM and PBUF_REF).
 * If the application sends a lot of data out of ROM (or other static memory),
 * this should be set high.
 */
#define MEMP_NUM_PBUF           4

/**
 * MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
 * per active UDP "connection".
 * (requires the LWIP_UDP option)
 */
#define MEMP_NUM_UDP_PCB        16

/**
 * MEMP_NUM_TCP_PCB: the number of simultaneously active TCP connections.
 * (requires the LWIP_TCP option)
 */
#define MEMP_NUM_TCP_PCB        20

/**
 * MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP connections.
 * (requires the LWIP_TCP option)
 */
#define MEMP_NUM_TCP_PCB_LISTEN 16

/**
 * MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP segments.
 * (requires the LWIP_TCP option)
 */
#define MEMP_NUM_TCP_SEG       TCP_SND_QUEUELEN //((4 * (TCP_SND_BUF) + (TCP_MSS - 1))/(TCP_MSS))

/**
 * MEMP_NUM_SYS_TIMEOUT: the number of simultaneously active timeouts.
 * The default number of timeouts is calculated here for all enabled modules.
 * The formula expects settings to be either '0' or '1'.
 */
#define MEMP_NUM_SYS_TIMEOUT    16

/*---------- Internal Memory Pool Sizes ----------*/
/**
 * MEMP_NUM_RAW_PCB: Number of raw connection PCBs
 * (requires the LWIP_RAW option)
 */
#if LWIP_RAW
#define MEMP_NUM_RAW_PCB                3
#else
#define MEMP_NUM_RAW_PCB                0
#endif

/**
 * MEMP_NUM_REASSDATA: the number of IP packets simultaneously queued for
 * reassembly (whole packets, not fragments!)
 */
#define MEMP_NUM_REASSDATA              44

/**
 * MEMP_NUM_FRAG_PBUF: the number of IP fragments simultaneously sent
 * (fragments, not whole packets!).
 * This is only used with LWIP_NETIF_TX_SINGLE_PBUF==0 and only has to be > 1
 * with DMA-enabled MACs where the packet is not yet sent when netif->output
 * returns.
 */
#define MEMP_NUM_FRAG_PBUF              32

/* ---------- Pbuf options ---------- */
/**
 * PBUF_POOL_SIZE: the number of buffers in the pbuf pool.
 */
#define PBUF_POOL_SIZE          120

/**
 * PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. The default is
 * designed to accommodate single full size TCP frame in one pbuf, including
 * TCP_MSS, IP header, and link header.
 */
#define PBUF_POOL_BUFSIZE       LWIP_MEM_ALIGN_SIZE(TCP_MSS+40+PBUF_LINK_ENCAPSULATION_HLEN+PBUF_LINK_HLEN)

/**
 * MEMP_OVERFLOW_CHECK: memp overflow protection reserves a configurable
 * amount of bytes before and after each memp element in every pool and fills
 * it with a prominent default value.
 *    MEMP_OVERFLOW_CHECK == 0 no checking
 *    MEMP_OVERFLOW_CHECK == 1 checks each element when it is freed
 *    MEMP_OVERFLOW_CHECK >= 2 checks each element in every pool every time
 *      memp_malloc() or memp_free() is called (useful but slow!)
 */
#define MEMP_OVERFLOW_CHECK             0//2

/**
 * MEMP_SANITY_CHECK==1: run a sanity check after each memp_free() to make
 * sure that there are no cycles in the linked lists.
 */
#define MEMP_SANITY_CHECK               0//1

/**
 * LWIP_SO_RCVBUF==1: Enable SO_RCVBUF processing.
 */
#define LWIP_SO_RCVBUF                  1

/**
 * If LWIP_SO_RCVBUF is used, this is the default value for recv_bufsize.
 */
#define RECV_BUFSIZE_DEFAULT            65535

extern unsigned char __attribute__((aligned(4)))  __attribute__((section(".memp_memory_x")))  memp_memory_NETBUF_base[]  ;
extern unsigned char __attribute__((aligned(4)))  __attribute__((section(".memp_memory_x"))) memp_memory_NETCONN_base[]  ;
extern unsigned char __attribute__((aligned(4)))  __attribute__((section(".memp_memory_x"))) memp_memory_PBUF_base[]  ;
extern unsigned char __attribute__((aligned(4)))  __attribute__((section(".memp_memory_x"))) memp_memory_NETDB_base[]  ;
extern unsigned char __attribute__((aligned(4)))  __attribute__((section(".memp_memory_x")))memp_memory_REASSDATA_base[]  ;
extern unsigned char __attribute__((aligned(4)))  __attribute__((section(".memp_memory_x")))memp_memory_TCP_PCB_LISTEN_base[]  ;
extern unsigned char __attribute__((aligned(4)))  __attribute__((section(".memp_memory_x"))) memp_memory_SYS_TIMEOUT_base[]  ;
extern unsigned char __attribute__((aligned(4)))  __attribute__((section(".memp_memory_x"))) memp_memory_UDP_PCB_base[]  ;
extern unsigned char __attribute__((aligned(4)))  __attribute__((section(".memp_memory_x"))) memp_memory_TCPIP_MSG_INPKT_base[]  ;
extern unsigned char __attribute__((aligned(4)))  __attribute__((section(".memp_memory_x"))) memp_memory_FRAG_PBUF_base[]  ;
extern unsigned char __attribute__((aligned(4)))  __attribute__((section(".memp_memory_x"))) memp_memory_TCP_PCB_base[]  ;
extern unsigned char __attribute__((aligned(4)))  __attribute__((section(".memp_memory_x"))) memp_memory_PBUF_POOL_base[]  ;
extern unsigned char __attribute__((aligned(4)))  __attribute__((section(".memp_memory_x"))) memp_memory_TCPIP_MSG_API_base[]  ;
extern unsigned char __attribute__((aligned(4)))  __attribute__((section(".memp_memory_x"))) memp_memory_TCP_SEG_base[]  ;


/* ---------- TCP options ---------- */
/**
 * LWIP_TCP==1: Turn on TCP.
 */
#define LWIP_TCP                1

/**
 * TCP_TTL: Default Time-To-Live value.
 */
#define TCP_TTL                 255

/**
 * LWIP_TCP_KEEPALIVE==1: Enable TCP_KEEPIDLE, TCP_KEEPINTVL and TCP_KEEPCNT
 * options processing. Note that TCP_KEEPIDLE and TCP_KEEPINTVL have to be set
 * in seconds. (does not require sockets.c, and will affect tcp.c)
 */
#define LWIP_TCP_KEEPALIVE              0

/**
 * SO_REUSE==1: Enable SO_REUSEADDR option.
 */
#define SO_REUSE                        1 //not suggest use

/*Don't change this unless you know what you're doing */
/* The maximum segment lifetime in milliseconds */
#define  TCP_MSL 0//60000UL


/* ---------- UDP options ---------- */
/**
 * LWIP_UDP==1: Turn on UDP.
 */
#define LWIP_UDP                1

/**
 * LWIP_UDPLITE==1: Turn on UDP-Lite. (Requires LWIP_UDP)
 */
#define LWIP_UDPLITE            0

/**
 * UDP_TTL: Default Time-To-Live value.
 */
#define UDP_TTL                 255

/**
 * CHECKSUM_GEN_UDP==1: Generate checksums in software for outgoing UDP packets.
 */
#define CHECKSUM_GEN_UDP                1

/**
 * CHECKSUM_CHECK_UDP==1: Check checksums in software for incoming UDP packets.
 */
#define CHECKSUM_CHECK_UDP               0

/* ---------- ICMP options ---------- */
/**
 * LWIP_ICMP==1: Enable ICMP module inside the IP stack.
 * Be careful, disable that make your product non-compliant to RFC1122
 */
#define LWIP_ICMP                 1

/**
 * ICMP_TTL: Default value for Time-To-Live used by ICMP packets.
 */
#define ICMP_TTL                255

/*  ---------- IGMP options ----------*/
/**
 * LWIP_IGMP==1: Turn on IGMP module.
 */
#define LWIP_IGMP                  1

/**
 * MEMP_NUM_IGMP_GROUP: The number of multicast groups whose network interfaces
 * can be members at the same time (one per netif - allsystems group -, plus one
 * per netif membership).
 * (requires the LWIP_IGMP option)
 */
#define MEMP_NUM_IGMP_GROUP             2

/*---------- DHCP options ----------*/
/**
 * LWIP_DHCP==1: Enable DHCP module.
 */
#define LWIP_DHCP                       1

/*---------- AUTOIP options ----------*/
/**
 * LWIP_AUTOIP==1: Enable AUTOIP module.
 */
#define LWIP_AUTOIP                     0

/**
 * LWIP_DHCP_AUTOIP_COOP==1: Allow DHCP and AUTOIP to be both enabled on
 * the same interface at the same time.
 */
#define LWIP_DHCP_AUTOIP_COOP           0

/**
 * LWIP_DHCP_AUTOIP_COOP_TRIES: Set to the number of DHCP DISCOVER probes
 * that should be sent before falling back on AUTOIP (the DHCP client keeps
 * running in this case). This can be set as low as 1 to get an AutoIP address
 * very  quickly, but you should be prepared to handle a changing IP address
 * when DHCP overrides AutoIP.
 */
#define LWIP_DHCP_AUTOIP_COOP_TRIES     20

/**
 * LWIP_DHCP_DOES_ACD_CHECK==1: Perform address conflict detection on the dhcp address.
 */
#define LWIP_DHCP_DOES_ACD_CHECK        0

/*---------- DNS options -----------*/
/**
 * MEMP_NUM_NETDB: the number of concurrently running lwip_addrinfo() calls
 * (before freeing the corresponding memory using lwip_freeaddrinfo()).
 */
#define MEMP_NUM_NETDB                  4

/**
 * LWIP_DNS==1: Turn on DNS module. UDP must be available for DNS
 * transport.
 */
#define LWIP_DNS                        1

/** DNS maximum number of entries to maintain locally. */
#define DNS_TABLE_SIZE                  12//4

/** DNS maximum host name length supported in the name table. */
#define DNS_MAX_NAME_LENGTH             256

/** The maximum of DNS servers
 * The first server can be initialized automatically by defining
 * DNS_SERVER_ADDRESS(ipaddr), where 'ipaddr' is an 'ip_addr_t*'
 */
#define DNS_MAX_SERVERS          5


/** DNS_LOCAL_HOSTLIST: Implements a local host-to-address list. If enabled, you have to define an initializer:
 *  \#define DNS_LOCAL_HOSTLIST_INIT {DNS_LOCAL_HOSTLIST_ELEM("host_ip4", IPADDR4_INIT_BYTES(1,2,3,4)), \
 *                                    DNS_LOCAL_HOSTLIST_ELEM("host_ip6", IPADDR6_INIT_HOST(123, 234, 345, 456)}
 *
 *  Instead, you can also use an external function:
 *  \#define DNS_LOOKUP_LOCAL_EXTERN(name, namelen, addr, dns_addrtype) my_lookup_function(name, namelen, addr, dns_addrtype)
 *  with function signature:
 *  extern err_t my_lookup_function(const char *name, size_t namelen, ip_addr_t *addr, u8_t dns_addrtype)
 *  that looks up the IP address and returns ERR_OK if found (LWIP_DNS_ADDRTYPE_xxx is passed in dns_addrtype).
 */
#define DNS_LOCAL_HOSTLIST              1

/** If this is turned on, the local host-list can be dynamically changed
 *  at runtime. */
#define DNS_LOCAL_HOSTLIST_IS_DYNAMIC   1

/**
 * MEMP_NUM_LOCALHOSTLIST: the number of host entries in the local host list
 * if DNS_LOCAL_HOSTLIST_IS_DYNAMIC==1.
 */
#define MEMP_NUM_LOCALHOSTLIST          8   //2

/**
 * LWIP_IPV6_DHCP6_STATEFUL==1: enable DHCPv6 stateful address autoconfiguration.
 */
#define LWIP_IPV6_DHCP6_STATEFUL        0

#define LOCAL_WIRELESS_HOST_NAME "lwip_wireless_host"
#define LOCAL_WIRE_HOST_NAME "lwip_wire_host"
#define LOCAL_BT_HOST_NAME "lwip_bt_host"
#define LOCAL_LTE_HOST_NAME "lwip_lte_host"
#define LOCAL_EXT_WIRELESS_HOST_NAME "lwip_ext_wireless_host"
//#define DNS_LOCAL_HOSTLIST_INIT {{LOCAL_WIRELESS_HOST_NAME, 0x0101a8c0},{LOCAL_WIRE_HOST_NAME, 0x0201a8c0},}

/**
 * LWIP_NETIF_HOSTNAME==1: use DHCP_OPTION_HOSTNAME with netif's hostname
 * field.
 */
#define LWIP_NETIF_HOSTNAME             1 //AP端显示名称 netif->hostname
#define LWIP_LOCAL_HOSTNAME             "AC79NN_LWIP"

extern void dns_set_server(unsigned int *dnsserver);
#define DNS_SERVER_ADDRESS dns_set_server

/*---------- Statistics options ----------*/
/**
 * LWIP_STATS==1: Enable statistics collection in lwip_stats.
 */
#define LWIP_STATS                      1

/**
 * LWIP_STATS_DISPLAY==1: Compile in the statistics output functions.
 */
#define LWIP_STATS_DISPLAY              1

/**
 * LINK_STATS==1: Enable link stats.
 */
#define LINK_STATS                      1

/**
 * ETHARP_STATS==1: Enable etharp stats.
 */
#define ETHARP_STATS                    (LWIP_ARP)

/**
 * IP_STATS==1: Enable IP stats.
 */
#define IP_STATS                        1

/**
 * IPFRAG_STATS==1: Enable IP fragmentation stats. Default is
 * on if using either frag or reass.
 */
#define IPFRAG_STATS                    (IP_REASSEMBLY || IP_FRAG)

/**
 * ICMP_STATS==1: Enable ICMP stats.
 */
#define ICMP_STATS                      (LWIP_ICMP)

/**
 * IGMP_STATS==1: Enable IGMP stats.
 */
#define IGMP_STATS                      (LWIP_IGMP)

/**
 * UDP_STATS==1: Enable UDP stats. Default is on if
 * UDP enabled, otherwise off.
 */
#define UDP_STATS                       (LWIP_UDP)

/**
 * TCP_STATS==1: Enable TCP stats. Default is on if TCP
 * enabled, otherwise off.
 */
#define TCP_STATS                       (LWIP_TCP)

/**
 * MEM_STATS==1: Enable mem.c stats.
 */
#define MEM_STATS                       0//((MEM_LIBC_MALLOC == 0) && (MEM_USE_POOLS == 0)) ÄÚ´æ¹ÜÀí±»Å²µ½Íâ²¿Ê¹ÓÃ?

/**
 * MEMP_STATS==1: Enable memp.c pool stats.
 */
#define MEMP_STATS                      (MEMP_MEM_MALLOC == 0)

/**
 * SYS_STATS==1: Enable system stats (sem and mbox counts, etc).
 */
#define SYS_STATS                       0//(NO_SYS == 0)

/**
 * IP6_STATS==1: Enable IPv6 stats.
 */
#define IP6_STATS                       0

/**
 * ICMP6_STATS==1: Enable ICMP for IPv6 stats.
 */
#define ICMP6_STATS                     0

/**
 * IP6_FRAG_STATS==1: Enable IPv6 fragmentation stats.
 */
#define IP6_FRAG_STATS                  0

/**
 * MLD6_STATS==1: Enable MLD for IPv6 stats.
 */
#define MLD6_STATS                      0

/**
 * ND6_STATS==1: Enable Neighbor discovery for IPv6 stats.
 */
#define ND6_STATS                       0

/**
 * MIB2_STATS==1: Stats for SNMP MIB2.
 */
#define MIB2_STATS                      0

/*---------- Checksum options ----------*/
// #include "eth/ethmac.h"
//#define LWIP_CHKSUM  ethernet_checksum

/*
   ---------------------------------------
   ---------- Debugging options ----------
   ---------------------------------------
*/
//#define LWIP_NOASSERT

#define LWIP_DEBUG  1

#ifdef LWIP_DEBUG

#define LWIP_DBG_TYPES_ON              LWIP_DBG_STATE|LWIP_DBG_FRESH //LWIP_DBG_TRACE LWIP_DBG_STATE LWIP_DBG_FRESH

#define LWIP_DBG_MIN_LEVEL             LWIP_DBG_LEVEL_ALL//LWIP_DBG_LEVEL_ALL LWIP_DBG_LEVEL_WARNING LWIP_DBG_LEVEL_SERIOUS  LWIP_DBG_LEVEL_SEVERE

#define ETHARP_DEBUG                   LWIP_DBG_OFF
#define NETIF_DEBUG                    LWIP_DBG_OFF
#define PBUF_DEBUG                     LWIP_DBG_OFF
#define API_LIB_DEBUG                  LWIP_DBG_OFF
#define API_MSG_DEBUG                  LWIP_DBG_OFF
#define SOCKETS_DEBUG                  LWIP_DBG_OFF
#define ICMP_DEBUG                     LWIP_DBG_OFF
#define IGMP_DEBUG                     LWIP_DBG_OFF
#define INET_DEBUG                     LWIP_DBG_OFF
#define IP_DEBUG                       LWIP_DBG_OFF
#define IP_REASS_DEBUG                 LWIP_DBG_OFF
#define RAW_DEBUG                      LWIP_DBG_OFF
#define MEM_DEBUG                      LWIP_DBG_OFF
#define MEMP_DEBUG                     LWIP_DBG_OFF
#define SYS_DEBUG                      LWIP_DBG_OFF
#define TIMERS_DEBUG                   LWIP_DBG_OFF
#define TCP_DEBUG                      LWIP_DBG_OFF
#define TCP_INPUT_DEBUG                LWIP_DBG_OFF
#define TCP_FR_DEBUG                   LWIP_DBG_OFF
#define TCP_RTO_DEBUG                  LWIP_DBG_OFF
#define TCP_CWND_DEBUG                 LWIP_DBG_OFF
#define TCP_WND_DEBUG                  LWIP_DBG_OFF
#define TCP_OUTPUT_DEBUG               LWIP_DBG_OFF
#define TCP_RST_DEBUG                  LWIP_DBG_OFF
#define TCP_QLEN_DEBUG                 LWIP_DBG_OFF
#define UDP_DEBUG                      LWIP_DBG_OFF
#define TCPIP_DEBUG                    LWIP_DBG_OFF
#define PPP_DEBUG                      LWIP_DBG_OFF
#define SLIP_DEBUG                     LWIP_DBG_OFF
#define DHCP_DEBUG                     LWIP_DBG_OFF
#define AUTOIP_DEBUG                   LWIP_DBG_OFF
#define SNMP_MSG_DEBUG                 LWIP_DBG_OFF
#define SNMP_MIB_DEBUG                 LWIP_DBG_OFF
#define DNS_DEBUG                      LWIP_DBG_OFF
#define IP6_DEBUG                      LWIP_DBG_OFF
#define DHCP6_DEBUG                    LWIP_DBG_OFF

#define SMTP_DEBUG                     LWIP_DBG_OFF
#define HTTPD_DEBUG                    LWIP_DBG_OFF
#define HTTPD_DEBUG_TIMING		       LWIP_DBG_OFF
#define SNTP_DEBUG                     LWIP_DBG_OFF
#define RTP_DEBUG                      LWIP_DBG_OFF
#define PING_DEBUG                     LWIP_DBG_OFF
#define NAPT_DEBUG                     LWIP_DBG_OFF
#define ACD_DEBUG                      LWIP_DBG_OFF
#endif

#ifdef __cplusplus
}
#endif

#endif

