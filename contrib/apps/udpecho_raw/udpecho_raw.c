/*
 * Copyright (c) 2016 Stephan Linz <linz@li-pro.net>, Li-Pro.Net
 * All rights reserved.
 *
 * Based on examples provided by
 * Iwan Budi Kusnanto <ibk@labhijau.net> (https://gist.github.com/iwanbk/1399729)
 * Juri Haberland <juri@sapienti-sat.org> (https://lists.gnu.org/archive/html/lwip-users/2007-06/msg00078.html)
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
 * This file is part of and a contribution to the lwIP TCP/IP stack.
 *
 * Credits go to Adam Dunkels (and the current maintainers) of this software.
 *
 * Stephan Linz rewrote this file to get a basic echo example.
 */

/**
 * @file
 * UDP echo server example using raw API.
 *
 * Echos all bytes sent by connecting client,
 * and passively closes when client is done.
 *
 */

#include "lwip/opt.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/udp.h"
#include "udpecho_raw.h"

#if LWIP_UDP

static struct udp_pcb *udpecho_raw_pcb;

struct netif *udpecho_get_current_netif(struct udp_pcb *pcb, const ip_addr_t *dst_ip, u16_t dst_port);
err_t udpecho_sendto(struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);

struct netif *udpecho_get_current_netif(struct udp_pcb *pcb, const ip_addr_t *dst_ip, u16_t dst_port)
{
  struct netif *netif;
  LWIP_UNUSED_ARG(dst_port);

  LWIP_DEBUGF(UDP_DEBUG | LWIP_DBG_TRACE, ("udp_send\n"));
  if (pcb->netif_idx != NETIF_NO_INDEX)
  {
    netif = netif_get_by_index(pcb->netif_idx);
  }
  else
  {
#if LWIP_MULTICAST_TX_OPTIONS
    netif = NULL;
    if (ip_addr_ismulticast(dst_ip))
    {
      /* For IPv6, the interface to use for packets with a multicast destination
       * is specified using an interface index. The same approach may be used for
       * IPv4 as well, in which case it overrides the IPv4 multicast override
       * address below. Here we have to look up the netif by going through the
       * list, but by doing so we skip a route lookup. If the interface index has
       * gone stale, we fall through and do the regular route lookup after all. */
      if (pcb->mcast_ifindex != NETIF_NO_INDEX)
      {
        netif = netif_get_by_index(pcb->mcast_ifindex);
      }
#if LWIP_IPV4
      else
#if LWIP_IPV6
          if (IP_IS_V4(dst_ip))
#endif /* LWIP_IPV6 */
      {
        /* IPv4 does not use source-based routing by default, so we use an
             administratively selected interface for multicast by default.
             However, this can be overridden by setting an interface address
             in pcb->mcast_ip4 that is used for routing. If this routing lookup
             fails, we try regular routing as though no override was set. */
        if (!ip4_addr_isany_val(pcb->mcast_ip4) &&
            !ip4_addr_cmp(&pcb->mcast_ip4, IP4_ADDR_BROADCAST))
        {
          netif = ip4_route_src(ip_2_ip4(&pcb->local_ip), &pcb->mcast_ip4);
        }
      }
#endif /* LWIP_IPV4 */
    }

    if (netif == NULL)
#endif /* LWIP_MULTICAST_TX_OPTIONS */
    {
      /* find the outgoing network interface for this packet */
      netif = ip_route(&pcb->local_ip, dst_ip);
    }
  }
  return netif;
}

err_t udpecho_sendto(struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
  return udp_sendto_if_src(upcb, p, addr, port, udpecho_get_current_netif(upcb, addr, port), &upcb->local_ip);
}

static void
udpecho_raw_recv(void *arg, struct udp_pcb *upcb, struct pbuf *p,
                 const ip_addr_t *addr, u16_t port)
{
  LWIP_UNUSED_ARG(arg);
  if (p != NULL) {
    /* send received packet back to sender */
    udpecho_sendto(upcb, p, addr, port);
    /* free the pbuf */
    pbuf_free(p);
  }
}

void
udpecho_raw_init(void)
{
  udpecho_raw_pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
  if (udpecho_raw_pcb != NULL) {
    err_t err;

    err = udp_bind(udpecho_raw_pcb, IP_ANY_TYPE, 7);
    if (err == ERR_OK) {
      udp_recv(udpecho_raw_pcb, udpecho_raw_recv, NULL);
    } else {
      /* abort? output diagnostic? */
    }
  } else {
    /* abort? output diagnostic? */
  }
}

#endif /* LWIP_UDP */
