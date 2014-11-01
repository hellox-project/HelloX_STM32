//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 26,2014
//    Module Name               : marvel.c
//    Module Funciton           : 
//                                This module countains Marvell WiFi ethernet
//                                driver's implementation code.
//                                The hardware driver only need implement several
//                                low level routines and link them into HelloX's
//                                ethernet skeleton.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

 #ifndef __STDAFX_H__
 #include "StdAfx.h"
 #endif
 
 #ifndef __KAPI_H__
 #include "kapi.h"
 #endif

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "string.h"
#include "stdio.h"

#include "ethif.h"
#include "netif/etharp.h"
#include "marvell_ops.h"

#ifndef __MARVELIF_H__
#include "marvelif.h"
#endif

//A helper local variable to record the lbs_private.
static struct lbs_private* priv = NULL;

//Do some preparations before entry low_level_init,for example initializes the
//hardware,set up operation environment,etc.
BOOL PrepareInit()
{
	//Initialize Ethernet Interface Driver.
#ifdef __WIFI_DEBUG
	_hx_printf("  WiFi debugging: Begin to initialize SDIO and WiFi devices...\r\n");
#endif
	priv = init_marvell_driver();
	lbs_scan_worker(priv);
#ifdef __WIFI_DEBUG
	_hx_printf("  WiFi debugging: End of SDIO and WiFi initialization.\r\n");
#endif
	//Try to associate to the default SSID,use INFRASTRUCTURE mode.
	marvel_assoc_network(priv,WIFI_DEFAULT_SSID,WIFI_DEFAULT_KEY,WIFI_MODE_INFRA);
	return TRUE;
}

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
void low_level_init(struct netif *netif)
{
  netif->hwaddr_len = 6;
  /* maximum transfer unit */
  netif->mtu = 1500;
  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
	//Set the MAC address of this interface.
	netif->hwaddr_len = ETHARP_HWADDR_LEN;
	memcpy(netif->hwaddr,pgmarvel_priv->current_addr,ETHARP_HWADDR_LEN);
}

//Do the actual sending of a frame,by calling hardware level interface to copy
//the frame into WiFi chips buffer.
int DoPhysicalSend(struct netif* outif,struct pbuf* p)
{
	int                    ret      = 0;
	__ETH_INTERFACE_STATE* pIfState = (__ETH_INTERFACE_STATE*)outif->state;
	
	if (priv->resp_len[0] > 0) {
		ret = if_sdio_send_data(priv,priv->resp_buf[0],priv->resp_len[0]);
		if (ret){
#ifdef __WIFI_DEBUG
			_hx_printf("  WiFi debugging: host_to_card failed %d\n", ret);
#endif
			priv->dnld_sent = DNLD_RES_RECEIVED;
	  }
		priv->resp_len[0] = 0;
	}
	else
	{
		//Update interface statistics info.
		pIfState->dwFrameSendSuccess += 1;
#ifdef __WIFI_DEBUG
		_hx_printf("  WiFi debugging: host_to_card successfully.\r\n");
#endif
	}
	//wait_for_data_end();  //Wait for sending over.
	sdio_sys_wait=1;
	return ret;
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
 * 
 * In HelloX's current implementation,a dedicated WiFi driver thread is running in
 * background,the low_level_output just copy the frame to be sent into the sending
 * buffer of lbs_private struct,and send a message to WiFi driver thread,which will
 * wakeup the thread and do actual sending operation.
 */

err_t low_level_output(struct netif *netif, struct pbuf *p)
{
	struct txpd   *txpd         = NULL;
	struct pbuf   *q            = NULL;
	char          *p802x_hdr    = NULL;
	char          *buffer       = NULL;
	uint16_t      pkt_len       = 0;
	int           l             = 0;
	__KERNEL_THREAD_MESSAGE     msg;
	__ETH_INTERFACE_STATE*      pIfState = (__ETH_INTERFACE_STATE*)netif->state;
	
	sdio_sys_wait = 0;
	
#ifdef __WIFI_DEBUG
	_hx_printf("  WiFi debugging: low_level_output routine is called.\r\n");
#endif

	txpd=(void *)&priv->resp_buf[0][4];     //Why start from 4?
	memset(txpd, 0, sizeof(struct txpd));
	p802x_hdr = (char *)p->payload;         //802.3 mac.
	pkt_len = (uint16_t)p->tot_len;
	
	memcpy(txpd->tx_dest_addr_high, p802x_hdr, ETH_ALEN);
	txpd->tx_packet_length = cpu_to_le16(pkt_len);
	txpd->tx_packet_location = cpu_to_le32(sizeof(struct txpd));
	
	//Copy the frame to be sent into buffer.
	buffer=(char *)&txpd[1];
	for(q = p; q != NULL; q = q->next)
	{
		memcpy(buffer+l, q->payload, q->len);
		l+= (int)q->len;
	}
	priv->resp_len[0] = pkt_len + sizeof(struct txpd);//Total sending length,include txpd.
	
	//Indicate the ethernet thread that a packet is pending to send.
	pIfState->bSendPending = TRUE;
	
	//Update statistics info.
	pIfState->dwFrameSend += 1;
	pIfState->dwTotalSendSize += pkt_len;
	
	//Send a message to WiFi driver damon thread,this will wakeup the ethernet thread and
	//triger the physical sending process.
	msg.wCommand = WIFI_MSG_SEND;
	msg.wParam   = 0;
	msg.dwParam  = 0;
	SendMessage((HANDLE)g_pWiFiDriverThread,&msg);

	//lbs_sendpbuf(priv,p);
	//wait_for_data_end();
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
	struct eth_packet   *rx_pkt    = &pgmarvel_priv->rx_pkt;
  struct pbuf         *p, *q;
  u16                 len        = 0;
  int                 l          = 0;
  char                *buffer    = NULL;
  
	p = NULL;
  /* Obtain the size of the packet and put it into the "len"
     variable. */ 
  len = lbs_rev_pkt();
	
  if(len>0){
		buffer = rx_pkt->data;
		/* We allocate a pbuf chain of pbufs from the pool. */
		p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
		if (p != NULL){
			for (q = p; q != NULL; q = q->next){
				memcpy((u8_t*)q->payload, (u8_t*)&buffer[l], q->len);
				l = l + q->len;
	    }    
	  }
	  else{
	  	_hx_printf("low_level_input: Allocate pbuf failed.\r\n");
	  }
  }
  return p;
}

//Assist functions to handle the associate request.
void DoAssoc(__WIFI_ASSOC_INFO* pAssocInfo)
{
	if(0 == pAssocInfo->mode)  //Infrastructure mode.
	{
		marvel_assoc_network(priv,pAssocInfo->ssid,pAssocInfo->key,WIFI_MODE_INFRA);
  }
	else
	{
		marvel_assoc_network(priv,pAssocInfo->ssid,pAssocInfo->key,WIFI_MODE_ADHOC);
	}
}

//Handle the scan request.
void DoScan()
{
	struct bss_descriptor* iter = NULL;
	int i = 0;
	
	lbs_scan_worker(priv);				
	//Dump out the scan result.
#define list_for_each_entry_bssdes(pos, head, member)                 \
	for (pos = list_entry((head)->next,struct bss_descriptor, member);	\
	&pos->member != (head);                                             \
	pos = list_entry(pos->member.next,struct bss_descriptor, member))

	_hx_printf("  Available WiFi list:\r\n");
	_hx_printf("  ----------------------------- \r\n");
	list_for_each_entry_bssdes(iter, &priv->network_list, list)
	{
		_hx_printf("  %02d: BSSID = %06X, RSSI = %d, SSID = '%s'\n", \
						  i++, iter->bssid, iter->rssi, \
						  iter->ssid);
  }
}
