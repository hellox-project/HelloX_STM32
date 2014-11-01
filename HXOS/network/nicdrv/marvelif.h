//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 26,2014
//    Module Name               : marvel.h
//    Module Funciton           : 
//                                This module countains Marvell WiFi ethernet
//                                driver's low level function definitions.
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

#ifndef __MARVELIF_H__
#define __MARVELIF_H__

//Do some preparations before entry low_level_init,for example initializes the
//hardware,set up operation environment,etc.
BOOL PrepareInit(void);

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
void low_level_init(struct netif *netif);

//Do the actual sending of a frame,by calling hardware level interface to copy
//the frame into WiFi chips buffer.
int DoPhysicalSend(struct netif* outif,struct pbuf* p);

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
err_t low_level_output(struct netif *netif, struct pbuf *p);

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
struct pbuf *low_level_input(struct netif *netif);

//Assist functions to handle the associate request.
void DoAssoc(__WIFI_ASSOC_INFO* pAssocInfo);

//Handle the scan request.
void DoScan(void);

#endif //__MARVELIF_H__
