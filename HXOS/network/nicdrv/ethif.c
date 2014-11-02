//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 26,2014
//    Module Name               : ethif.c
//    Module Funciton           : 
//                                This module countains HelloX ethernet skeleton's
//                                implementation code.
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

#ifdef __STM32__
#ifndef __MARVELIF_H__
#include "marvelif.h"
#endif
#endif

//Global handle of WiFi driver thread.
__KERNEL_THREAD_OBJECT* g_pWiFiDriverThread = NULL;

//Interface object corresponding the WiFi interface.Only one ethernet interface is
//supported in current HelloX's version so we define it staticly...
static struct    netif netif_eth = {0};

//Ethernet interface state information.
static __ETH_INTERFACE_STATE ifState = {0};

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
  err_t  err     = ERR_OK;
  struct pbuf *p = NULL;
	
	//Receiving loop,receive as many as possible in one polling period,to
	//improve efficiency.
	while(TRUE)
	{
#ifdef __STM32__
		/* move received packet into a new pbuf */
		p = low_level_input(netif);
#else
		p = NULL;
#endif
		/* no packet could be read, give the polling */
		if (p == NULL)
		{
			break;
    }
		
		//Update statistics info.
		ifState.dwFrameRecv ++;
		ifState.dwTotalRecvSize += p->tot_len;		
#ifdef __WIFI_DEBUG
	  _hx_printf("  WiFi Debugging: Received a pkt from WiFi and delivery...\r\n");
#endif
		
		//Delivery the received pkt to uper layer.
    err = netif->input(p, netif);
    if (err != ERR_OK)
		{
#ifdef __WIFI_DEBUG
			_hx_printf("  WiFi Debugging: Failed to delivery the pkt to IP layer.\r\n");
#endif
      pbuf_free(p);  //The pbuf will be released by upper layer when successful delivered.
      p = NULL;
    }
		else
		{
			ifState.dwFrameRecvSuccess ++;
#ifdef __WIFI_DEBUG
			_hx_printf("  WiFi Debugging: Delivery an IP pkt to TCP/IP successfully.\r\n");
#endif
		}
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
  LWIP_ASSERT("netif != NULL", (netif != NULL));
  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, 10000000);
  netif->state   = &ifState;
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
	
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output       = etharp_output;
	netif->hwaddr_len = ETHARP_HWADDR_LEN;
#ifdef __STM32__
  netif->linkoutput   = low_level_output;
  low_level_init(netif);
#endif
  return ERR_OK;
}

//A helper routine to check assist the DHCP process.It checks if the DHCP
//process is successful,and do proper actions(such as set the offered IP
//address to interface) according DHCP status.
static void dhcpAssist(struct netif* pif,__ETH_INTERFACE_STATE* pifState)
{
	if(!pifState->bDhcpCltEnabled)  //DHCP client is disabled.
	{
		return;
	}
	if(pifState->bDhcpCltOK)  //DHCP process is over.
	{
		//Just update the DHCP leased timer.
		pifState->dwDhcpLeasedTime += WIFI_POLL_TIME;
		return;
	}
	
	//Now try to check if we get the valid configuration from DHCP server,which
	//stored in netif->dhcp by DHCP client function,implemented in lwIP.
	if(pif->dhcp)
	{
		if((pif->dhcp->offered_ip_addr.addr != 0) &&
			 (pif->dhcp->offered_gw_addr.addr != 0) &&
		   (pif->dhcp->offered_sn_mask.addr != 0))
		{
			pifState->bDhcpCltOK = TRUE;
			//Set DHCP server offered IP configurations to interface.
			netif_set_down(pif);
			netif_set_addr(pif,&pif->dhcp->offered_ip_addr,
			    &pif->dhcp->offered_sn_mask,&pif->dhcp->offered_gw_addr);
			netif_set_up(pif);
#ifdef __WIFI_DEBUG
			_hx_printf("  WiFi debugging: Get IP configuration from DHCP server successfully.\r\n");
#endif
			//Stop DHCP process in the interface.
			dhcp_stop(pif);
			pifState->dwDhcpLeasedTime = 0;  //Start to count DHCP time.
		}
	}
	return;
}

//Dedicated Ethernet receiving thread,repeatly call ethernetif_input to
//try to find a valid ethernet frame.
//It's a temporary solution for Ethernet driver,interrupt driving mechanism
//will be introduced in the future.
static DWORD WiFiDriverThreadEntry(LPVOID pData)
{
	__KERNEL_THREAD_MESSAGE msg;
	HANDLE                  hTimer = NULL;           //Handle of receiving poll timer.
	struct ip_addr          ipaddr;                  //IP address of Ethernet interface.
	struct ip_addr          netmask;                 //Subnet mask of Ethernet interface.
	struct ip_addr          gw;                      //Default gateway.
	struct netif*           pif    = (struct netif*)pData;
	__WIFI_ASSOC_INFO*      pAssocInfo = NULL;

  //Call prepare_init routine first.
#ifdef __STM32__
  if(!PrepareInit())
	{
		goto __TERMINAL;
	}
#endif
	
	//Initialize IP parameters of the interface according history configuration.
	if(!ifState.bDhcpCltEnabled)  //Config staticaly.
	{
		//Configure the interface.
		netif_add(pif, &ifState.IpConfig.ipaddr,&ifState.IpConfig.mask, &ifState.IpConfig.defgw, \
		  &ifState, &ethernetif_init, &tcpip_input);
		netif_set_default(pif);
		netif_set_up(pif);
	}
	else  //Config the interface through DHCP process.
	{
		//Set IP address to 0.
		IP4_ADDR(&ipaddr,0,0,0,0);
		IP4_ADDR(&netmask,0,0,0,0);
		IP4_ADDR(&gw,0,0,0,0);
		netif_add(pif,&ipaddr,&netmask,&gw,&ifState,&ethernetif_init, &tcpip_input);
		netif_set_default(pif);
		netif_set_up(pif);
		
		ifState.bDhcpCltOK = FALSE;
		dhcp_start(pif);     //Start DHCL client process under this interface.
	}
	
	//Set the receive polling timer.
	hTimer = SetTimer(WIFI_TIMER_ID,WIFI_POLL_TIME,NULL,NULL,TIMER_FLAGS_ALWAYS);
	if(NULL == hTimer)
	{
		goto __TERMINAL;
	}
	
	while(TRUE)
	{
		if(GetMessage(&msg))
		{
			switch(msg.wCommand)
			{
				case KERNEL_MESSAGE_TERMINAL:
					goto __TERMINAL;
				
				case WIFI_MSG_SEND:                //Send a link level frame.
					if(!ifState.bSendPending)        //Maybe duplicated send message.
					{
						break;
					}
#ifdef __STM32__
					DoPhysicalSend(&netif_eth,NULL); //Send all pending packets.
#endif
					ifState.bSendPending = FALSE;
					break;
					
				case WIFI_MSG_RECEIVE:             //Receive frame,may triggered by interrupt.
					ethernetif_input(pif);
					break;
				
				case KERNEL_MESSAGE_TIMER:
					if(WIFI_TIMER_ID == msg.dwParam) //Must match the receiving timer ID.
					{
						ethernetif_input(pif);
					}
					dhcpAssist(pif,&ifState);        //Call DHCP assist function routinely.
					break;
					
				case WIFI_MSG_SCAN:                //Scan WiFi hot spot.
#ifdef __STM32__
				  DoScan();
#endif
					break;
					
				case WIFI_MSG_ASSOC:               //Associate to a specified WiFi hot spot.
					pAssocInfo = (__WIFI_ASSOC_INFO*)msg.dwParam;
				  if(NULL == pAssocInfo)
					{
						_hx_printf("  A valid association info object must be specified.\r\n");
						break;
					}
					_hx_printf("\r\n");
				  _hx_printf("  Try to associate to [%s] in [%s] mode...\r\n", \
					    pAssocInfo->ssid, \
					    (0 == pAssocInfo->mode) ? "INFRA" : "ADHOC");
#ifdef __STM32__
					DoAssoc(pAssocInfo);
#endif
					//Destroy the association information object.
					KMemFree(pAssocInfo,KMEM_SIZE_TYPE_ANY,0);
					break;
					
				case WIFI_MSG_SHOWINT:             //Show interface's statistics info.
					_hx_printf("\r\n");
					_hx_printf("  Ethernet statistics information:\r\n");
				  _hx_printf("    Send frame #       : %d\r\n",ifState.dwFrameSend);
				  _hx_printf("    Success send #     : %d\r\n",ifState.dwFrameSendSuccess);
				  _hx_printf("    Send bytes size    : %d\r\n",ifState.dwTotalSendSize);
				  _hx_printf("    Receive frame #    : %d\r\n",ifState.dwFrameRecv);
				  _hx_printf("    Success recv #     : %d\r\n",ifState.dwFrameRecvSuccess);
				  _hx_printf("    Receive bytes size : %d\r\n",ifState.dwTotalRecvSize);
				  break;
				
				default:
					break;
			}
		}
	}
	
__TERMINAL:
	if(hTimer)  //Should cancel it.
	{
		CancelTimer(hTimer);
	}
	//Remove the WiFi interface.
	netif_remove(pif);
	return 0;
}

//Initialization code of Ethernet Interface.It adds a net interface object,
//install interrupt handler of NIC,or start a dedicated kernel thread to poll
//Ethernet interface.
BOOL InitializeEthernetIf()
{
	//Restore ethernet interface configuration from flash or NVROM.
	ifState.bDhcpCltEnabled   = TRUE;
	ifState.bDhcpCltOK        = FALSE;
	ifState.bDhcpSrvEnabled   = FALSE;
	IP4_ADDR(&ifState.IpConfig.ipaddr,169,254,0,10);
	IP4_ADDR(&ifState.IpConfig.mask,255,255,0,0);
	IP4_ADDR(&ifState.IpConfig.defgw,169,254,0,1);
	
	//Initialize statistics or state controlling variables.
	ifState.bSendPending         = FALSE;
	ifState.dwFrameRecv          = 0;
	ifState.dwFrameRecvSuccess   = 0;
	ifState.dwTotalRecvSize      = 0;
	ifState.dwFrameSend          = 0;
	ifState.dwFrameSendSuccess   = 0;
	ifState.dwTotalSendSize      = 0;
	
	//Create and start ethernet receiving thread.
	g_pWiFiDriverThread = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,    //Use enough stack size.
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		WiFiDriverThreadEntry,
		(LPVOID)&netif_eth,
		NULL,
		ETH_THREAD_NAME);
	if(NULL == g_pWiFiDriverThread)  //Can not create receiving thread.
	{
		//netif_remove(&netif_eth);
		return FALSE;
	}
	return TRUE;
}
