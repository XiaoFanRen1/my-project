/**
 * @file
 *
 * @defgroup dhcp6 DHCPv6
 * @ingroup ip6
 * DHCPv6 client: IPv6 address autoconfiguration as per
 * RFC 3315 (stateful DHCPv6) and
 * RFC 3736 (stateless DHCPv6).
 *
 * For now, only stateless DHCPv6 is implemented!
 *
 * TODO:
 * - enable/disable API to not always start when RA is received
 * - stateful DHCPv6 (for now, only stateless DHCPv6 for DNS and NTP servers works)
 * - create Client Identifier?
 * - only start requests if a valid local address is available on the netif
 * - only start information requests if required (not for every RA)
 *
 * dhcp6_enable_stateful() enables stateful DHCPv6 for a netif (stateless disabled)<br>
 * dhcp6_enable_stateless() enables stateless DHCPv6 for a netif (stateful disabled)<br>
 * dhcp6_disable() disable DHCPv6 for a netif
 *
 * When enabled, requests are only issued after receipt of RA with the
 * corresponding bits set.
 */

/*
 * Copyright (c) 2018 Simon Goldschmidt
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Simon Goldschmidt <goldsimon@gmx.de>
 */

#include "lwip/opt.h"

#if LWIP_IPV6 && LWIP_IPV6_DHCP6 /* don't build if not configured for use in lwipopts.h */

#include "lwip/dhcp6.h"
#include "lwip/prot/dhcp6.h"
#include "lwip/def.h"
#include "lwip/udp.h"
#include "lwip/dns.h"

#include <string.h>

#ifdef LWIP_HOOK_FILENAME
#include LWIP_HOOK_FILENAME
#endif
#ifndef LWIP_HOOK_DHCP6_APPEND_OPTIONS
#define LWIP_HOOK_DHCP6_APPEND_OPTIONS(netif, dhcp6, state, msg, msg_type, options_len_ptr, max_len)
#endif
#ifndef LWIP_HOOK_DHCP6_PARSE_OPTION
#define LWIP_HOOK_DHCP6_PARSE_OPTION(netif, dhcp6, state, msg, msg_type, option, len, pbuf, offset) do { LWIP_UNUSED_ARG(msg); } while(0)
#endif

#if LWIP_DNS && LWIP_DHCP6_MAX_DNS_SERVERS
#if DNS_MAX_SERVERS > LWIP_DHCP6_MAX_DNS_SERVERS
#define LWIP_DHCP6_PROVIDE_DNS_SERVERS LWIP_DHCP6_MAX_DNS_SERVERS
#else
#define LWIP_DHCP6_PROVIDE_DNS_SERVERS DNS_MAX_SERVERS
#endif
#else
#define LWIP_DHCP6_PROVIDE_DNS_SERVERS 0
#endif


/** Option handling: options are parsed in dhcp6_parse_reply
 * and saved in an array where other functions can load them from.
 * This might be moved into the struct dhcp6 (not necessarily since
 * lwIP is single-threaded and the array is only used while in recv
 * callback). */
enum dhcp6_option_idx {
    DHCP6_OPTION_IDX_CLI_ID = 0,
    DHCP6_OPTION_IDX_SERVER_ID,
#if LWIP_DHCP6_PROVIDE_DNS_SERVERS
    DHCP6_OPTION_IDX_DNS_SERVER,
    DHCP6_OPTION_IDX_DOMAIN_LIST,
#endif /* LWIP_DHCP_PROVIDE_DNS_SERVERS */
#if LWIP_DHCP6_GET_NTP_SRV
    DHCP6_OPTION_IDX_NTP_SERVER,
#endif /* LWIP_DHCP_GET_NTP_SRV */
#if LWIP_IPV6_DHCP6_STATEFUL
    DHCP6_OPTION_IDX_IA_NA,
#endif /* LWIP_IPV6_DHCP6_STATEFUL */
    DHCP6_OPTION_IDX_MAX
};

struct dhcp6_option_info {
    u8_t option_given;
    u16_t val_start;
    u16_t val_length;
};

/** Holds the decoded option info, only valid while in dhcp6_recv. */
struct dhcp6_option_info dhcp6_rx_options[DHCP6_OPTION_IDX_MAX];

#define dhcp6_option_given(dhcp6, idx)           (dhcp6_rx_options[idx].option_given != 0)
#define dhcp6_got_option(dhcp6, idx)             (dhcp6_rx_options[idx].option_given = 1)
#define dhcp6_clear_option(dhcp6, idx)           (dhcp6_rx_options[idx].option_given = 0)
#define dhcp6_clear_all_options(dhcp6)           (memset(dhcp6_rx_options, 0, sizeof(dhcp6_rx_options)))
#define dhcp6_get_option_start(dhcp6, idx)       (dhcp6_rx_options[idx].val_start)
#define dhcp6_get_option_length(dhcp6, idx)      (dhcp6_rx_options[idx].val_length)
#define dhcp6_set_option(dhcp6, idx, start, len) do { dhcp6_rx_options[idx].val_start = (start); dhcp6_rx_options[idx].val_length = (len); }while(0)


const ip_addr_t dhcp6_All_DHCP6_Relay_Agents_and_Servers = IPADDR6_INIT_HOST(0xFF020000, 0, 0, 0x00010002);
const ip_addr_t dhcp6_All_DHCP6_Servers = IPADDR6_INIT_HOST(0xFF020000, 0, 0, 0x00010003);

static struct udp_pcb *dhcp6_pcb;
static u8_t dhcp6_pcb_refcount;


/* receive, unfold, parse and free incoming messages */
static void dhcp6_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);

#if LWIP_IPV6_DHCP6_STATEFUL
static u16_t dhcp6_get_duid_len(DUID duid);
static void dhcp6_copy_duid_buf(DUID *dst_duid, u8_t *duid_buf, u32_t buf_len);
static void dhcp6_generate_duid(int duid_type, uint8_t *mac_addr, uint32_t timestamp, uint32_t enterprise_number, DUID *duid_buf);
static void dhcp6_t1_timeout(struct netif *netif);
static void dhcp6_t2_timeout(struct netif *netif);
static void dhcp6_bind(struct netif *netif);
static err_t dhcp6_solicit(struct netif *netif);
static err_t dhcp6_request(struct netif *netif);
static err_t dhcp6_confirm(struct netif *netif);
static err_t dhcp6_renew(struct netif *netif);
static err_t dhcp6_rebind(struct netif *netif);
static err_t dhcp6_delicine(struct netif *netif);
static err_t dhcp6_release(struct netif *netif);
static void dhcp6_handle_advertise(struct netif *netif, struct pbuf *p_msg_in);
static void dhcp6_handle_reply(struct netif *netif, struct pbuf *p_msg_in);
static err_t dhcp6_parse_ia_na(struct pbuf *p, u16_t op_start, u16_t op_len, u32_t *ia_id, ip6_addr_t *addr, u32_t *t1, u32_t *t2, u32_t *preferred_lifetime, u32_t *valid_lifetime);
#endif /* LWIP_IPV6_DHCP6_STATEFUL */

/** Ensure DHCP PCB is allocated and bound */
static err_t
dhcp6_inc_pcb_refcount(void)
{
    if (dhcp6_pcb_refcount == 0) {
        LWIP_ASSERT("dhcp6_inc_pcb_refcount(): memory leak", dhcp6_pcb == NULL);

        /* allocate UDP PCB */
        dhcp6_pcb = udp_new_ip6();

        if (dhcp6_pcb == NULL) {
            return ERR_MEM;
        }

        ip_set_option(dhcp6_pcb, SOF_BROADCAST);

        /* set up local and remote port for the pcb -> listen on all interfaces on all src/dest IPs */
        udp_bind(dhcp6_pcb, IP6_ADDR_ANY, DHCP6_CLIENT_PORT);
        udp_recv(dhcp6_pcb, dhcp6_recv, NULL);
    }

    dhcp6_pcb_refcount++;

    return ERR_OK;
}

/** Free DHCP PCB if the last netif stops using it */
static void
dhcp6_dec_pcb_refcount(void)
{
    LWIP_ASSERT("dhcp6_pcb_refcount(): refcount error", (dhcp6_pcb_refcount > 0));
    dhcp6_pcb_refcount--;

    if (dhcp6_pcb_refcount == 0) {
        udp_remove(dhcp6_pcb);
        dhcp6_pcb = NULL;
    }
}

/**
 * @ingroup dhcp6
 * Set a statically allocated struct dhcp6 to work with.
 * Using this prevents dhcp6_start to allocate it using mem_malloc.
 *
 * @param netif the netif for which to set the struct dhcp
 * @param dhcp6 (uninitialised) dhcp6 struct allocated by the application
 */
void
dhcp6_set_struct(struct netif *netif, struct dhcp6 *dhcp6)
{
    LWIP_ASSERT("netif != NULL", netif != NULL);
    LWIP_ASSERT("dhcp6 != NULL", dhcp6 != NULL);
    LWIP_ASSERT("netif already has a struct dhcp6 set", netif_dhcp6_data(netif) == NULL);

    /* clear data structure */
    memset(dhcp6, 0, sizeof(struct dhcp6));
    /* dhcp6_set_state(&dhcp, DHCP6_STATE_OFF); */
    netif_set_client_data(netif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP6, dhcp6);
}

/**
 * @ingroup dhcp6
 * Removes a struct dhcp6 from a netif.
 *
 * ATTENTION: Only use this when not using dhcp6_set_struct() to allocate the
 *            struct dhcp6 since the memory is passed back to the heap.
 *
 * @param netif the netif from which to remove the struct dhcp
 */
void dhcp6_cleanup(struct netif *netif)
{
    LWIP_ASSERT("netif != NULL", netif != NULL);

    if (netif_dhcp6_data(netif) != NULL) {
        mem_free(netif_dhcp6_data(netif));
        netif_set_client_data(netif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP6, NULL);
    }
}

static struct dhcp6 *
    dhcp6_get_struct(struct netif *netif, const char *dbg_requester)
{
    struct dhcp6 *dhcp6 = netif_dhcp6_data(netif);
    if (dhcp6 == NULL) {
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("%s: mallocing new DHCPv6 client\n", dbg_requester));
        dhcp6 = (struct dhcp6 *)mem_malloc(sizeof(struct dhcp6));
        if (dhcp6 == NULL) {
            LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("%s: could not allocate dhcp6\n", dbg_requester));
            return NULL;
        }

        /* clear data structure, this implies DHCP6_STATE_OFF */
        memset(dhcp6, 0, sizeof(struct dhcp6));
#if LWIP_IPV6_DHCP6_STATEFUL
        dhcp6->netif_addr_idx = 0xff;
#endif /* LWIP_IPV6_DHCP6_STATEFUL */
        /* store this dhcp6 client in the netif */
        netif_set_client_data(netif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP6, dhcp6);
    } else {
        /* already has DHCP6 client attached */
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("%s: using existing DHCPv6 client\n", dbg_requester));
    }

    if (!dhcp6->pcb_allocated) {
        if (dhcp6_inc_pcb_refcount() != ERR_OK) { /* ensure DHCP6 PCB is allocated */
            mem_free(dhcp6);
            netif_set_client_data(netif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP6, NULL);
            return NULL;
        }
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("%s: allocated dhcp6\n", dbg_requester));
        dhcp6->pcb_allocated = 1;
    }
    return dhcp6;
}

/*
 * Set the DHCPv6 state
 * If the state changed, reset the number of tries.
 */
static void
dhcp6_set_state(struct dhcp6 *dhcp6, u8_t new_state, const char *dbg_caller)
{
    LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("DHCPv6 state: %d -> %d (%s)\n",
                dhcp6->state, new_state, dbg_caller));
    if (new_state != dhcp6->state) {
        dhcp6->state = new_state;
        dhcp6->tries = 0;
        dhcp6->request_timeout = 0;
    }
}

static int
dhcp6_stateless_enabled(struct dhcp6 *dhcp6)
{
    if ((dhcp6->state == DHCP6_STATE_STATELESS_IDLE) ||
        (dhcp6->state == DHCP6_STATE_REQUESTING_CONFIG)) {
        return 1;
    }
    return 0;
}

#if LWIP_IPV6_DHCP6_STATEFUL
static int
dhcp6_stateful_enabled(struct dhcp6 *dhcp6)
{
    //  if (dhcp6->state == DHCP6_STATE_OFF) {
    //    return 0;
    //  }
    //  if (dhcp6_stateless_enabled(dhcp6)) {
    //    return 0;
    //  }
    //  return 1;
    if ((dhcp6->state >= DHCP6_STATE_STATEFUL_IDLE) &&
        (dhcp6->state <= DHCP6_STATE_STATEFUL_DELICINING)) {
        return 1;
    }
    return 0;
}

err_t
dhcp6_disable_stateful(struct netif *netif)
{
    struct dhcp6 *dhcp6;
    dhcp6 = dhcp6_get_struct(netif, "dhcp6_enable_stateful()");
    if (dhcp6 == NULL) {
        return ERR_MEM;
    }

    if (dhcp6_stateful_enabled(dhcp6)) {
        dhcp6_disable(netif);
    }

    /* netif_create_ip6_linklocal_address(netif, 1); */
    return ERR_OK;
}

/**
 * @ingroup dhcp6
 * Enable stateful DHCPv6 on this netif
 * Requests are sent on receipt of an RA message with the
 * ND6_RA_FLAG_MANAGED_ADDR_CONFIG flag set.
 *
 * A struct dhcp6 will be allocated for this netif if not
 * set via @ref dhcp6_set_struct before.
 *
 * @todo: stateful DHCPv6 not supported, yet
 */
err_t
dhcp6_enable_stateful(struct netif *netif)
{
    //  LWIP_UNUSED_ARG(netif);
    //  LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("stateful dhcp6 not implemented yet"));
    //  return ERR_VAL;
    struct dhcp6 *dhcp6;

    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_enable_stateful(netif=%p) %c%c%" U16_F "\n", (void *)netif, netif->name[0], netif->name[1], (u16_t)netif->num));

    dhcp6 = dhcp6_get_struct(netif, "dhcp6_enable_stateful()");
    if (dhcp6 == NULL) {
        return ERR_MEM;
    }
    if (dhcp6_stateful_enabled(dhcp6)) {
        LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp6_enable_stateful(): stateful DHCPv6 already enabled"));
        return ERR_OK;
    } else if (dhcp6->state != DHCP6_STATE_OFF) {
        /* stateless running */
        /* @todo: stop stateless once it is implemented */
        LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp6_enable_stateful(): switching from stateless to stateful DHCPv6"));
    }
    dhcp6_generate_duid(DHCP6_DUID_LL, netif->hwaddr, 0x2a978772, 0, &dhcp6->client_duid);
    //		dhcp6->ia_id = (dhcp6->client_duid.data.ll.mac_addr[2] << 24)\
//									| (dhcp6->client_duid.data.ll.mac_addr[3] << 16)\
//									| (dhcp6->client_duid.data.ll.mac_addr[4] << 8) \
//									| (dhcp6->client_duid.data.ll.mac_addr[5]);
    memcpy(&dhcp6->ia_id, dhcp6->client_duid.data.ll.mac_addr + 2, 4);

    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp6_enable_stateful(): stateful DHCPv6 enabled\n"));
    /* (re)start the DHCP negotiation */
    err_t result = dhcp6_solicit(netif);
    if (result != ERR_OK) {
        /* free resources allocated above */
        //    dhcp6_release_and_stop(netif);
        return ERR_MEM;
    }
    return result;
}
#endif /* LWIP_IPV6_DHCP6_STATEFUL */

/**
 * @ingroup dhcp6
 * Enable stateless DHCPv6 on this netif
 * Requests are sent on receipt of an RA message with the
 * ND6_RA_FLAG_OTHER_CONFIG flag set.
 *
 * A struct dhcp6 will be allocated for this netif if not
 * set via @ref dhcp6_set_struct before.
 */
err_t
dhcp6_enable_stateless(struct netif *netif)
{
    struct dhcp6 *dhcp6;

    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_enable_stateless(netif=%p) %c%c%"U16_F"\n", (void *)netif, netif->name[0], netif->name[1], (u16_t)netif->num));

    dhcp6 = dhcp6_get_struct(netif, "dhcp6_enable_stateless()");
    if (dhcp6 == NULL) {
        return ERR_MEM;
    }
    if (dhcp6_stateless_enabled(dhcp6)) {
        LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp6_enable_stateless(): stateless DHCPv6 already enabled\n"));
        return ERR_OK;
    } else if (dhcp6->state != DHCP6_STATE_OFF) {
        /* stateful running */
        /* @todo: stop stateful once it is implemented */
        LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp6_enable_stateless(): switching from stateful to stateless DHCPv6\n"));
    }
    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp6_enable_stateless(): stateless DHCPv6 enabled\n"));
    dhcp6_set_state(dhcp6, DHCP6_STATE_STATELESS_IDLE, "dhcp6_enable_stateless");
    return ERR_OK;
}

/**
 * @ingroup dhcp6
 * Disable stateful or stateless DHCPv6 on this netif
 * Requests are sent on receipt of an RA message with the
 * ND6_RA_FLAG_OTHER_CONFIG flag set.
 */
void
dhcp6_disable(struct netif *netif)
{
    struct dhcp6 *dhcp6;

    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_disable(netif=%p) %c%c%"U16_F"\n", (void *)netif, netif->name[0], netif->name[1], (u16_t)netif->num));

    dhcp6 = netif_dhcp6_data(netif);
    if (dhcp6 != NULL) {
        if (dhcp6->state != DHCP6_STATE_OFF) {
            LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_disable(): DHCPv6 disabled (old state: %s)\n",
                        (dhcp6_stateless_enabled(dhcp6) ? "stateless" : "stateful")));
            dhcp6_set_state(dhcp6, DHCP6_STATE_OFF, "dhcp6_disable");
            if (dhcp6->pcb_allocated != 0) {
                dhcp6_dec_pcb_refcount(); /* free DHCPv6 PCB if not needed any more */
                dhcp6->pcb_allocated = 0;
            }
        }
    }
}

/**
 * Create a DHCPv6 request, fill in common headers
 *
 * @param netif the netif under DHCPv6 control
 * @param dhcp6 dhcp6 control struct
 * @param message_type message type of the request
 * @param opt_len_alloc option length to allocate
 * @param options_out_len option length on exit
 * @return a pbuf for the message
 */
static struct pbuf *
dhcp6_create_msg(struct netif *netif, struct dhcp6 *dhcp6, u8_t message_type,
                 u16_t opt_len_alloc, u16_t *options_out_len)
{
    struct pbuf *p_out;
    struct dhcp6_msg *msg_out;

    LWIP_ERROR("dhcp6_create_msg: netif != NULL", (netif != NULL), return NULL;);
    LWIP_ERROR("dhcp6_create_msg: dhcp6 != NULL", (dhcp6 != NULL), return NULL;);
    p_out = pbuf_alloc(PBUF_TRANSPORT, sizeof(struct dhcp6_msg) + opt_len_alloc, PBUF_RAM);
    if (p_out == NULL) {
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_SERIOUS,
                    ("dhcp6_create_msg(): could not allocate pbuf\n"));
        return NULL;
    }
    LWIP_ASSERT("dhcp6_create_msg: check that first pbuf can hold struct dhcp6_msg",
                (p_out->len >= sizeof(struct dhcp6_msg) + opt_len_alloc));

    /* @todo: limit new xid for certain message types? */
    /* reuse transaction identifier in retransmissions */
    if (dhcp6->tries == 0) {
        dhcp6->xid = LWIP_RAND() & 0xFFFFFF;
    }

    LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE,
                ("transaction id xid(%"X32_F")\n", dhcp6->xid));

    msg_out = (struct dhcp6_msg *)p_out->payload;
    memset(msg_out, 0, sizeof(struct dhcp6_msg) + opt_len_alloc);

    msg_out->msgtype = message_type;
    msg_out->transaction_id[0] = (u8_t)(dhcp6->xid >> 16);
    msg_out->transaction_id[1] = (u8_t)(dhcp6->xid >> 8);
    msg_out->transaction_id[2] = (u8_t)dhcp6->xid;
    *options_out_len = 0;
    return p_out;
}

static u16_t
dhcp6_option_short(u16_t options_out_len, u8_t *options, u16_t value)
{
    options[options_out_len++] = (u8_t)((value & 0xff00U) >> 8);
    options[options_out_len++] = (u8_t)(value & 0x00ffU);
    return options_out_len;
}

#if LWIP_IPV6_DHCP6_STATEFUL
/*
 * Concatenate an option type and length field to the outgoing
 * DHCP6 message.
 *
 */
static u16_t
dhcp6_option(u16_t options_out_len, u8_t *options, u16_t option_type, u16_t option_len)
{
    options_out_len = dhcp6_option_short(options_out_len, options, option_type);
    options_out_len = dhcp6_option_short(options_out_len, options, option_len);
    return options_out_len;
}

/*
 * Concatenate a single byte to the outgoing DHCP6 message.
 *
 */
static u16_t
dhcp6_option_byte(u16_t options_out_len, u8_t *options, u8_t value)
{
    options[options_out_len++] = value;
    return options_out_len;
}

static u16_t
dhcp6_option_long(u16_t options_out_len, u8_t *options, u32_t value)
{
    options[options_out_len++] = (u8_t)((value & 0xff000000UL) >> 24);
    options[options_out_len++] = (u8_t)((value & 0x00ff0000UL) >> 16);
    options[options_out_len++] = (u8_t)((value & 0x0000ff00UL) >> 8);
    options[options_out_len++] = (u8_t)((value & 0x000000ffUL));
    return options_out_len;
}

static u16_t
dhcp6_option_duid(u16_t options_out_len, u8_t *options, DUID duid)
{
    size_t i;
    options_out_len = dhcp6_option_short(options_out_len, options, duid.type);
    switch (duid.type) {
    case DHCP6_DUID_LLT:
        options_out_len = dhcp6_option_short(options_out_len, options, duid.data.llt.hw_type);
        options_out_len = dhcp6_option_long(options_out_len, options, duid.data.llt.timestamp);
        for (i = 0; i < ETH_HWADDR_LEN; i++) {
            options_out_len = dhcp6_option_byte(options_out_len, options, duid.data.llt.mac_addr[i]);
        }
        break;
    case DHCP6_DUID_EN:
        options_out_len = dhcp6_option_long(options_out_len, options, duid.data.en.enterprise_number);
        for (i = 0; i < (sizeof(duid.data.en.variable) / sizeof(duid.data.en.variable[0])); i++) {
            options_out_len = dhcp6_option_byte(options_out_len, options, duid.data.en.variable[i]);
        }
        break;
    case DHCP6_DUID_LL:
        options_out_len = dhcp6_option_short(options_out_len, options, duid.data.ll.hw_type);
        for (i = 0; i < ETH_HWADDR_LEN; i++) {
            options_out_len = dhcp6_option_byte(options_out_len, options, duid.data.ll.mac_addr[i]);
        }
        break;
    case DHCP6_DUID_UUID:
        for (i = 0; i < (sizeof(duid.data.uuid.uuid) / sizeof(duid.data.uuid.uuid[0])); i++) {
            options_out_len = dhcp6_option_byte(options_out_len, options, duid.data.uuid.uuid[i]);
        }
        break;
    default:
        break;
    }
    return options_out_len;
}
#endif /* LWIP_IPV6_DHCP6_STATEFUL */

static u16_t
dhcp6_option_optionrequest(u16_t options_out_len, u8_t *options, const u16_t *req_options,
                           u16_t num_req_options, u16_t max_len)
{
    size_t i;
    u16_t ret;

    LWIP_ASSERT("dhcp6_option_optionrequest: options_out_len + sizeof(struct dhcp6_msg) + addlen <= max_len",
                sizeof(struct dhcp6_msg) + options_out_len + 4U + (2U * num_req_options) <= max_len);
    LWIP_UNUSED_ARG(max_len);

    ret = dhcp6_option_short(options_out_len, options, DHCP6_OPTION_ORO);
    ret = dhcp6_option_short(ret, options, 2 * num_req_options);
    for (i = 0; i < num_req_options; i++) {
        ret = dhcp6_option_short(ret, options, req_options[i]);
    }
    return ret;
}

/* All options are added, shrink the pbuf to the required size */
static void
dhcp6_msg_finalize(u16_t options_out_len, struct pbuf *p_out)
{
    /* shrink the pbuf to the actual content length */
    pbuf_realloc(p_out, (u16_t)(sizeof(struct dhcp6_msg) + options_out_len));
}

#if LWIP_IPV6_DHCP6_STATELESS
static void
dhcp6_information_request(struct netif *netif, struct dhcp6 *dhcp6)
{
    const u16_t requested_options[] = {
#if LWIP_DHCP6_PROVIDE_DNS_SERVERS
        DHCP6_OPTION_DNS_SERVERS,
        DHCP6_OPTION_DOMAIN_LIST
#endif
#if LWIP_DHCP6_GET_NTP_SRV
        , DHCP6_OPTION_SNTP_SERVERS
#endif
    };

    u16_t msecs;
    struct pbuf *p_out;
    u16_t options_out_len;
    LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_information_request()\n"));
    /* create and initialize the DHCP message header */
    p_out = dhcp6_create_msg(netif, dhcp6, DHCP6_INFOREQUEST, 4 + sizeof(requested_options), &options_out_len);
    if (p_out != NULL) {
        err_t err;
        struct dhcp6_msg *msg_out = (struct dhcp6_msg *)p_out->payload;
        u8_t *options = (u8_t *)(msg_out + 1);
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_information_request: making request\n"));

        options_out_len = dhcp6_option_optionrequest(options_out_len, options, requested_options,
                          LWIP_ARRAYSIZE(requested_options), p_out->len);
        LWIP_HOOK_DHCP6_APPEND_OPTIONS(netif, dhcp6, DHCP6_STATE_REQUESTING_CONFIG, msg_out,
                                       DHCP6_INFOREQUEST, options_out_len, p_out->len);
        dhcp6_msg_finalize(options_out_len, p_out);

        err = udp_sendto_if(dhcp6_pcb, p_out, &dhcp6_All_DHCP6_Relay_Agents_and_Servers, DHCP6_SERVER_PORT, netif);
        pbuf_free(p_out);
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_information_request: INFOREQUESTING -> %d\n", (int)err));
        LWIP_UNUSED_ARG(err);
    } else {
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_SERIOUS, ("dhcp6_information_request: could not allocate DHCP6 request\n"));
    }
    dhcp6_set_state(dhcp6, DHCP6_STATE_REQUESTING_CONFIG, "dhcp6_information_request");
    if (dhcp6->tries < 255) {
        dhcp6->tries++;
    }
    msecs = (u16_t)((dhcp6->tries < 6 ? 1 << dhcp6->tries : 60) * 1000);
    dhcp6->request_timeout = (u16_t)((msecs + DHCP6_TIMER_MSECS - 1) / DHCP6_TIMER_MSECS);
    LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_information_request(): set request timeout %"U16_F" msecs\n", msecs));
}

static err_t
dhcp6_request_config(struct netif *netif, struct dhcp6 *dhcp6)
{
    /* stateless mode enabled and no request running? */
    if (dhcp6->state == DHCP6_STATE_STATELESS_IDLE) {
        /* send Information-request and wait for answer; setup receive timeout */
        dhcp6_information_request(netif, dhcp6);
    }

    return ERR_OK;
}

static void
dhcp6_abort_config_request(struct dhcp6 *dhcp6)
{
    if (dhcp6->state == DHCP6_STATE_REQUESTING_CONFIG) {
        /* abort running request */
        dhcp6_set_state(dhcp6, DHCP6_STATE_STATELESS_IDLE, "dhcp6_abort_config_request");
    }
}

/* Handle a REPLY to INFOREQUEST
 * This parses DNS and NTP server addresses from the reply.
 */
static void
dhcp6_handle_config_reply(struct netif *netif, struct pbuf *p_msg_in)
{
    struct dhcp6 *dhcp6 = netif_dhcp6_data(netif);

    LWIP_UNUSED_ARG(dhcp6);
    LWIP_UNUSED_ARG(p_msg_in);

#if LWIP_DHCP6_PROVIDE_DNS_SERVERS
    if (dhcp6_option_given(dhcp6, DHCP6_OPTION_IDX_DNS_SERVER)) {
        ip_addr_t dns_addr;
        ip6_addr_t *dns_addr6;
        u16_t op_start = dhcp6_get_option_start(dhcp6, DHCP6_OPTION_IDX_DNS_SERVER);
        u16_t op_len = dhcp6_get_option_length(dhcp6, DHCP6_OPTION_IDX_DNS_SERVER);
        u16_t idx;
        u8_t n;

        ip_addr_set_zero_ip6(&dns_addr);
        dns_addr6 = ip_2_ip6(&dns_addr);
        for (n = 0, idx = op_start; (idx < op_start + op_len) && (n < LWIP_DHCP6_PROVIDE_DNS_SERVERS);
             n++, idx += sizeof(struct ip6_addr_packed)) {
            u16_t copied = pbuf_copy_partial(p_msg_in, dns_addr6, sizeof(struct ip6_addr_packed), idx);
            if (copied != sizeof(struct ip6_addr_packed)) {
                /* pbuf length mismatch */
                return;
            }
            ip6_addr_assign_zone(dns_addr6, IP6_UNKNOWN, netif);
            /* @todo: do we need a different offset than DHCP(v4)? */
#if MULTI_NETIF_SUPPORT
            dns_setserver_with_netif(netif, n, &dns_addr);
#else
            dns_setserver(n, &dns_addr);
#endif
        }
    }
    /* @ todo: parse and set Domain Search List */
#endif /* LWIP_DHCP6_PROVIDE_DNS_SERVERS */

#if LWIP_DHCP6_GET_NTP_SRV
    if (dhcp6_option_given(dhcp6, DHCP6_OPTION_IDX_NTP_SERVER)) {
        ip_addr_t ntp_server_addrs[LWIP_DHCP6_MAX_NTP_SERVERS];
        u16_t op_start = dhcp6_get_option_start(dhcp6, DHCP6_OPTION_IDX_NTP_SERVER);
        u16_t op_len = dhcp6_get_option_length(dhcp6, DHCP6_OPTION_IDX_NTP_SERVER);
        u16_t idx;
        u8_t n;

        for (n = 0, idx = op_start; (idx < op_start + op_len) && (n < LWIP_DHCP6_MAX_NTP_SERVERS);
             n++, idx += sizeof(struct ip6_addr_packed)) {
            u16_t copied;
            ip6_addr_t *ntp_addr6 = ip_2_ip6(&ntp_server_addrs[n]);
            ip_addr_set_zero_ip6(&ntp_server_addrs[n]);
            copied = pbuf_copy_partial(p_msg_in, ntp_addr6, sizeof(struct ip6_addr_packed), idx);
            if (copied != sizeof(struct ip6_addr_packed)) {
                /* pbuf length mismatch */
                return;
            }
            ip6_addr_assign_zone(ntp_addr6, IP6_UNKNOWN, netif);
        }
        dhcp6_set_ntp_servers(n, ntp_server_addrs);
    }
#endif /* LWIP_DHCP6_GET_NTP_SRV */
}
#endif /* LWIP_IPV6_DHCP6_STATELESS */

/** This function is called from nd6 module when an RA message is received
 * It triggers DHCPv6 requests (if enabled).
 */
void
dhcp6_nd6_ra_trigger(struct netif *netif, u8_t managed_addr_config, u8_t other_config)
{
    struct dhcp6 *dhcp6;

    LWIP_ASSERT("netif != NULL", netif != NULL);
    dhcp6 = netif_dhcp6_data(netif);

    LWIP_UNUSED_ARG(managed_addr_config);
    LWIP_UNUSED_ARG(other_config);
    LWIP_UNUSED_ARG(dhcp6);

#if LWIP_IPV6_DHCP6_STATELESS
    if (dhcp6 != NULL) {
        if (dhcp6_stateless_enabled(dhcp6)) {
            if (other_config) {
                dhcp6_request_config(netif, dhcp6);
            } else {
                dhcp6_abort_config_request(dhcp6);
            }
        }
    }
#endif /* LWIP_IPV6_DHCP6_STATELESS */
}

/**
 * Parse the DHCPv6 message and extract the DHCPv6 options.
 *
 * Extract the DHCPv6 options (offset + length) so that we can later easily
 * check for them or extract the contents.
 */
static err_t
dhcp6_parse_reply(struct pbuf *p, struct dhcp6 *dhcp6)
{
    u16_t offset;
    u16_t offset_max;
    u16_t options_idx;
    struct dhcp6_msg *msg_in;

    LWIP_UNUSED_ARG(dhcp6);

    /* clear received options */
    dhcp6_clear_all_options(dhcp6);
    msg_in = (struct dhcp6_msg *)p->payload;

    /* parse options */

    options_idx = sizeof(struct dhcp6_msg);
    /* parse options to the end of the received packet */
    offset_max = p->tot_len;

    offset = options_idx;
    /* at least 4 byte to read? */
    while ((offset + 4 <= offset_max)) {
        u8_t op_len_buf[4];
        u8_t *op_len;
        u16_t op;
        u16_t len;
        u16_t val_offset = (u16_t)(offset + 4);
        if (val_offset < offset) {
            /* overflow */
            return ERR_BUF;
        }
        /* copy option + length, might be split across pbufs */
        op_len = (u8_t *)pbuf_get_contiguous(p, op_len_buf, 4, 4, offset);
        if (op_len == NULL) {
            /* failed to get option and length */
            return ERR_VAL;
        }
        op = (op_len[0] << 8) | op_len[1];
        len = (op_len[2] << 8) | op_len[3];
        offset = val_offset + len;
        if (offset < val_offset) {
            /* overflow */
            return ERR_BUF;
        }

        switch (op) {
        case (DHCP6_OPTION_CLIENTID):
            dhcp6_got_option(dhcp6, DHCP6_OPTION_IDX_CLI_ID);
            dhcp6_set_option(dhcp6, DHCP6_OPTION_IDX_CLI_ID, val_offset, len);
            break;
        case (DHCP6_OPTION_SERVERID):
            dhcp6_got_option(dhcp6, DHCP6_OPTION_IDX_SERVER_ID);
            dhcp6_set_option(dhcp6, DHCP6_OPTION_IDX_SERVER_ID, val_offset, len);
            break;
#if LWIP_DHCP6_PROVIDE_DNS_SERVERS
        case (DHCP6_OPTION_DNS_SERVERS):
            dhcp6_got_option(dhcp6, DHCP6_OPTION_IDX_DNS_SERVER);
            dhcp6_set_option(dhcp6, DHCP6_OPTION_IDX_DNS_SERVER, val_offset, len);
            break;
        case (DHCP6_OPTION_DOMAIN_LIST):
            dhcp6_got_option(dhcp6, DHCP6_OPTION_IDX_DOMAIN_LIST);
            dhcp6_set_option(dhcp6, DHCP6_OPTION_IDX_DOMAIN_LIST, val_offset, len);
            break;
#endif /* LWIP_DHCP6_PROVIDE_DNS_SERVERS */
#if LWIP_DHCP6_GET_NTP_SRV
        case (DHCP6_OPTION_SNTP_SERVERS):
            dhcp6_got_option(dhcp6, DHCP6_OPTION_IDX_NTP_SERVER);
            dhcp6_set_option(dhcp6, DHCP6_OPTION_IDX_NTP_SERVER, val_offset, len);
            break;
#endif /* LWIP_DHCP6_GET_NTP_SRV*/
#if LWIP_IPV6_DHCP6_STATEFUL
        case (DHCP6_OPTION_IA_NA):
            dhcp6_got_option(dhcp6, DHCP6_OPTION_IDX_IA_NA);
            dhcp6_set_option(dhcp6, DHCP6_OPTION_IDX_IA_NA, val_offset, len);
            break;
#endif /* LWIP_IPV6_DHCP6_STATEFUL */
        default:
            LWIP_DEBUGF(DHCP6_DEBUG, ("skipping option %"U16_F" in options\n", op));
            LWIP_HOOK_DHCP6_PARSE_OPTION(ip_current_netif(), dhcp6, dhcp6->state, msg_in,
                                         msg_in->msgtype, op, len, q, val_offset);
            break;
        }
    }
    return ERR_OK;
}

static void
dhcp6_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    struct netif *netif = ip_current_input_netif();
    struct dhcp6 *dhcp6 = netif_dhcp6_data(netif);
    struct dhcp6_msg *reply_msg = (struct dhcp6_msg *)p->payload;
    u8_t msg_type;
    u32_t xid;

    LWIP_UNUSED_ARG(arg);

    /* Caught DHCPv6 message from netif that does not have DHCPv6 enabled? -> not interested */
    if ((dhcp6 == NULL) || (dhcp6->pcb_allocated == 0)) {
        goto free_pbuf_and_return;
    }

    LWIP_ERROR("invalid server address type", IP_IS_V6(addr), goto free_pbuf_and_return;);

    LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_recv(pbuf = %p) from DHCPv6 server %s port %"U16_F"\n", (void *)p,
                ipaddr_ntoa(addr), port));
    LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("pbuf->len = %"U16_F"\n", p->len));
    LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("pbuf->tot_len = %"U16_F"\n", p->tot_len));
    /* prevent warnings about unused arguments */
    LWIP_UNUSED_ARG(pcb);
    LWIP_UNUSED_ARG(addr);
    LWIP_UNUSED_ARG(port);

    if (p->len < sizeof(struct dhcp6_msg)) {
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_WARNING, ("DHCPv6 reply message or pbuf too short\n"));
        goto free_pbuf_and_return;
    }

    /* match transaction ID against what we expected */
    xid = reply_msg->transaction_id[0] << 16;
    xid |= reply_msg->transaction_id[1] << 8;
    xid |= reply_msg->transaction_id[2];
    if (xid != dhcp6->xid) {
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_WARNING,
                    ("transaction id mismatch reply_msg->xid(%"X32_F")!= dhcp6->xid(%"X32_F")\n", xid, dhcp6->xid));
        goto free_pbuf_and_return;
    }
    /* option fields could be unfold? */
    if (dhcp6_parse_reply(p, dhcp6) != ERR_OK) {
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_SERIOUS,
                    ("problem unfolding DHCPv6 message - too short on memory?\n"));
        goto free_pbuf_and_return;
    }

    /* read DHCP message type */
    msg_type = reply_msg->msgtype;
    /* message type is DHCP6 REPLY? */
    if (msg_type == DHCP6_REPLY) {
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("DHCP6_REPLY received\n"));
#if LWIP_IPV6_DHCP6_STATELESS
        /* in info-requesting state? */
        if (dhcp6->state == DHCP6_STATE_REQUESTING_CONFIG) {
            dhcp6_set_state(dhcp6, DHCP6_STATE_STATELESS_IDLE, "dhcp6_recv");
            dhcp6_handle_config_reply(netif, p);
        } else
#endif /* LWIP_IPV6_DHCP6_STATELESS */
#if LWIP_IPV6_DHCP6_STATEFUL
            if (dhcp6->state == DHCP6_STATE_STATEFUL_REQUESTING) {
                dhcp6->server_ip_addr = *addr;
                dhcp6_handle_reply(netif, p);
                dhcp6_bind(netif);
                dhcp6_set_state(dhcp6, DHCP6_STATE_STATEFUL_DAD, "dhcp6_recv");
                dhcp6->request_timeout = DHCP6_STATEFUL_DAD_TIMEOUT; // wait 10s for DAD(nd6)
            } else if (dhcp6->state == DHCP6_STATE_STATEFUL_CONFIRMING
                       | dhcp6->state == DHCP6_STATE_STATEFUL_RENEWING
                       | dhcp6->state == DHCP6_STATE_STATEFUL_REBINDING) {
                dhcp6_handle_reply(netif, p);
                dhcp6_bind(netif);
            }
        /*
         * RFC8415/page-72
         * When the client receives a valid Reply message in response to a Release and
         * Decline message, the client considers the Release and Decline event completed
         */
            else if (dhcp6->state == DHCP6_STATE_STATEFUL_RELEASEING) {
                dhcp6_disable(netif);
            } else if (dhcp6->state == DHCP6_STATE_STATEFUL_DELICINING) {
                dhcp6_solicit(netif);
            } else
#endif /* LWIP_IPV6_DHCP6_STATEFUL */
            {
                /* @todo: handle reply in other states? */
            }
    } else {
        /* @todo: handle other message types */
#if LWIP_IPV6_DHCP6_STATEFUL
        if (msg_type == DHCP6_ADVERTISE && dhcp6->state == DHCP6_STATE_STATEFUL_SOLICITING) {
            dhcp6_handle_advertise(netif, p);
        }
#endif /* LWIP_IPV6_DHCP6_STATEFUL */
    }

free_pbuf_and_return:
    pbuf_free(p);
}

/**
 * A DHCPv6 request has timed out.
 *
 * The timer that was started with the DHCPv6 request has
 * timed out, indicating no response was received in time.
 */
static void
dhcp6_timeout(struct netif *netif, struct dhcp6 *dhcp6)
{
    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp6_timeout()\n"));

    LWIP_UNUSED_ARG(netif);
    LWIP_UNUSED_ARG(dhcp6);

#if LWIP_IPV6_DHCP6_STATELESS
    /* back-off period has passed, or server selection timed out */
    if (dhcp6->state == DHCP6_STATE_REQUESTING_CONFIG) {
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_timeout(): retrying information request\n"));
        dhcp6_information_request(netif, dhcp6);
    }
#endif /* LWIP_IPV6_DHCP6_STATELESS */
#if LWIP_IPV6_DHCP6_STATEFUL
    else if (dhcp6->state == DHCP6_STATE_STATEFUL_SOLICITING) {
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_timeout(): retrying  solicit\n"));
        dhcp6_solicit(netif);
    } else if (dhcp6->state == DHCP6_STATE_STATEFUL_REQUESTING) {
        if (dhcp6->tries < REQ_MAX_RC) {
            LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_timeout(): retrying  request\n"));
            dhcp6_request(netif);
        } else {
            LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_timeout(): REQUESTING, soliciting\n"));
            dhcp6_solicit(netif);
        }
    } else if (dhcp6->state == DHCP6_STATE_STATEFUL_CONFIRMING) {
        if (((1 << dhcp6->tries) - 2) < CNF_MAX_RD) {
            LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_timeout(): retrying  confirm\n"));
            dhcp6_confirm(netif);
        } else {
            LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_timeout(): CONFIRMING, soliciting\n"));
            dhcp6_solicit(netif);
        }
    } else if (dhcp6->state == DHCP6_STATE_STATEFUL_RELEASEING) {
        if (dhcp6->tries < REL_MAX_RC) {
            LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_timeout(): retrying  release\n"));
            dhcp6_release(netif);
        } else {
            LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_timeout(): RELEASEING, disable\n"));
            dhcp6_disable(netif);
        }
    } else if (dhcp6->state == DHCP6_STATE_STATEFUL_DELICINING) {
        if (dhcp6->tries < DEC_MAX_RC) {
            dhcp6_delicine(netif);
        } else {
            LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_timeout(): DELICINING, bind\n"));
            dhcp6_bind(netif);
        }
    }
    /* Uncertainty, may lead to errors */
    else if (dhcp6->state == DHCP6_STATE_STATEFUL_RENEWING) {
        dhcp6_renew(netif);
    }
    /* Uncertainty, may lead to errors */
    else if (dhcp6->state == DHCP6_STATE_STATEFUL_REBINDING) {
        dhcp6_rebind(netif);
    } else if (dhcp6->state == DHCP6_STATE_STATEFUL_DAD) {
        if (ip6_addr_isduplicated(netif->ip6_addr_state[dhcp6->netif_addr_idx])) {
            LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_timeout(): DAD, delicine\n"));
            dhcp6_delicine(netif);
        } else {
            LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_timeout(): DAD, continue\n"));
            dhcp6_set_state(dhcp6, DHCP6_STATE_STATEFUL_IDLE, "dhcp6_timeout");
        }
    }
#endif /* LWIP_IPV6_DHCP6_STATEFUL */
}

/**
 * DHCPv6 timeout handling (this function must be called every 500ms,
 * see @ref DHCP6_TIMER_MSECS).
 *
 * A DHCPv6 server is expected to respond within a short period of time.
 * This timer checks whether an outstanding DHCPv6 request is timed out.
 */
void
dhcp6_tmr(void)
{
    struct netif *netif;
    /* loop through netif's */
    NETIF_FOREACH(netif) {
        struct dhcp6 *dhcp6 = netif_dhcp6_data(netif);
        /* only act on DHCPv6 configured interfaces */
        if (dhcp6 != NULL) {
            /* timer is active (non zero), and is about to trigger now */
            if (dhcp6->request_timeout > 1) {
                dhcp6->request_timeout--;
            } else if (dhcp6->request_timeout == 1) {
                dhcp6->request_timeout--;
                /* { dhcp6->request_timeout == 0 } */
                LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_tmr(): request timeout\n"));
                /* this client's request timeout triggered */
                dhcp6_timeout(netif, dhcp6);
            }
        }
    }
}

#if LWIP_IPV6_DHCP6_STATEFUL
static u16_t dhcp6_stateful_requested_options[] = {
#if LWIP_DHCP6_PROVIDE_DNS_SERVERS
    DHCP6_OPTION_DNS_SERVERS,
    DHCP6_OPTION_DOMAIN_LIST
#endif
#if LWIP_DHCP6_GET_NTP_SRV
    , DHCP6_OPTION_SNTP_SERVERS
#endif
};

static u16_t
dhcp6_get_duid_len(DUID duid)
{
    switch (duid.type) {
    case DHCP6_DUID_LLT:
        return DHCP6_DUID_LLT_LEN;
    case DHCP6_DUID_EN:
        return DHCP6_DUID_EN_LEN;
    case DHCP6_DUID_LL:
        return DHCP6_DUID_LL_LEN;
    case DHCP6_DUID_UUID:
        return DHCP6_DUID_UUID_LEN;
    default:
        // Unsupported DUID type
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_generate_duid(): Unsupported DUID type!\n"));
        return 0;
    }
}

static void
dhcp6_copy_duid_buf(DUID *dst_duid, u8_t *duid_buf, u32_t buf_len)
{
    dst_duid->type = (duid_buf[0] << 8) | duid_buf[1];
    switch (dst_duid->type) {
    case DHCP6_DUID_LLT:
        // Set hardware type
        dst_duid->data.llt.hw_type = (duid_buf[2] << 8) | duid_buf[3];
        // Set timestamp
        dst_duid->data.llt.timestamp = (duid_buf[4] << 24) | (duid_buf[5] << 16) | (duid_buf[6] << 8) | duid_buf[7];
        // Copy MAC address
        memcpy(dst_duid->data.llt.mac_addr, duid_buf + 8, ETH_HWADDR_LEN);
        break;
    case DHCP6_DUID_EN:
        // Set enterprise number
        dst_duid->data.en.enterprise_number = (duid_buf[2] << 24) | (duid_buf[3] << 16) | (duid_buf[4] << 8) | duid_buf[5];
        memcpy(dst_duid->data.en.variable, duid_buf + 6, buf_len - 6);
        // Copy variable part
        // (In this example, we assume variable part has length 0)
        break;
    case DHCP6_DUID_LL:
        // Set hardware type
        dst_duid->data.ll.hw_type = (duid_buf[2] << 8) | duid_buf[3];
        // Set hardware address length (MAC address length)
        // Copy MAC address
        memcpy(dst_duid->data.ll.mac_addr, duid_buf + 4, ETH_HWADDR_LEN);
        break;
    case DHCP6_DUID_UUID:
        // Generate UUID (not implemented in this example)
        memcpy(dst_duid->data.uuid.uuid, duid_buf, 16);
        break;
    default:
        // Unsupported DUID type
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_generate_duid(): Unsupported DUID type!\n"));
        return;
    }
}

static void
dhcp6_t1_timeout(struct netif *netif)
{
    struct dhcp6 *dhcp6 = netif_dhcp6_data(netif);

    LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_STATE, ("dhcp_t1_timeout()\n"));
    if ((dhcp6->state == DHCP6_STATE_STATEFUL_IDLE) || (dhcp6->state == DHCP6_STATE_STATEFUL_RENEWING)) {
        /* just retry to renew - note that the rebind timer (t2) will
         * eventually time-out if renew tries fail. */
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE,
                    ("dhcp_t1_timeout(): must renew\n"));
        dhcp6_renew(netif);
        /* Calculate next timeout */
        if (((dhcp6->t2_timeout - dhcp6->lease_used) / 2) >= ((60 + DHCP6_COARSE_TIMER_SECS / 2) / DHCP6_COARSE_TIMER_SECS)) {
            dhcp6->t1_renew_time = (u16_t)((dhcp6->t2_timeout - dhcp6->lease_used) / 2);
        }
    }
}

static void
dhcp6_t2_timeout(struct netif *netif)
{
    struct dhcp6 *dhcp6 = netif_dhcp6_data(netif);

    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_STATE, ("dhcp_t1_timeout()\n"));
    if ((dhcp6->state == DHCP6_STATE_STATEFUL_IDLE) || (dhcp6->state == DHCP6_STATE_STATEFUL_RENEWING) ||
        (dhcp6->state == DHCP6_STATE_STATEFUL_REBINDING)) {
        /* just retry to rebind */
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE,
                    ("dhcp_t2_timeout(): must rebind\n"));
        dhcp6_rebind(netif);
        /* Calculate next timeout */
        if (((dhcp6->t0_timeout - dhcp6->lease_used) / 2) >= ((60 + DHCP6_COARSE_TIMER_SECS / 2) / DHCP6_COARSE_TIMER_SECS)) {
            dhcp6->t2_rebind_time = (u16_t)((dhcp6->t0_timeout - dhcp6->lease_used) / 2);
        }
    }
}

static void
dhcp6_generate_duid(int duid_type, uint8_t *mac_addr, uint32_t timestamp, uint32_t enterprise_number, DUID *duid_buf)
{
    DUID duid;
    // Set DUID type
    duid.type = duid_type;
    switch (duid_type) {
    case DHCP6_DUID_LLT:
        // Set hardware type (Ethernet)
        duid.data.llt.hw_type = HW_TYPE_ETHERNET;
        // Set timestamp
        duid.data.llt.timestamp = timestamp;
        // Copy MAC address
        memcpy(duid.data.llt.mac_addr, mac_addr, ETH_HWADDR_LEN);
        break;
    case DHCP6_DUID_EN:
        // Set enterprise number
        duid.data.en.enterprise_number = enterprise_number;
        // Copy variable part
        // (In this example, we assume variable part has length 0)
        break;
    case DHCP6_DUID_LL:
        // Set hardware type (Ethernet)
        duid.data.ll.hw_type = HW_TYPE_ETHERNET;
        // Set hardware address length (MAC address length)
        // Copy MAC address
        memcpy(duid.data.ll.mac_addr, mac_addr, ETH_HWADDR_LEN);
        break;
    case DHCP6_DUID_UUID:
        // Generate UUID (not implemented in this example)
        for (int i = 0; i < 16; i += 4) {
            uint32_t rand_value = LWIP_RAND();
            duid.data.uuid.uuid[i] = (rand_value >> 24) & 0xFF;
            duid.data.uuid.uuid[i + 1] = (rand_value >> 16) & 0xFF;
            duid.data.uuid.uuid[i + 2] = (rand_value >> 8) & 0xFF;
            duid.data.uuid.uuid[i + 3] = rand_value & 0xFF;
        }
        break;
    default:
        // Unsupported DUID type
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_generate_duid(): Unsupported DUID type!\n"));
        return;
    }

    // Copy DUID structure to buffer
    memcpy(duid_buf, &duid, sizeof(DUID));
}

/**
 * Bind the interface to the offered IP address.
 *
 * @param netif network interface to bind to the offered address
 */
static void
dhcp6_bind(struct netif *netif)
{
    struct dhcp6 *dhcp6 = netif_dhcp6_data(netif);
    u32_t timeout;
    u32_t t1;
    u32_t t2;

    /* reset time used of lease */
    dhcp6->lease_used = 0;

    // Check if T1 is non-zero and T2 is non-zero , If *t1 > *t2, consider the times as invalid
    if (dhcp6->offered_t1_renew != 0 && dhcp6->offered_t2_rebind != 0 && dhcp6->offered_t1_renew < dhcp6->offered_t2_rebind) {
        t1 = dhcp6->offered_t1_renew;
        t2 = dhcp6->offered_t2_rebind;
    }
    // Check if T1 is zero and T2 is non-zero
    else if (dhcp6->offered_t1_renew == 0 && dhcp6->offered_t2_rebind != 0) {
        t1 = dhcp6->offered_preferred_lifetime / 2;
        t2 = dhcp6->offered_t2_rebind;
        if (t1 > t2) {
            goto valid_cal_time;
        }
    }
    // Check if T1 is non-zero and T2 is zero
    else if (dhcp6->offered_t1_renew != 0 && dhcp6->offered_t2_rebind == 0) {
        t1 = dhcp6->offered_t1_renew;
        t2 = dhcp6->offered_preferred_lifetime * 0.8;
        // If *t1 > *t2, consider the times as invalid
        if (t1 > t2) {
            goto valid_cal_time;
        }
    }
    // Default case: both T1 and T2 are zero or invalid calculation
    else if (dhcp6->offered_t1_renew == 0 && dhcp6->offered_t2_rebind == 0) {
valid_cal_time:
        // Calculate renew and rebind based on preferred lifetime
        t1 = dhcp6->offered_preferred_lifetime / 2;
        t2 = dhcp6->offered_preferred_lifetime * 0.8;
    }

    if (t1 != 0xffffffffUL) {
        /* set renewal period timer */
        LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp6_bind(): t1 renewal timer %"U32_F" secs\n", t1));
        timeout = (t1 + DHCP6_COARSE_TIMER_SECS / 2) / DHCP6_COARSE_TIMER_SECS;
        if (timeout > 0xffff) {
            timeout = 0xffff;
        }
        dhcp6->t1_timeout = (u16_t)timeout;
        if (dhcp6->t1_timeout == 0) {
            dhcp6->t1_timeout = 1;
        }
        LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_bind(): set request timeout %"U32_F" msecs\n", t1 * 1000));
        dhcp6->t1_renew_time = dhcp6->t1_timeout;
        dhcp6->t0_timeout = 2 * dhcp6->t1_timeout; /* default T0 = 2 * T1 */
    }
    /* set renewal period timer */
    if (t2 != 0xffffffffUL) {
        LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp6_bind(): t2 rebind timer %"U32_F" secs\n", t2));
        timeout = (t2 + DHCP6_COARSE_TIMER_SECS / 2) / DHCP6_COARSE_TIMER_SECS;
        if (timeout > 0xffff) {
            timeout = 0xffff;
        }
        dhcp6->t2_timeout = (u16_t)timeout;
        if (dhcp6->t2_timeout == 0) {
            dhcp6->t2_timeout = 1;
        }
        LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_bind(): set request timeout %"U32_F" msecs\n", t2 * 1000));
        dhcp6->t2_rebind_time = dhcp6->t2_timeout;
    }

    /* If we have sub 1 minute lease, t2 and t1 will kick in at the same time. */
    if ((dhcp6->t1_timeout >= dhcp6->t2_timeout) && (dhcp6->t2_timeout > 0)) {
        dhcp6->t1_timeout = 0;
    }

    if (dhcp6->netif_addr_idx == 0xff) {
        /* Construct the new address that we intend to use, and then see if that
         * address really does not exist. It might have been added manually, after
         * all. As a side effect, find a free slot. Note that we cannot use
         * netif_add_ip6_address() here, as it would return ERR_OK if the address
         * already did exist, resulting in that address being given lifetimes. */
        s8_t i, free_idx;
        free_idx = 0;
        for (i = 1; i < LWIP_IPV6_NUM_ADDRESSES; i++) {
            if (!ip6_addr_isinvalid(netif_ip6_addr_state(netif, i))) {
                if (ip6_addr_cmp(&dhcp6->offered_ip_addr, netif_ip6_addr(netif, i))) {
                    return; /* formed address already exists */
                }
            } else if (free_idx == 0) {
                free_idx = i;
            }
        }
        if (free_idx == 0) {
            return; /* no address slots available, try again on next advertisement */
        }

        /* Assign the new address to the interface. */
        ip_addr_copy_from_ip6(netif->ip6_addr[free_idx], dhcp6->offered_ip_addr);
        netif_ip6_addr_set_valid_life(netif, free_idx, dhcp6->offered_valid_lifetime);
        netif_ip6_addr_set_pref_life(netif, free_idx, dhcp6->offered_preferred_lifetime);
        netif_ip6_addr_set_state(netif, free_idx, IP6_ADDR_TENTATIVE);
        dhcp6->netif_addr_idx = free_idx;
    } else {
        ip_addr_copy_from_ip6(netif->ip6_addr[dhcp6->netif_addr_idx], dhcp6->offered_ip_addr);
        netif_ip6_addr_set_valid_life(netif, dhcp6->netif_addr_idx, dhcp6->offered_valid_lifetime);
        netif_ip6_addr_set_pref_life(netif, dhcp6->netif_addr_idx, dhcp6->offered_preferred_lifetime);
        netif_ip6_addr_set_state(netif, dhcp6->netif_addr_idx, IP6_ADDR_TENTATIVE);
    }

    dhcp6_set_state(dhcp6, DHCP6_STATE_STATEFUL_IDLE, "dhcp6_bind");
}

/**
 * Start the DHCPv6 process, discover a DHCPv6 server.
 *
 * @param netif the netif under DHCPv6 control
 */
static err_t
dhcp6_solicit(struct netif *netif)
{
    struct dhcp6 *dhcp6 = netif_dhcp6_data(netif);
    err_t result = ERR_OK;
    u16_t msecs;
    u8_t i;
    struct pbuf *p_out;
    u16_t options_out_len;
    u16_t pbuf_len_alloc;

    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp6_solicit()\n"));

    ip6_addr_set_any(&dhcp6->offered_ip_addr);
    dhcp6_set_state(dhcp6, DHCP6_STATE_STATEFUL_SOLICITING, "dhcp6_solicit");
    /* create and initialize the DHCPv6 message header */
    pbuf_len_alloc = 4 + 2;
#if LWIP_NETIF_HOSTNAME
    pbuf_len_alloc += 4 + 2 + strlen(netif->hostname);
#endif /* LWIP_NETIF_HOSTNAME */
    pbuf_len_alloc +=	4 + dhcp6_get_duid_len(dhcp6->client_duid);
    pbuf_len_alloc += 4 + 12;
    pbuf_len_alloc += 4 + sizeof(dhcp6_stateful_requested_options);
    p_out = dhcp6_create_msg(netif, dhcp6, DHCP6_SOLICIT, pbuf_len_alloc, &options_out_len);
    if (p_out != NULL) {
        struct dhcp6_msg *msg_out = (struct dhcp6_msg *)p_out->payload;
        u8_t *options = (u8_t *)(msg_out + 1);

        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_ELAPSED_TIME, 2); //ELAPSED_TIME
        options_out_len = dhcp6_option_short(options_out_len, options, dhcp6->tries * DHCP6_TIMER_MSECS); //Estimated time

#if LWIP_NETIF_HOSTNAME
        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_CLIENT_FQDN, 2 + strlen(netif->hostname)); //DOMAIN_NAME
        options_out_len = dhcp6_option_byte(options_out_len, options, 0x00); // FLAG
        options_out_len = dhcp6_option_byte(options_out_len, options, strlen(netif->hostname)); //DOMAIN_NAME_LEN
        for (i = 0; i < strlen(netif->hostname); i++) {
            options_out_len = dhcp6_option_byte(options_out_len, options, netif->hostname[i]); //DOMAIN_NAME
        }
#endif /* LWIP_NETIF_HOSTNAME */

        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_CLIENTID, dhcp6_get_duid_len(dhcp6->client_duid));
        options_out_len = dhcp6_option_duid(options_out_len, options, dhcp6->client_duid);

        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_IA_NA, 12);
        options_out_len = dhcp6_option_long(options_out_len, options, dhcp6->ia_id); // IA_ID
        options_out_len = dhcp6_option_long(options_out_len, options, 0); // T1
        options_out_len = dhcp6_option_long(options_out_len, options, 0); // T2

        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_solicit: making request\n"));
        options_out_len = dhcp6_option_optionrequest(options_out_len, options, dhcp6_stateful_requested_options,
                          LWIP_ARRAYSIZE(dhcp6_stateful_requested_options), p_out->len);
        LWIP_HOOK_DHCP6_APPEND_OPTIONS(netif, dhcp6, DHCP6_STATE_REQUESTING_CONFIG, msg_out,
                                       DHCP6_SOLICIT, options_out_len, p_out->len);
        dhcp6_msg_finalize(options_out_len, p_out);

        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_solicit: sendto(solicit, dhcp6_All_DHCP6_Relay_Agents_and_Servers, DHCP6_SERVER_PORT)\n"));
        udp_sendto_if(dhcp6_pcb, p_out, &dhcp6_All_DHCP6_Relay_Agents_and_Servers, DHCP6_SERVER_PORT, netif);
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_solicit: deleting()ing\n"));
        pbuf_free(p_out);
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_solicit: SOLICITING\n"));
    } else {
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_SERIOUS, ("dhcp6_solicit: could not allocate DHCP request\n"));
    }
    if (dhcp6->tries < 255) {
        dhcp6->tries++;
    }
    msecs = (u16_t)((dhcp6->tries < 6 ? 1 << dhcp6->tries : 60) * 1000);
    dhcp6->request_timeout = (u16_t)((msecs + DHCP6_TIMER_MSECS - 1) / DHCP6_TIMER_MSECS);
    LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_solicit(): set request timeout %"U16_F" msecs\n", msecs));
    return result;
}

/**
 * Select a DHCPv6 server offer out of all offers.
 *
 * Simply select the first offer received.
 *
 * @param netif the netif under DHCPv6 control
 * @return lwIP specific error (see error.h)
 */
static
err_t dhcp6_request(struct netif *netif)
{
    struct dhcp6 *dhcp6 = netif_dhcp6_data(netif);
    err_t result = ERR_OK;
    u16_t msecs;
    u8_t i;
    struct pbuf *p_out;
    u16_t options_out_len;
    u16_t pbuf_len_alloc;

    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp6_request()\n"));
    dhcp6_set_state(dhcp6, DHCP6_STATE_STATEFUL_REQUESTING, "dhcp6_request");
    /* create and initialize the DHCPv6 message header */
    pbuf_len_alloc = 4 + 2;
#if LWIP_NETIF_HOSTNAME
    pbuf_len_alloc += 4 + 2 + strlen(netif->hostname);
#endif /* LWIP_NETIF_HOSTNAME */
    pbuf_len_alloc +=	4 + dhcp6_get_duid_len(dhcp6->client_duid);
    pbuf_len_alloc += 4 + dhcp6_get_duid_len(dhcp6->server_duid);
    pbuf_len_alloc += 4 + 12 + 4 + 24;
    pbuf_len_alloc += 4 + sizeof(dhcp6_stateful_requested_options);
    p_out = dhcp6_create_msg(netif, dhcp6, DHCP6_REQUEST, pbuf_len_alloc, &options_out_len);
    if (p_out != NULL) {
        struct dhcp6_msg *msg_out = (struct dhcp6_msg *)p_out->payload;
        u8_t *options = (u8_t *)(msg_out + 1);

        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_ELAPSED_TIME, 2); //ELAPSED_TIME
        options_out_len = dhcp6_option_short(options_out_len, options, dhcp6->tries * DHCP6_TIMER_MSECS); //Estimated time

#if LWIP_NETIF_HOSTNAME
        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_CLIENT_FQDN, 2 + strlen(netif->hostname)); //DOMAIN_NAME
        options_out_len = dhcp6_option_byte(options_out_len, options, 0x00); // FLAG
        options_out_len = dhcp6_option_byte(options_out_len, options, strlen(netif->hostname)); //DOMAIN_NAME_LEN
        for (i = 0; i < strlen(netif->hostname); i++) {
            options_out_len = dhcp6_option_byte(options_out_len, options, netif->hostname[i]); //DOMAIN_NAME
        }
#endif /* LWIP_NETIF_HOSTNAME */

        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_CLIENTID, dhcp6_get_duid_len(dhcp6->client_duid));
        options_out_len = dhcp6_option_duid(options_out_len, options, dhcp6->client_duid);

        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_SERVERID, dhcp6_get_duid_len(dhcp6->server_duid));
        options_out_len = dhcp6_option_duid(options_out_len, options, dhcp6->server_duid);

        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_IA_NA, 40);
        options_out_len = dhcp6_option_long(options_out_len, options, dhcp6->ia_id); // IA_ID
        options_out_len = dhcp6_option_long(options_out_len, options, dhcp6->offered_t1_renew); // T1
        options_out_len = dhcp6_option_long(options_out_len, options, dhcp6->offered_t2_rebind); // T2
        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_IAADDR, 24);
        for (i = 0; i < sizeof(dhcp6->offered_ip_addr.addr); i++) {
            options_out_len = dhcp6_option_byte(options_out_len, options, ((u8_t *)dhcp6->offered_ip_addr.addr)[i]); //OFFERED_IP
        }
        options_out_len = dhcp6_option_long(options_out_len, options, dhcp6->offered_preferred_lifetime);
        options_out_len = dhcp6_option_long(options_out_len, options, dhcp6->offered_valid_lifetime);

        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_request: making request\n"));
        options_out_len = dhcp6_option_optionrequest(options_out_len, options, dhcp6_stateful_requested_options,
                          LWIP_ARRAYSIZE(dhcp6_stateful_requested_options), p_out->len);
        LWIP_HOOK_DHCP6_APPEND_OPTIONS(netif, dhcp6, DHCP6_STATE_REQUESTING_CONFIG, msg_out,
                                       DHCP6_REQUEST, options_out_len, p_out->len);
        dhcp6_msg_finalize(options_out_len, p_out);

        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_request: sendto(request, dhcp6_All_DHCP6_Relay_Agents_and_Servers, DHCP6_SERVER_PORT)\n"));
        udp_sendto_if(dhcp6_pcb, p_out, &dhcp6_All_DHCP6_Relay_Agents_and_Servers, DHCP6_SERVER_PORT, netif);
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_request: deleting()ing\n"));
        pbuf_free(p_out);
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_request: REQUESTING\n"));
    } else {
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_SERIOUS, ("dhcp6_request: could not allocate DHCPv6 request\n"));
    }

    msecs = (u16_t)((((1 << (dhcp6->tries)) * REQ_TIMEOUT) < REQ_MAX_RT ? ((1 << dhcp6->tries) * REQ_TIMEOUT) : REQ_MAX_RT) * 1000);
    if (dhcp6->tries < 255) {
        dhcp6->tries++;
    }
    dhcp6->request_timeout = (u16_t)((msecs + DHCP6_TIMER_MSECS - 1) / DHCP6_TIMER_MSECS);
    LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_request(): set request timeout %"U16_F" msecs\n", msecs));
    return result;
}

static err_t
dhcp6_confirm(struct netif *netif)
{
    struct dhcp6 *dhcp6 = netif_dhcp6_data(netif);
    err_t result = ERR_OK;
    u16_t msecs;
    u8_t i;
    struct pbuf *p_out;
    u16_t options_out_len;
    u16_t pbuf_len_alloc;

    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp6_confirm()\n"));
    dhcp6_set_state(dhcp6, DHCP6_STATE_STATEFUL_CONFIRMING, "dhcp6_confirm");
    /* create and initialize the DHCPv6 message header */
    pbuf_len_alloc = 4 + 2;
#if LWIP_NETIF_HOSTNAME
    pbuf_len_alloc += 4 + 2 + strlen(netif->hostname);
#endif /* LWIP_NETIF_HOSTNAME */
    pbuf_len_alloc +=	4 + dhcp6_get_duid_len(dhcp6->client_duid);
    pbuf_len_alloc += 4 + 12 + 4 + 24;
    p_out = dhcp6_create_msg(netif, dhcp6, DHCP6_CONFIRM, pbuf_len_alloc, &options_out_len);
    if (p_out != NULL) {
        struct dhcp6_msg *msg_out = (struct dhcp6_msg *)p_out->payload;
        u8_t *options = (u8_t *)(msg_out + 1);

        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_ELAPSED_TIME, 2); //ELAPSED_TIME
        options_out_len = dhcp6_option_short(options_out_len, options, dhcp6->tries * DHCP6_TIMER_MSECS); //Estimated time

#if LWIP_NETIF_HOSTNAME
        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_CLIENT_FQDN, 2 + strlen(netif->hostname)); //DOMAIN_NAME
        options_out_len = dhcp6_option_byte(options_out_len, options, 0x00); // FLAG
        options_out_len = dhcp6_option_byte(options_out_len, options, strlen(netif->hostname)); //DOMAIN_NAME_LEN
        for (i = 0; i < strlen(netif->hostname); i++) {
            options_out_len = dhcp6_option_byte(options_out_len, options, netif->hostname[i]); //DOMAIN_NAME
        }
#endif /* LWIP_NETIF_HOSTNAME */

        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_CLIENTID, dhcp6_get_duid_len(dhcp6->client_duid));
        options_out_len = dhcp6_option_duid(options_out_len, options, dhcp6->client_duid);

        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_IA_NA, 40);
        options_out_len = dhcp6_option_long(options_out_len, options, dhcp6->ia_id); // IA_ID
        options_out_len = dhcp6_option_long(options_out_len, options, 0); // T1
        options_out_len = dhcp6_option_long(options_out_len, options, 0); // T2
        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_IAADDR, 24);
        for (i = 0; i < sizeof(dhcp6->offered_ip_addr.addr); i++) {
            options_out_len = dhcp6_option_byte(options_out_len, options, ((u8_t *)dhcp6->offered_ip_addr.addr)[i]); //OFFERED_IP
        }
        options_out_len = dhcp6_option_long(options_out_len, options, 0);
        options_out_len = dhcp6_option_long(options_out_len, options, 0);

        dhcp6_msg_finalize(options_out_len, p_out);

        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_confirm: sendto(confirm, dhcp6_All_DHCP6_Relay_Agents_and_Servers, DHCP6_SERVER_PORT)\n"));
        udp_sendto_if(dhcp6_pcb, p_out, &dhcp6_All_DHCP6_Relay_Agents_and_Servers, DHCP6_SERVER_PORT, netif);
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_confirm: deleting()ing\n"));
        pbuf_free(p_out);
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_confirm: CONFIRMING\n"));
    } else {
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_SERIOUS, ("dhcp6_confirm: could not allocate DHCPv6 request\n"));
    }

    msecs = (u16_t)((((1 << dhcp6->tries) * CNF_TIMEOUT) < CNF_MAX_RT ? (1 << dhcp6->tries) * CNF_TIMEOUT : CNF_MAX_RT) * 1000);
    if (dhcp6->tries < 255) {
        dhcp6->tries++;
    }
    dhcp6->request_timeout = (u16_t)((msecs + DHCP6_TIMER_MSECS - 1) / DHCP6_TIMER_MSECS);
    LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_confirm(): set request timeout %"U16_F" msecs\n", msecs));
    return result;
}

/**
 * @ingroup dhcp6
 * Renew an existing DHCPv6 lease at the involved DHCPv6 server.
 *
 * @param netif network interface which must renew its lease
 */
static err_t
dhcp6_renew(struct netif *netif)
{
    struct dhcp6 *dhcp6 = netif_dhcp6_data(netif);
    err_t result = ERR_OK;
    u16_t msecs;
    u8_t i;
    struct pbuf *p_out;
    u16_t options_out_len;
    u16_t pbuf_len_alloc;

    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp6_renew()\n"));

//  ip6_addr_set_any(&dhcp6->offered_ip_addr);
    dhcp6_set_state(dhcp6, DHCP6_STATE_STATEFUL_RENEWING, "dhcp6_renew");
    /* create and initialize the DHCPv6 message header */
    pbuf_len_alloc = 4 + 2;
#if LWIP_NETIF_HOSTNAME
    pbuf_len_alloc += 4 + 2 + strlen(netif->hostname);
#endif /* LWIP_NETIF_HOSTNAME */
    pbuf_len_alloc +=	4 + dhcp6_get_duid_len(dhcp6->client_duid);
    pbuf_len_alloc += 4 + dhcp6_get_duid_len(dhcp6->server_duid);
    pbuf_len_alloc += 4 + 12 + 4 + 24;
    pbuf_len_alloc += 4 + sizeof(dhcp6_stateful_requested_options);
    p_out = dhcp6_create_msg(netif, dhcp6, DHCP6_RENEW, pbuf_len_alloc, &options_out_len);
    if (p_out != NULL) {
        struct dhcp6_msg *msg_out = (struct dhcp6_msg *)p_out->payload;
        u8_t *options = (u8_t *)(msg_out + 1);

        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_ELAPSED_TIME, 2); //ELAPSED_TIME
        options_out_len = dhcp6_option_short(options_out_len, options, dhcp6->tries * DHCP6_TIMER_MSECS); //Estimated time

#if LWIP_NETIF_HOSTNAME
        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_CLIENT_FQDN, 2 + strlen(netif->hostname)); //DOMAIN_NAME
        options_out_len = dhcp6_option_byte(options_out_len, options, 0x00); // FLAG
        options_out_len = dhcp6_option_byte(options_out_len, options, strlen(netif->hostname)); //DOMAIN_NAME_LEN
        for (i = 0; i < strlen(netif->hostname); i++) {
            options_out_len = dhcp6_option_byte(options_out_len, options, netif->hostname[i]); //DOMAIN_NAME
        }
#endif /* LWIP_NETIF_HOSTNAME */

        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_CLIENTID, dhcp6_get_duid_len(dhcp6->client_duid));
        options_out_len = dhcp6_option_duid(options_out_len, options, dhcp6->client_duid);

        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_SERVERID, dhcp6_get_duid_len(dhcp6->server_duid));
        options_out_len = dhcp6_option_duid(options_out_len, options, dhcp6->server_duid);

        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_IA_NA, 40);
        options_out_len = dhcp6_option_long(options_out_len, options, dhcp6->ia_id); // IA_ID
        options_out_len = dhcp6_option_long(options_out_len, options, dhcp6->offered_t1_renew); // T1
        options_out_len = dhcp6_option_long(options_out_len, options, dhcp6->offered_t2_rebind); // T2
        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_IAADDR, 24);
        for (i = 0; i < sizeof(dhcp6->offered_ip_addr.addr); i++) {
            options_out_len = dhcp6_option_byte(options_out_len, options, ((u8_t *)dhcp6->offered_ip_addr.addr)[i]); //OFFERED_IP
        }
        options_out_len = dhcp6_option_long(options_out_len, options, dhcp6->offered_preferred_lifetime);
        options_out_len = dhcp6_option_long(options_out_len, options, dhcp6->offered_valid_lifetime);

        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_renew: making request\n"));
        options_out_len = dhcp6_option_optionrequest(options_out_len, options, dhcp6_stateful_requested_options,
                          LWIP_ARRAYSIZE(dhcp6_stateful_requested_options), p_out->len);
        LWIP_HOOK_DHCP6_APPEND_OPTIONS(netif, dhcp6, DHCP6_STATE_REQUESTING_CONFIG, msg_out,
                                       DHCP6_RENEW, options_out_len, p_out->len);
        dhcp6_msg_finalize(options_out_len, p_out);

        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_renew: sendto(renew, dhcp6_All_DHCP6_Relay_Agents_and_Servers, DHCP6_SERVER_PORT)\n"));
        udp_sendto_if(dhcp6_pcb, p_out, &dhcp6_All_DHCP6_Relay_Agents_and_Servers, DHCP6_SERVER_PORT, netif);
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_renew: deleting()ing\n"));
        pbuf_free(p_out);
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_renew: RENEWING\n"));
    } else {
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_SERIOUS, ("dhcp6_renew: could not allocate DHCPv6 request\n"));
    }

    msecs = (u16_t)((((1 << (dhcp6->tries)) * REN_TIMEOUT) < REN_MAX_RT ? ((1 << dhcp6->tries) * REN_TIMEOUT) : REN_MAX_RT) * 1000);
    if (dhcp6->tries < 255) {
        dhcp6->tries++;
    }
    dhcp6->request_timeout = (u16_t)((msecs + DHCP6_TIMER_MSECS - 1) / DHCP6_TIMER_MSECS);
    LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_renew(): set request timeout %"U16_F" msecs\n", msecs));
    return result;
}

/**
 * @ingroup dhcp6
 * Rebind with a DHCPv6 server for an existing DHCPv6 lease.
 *
 * @param netif network interface which must renew its lease
 */
static err_t
dhcp6_rebind(struct netif *netif)
{
    struct dhcp6 *dhcp6 = netif_dhcp6_data(netif);
    err_t result = ERR_OK;
    u16_t msecs;
    u8_t i;
    struct pbuf *p_out;
    u16_t options_out_len;
    u16_t pbuf_len_alloc;

    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp6_rebind()\n"));

//  ip6_addr_set_any(&dhcp6->offered_ip_addr);
    dhcp6_set_state(dhcp6, DHCP6_STATE_STATEFUL_REBINDING, "dhcp6_rebind");
    /* create and initialize the DHCPv6 message header */
    pbuf_len_alloc = 4 + 2;
#if LWIP_NETIF_HOSTNAME
    pbuf_len_alloc += 4 + 2 + strlen(netif->hostname);
#endif /* LWIP_NETIF_HOSTNAME */
    pbuf_len_alloc +=	4 + dhcp6_get_duid_len(dhcp6->client_duid);
    pbuf_len_alloc += 4 + 12 + 4 + 24;
    pbuf_len_alloc += 4 + sizeof(dhcp6_stateful_requested_options);
    p_out = dhcp6_create_msg(netif, dhcp6, DHCP6_REBIND, pbuf_len_alloc, &options_out_len);
    if (p_out != NULL) {
        struct dhcp6_msg *msg_out = (struct dhcp6_msg *)p_out->payload;
        u8_t *options = (u8_t *)(msg_out + 1);

        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_ELAPSED_TIME, 2); //ELAPSED_TIME
        options_out_len = dhcp6_option_short(options_out_len, options, dhcp6->tries * DHCP6_TIMER_MSECS); //Estimated time

#if LWIP_NETIF_HOSTNAME
        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_CLIENT_FQDN, 2 + strlen(netif->hostname)); //DOMAIN_NAME
        options_out_len = dhcp6_option_byte(options_out_len, options, 0x00); // FLAG
        options_out_len = dhcp6_option_byte(options_out_len, options, strlen(netif->hostname)); //DOMAIN_NAME_LEN
        for (i = 0; i < strlen(netif->hostname); i++) {
            options_out_len = dhcp6_option_byte(options_out_len, options, netif->hostname[i]); //DOMAIN_NAME
        }
#endif /* LWIP_NETIF_HOSTNAME */

        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_CLIENTID, dhcp6_get_duid_len(dhcp6->client_duid));
        options_out_len = dhcp6_option_duid(options_out_len, options, dhcp6->client_duid);

        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_IA_NA, 40);
        options_out_len = dhcp6_option_long(options_out_len, options, dhcp6->ia_id); // IA_ID
        options_out_len = dhcp6_option_long(options_out_len, options, 0); // T1
        options_out_len = dhcp6_option_long(options_out_len, options, 0); // T2
        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_IAADDR, 24);
        for (i = 0; i < sizeof(dhcp6->offered_ip_addr.addr); i++) {
            options_out_len = dhcp6_option_byte(options_out_len, options, ((u8_t *)dhcp6->offered_ip_addr.addr)[i]); //OFFERED_IP
        }
        options_out_len = dhcp6_option_long(options_out_len, options, dhcp6->offered_preferred_lifetime);
        options_out_len = dhcp6_option_long(options_out_len, options, dhcp6->offered_valid_lifetime);

        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_rebind: making request\n"));
        options_out_len = dhcp6_option_optionrequest(options_out_len, options, dhcp6_stateful_requested_options,
                          LWIP_ARRAYSIZE(dhcp6_stateful_requested_options), p_out->len);
        LWIP_HOOK_DHCP6_APPEND_OPTIONS(netif, dhcp6, DHCP6_STATE_REQUESTING_CONFIG, msg_out,
                                       DHCP6_RENEW, options_out_len, p_out->len);
        dhcp6_msg_finalize(options_out_len, p_out);

        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_rebind: sendto(rebind, dhcp6_All_DHCP6_Relay_Agents_and_Servers, DHCP6_SERVER_PORT)\n"));
        udp_sendto_if(dhcp6_pcb, p_out, &dhcp6_All_DHCP6_Relay_Agents_and_Servers, DHCP6_SERVER_PORT, netif);
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_rebind: deleting()ing\n"));
        pbuf_free(p_out);
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_rebind: REBINDING\n"));
    } else {
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_SERIOUS, ("dhcp6_rebind: could not allocate DHCPv6 request\n"));
    }

    msecs = (u16_t)((((1 << (dhcp6->tries)) * REB_TIMEOUT) < REB_MAX_RT ? ((1 << dhcp6->tries) * REN_TIMEOUT) : REB_MAX_RT) * 1000);
    if (dhcp6->tries < 255) {
        dhcp6->tries++;
    }
    dhcp6->request_timeout = (u16_t)((msecs + DHCP6_TIMER_MSECS - 1) / DHCP6_TIMER_MSECS);
    LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_rebind(): set request timeout %"U16_F" msecs\n", msecs));
    return result;
}

static err_t
dhcp6_delicine(struct netif *netif)
{
    struct dhcp6 *dhcp6 = netif_dhcp6_data(netif);
    err_t result = ERR_OK;
    u16_t msecs;
    u8_t i;
    struct pbuf *p_out;
    u16_t options_out_len;
    u16_t pbuf_len_alloc;

    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp6_delicine()\n"));

//  ip6_addr_set_any(&dhcp6->offered_ip_addr);
    dhcp6_set_state(dhcp6, DHCP6_STATE_STATEFUL_DELICINING, "dhcp6_delicine");
    /* create and initialize the DHCPv6 message header */
    pbuf_len_alloc = 4 + 2;
#if LWIP_NETIF_HOSTNAME
    pbuf_len_alloc += 4 + 2 + strlen(netif->hostname);
#endif /* LWIP_NETIF_HOSTNAME */
    pbuf_len_alloc +=	4 + dhcp6_get_duid_len(dhcp6->client_duid);
    pbuf_len_alloc += 4 + dhcp6_get_duid_len(dhcp6->server_duid);
    pbuf_len_alloc += 4 + 12 + 4 + 24;
    p_out = dhcp6_create_msg(netif, dhcp6, DHCP6_DECLINE, pbuf_len_alloc, &options_out_len);
    if (p_out != NULL) {
        struct dhcp6_msg *msg_out = (struct dhcp6_msg *)p_out->payload;
        u8_t *options = (u8_t *)(msg_out + 1);

        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_ELAPSED_TIME, 2); //ELAPSED_TIME
        options_out_len = dhcp6_option_short(options_out_len, options, dhcp6->tries * DHCP6_TIMER_MSECS); //Estimated time

#if LWIP_NETIF_HOSTNAME
        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_CLIENT_FQDN, 2 + strlen(netif->hostname)); //DOMAIN_NAME
        options_out_len = dhcp6_option_byte(options_out_len, options, 0x00); // FLAG
        options_out_len = dhcp6_option_byte(options_out_len, options, strlen(netif->hostname)); //DOMAIN_NAME_LEN
        for (i = 0; i < strlen(netif->hostname); i++) {
            options_out_len = dhcp6_option_byte(options_out_len, options, netif->hostname[i]); //DOMAIN_NAME
        }
#endif /* LWIP_NETIF_HOSTNAME */

        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_CLIENTID, dhcp6_get_duid_len(dhcp6->client_duid));
        options_out_len = dhcp6_option_duid(options_out_len, options, dhcp6->client_duid);

        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_SERVERID, dhcp6_get_duid_len(dhcp6->server_duid));
        options_out_len = dhcp6_option_duid(options_out_len, options, dhcp6->server_duid);

        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_IA_NA, 40);
        options_out_len = dhcp6_option_long(options_out_len, options, dhcp6->ia_id); // IA_ID
        options_out_len = dhcp6_option_long(options_out_len, options, dhcp6->offered_t1_renew); // T1
        options_out_len = dhcp6_option_long(options_out_len, options, dhcp6->offered_t2_rebind); // T2
        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_IAADDR, 24);
        for (i = 0; i < sizeof(dhcp6->offered_ip_addr.addr); i++) {
            options_out_len = dhcp6_option_byte(options_out_len, options, ((u8_t *)dhcp6->offered_ip_addr.addr)[i]); //OFFERED_IP
        }
        options_out_len = dhcp6_option_long(options_out_len, options, dhcp6->offered_preferred_lifetime);
        options_out_len = dhcp6_option_long(options_out_len, options, dhcp6->offered_valid_lifetime);

        dhcp6_msg_finalize(options_out_len, p_out);

        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_delicine: sendto(delicine, dhcp6_All_DHCP6_Relay_Agents_and_Servers, DHCP6_SERVER_PORT)\n"));
        udp_sendto_if(dhcp6_pcb, p_out, &dhcp6_All_DHCP6_Relay_Agents_and_Servers, DHCP6_SERVER_PORT, netif);
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_delicine: deleting()ing\n"));
        pbuf_free(p_out);
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_delicine: DELICINEING\n"));
    } else {
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_SERIOUS, ("dhcp6_delicine: could not allocate DHCPv6 request\n"));
    }
    if (dhcp6->tries < 255) {
        dhcp6->tries++;
    }

    msecs = DEC_TIMEOUT * 1000;
    dhcp6->request_timeout = (u16_t)((msecs + DHCP6_TIMER_MSECS - 1) / DHCP6_TIMER_MSECS);
    LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_delicine(): set request timeout %"U16_F" msecs\n", msecs));
    return result;
}

static err_t
dhcp6_release(struct netif *netif)
{
    struct dhcp6 *dhcp6 = netif_dhcp6_data(netif);
    err_t result = ERR_OK;
    u16_t msecs;
    u8_t i;
    struct pbuf *p_out;
    u16_t options_out_len;
    u16_t pbuf_len_alloc;

    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp6_release()\n"));

//  ip6_addr_set_any(&dhcp6->offered_ip_addr);
    dhcp6_set_state(dhcp6, DHCP6_STATE_STATEFUL_RELEASEING, "dhcp6_release");
    /* create and initialize the DHCPv6 message header */
    pbuf_len_alloc = 4 + 2;
#if LWIP_NETIF_HOSTNAME
    pbuf_len_alloc += 4 + 2 + strlen(netif->hostname);
#endif /* LWIP_NETIF_HOSTNAME */
    pbuf_len_alloc +=	4 + dhcp6_get_duid_len(dhcp6->client_duid);
    pbuf_len_alloc += 4 + dhcp6_get_duid_len(dhcp6->server_duid);
    pbuf_len_alloc += 4 + 12 + 4 + 24;
    p_out = dhcp6_create_msg(netif, dhcp6, DHCP6_RELEASE, pbuf_len_alloc, &options_out_len);
    if (p_out != NULL) {
        struct dhcp6_msg *msg_out = (struct dhcp6_msg *)p_out->payload;
        u8_t *options = (u8_t *)(msg_out + 1);

        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_ELAPSED_TIME, 2); //ELAPSED_TIME
        options_out_len = dhcp6_option_short(options_out_len, options, dhcp6->tries * DHCP6_TIMER_MSECS); //Estimated time

#if LWIP_NETIF_HOSTNAME
        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_CLIENT_FQDN, 2 + strlen(netif->hostname)); //DOMAIN_NAME
        options_out_len = dhcp6_option_byte(options_out_len, options, 0x00); // FLAG
        options_out_len = dhcp6_option_byte(options_out_len, options, strlen(netif->hostname)); //DOMAIN_NAME_LEN
        for (i = 0; i < strlen(netif->hostname); i++) {
            options_out_len = dhcp6_option_byte(options_out_len, options, netif->hostname[i]); //DOMAIN_NAME
        }
#endif /* LWIP_NETIF_HOSTNAME */

        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_CLIENTID, dhcp6_get_duid_len(dhcp6->client_duid));
        options_out_len = dhcp6_option_duid(options_out_len, options, dhcp6->client_duid);

        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_SERVERID, dhcp6_get_duid_len(dhcp6->server_duid));
        options_out_len = dhcp6_option_duid(options_out_len, options, dhcp6->server_duid);

        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_IA_NA, 40);
        options_out_len = dhcp6_option_long(options_out_len, options, dhcp6->ia_id); // IA_ID
        options_out_len = dhcp6_option_long(options_out_len, options, dhcp6->offered_t1_renew); // T1
        options_out_len = dhcp6_option_long(options_out_len, options, dhcp6->offered_t2_rebind); // T2
        options_out_len = dhcp6_option(options_out_len, options, DHCP6_OPTION_IAADDR, 24);
        for (i = 0; i < sizeof(dhcp6->offered_ip_addr.addr); i++) {
            options_out_len = dhcp6_option_byte(options_out_len, options, ((u8_t *)dhcp6->offered_ip_addr.addr)[i]); //OFFERED_IP
        }
        options_out_len = dhcp6_option_long(options_out_len, options, dhcp6->offered_preferred_lifetime);
        options_out_len = dhcp6_option_long(options_out_len, options, dhcp6->offered_valid_lifetime);

        dhcp6_msg_finalize(options_out_len, p_out);

        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_release: sendto(release, dhcp6_All_DHCP6_Relay_Agents_and_Servers, DHCP6_SERVER_PORT)\n"));
        udp_sendto_if(dhcp6_pcb, p_out, &dhcp6_All_DHCP6_Relay_Agents_and_Servers, DHCP6_SERVER_PORT, netif);
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_release: deleting()ing\n"));
        pbuf_free(p_out);
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_release: RELEASEING\n"));
    } else {
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_SERIOUS, ("dhcp6_release: could not allocate DHCPv6 request\n"));
    }
    if (dhcp6->tries < 255) {
        dhcp6->tries++;
    }

    msecs = REL_TIMEOUT * 1000;
    dhcp6->request_timeout = (u16_t)((msecs + DHCP6_TIMER_MSECS - 1) / DHCP6_TIMER_MSECS);
    LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_release(): set request timeout %"U16_F" msecs\n", msecs));
    return result;
}

/**
 *
 * @param
 * @return lwIP specific error (see error.h)
 */
static err_t
dhcp6_parse_ia_na(struct pbuf *p, u16_t op_start, u16_t op_len, u32_t *ia_id, ip6_addr_t *addr, u32_t *t1, u32_t *t2, u32_t *preferred_lifetime, u32_t *valid_lifetime)
{
    err_t ret = ERR_VAL;
    if (op_start > 0 && op_len >= 12) {
        u8_t offset = 0;
        u8_t *buffer = (u8_t *)malloc(op_len);
        u16_t copied = pbuf_copy_partial(p, buffer, op_len, op_start);
        if (copied != op_len) {
            /* pbuf length mismatch */
            free(buffer);
            return ERR_MEM;
        }
//		*ia_id = (buffer[offset] << 24)|(buffer[offset+1] << 16) |(buffer[offset+2] << 8)| buffer[offset+3];
        memcpy(ia_id, buffer + offset, 4);
        offset += 4;
        *t1 = (buffer[offset] << 24) | (buffer[offset + 1] << 16) | (buffer[offset + 2] << 8) | buffer[offset + 3];
        offset += 4;
        *t2 = (buffer[offset] << 24) | (buffer[offset + 1] << 16) | (buffer[offset + 2] << 8) | buffer[offset + 3];
        offset += 4;
        if (op_len >= 40) {
            while ((offset + 4 <= op_len)) {
                u16_t op = (buffer[offset] << 8) | buffer[offset + 1];
                u16_t len = (buffer[offset + 2] << 8) | buffer[offset + 3];
                offset += 4;
                if (op == DHCP6_OPTION_IAADDR && len == 24) {
                    ip6_addr_t *ip6_addr = (ip6_addr_t *)(buffer + offset);
                    ip6_addr_set(addr, ip6_addr);
                    offset += 16;
                    *preferred_lifetime = (buffer[offset] << 24) | (buffer[offset + 1] << 16) | (buffer[offset + 2] << 8) | buffer[offset + 3];
                    offset += 4;
                    *valid_lifetime = (buffer[offset] << 24) | (buffer[offset + 1] << 16) | (buffer[offset + 2] << 8) | buffer[offset + 3];
                    ret = ERR_OK;
                } else {
                    offset += len;
                }
            }
        }
        free(buffer);
    }

    return ret;
}

/**
 * Handle a DHCPv6 ADVERTISE packet
 *
 * @param netif the netif under DHCPv6 control
 */
static void
dhcp6_handle_advertise(struct netif *netif, struct pbuf *p_msg_in)
{
    struct dhcp6 *dhcp6 = netif_dhcp6_data(netif);

    LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_handle_advertise(netif=%p) %c%c%"U16_F"\n",
                (void *)netif, netif->name[0], netif->name[1], (u16_t)netif->num));
    /* obtain the server duid */
    if (dhcp6_option_given(dhcp6, DHCP6_OPTION_IDX_SERVER_ID)) {
        dhcp6->request_timeout = 0; /* stop timer */

        u16_t op_start = dhcp6_get_option_start(dhcp6, DHCP6_OPTION_IDX_SERVER_ID);
        u16_t op_len = dhcp6_get_option_length(dhcp6, DHCP6_OPTION_IDX_SERVER_ID);
        DUID server_duid;
        u8_t *duid_buf = (u8_t *)malloc(op_len);
        u16_t copied = pbuf_copy_partial(p_msg_in, duid_buf, op_len, op_start);
        if (copied != op_len) {
            /* pbuf length mismatch */
            free(duid_buf);
            return;
        }
        dhcp6_copy_duid_buf(&server_duid, duid_buf, op_len);
        dhcp6->server_duid = server_duid;
        free(duid_buf);
    } else {
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_SERIOUS,
                    ("dhcp6_handle_advertise(netif=%p) did not get server ID!\n", (void *)netif));
        return;
    }
    if (dhcp6_option_given(dhcp6, DHCP6_OPTION_IDX_IA_NA)) {
        dhcp6->request_timeout = 0; /* stop timer */
        u16_t op_start = dhcp6_get_option_start(dhcp6, DHCP6_OPTION_IDX_IA_NA);
        u16_t op_len = dhcp6_get_option_length(dhcp6, DHCP6_OPTION_IDX_IA_NA);
        u32_t ia_id;
        u32_t t1;
        u32_t t2;
        u32_t pref_lifetime;
        u32_t valid_lifetime;
        ip6_addr_t addr;

        if (dhcp6_parse_ia_na(p_msg_in, op_start, op_len, &ia_id, &addr, &t1, &t2, &pref_lifetime, &valid_lifetime)) {
            return;
        }

        if (ntohl(ia_id) == dhcp6->ia_id) {
            dhcp6->offered_t1_renew = t1;
            dhcp6->offered_t2_rebind = t2;
            dhcp6->offered_preferred_lifetime = pref_lifetime;
            dhcp6->offered_valid_lifetime = valid_lifetime;
            ip6_addr_copy(dhcp6->offered_ip_addr, addr);
            LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_SERIOUS,
                        ("offered_ip_addr(%s), mode(STATEFUL)\n", ip6addr_ntoa(&dhcp6->offered_ip_addr)));
            ip6_addr_assign_zone(&dhcp6->offered_ip_addr, IP6_UNICAST, netif);
        } else {

            LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_SERIOUS,
                        ("dhcp6_handle_advertise(netif=%p) mismatch IA_NA  !\n", (void *)netif));
            return;
        }
    } else {
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_SERIOUS,
                    ("dhcp6_handle_advertise(netif=%p) did not get IA_NA !\n", (void *)netif));
        return;
    }

    dhcp6_request(netif);

}
/**
 * Handle a DHCPv6 REPLY packet
 *
 * @param netif the netif under DHCPv6 control
 */
static void
dhcp6_handle_reply(struct netif *netif, struct pbuf *p_msg_in)
{
    struct dhcp6 *dhcp6 = netif_dhcp6_data(netif);

    /* obtain the server duid */
    if (dhcp6_option_given(dhcp6, DHCP6_OPTION_IDX_SERVER_ID)) {
        dhcp6->request_timeout = 0; /* stop timer */

        u16_t op_start = dhcp6_get_option_start(dhcp6, DHCP6_OPTION_IDX_SERVER_ID);
        u16_t op_len = dhcp6_get_option_length(dhcp6, DHCP6_OPTION_IDX_SERVER_ID);
        DUID server_duid;
        u8_t *duid_buf = (u8_t *)malloc(op_len);
        u16_t copied = pbuf_copy_partial(p_msg_in, duid_buf, op_len, op_start);
        if (copied != op_len) {
            /* pbuf length mismatch */
            free(duid_buf);
            return;
        }
        dhcp6_copy_duid_buf(&server_duid, duid_buf, op_len);
        dhcp6->server_duid = server_duid;
        free(duid_buf);
    } else {
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_SERIOUS,
                    ("dhcp6_handle_advertise(netif=%p) did not get server ID!\n", (void *)netif));
        return;
    }
    if (dhcp6_option_given(dhcp6, DHCP6_OPTION_IDX_IA_NA)) {
        dhcp6->request_timeout = 0; /* stop timer */
        u16_t op_start = dhcp6_get_option_start(dhcp6, DHCP6_OPTION_IDX_IA_NA);
        u16_t op_len = dhcp6_get_option_length(dhcp6, DHCP6_OPTION_IDX_IA_NA);
        u32_t ia_id;
        u32_t t1;
        u32_t t2;
        u32_t pref_lifetime;
        u32_t valid_lifetime;
        ip6_addr_t addr;

        if (dhcp6_parse_ia_na(p_msg_in, op_start, op_len, &ia_id, &addr, &t1, &t2, &pref_lifetime, &valid_lifetime)) {
            return;
        }

        if (ntohl(ia_id) == dhcp6->ia_id) {
            dhcp6->offered_t1_renew = t1;
            dhcp6->offered_t2_rebind = t2;
            dhcp6->offered_preferred_lifetime = pref_lifetime;
            dhcp6->offered_valid_lifetime = valid_lifetime;
            ip6_addr_copy(dhcp6->offered_ip_addr, addr);
            ip6_addr_assign_zone(&dhcp6->offered_ip_addr, IP6_UNICAST, netif);
        } else {
            LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_SERIOUS,
                        ("dhcp6_handle_advertise(netif=%p) mismatch IA_NA  !\n", (void *)netif));
            return;
        }
    } else {
        LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_SERIOUS,
                    ("dhcp6_handle_advertise(netif=%p) did not get IA_NA !\n", (void *)netif));
        return;
    }
#if LWIP_DHCP6_PROVIDE_DNS_SERVERS
    if (dhcp6_option_given(dhcp6, DHCP6_OPTION_IDX_DNS_SERVER)) {
        ip_addr_t dns_addr;
        ip6_addr_t *dns_addr6;
        u16_t op_start = dhcp6_get_option_start(dhcp6, DHCP6_OPTION_IDX_DNS_SERVER);
        u16_t op_len = dhcp6_get_option_length(dhcp6, DHCP6_OPTION_IDX_DNS_SERVER);
        u16_t idx;
        u8_t n;

        memset(&dns_addr, 0, sizeof(dns_addr));
        dns_addr6 = ip_2_ip6(&dns_addr);
        for (n = 0, idx = op_start; (idx < op_start + op_len) && (n < LWIP_DHCP6_PROVIDE_DNS_SERVERS);
             n++, idx += sizeof(struct ip6_addr_packed)) {
            u16_t copied = pbuf_copy_partial(p_msg_in, dns_addr6, sizeof(struct ip6_addr_packed), idx);
            if (copied != sizeof(struct ip6_addr_packed)) {
                /* pbuf length mismatch */
                return;
            }
            ip6_addr_assign_zone(dns_addr6, IP6_UNKNOWN, netif);
            /* @todo: do we need a different offset than DHCP(v4)? */
#if MULTI_NETIF_SUPPORT
            dns_setserver_with_netif(netif, n, &dns_addr);
#else
            dns_setserver(n, &dns_addr);
#endif
        }
    }
    /* @ todo: parse and set Domain Search List */
#endif /* LWIP_DHCP6_PROVIDE_DNS_SERVERS */
#if LWIP_DHCP6_GET_NTP_SRV
    if (dhcp6_option_given(dhcp6, DHCP6_OPTION_IDX_NTP_SERVER)) {
        ip_addr_t ntp_server_addrs[LWIP_DHCP6_MAX_NTP_SERVERS];
        u16_t op_start = dhcp6_get_option_start(dhcp6, DHCP6_OPTION_IDX_NTP_SERVER);
        u16_t op_len = dhcp6_get_option_length(dhcp6, DHCP6_OPTION_IDX_NTP_SERVER);
        u16_t idx;
        u8_t n;

        for (n = 0, idx = op_start; (idx < op_start + op_len) && (n < LWIP_DHCP6_MAX_NTP_SERVERS);
             n++, idx += sizeof(struct ip6_addr_packed)) {
            u16_t copied;
            ip6_addr_t *ntp_addr6 = ip_2_ip6(&ntp_server_addrs[n]);
            ip_addr_set_zero_ip6(&ntp_server_addrs[n]);
            copied = pbuf_copy_partial(p_msg_in, ntp_addr6, sizeof(struct ip6_addr_packed), idx);
            if (copied != sizeof(struct ip6_addr_packed)) {
                /* pbuf length mismatch */
                return;
            }
            ip6_addr_assign_zone(ntp_addr6, IP6_UNKNOWN, netif);
        }
        dhcp6_set_ntp_servers(n, ntp_server_addrs);
    }
#endif /* LWIP_DHCP6_GET_NTP_SRV */
}

void dhcp6_coarse_tmr(void)
{
    struct netif *netif;
    LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE, ("dhcp6_coarse_tmr()\n"));
    /* iterate through all network interfaces */
    NETIF_FOREACH(netif) {
        /* only act on DHCP configured interfaces */
        struct dhcp6 *dhcp6 = netif_dhcp6_data(netif);
        if ((dhcp6 != NULL) && (dhcp6->state >= DHCP6_STATE_STATEFUL_IDLE) &&
            (dhcp6->state <= DHCP6_STATE_STATEFUL_DELICINING)) {
            /* compare lease time to expire timeout */
            if (dhcp6->t0_timeout && (++dhcp6->lease_used == dhcp6->t0_timeout)) {
                LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_coarse_tmr(): t0 timeout\n"));
                /* this clients' lease time has expired */
                dhcp6_release(netif);
                dhcp6_enable_stateful(netif);
                /* timer is active (non zero), and triggers (zeroes) now? */
            } else if (dhcp6->t2_rebind_time && (dhcp6->t2_rebind_time-- == 1)) {
                LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_coarse_tmr(): t2 timeout\n"));
                /* this clients' rebind timeout triggered */
                dhcp6_t2_timeout(netif);
                /* timer is active (non zero), and triggers (zeroes) now */
            } else if (dhcp6->t1_renew_time && (dhcp6->t1_renew_time-- == 1)) {
                LWIP_DEBUGF(DHCP6_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp6_coarse_tmr(): t1 timeout\n"));
                /* this clients' renewal timeout triggered */
                dhcp6_t1_timeout(netif);
            }
        }
    }
}

void
dhcp6_network_changed(struct netif *netif)
{
    struct dhcp6 *dhcp6 = netif_dhcp6_data(netif);

    if (!dhcp6) {
        return;
    }
    switch (dhcp6->state) {
    case DHCP6_STATE_STATEFUL_IDLE:
    case DHCP6_STATE_STATEFUL_RENEWING:
    case DHCP6_STATE_STATEFUL_CONFIRMING:
    case DHCP6_STATE_STATEFUL_REBINDING:
    case DHCP6_STATE_STATEFUL_RELEASEING:
    case DHCP6_STATE_STATEFUL_DELICINING:
        dhcp6->tries = 0; // if DHCP6_STATE_STATEFUL_CONFIRMING
        dhcp6_confirm(netif);
        break;
    case DHCP6_STATE_OFF:
        /* stay off */
        break;
    default:
        LWIP_ASSERT("invalid dhcp6->state", dhcp6->state <= DHCP6_STATE_STATEFUL_DELICINING);
        /* INIT/REQUESTING/CHECKING/BACKING_OFF restart with new 'rid' because the
           state changes, SELECTING: continue with current 'rid' as we stay in the
           same state */
        /* ensure we start with short timeouts, even if already discovering */
        dhcp6->tries = 0;
        dhcp6_solicit(netif);
        break;
    }
}
#endif /* LWIP_IPV6_DHCP6_STATEFUL */
#endif /* LWIP_IPV6 && LWIP_IPV6_DHCP6 */

