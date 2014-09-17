/**
 * @file
 * Ethernet Interface Skeleton
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include "string.h"
#include "stdio.h"

#include "netif/etharp.h"
//#include "netif/ppp_oe.h"
//#include "netif/ethernetif.h"
//#include "marvell_ops.h"

/* Define those to better describe your network interface. */
#define IFNAME0 's'
#define IFNAME1 't'
/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */
struct ethernetif
{
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
  int unused;
};

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void low_level_init(struct netif *netif)
{
  netif->hwaddr_len = 6;
  /* maximum transfer unit */
  netif->mtu = 1500;
  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
}

//A local helper routine to dump out an ethernet frame,include the IP packet
//content if the frame contains one.
static void dump_eth_frame(struct pbuf* p)
{
	unsigned char* pEthernetHdr    = (unsigned char*)p->payload;
	struct ip_hdr* pIpHdr = (struct ip_hdr*)(pEthernetHdr + 14);  //skip the Ethernet_II header.
	unsigned int mac[6],*pmac;
	
	pmac = mac;
	_hx_printf("  ----------------  Ethernet Frame ----------------  \r\n");
	*pmac ++ = *pEthernetHdr ++;
	*pmac ++ = *pEthernetHdr ++;
	*pmac ++ = *pEthernetHdr ++;
	*pmac ++ = *pEthernetHdr ++;
	*pmac ++ = *pEthernetHdr ++;
	*pmac ++ = *pEthernetHdr ++;
	pmac = mac;
	_hx_printf("  DMAC addr       : 0x%02X%02X%02X%02X%02X%02X \r\n",
		*pmac ++,
		*pmac ++,
		*pmac ++,
		*pmac ++,
		*pmac ++,
		*pmac ++);

	pmac = mac;
	*pmac ++ = *pEthernetHdr ++;
	*pmac ++ = *pEthernetHdr ++;
	*pmac ++ = *pEthernetHdr ++;
	*pmac ++ = *pEthernetHdr ++;
	*pmac ++ = *pEthernetHdr ++;
	*pmac ++ = *pEthernetHdr ++;
	pmac = mac;
	_hx_printf("  SMAC addr       : 0x%02X%02X%02X%02X%02X%02X \r\n",
		*pmac ++,
		*pmac ++,
		*pmac ++,
		*pmac ++,
		*pmac ++,
		*pmac ++);

	_hx_printf("  Frame type      : 0x%04X                     \r\n",
		*(short*)(pEthernetHdr + 2));
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
  /* Interrupts are disabled through this whole thing to support multi-threading
	   transmit calls. Also this function might be called from an ISR. */
  // pbuf_free(p);
	_hx_printf("  TCP/IP: low_level_output routine is called.\r\n");
	dump_eth_frame(p);
	return ERR_OK;
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
struct pbuf *low_level_input(struct netif *netif)
{
  return NULL;
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
static err_t ethernetif_input(struct netif *netif)
{
  err_t err;
  //INT8U ret;
  struct pbuf *p=NULL;
  /* move received packet into a new pbuf */
  p = low_level_input(netif);
  /* no packet could be read, silently ignore this */
  if (p == NULL){
  	 return ERR_OK;
  }
  err = netif->input(p, netif); //Delivery packet to up layer.
  if (err != ERR_OK){
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
    pbuf_free(p);
    p = NULL;
	return err;
  }
  return err;
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
static err_t ethernetif_init(struct netif *netif)
{
  struct ethernetif *ethernetif;
  LWIP_ASSERT("netif != NULL", (netif != NULL));
  ethernetif = mem_malloc(sizeof(struct ethernetif));
  if (ethernetif == NULL)
  {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
    return ERR_MEM;
  }
  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, 10000000);
  netif->state = ethernetif;
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  netif->linkoutput = low_level_output;
  ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);
  low_level_init(netif);
  return ERR_OK;
}

//Dedicated Ethernet receiving thread,repeatly call ethernetif_input to
//try to find a valid ethernet frame.
//It's a temporary solution for Ethernet driver,interrupt driving mechanism
//will be introduced in the future.
static DWORD EthernetRecv(LPVOID pData)
{
	struct netif* pif = (struct netif*)pData;
	while(TRUE)
	{
		ethernetif_input(pif);
		KernelThreadManager.Sleep((__COMMON_OBJECT*)&KernelThreadManager,100);
	}
	return 0;
}

//Initialization code of Ethernet Interface.It adds a net interface object,
//install interrupt handler of NIC,or start a dedicated kernel thread to poll
//Ethernet interface.
BOOL InitializeEthernetIf()
{
	static struct    netif netif_eth;     //Global Ethernet Interface object.
	struct ip_addr   ipaddr;              //IP address of Ethernet interface.
	struct ip_addr   netmask;             //Subnet mask of Ethernet interface.
	struct ip_addr   gw;                  //Default gateway.
	__KERNEL_THREAD_OBJECT * pRecvThread = NULL;

	//Initialize IP parameters.
	IP4_ADDR(&ipaddr,192,168,0,10);
	IP4_ADDR(&netmask,255,255,255,0);
	IP4_ADDR(&gw,192,168,0,1);

	netif_add(&netif_eth, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);
	netif_set_default(&netif_eth);
	netif_set_up(&netif_eth);

	//Create and start ethernet receiving thread.
	pRecvThread = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		EthernetRecv,
		(LPVOID)&netif_eth,
		NULL,
		"eth_recv");
	if(NULL == pRecvThread)  //Can not create receiving thread.
	{
		netif_remove(&netif_eth);
		return FALSE;
	}

	return TRUE;
}
