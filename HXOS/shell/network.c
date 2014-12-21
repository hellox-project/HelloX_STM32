//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 29,2014
//    Module Name               : network.h
//    Module Funciton           : 
//    Description               : 
//                                Network diagnostic application,common used
//                                network tools such as ping/tracert,are implemented
//                                in network.c file.
//    Last modified Author      :
//    Last modified Date        : 
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//    Extra comment             : 
//***********************************************************************/

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif

#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/inet.h"
#include "nicdrv/ethif.h"

#include "kapi.h"
#include "shell.h"
#include "network.h"
#include "string.h"
#include "stdio.h"

#define  NETWORK_PROMPT_STR   "[network_view]"

//
//Pre-declare routines.
//
static DWORD CommandParser(LPCSTR);
static DWORD help(__CMD_PARA_OBJ*);        //help sub-command's handler.
static DWORD exit(__CMD_PARA_OBJ*);        //exit sub-command's handler.
static DWORD iflist(__CMD_PARA_OBJ*);
static DWORD ping(__CMD_PARA_OBJ*);
static DWORD route(__CMD_PARA_OBJ*);
static DWORD showint(__CMD_PARA_OBJ*);    //Display ethernet interface's statistics information.
static DWORD assoc(__CMD_PARA_OBJ*);      //Associate to a specified WiFi SSID.
static DWORD scan(__CMD_PARA_OBJ*);       //Rescan the WiFi networks.

//
//The following is a map between command and it's handler.
//
static struct __FDISK_CMD_MAP{
	LPSTR                lpszCommand;
	DWORD                (*CommandHandler)(__CMD_PARA_OBJ*);
	LPSTR                lpszHelpInfo;
}SysDiagCmdMap[] = {
	{"iflist",     iflist,    "  iflist   : Show all network interface(s) in system."},
	{"ping",       ping,      "  ping     : Check a specified host's reachbility."},
	{"route",      route,     "  route    : List all route entry(ies) in system."},
	{"exit",       exit,      "  exit     : Exit the application."},
	{"help",       help,      "  help     : Print out this screen."},
  {"showint",    showint,   "  showint  : Display ethernet interface's statistics information."},
  {"assoc",      assoc,     "  assoc    : Associate to a specified WiFi SSID."},
  {"scan",       scan,      "  scan     : Scan WiFi networks and show result."},
	{NULL,		   NULL,      NULL}
};


static DWORD QueryCmdName(LPSTR pMatchBuf,INT nBufLen)
{
	static DWORD dwIndex = 0;

	if(pMatchBuf == NULL)
	{
		dwIndex    = 0;	
		return SHELL_QUERY_CONTINUE;
	}

	if(NULL == SysDiagCmdMap[dwIndex].lpszCommand)
	{
		dwIndex = 0;
		return SHELL_QUERY_CANCEL;	
	}

	strncpy(pMatchBuf,SysDiagCmdMap[dwIndex].lpszCommand,nBufLen);
	dwIndex ++;

	return SHELL_QUERY_CONTINUE;	
}

//
//The following routine processes the input command string.
//
static DWORD CommandParser(LPCSTR lpszCmdLine)
{
	DWORD                  dwRetVal          = SHELL_CMD_PARSER_INVALID;
	DWORD                  dwIndex           = 0;
	__CMD_PARA_OBJ*        lpCmdParamObj     = NULL;

	if((NULL == lpszCmdLine) || (0 == lpszCmdLine[0]))    //Parameter check
	{
		return SHELL_CMD_PARSER_INVALID;
	}

	lpCmdParamObj = FormParameterObj(lpszCmdLine);
	
	
	if(NULL == lpCmdParamObj)    //Can not form a valid command parameter object.
	{
		return SHELL_CMD_PARSER_FAILED;
	}

	//if(0 == lpCmdParamObj->byParameterNum)  //There is not any parameter.
	//{
	//	return SHELL_CMD_PARSER_FAILED;
	//}
	//CD_PrintString(lpCmdParamObj->Parameter[0],TRUE);

	//
	//The following code looks up the command map,to find the correct handler that handle
	//the current command.Calls the corresponding command handler if found,otherwise SYS_DIAG_CMD_PARSER_INVALID
	//will be returned to indicate this case.
	//
	while(TRUE)
	{
		if(NULL == SysDiagCmdMap[dwIndex].lpszCommand)
		{
			dwRetVal = SHELL_CMD_PARSER_INVALID;
			break;
		}
		if(StrCmp(SysDiagCmdMap[dwIndex].lpszCommand,lpCmdParamObj->Parameter[0]))  //Find the handler.
		{
			dwRetVal = SysDiagCmdMap[dwIndex].CommandHandler(lpCmdParamObj);
			break;
		}
		else
		{
			dwIndex ++;
		}
	}

	//Release parameter object.
	if(NULL != lpCmdParamObj)
	{
		ReleaseParameterObj(lpCmdParamObj);
	}

	return dwRetVal;
}

//
//This is the application's entry point.
//
DWORD networkEntry(LPVOID p)
{
	return Shell_Msg_Loop(NETWORK_PROMPT_STR,CommandParser,QueryCmdName);	
}

//
//The exit command's handler.
//
static DWORD exit(__CMD_PARA_OBJ* lpCmdObj)
{
	return SHELL_CMD_PARSER_TERMINAL;
}

//
//The help command's handler.
//
static DWORD help(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD               dwIndex = 0;

	while(TRUE)
	{
		if(NULL == SysDiagCmdMap[dwIndex].lpszHelpInfo)
			break;

		PrintLine(SysDiagCmdMap[dwIndex].lpszHelpInfo);
		dwIndex ++;
	}
	return SHELL_CMD_PARSER_SUCCESS;
}

//route command's implementation.
static DWORD route(__CMD_PARA_OBJ* lpCmdObj)
{
	return SHELL_CMD_PARSER_SUCCESS;
}

//ping command's implementation.
static DWORD ping(__CMD_PARA_OBJ* lpCmdObj)
{	
	__PING_PARAM     PingParam;
	ip_addr_t        ipAddr;
	int              count      = 3;    //Ping counter.
	int              size       = 64;   //Ping packet size.
	BYTE             index      = 1;
	DWORD            dwRetVal   = SHELL_CMD_PARSER_FAILED;
	__CMD_PARA_OBJ*  pCurCmdObj = lpCmdObj;
	

	if(pCurCmdObj->byParameterNum <= 1)
	{
		return dwRetVal;
	}

	while(index < lpCmdObj->byParameterNum)
	{
		if(strcmp(pCurCmdObj->Parameter[index],"/c") == 0)
		{
			index ++;
			if(index >= lpCmdObj->byParameterNum)
			{
				break;
			}
			count    = atoi(pCurCmdObj->Parameter[index]);
		}
		else if(strcmp(pCurCmdObj->Parameter[index],"/l") == 0)
		{
			index ++;
			if(index >= lpCmdObj->byParameterNum)
			{
				break;
			}
			size   = atoi(pCurCmdObj->Parameter[index]);
		}
		else
		{
			ipAddr.addr = inet_addr(pCurCmdObj->Parameter[index]);
		}

		index ++;
	}
	
	if(ipAddr.addr != 0)
	{
		dwRetVal    = SHELL_CMD_PARSER_SUCCESS;
	}

	PingParam.count      = count;
	PingParam.targetAddr = ipAddr;
	PingParam.size       = size;

	//Call ping entry routine.
	ping_Entry((void*)&PingParam);
	
	return dwRetVal;
}

//A helper routine used to dumpout a network interface.
static void ShowIf(struct netif* pIf)
{
	char    buff[128];
	
	//Print out all information about this interface.
	PrintLine("  --------------------------------------");
	_hx_sprintf(buff,"  Inetface name : %c%c",pIf->name[0],pIf->name[1]);
	PrintLine(buff);
	_hx_sprintf(buff,"      IPv4 address   : %s",inet_ntoa(pIf->ip_addr));
	PrintLine(buff);
	_hx_sprintf(buff,"      IPv4 mask      : %s",inet_ntoa(pIf->netmask));
	PrintLine(buff);
	_hx_sprintf(buff,"      IPv4 gateway   : %s",inet_ntoa(pIf->gw));
	PrintLine(buff);
	_hx_sprintf(buff,"      Interface MTU  : %d",pIf->mtu);
	PrintLine(buff);
}

//iflist command's implementation.
static DWORD iflist(__CMD_PARA_OBJ* lpCmdObj)
{
	struct netif* pIfList = netif_list;
	//char   buff[128];

	while(pIfList)  //Travel the whole list and dumpout everyone.
	{
		ShowIf(pIfList);
		pIfList = pIfList->next;
	}

	return SHELL_CMD_PARSER_SUCCESS;
}

//showint command,display statistics information of ethernet interface.
static DWORD showint(__CMD_PARA_OBJ* lpCmdObj)
{
	__KERNEL_THREAD_MESSAGE msg;
	
	//Just send a message to ethernet main thread.
	msg.wCommand = ETH_MSG_SHOWINT;
	msg.dwParam  = 0;
	msg.wParam   = 0;
	SendMessage((HANDLE)g_pWiFiDriverThread,&msg);
	
	return NET_CMD_SUCCESS;
}

//WiFi association operation,associate to a specified WiFi SSID.
static DWORD assoc(__CMD_PARA_OBJ* lpCmdObj)
{
	 DWORD                   dwRetVal   = SHELL_CMD_PARSER_FAILED;
	__WIFI_ASSOC_INFO*      pAssocInfo  = NULL;
	BYTE                    index       = 1;
	__KERNEL_THREAD_MESSAGE msg;

	//Allocate a association information object,to contain user specified associating info.
	//This object will be destroyed by ethernet thread.
	pAssocInfo = (__WIFI_ASSOC_INFO*)KMemAlloc(sizeof(__WIFI_ASSOC_INFO),KMEM_SIZE_TYPE_ANY);
	if(NULL == pAssocInfo)
	{
		goto __TERMINAL;
	}
	
	//Initialize to default value.
	pAssocInfo->ssid[0] = 0;
	pAssocInfo->key[0]  = 0;
	pAssocInfo->mode    = 0;
	
	if(lpCmdObj->byParameterNum <= 1)
	{
		goto __TERMINAL;
	}

	//lpCmdObj[0].Parameter
	while(index < lpCmdObj->byParameterNum)
	{		
		if(strcmp(lpCmdObj->Parameter[index],"/k") == 0) //Key of association.		
		{
			index ++;
			if(index >= lpCmdObj->byParameterNum)
			{
				_hx_printf("  Error: Invalid parameter.\r\n");
				goto __TERMINAL;
			}
			if(strlen(lpCmdObj->Parameter[index]) >= 24)  //Key is too long.
			{
				_hx_printf("  Error: The key you specified is too long.\r\n");
				goto __TERMINAL;
			}
			//Copy the key into information object.
			strcpy(pAssocInfo->key,lpCmdObj->Parameter[index]);
		}
		else if(strcmp(lpCmdObj->Parameter[index],"/m") == 0) //Assoction mode.
		{
			index ++;
			if(index >= lpCmdObj->byParameterNum)
			{
				_hx_printf("  Error: Invalid parameter.\r\n");
				goto __TERMINAL;
			}						
			if(strcmp(lpCmdObj->Parameter[index],"adhoc") == 0)  //AdHoc mode.
			{
				pAssocInfo->mode = 1;
			}
		}		
		else
		{
			//Default parameter as IP address.			
			//index ++;
			if(index >= lpCmdObj->byParameterNum)
			{
				_hx_printf("  Error: Invalid parameter.\r\n");
				goto __TERMINAL;
			}

			if(strlen(lpCmdObj->Parameter[index]) >= 24)  //SSID too long.
			{
				_hx_printf("  Error: SSID you specified is too long.\r\n");
				goto __TERMINAL;
			}

			//Copy the SSID into information object.
			strcpy(pAssocInfo->ssid,lpCmdObj->Parameter[index]);		
		}
		index ++;
	}
	
	//Re-check the parameters.
	if(0 == pAssocInfo->ssid[0])
	{
		_hx_printf("  Error: Please specifiy the SSID to associate with.\r\n");
		goto __TERMINAL;
	}
	
	//Everything is OK,send a message to ethernet thread.
	msg.wCommand = ETH_MSG_ASSOC;
	msg.wParam   = 0;
	msg.dwParam  = (DWORD)pAssocInfo;
	SendMessage((HANDLE)g_pWiFiDriverThread,&msg);
	
	dwRetVal = SHELL_CMD_PARSER_SUCCESS;
	
__TERMINAL:
	if(dwRetVal != SHELL_CMD_PARSER_SUCCESS)  //Should release the Association object.
	{
		KMemFree(pAssocInfo,KMEM_SIZE_TYPE_ANY,0);
	}
	return dwRetVal;
}


//Scan WiFi networks and show the scanning result.
static DWORD scan(__CMD_PARA_OBJ* lpCmdObj)
{
	__KERNEL_THREAD_MESSAGE msg;
	
	//Just send a message to ethernet main thread.
	msg.wCommand = ETH_MSG_SCAN;
	msg.dwParam  = 0;
	msg.wParam   = 0;
	SendMessage((HANDLE)g_pWiFiDriverThread,&msg);
	
	return NET_CMD_SUCCESS;
}

