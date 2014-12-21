//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep,07 2014
//    Module Name               : ethif.h
//    Module Funciton           : 
//                                This file countains definitions of Ethernet
//                                interface.
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __ETHIF_H__
#define __ETHIF_H__

//Flags to enable or disable WiFi function debugging.
//#define __WIFI_DEBUG

/* Define those to better describe your network interface. */
#define IFNAME0 's'
#define IFNAME1 't'

//Global handle of WiFi driver thread.
extern __KERNEL_THREAD_OBJECT* g_pWiFiDriverThread;

//Messages that can be sent to WiFi driver thread.The WiFi driver thread is
//driven by message,other thread or application can call WiFi functions by
//send message to WiFi driver thread.
#define ETH_MSG_SEND    0x0001    //Send link level frame.
#define ETH_MSG_RECEIVE 0x0002    //Receive link level frame.
#define ETH_MSG_SCAN    0x0004    //Re-scan WiFi hot spot.
#define ETH_MSG_ASSOC   0x0008    //Associate a specified hot spot.
#define ETH_MSG_ACT     0x0010    //Activate the WiFi interface.
#define ETH_MSG_DEACT   0x0020    //De-activate the WiFi interface.
#define ETH_MSG_SETIP   0x0040    //Set IP address of WiFi interface.
#define ETH_MSG_DHCPSRV 0x0080    //Set interface's DHCP server configuration.
#define ETH_MSG_DHCPCLT 0x0100    //Set interface's DHCP client configuration.
#define ETH_MSG_SHOWINT 0x0200    //Display interface's statistics informtion.
#define ETH_MSG_DELIVER 0x0400    //Delivery a packet to upper layer.

//A helper structure used to change an ethernet interface's IP configuration.
//Shell thread can change the ethernet interface's configuration by sending
//a message contains this struct to ethernet thread.
typedef struct tag__ETH_IP_CONFIG{
	struct ip_addr ipaddr;           //IP address when DHCP disabled.
	struct ip_addr mask;             //Subnet mask of the interface.
	struct ip_addr defgw;            //Default gateway.
	struct ip_addr dnssrv_1;         //Primary DNS server address.
	struct ip_addr dnssrv_2;         //Second DNS server address.
}__ETH_IP_CONFIG;

//A object used to transfer associating information.
typedef struct tag__WIFI_ASSOC_INFO{
	char   ssid[24];
	char   key[24];
	char   mode;     //0 for infrastructure,1 for adHoc.
}__WIFI_ASSOC_INFO;

//Association of pbuf and netif,which is used by ethernet kernel thread
//to determine where the packet(pbuf) is received(netif).
typedef struct tag__IF_PBUF_ASSOC{
	struct pbuf*  p;
	struct netif* pnetif;
}__IF_PBUF_ASSOC;

//Interface state data associate to net interface,HelloX specified.All this state
//information is saved in static storage and is restored when HelloX boot up.
typedef struct tag__ETH_INTERFACE_STATE{
	__ETH_IP_CONFIG    IpConfig;
	__ETH_IP_CONFIG    DhcpConfig;   //DHCP server configuration,for example,the offering IP address range.
	BOOL               bDhcpCltEnabled;
	BOOL               bDhcpSrvEnabled;
	BOOL               bDhcpCltOK;   //If get IP address successfully in DHCP client mode.
	DWORD              dwDhcpLeasedTime;  //Leased time of the DHCP configuration,in million second.
	
	//Variables to control the sending process.
	BOOL               bSendPending; //Indicate if there is pending packets to send.
	
	//Statistics information from ethernet level.
	DWORD              dwFrameSend;         //How many frames has been sent since boot up.
	DWORD              dwFrameSendSuccess;  //The number of frames sent out successfully.
	DWORD              dwFrameRecv;         //How many frames has been received since boot up.
	DWORD              dwFrameRecvSuccess;  //Delivery pkt to upper layer successful.
	DWORD              dwTotalSendSize;     //How many bytes has been sent since boot.
	DWORD              dwTotalRecvSize;     //Receive size.
}__ETH_INTERFACE_STATE;

//Kernel thread name of Ethernet core.
#define ETH_THREAD_NAME  "eth_thread"

//Timer ID of receiving timer.We use polling mode to receive link level frame
//in current version,so a timer is used to wakeup the WiFi driver thread
//periodicly.
#define WIFI_TIMER_ID    0x1024

//The polling time period of receiving,in million-second.
#define WIFI_POLL_TIME   200

//Default WiFi hot spot SSID to associate when startup,and also the default
//SSID when HelloX running in AdHoc mode.
#define WIFI_DEFAULT_SSID "HelloX_HGW"

//Default key when associate with the default SSID,or the default key when
//run in AdHoc mode.
#define WIFI_DEFAULT_KEY  "0123456789012"

//Running mode of the HelloX.
#define WIFI_MODE_INFRA   '0'
#define WIFI_MODE_ADHOC   '1'

//Initializer of Ethernet interface.
BOOL InitializeEthernetIf(void);

#endif  //__ETHIF_H__
