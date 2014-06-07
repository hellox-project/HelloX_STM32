//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,27 2004
//    Module Name               : shell.cpp
//    Module Funciton           : 
//                                This module countains shell procedures.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/
#ifndef __STDAFX_H__
#include "..\INCLUDE\STDAFX.H"
#endif

#ifndef __KAPI_H__
#include "..\INCLUDE\KAPI.H"
#endif

#include "shell.h"
#include "IOCTRL_S.H"
#include "SYSD_S.H"
#include "extcmd.h"

#if defined(__I386__)
#ifndef __BIOS_H__
#include "..\arch\x86\BIOS.H"
#endif
#endif

#include "..\INCLUDE\MODMGR.H"
#include "..\include\console.h"
#include "..\lib\stdio.h"

//Host name array of the system.
#define MAX_HOSTNAME_LEN  16
CHAR    HostName[MAX_HOSTNAME_LEN] = {0};

//Shell thread's handle.
__KERNEL_THREAD_OBJECT*  g_lpShellThread = NULL;

//Command buffer and it's pointer.
static CHAR        CmdBuffer[MAX_BUFFER_LEN] = {0};
static WORD        BufferPtr = 0;  //Pointing to the first free byte of the CmdBuffer.

//
//The following function form the command parameter object link from the command
//line string.
//
__CMD_PARA_OBJ* FormParameterObj(LPSTR pszCmd)
{
	__CMD_PARA_OBJ*     pObjBuffer = NULL;    //Local variables.
	__CMD_PARA_OBJ*     pBasePtr   = NULL;
	__CMD_PARA_OBJ*     pTmpObj    = NULL;
	DWORD               dwCounter  = 0x0000;
	DWORD               index      = 0x0000;

	if(NULL == pszCmd)    //Parameter check.
		return NULL;

	pObjBuffer = (__CMD_PARA_OBJ*)KMemAlloc(1024,KMEM_SIZE_TYPE_ANY);
	if(NULL == pObjBuffer)
		goto __TERMINAL;

	pBasePtr = pObjBuffer;
	memzero(pBasePtr,1024);

	while(*pszCmd)
	{
		if(' ' == *pszCmd)
		{
			pszCmd ++;
			continue; 
		}                                 //Filter the space.
		
		if(('-' == *pszCmd) || ('/' == *pszCmd))
		{
			pszCmd ++;
			pObjBuffer->byFunctionLabel = *pszCmd;
			pszCmd ++;                    //Skip the function label byte.
			continue;
		}
		else
		{
			/*while((' ' != *pszCmd) && *pszCmd)  //To find the first parameter.
			{
				pszCmd ++;
			}
			if(!*pszCmd)
				break;
			while(' ' == *pszCmd)    //Filter the space.
				pszCmd ++;

			if(!*pszCmd)
				break;*/
			index = 0x0000;
			while(('-' != *pszCmd) && ('/' != *pszCmd) && *pszCmd)
			{
				while((' ' != *pszCmd) && (*pszCmd) && (dwCounter <= CMD_PARAMETER_LEN))
				{
					pObjBuffer->Parameter[index][dwCounter] = *pszCmd;
					pszCmd ++;
					dwCounter ++;
				}
				pObjBuffer->Parameter[index][dwCounter] = 0x00;  //Set the terminal flag.
				index ++;               //Ready to copy the next parameter to parameter object.
				dwCounter = 0;

				if(!*pszCmd)
					break;
				while(' ' != *pszCmd)
					pszCmd ++;          //Skip the no space characters if the parameter's length
				                        //is longer than the const CMD_PARAMETER_LEN.
				while(' ' == *pszCmd)
					pszCmd ++;          //Skip the space character.
			}

			pTmpObj = pObjBuffer;       //Update the current parameter object.
			pObjBuffer = (__CMD_PARA_OBJ*)NextParaAddr(pTmpObj,index);
			pTmpObj->byParameterNum = (BYTE)(index);
			if(!*pszCmd)
				break;
			pTmpObj->pNext = pObjBuffer;
		}
	}

__TERMINAL:
	return pBasePtr;
}

//
//Releases the parameter object created by FormParameterObj routine.
//
VOID ReleaseParameterObj(__CMD_PARA_OBJ* lpParamObj)
{
	if(NULL == lpParamObj)  //Parameter check.
		return;

	KMemFree((LPVOID)lpParamObj,KMEM_SIZE_TYPE_ANY,0);  //Release the memory.
	return;
}

//The following handlers are moved to shell1.cpp.
extern VOID VerHandler(LPSTR);          //Handles the version command.
extern VOID MemHandler(LPSTR);          //Handles the memory command.
extern VOID SysInfoHandler(LPSTR);      //Handles the sysinfo command.
extern VOID HlpHandler(LPSTR);
extern VOID LoadappHandler(LPSTR);
extern VOID GUIHandler(LPSTR);          //Handler for GUI command,resides in
                                        //shell2.cpp file.

static VOID CpuHandler(LPSTR);
static VOID SptHandler(LPSTR);
static VOID ClsHandler(LPSTR);
static VOID RunTimeHandler(LPSTR);
static VOID SysNameHandler(LPSTR);
static VOID IoCtrlApp(LPSTR);
static VOID SysDiagApp(LPSTR);
static VOID Reboot(LPSTR);
static VOID Poweroff(LPSTR);
static VOID ComDebug(LPSTR);

//Internal command handler array.
__CMD_OBJ  CmdObj[] = {
	{"version"  ,    VerHandler},
	{"memory"   ,    MemHandler},
	{"sysinfo"  ,    SysInfoHandler},
	{"sysname"  ,    SysNameHandler},
	{"help"     ,    HlpHandler},
	{"cpuinfo"  ,    CpuHandler},
	{"support"  ,    SptHandler},
	{"runtime"  ,    RunTimeHandler},
	{"ioctrl"   ,    IoCtrlApp},
	{"sysdiag"  ,    SysDiagApp},
	{"loadapp"  ,    LoadappHandler},
	{"gui"      ,    GUIHandler},
	{"reboot"   ,    Reboot},
	{"poff"     ,    Poweroff},
	{"comdebug" ,    ComDebug},
	{"cls"      ,    ClsHandler},
	//You can add your specific command and it's handler here.
	//{'yourcmd',    CmdHandler},

	//The last element of this array must be NULL.
	{NULL       ,    NULL}
};

//Com interface debugging application.
static VOID ComDebug(LPSTR pstr)
{
	HANDLE hCom1 = NULL;
	CHAR   *pData = "Hello China V1.76";  //Data to write to COM1 interface.
	DWORD  dwWriteSize = 0;
	CHAR   buff[16];

	//Try to open COM1 interface to debugging.
	hCom1 = IOManager.CreateFile(
		(__COMMON_OBJECT*)&IOManager,
		"\\\\.\\COM1",
		0,
		0,
		NULL);
	if(NULL == hCom1)
	{
		PrintLine("ComDebug : Can not open COM1 interface.");
		goto __TERMINAL;
	}
	else
	{
		PrintLine("ComDebug: Open COM1 interface successfully.");
		if(IOManager.WriteFile((__COMMON_OBJECT*)&IOManager,
			hCom1,
			strlen(pData),
			pData,
			&dwWriteSize))
		{
			PrintLine("Write data to COM1 interface successfully.");
		}
		else
		{
			PrintLine("Can not write data to COM1 interface.");
		}
		PrintLine("ComDebug: Try to read data from COM interface...");
		if(!IOManager.ReadFile((__COMMON_OBJECT*)&IOManager,
			hCom1,
			1,
			(LPVOID)&buff[0],
			NULL))
		{
			PrintLine("Can not read COM interface.");
		}
		else
		{
			PrintLine("Read COM interface sucessfully.");
		}
	}

__TERMINAL:
	if(NULL != hCom1)
	{
		IOManager.CloseFile((__COMMON_OBJECT*)&IOManager,
			hCom1);
	}
	return;
}

//
//sysname handler.
//This handler changes the system name,and save it to system config database.
//
static VOID SaveSysName(LPSTR pstr)
{
}

static VOID SysNameHandler(LPSTR pszSysName)
{
	__CMD_PARA_OBJ*    pCmdObj = NULL;

	pCmdObj = FormParameterObj(pszSysName);
	if(NULL == pCmdObj)
	{
		PrintLine("Not enough system resource to interpret the command.");
		goto __TERMINAL;
	}
	if((0 == pCmdObj->byParameterNum) || (0 == pCmdObj->Parameter[0][0]))
	{
		PrintLine("Invalid command parameter.");
		goto __TERMINAL;
	}

	if(StrLen(pCmdObj->Parameter[0]) >= MAX_HOSTNAME_LEN)
	{
		PrintLine("System name must not exceed 16 bytes.");
		goto __TERMINAL;
	}

	SaveSysName(pCmdObj->Parameter[0]);
	StrCpy(pCmdObj->Parameter[0],&HostName[0]);
__TERMINAL:
	if(NULL != pCmdObj)
		KMemFree((LPVOID)pCmdObj,KMEM_SIZE_TYPE_ANY,0);
	return;
}

//Handler for 'ioctrl' command.
VOID IoCtrlApp(LPSTR pstr)
{
	__KERNEL_THREAD_OBJECT*    lpIoCtrlThread    = NULL;

	lpIoCtrlThread = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		IoCtrlStart,
		NULL,
		NULL,
		"IO CTRL");
	if(NULL == lpIoCtrlThread)    //Can not create the IO control thread.
	{
		PrintLine("Can not create IO control thread.");
		return;
	}

	DeviceInputManager.SetFocusThread((__COMMON_OBJECT*)&DeviceInputManager,
		(__COMMON_OBJECT*)lpIoCtrlThread);    //Set the current focus to IO control
	                                          //application.

	lpIoCtrlThread->WaitForThisObject((__COMMON_OBJECT*)lpIoCtrlThread);  //Block the shell
	                                                                      //thread until
	                                                                      //the IO control
	                                                                      //application end.
	KernelThreadManager.DestroyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
		(__COMMON_OBJECT*)lpIoCtrlThread);  //Destroy the thread object.

}

//
//System diag application's shell start code.
//
VOID SysDiagApp(LPSTR pstr)
{
	__KERNEL_THREAD_OBJECT*        lpSysDiagThread    = NULL;

	lpSysDiagThread = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		SysDiagStart,
		NULL,
		NULL,
		"SYS DIAG");
	if(NULL == lpSysDiagThread)    //Can not create the kernel thread.
	{
		PrintLine("Can not start system diag application,please retry again.");
		return;
	}

	DeviceInputManager.SetFocusThread((__COMMON_OBJECT*)&DeviceInputManager,
		(__COMMON_OBJECT*)lpSysDiagThread);

	lpSysDiagThread->WaitForThisObject((__COMMON_OBJECT*)lpSysDiagThread);
	KernelThreadManager.DestroyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
		(__COMMON_OBJECT*)lpSysDiagThread);  //Destroy the kernel thread object.
}

//Entry point of reboot.
VOID Reboot(LPSTR pstr)
{
	ClsHandler(NULL); //Clear screen first.
#ifdef __I386__
	BIOSReboot();
#endif
}

//Entry point of poweroff.
VOID Poweroff(LPSTR pstr)
{
#ifdef __I386__
	BIOSPoweroff();
#endif
}

//Handler for 'runtime' command.
VOID RunTimeHandler(LPSTR pstr)
{
	CHAR  Buffer[192];
	DWORD week = 0,day = 0,hour = 0,minute = 0,second = 0;

	second = System.GetSysTick(NULL);  //Get system tick counter.
	//Convert to second.
	second *= SYSTEM_TIME_SLICE;
	second /= 1000;

	if(second >= 60)  //Use minute.
	{
		minute = second / 60;
		second = second % 60;
	}
	if(minute >= 60) //use hour.
	{
		hour   = minute / 60;
		minute = minute % 60;
	}
	if(hour >= 24) //Use day.
	{
		day  = hour / 24;
		hour = hour % 24;
	}
	if(day >= 7) //Use week.
	{
		week = day / 7;
		day  = day % 7;
	}

	//Show out the result.
	sprintf(Buffer,"System has running %d week(s), %d day(s), %d hour(s), %d minute(s), %d second(s).",
		week,day,hour,minute,second);
	PrintLine(Buffer);
}

//Handler for 'cls' command.
VOID ClsHandler(LPSTR pstr)
{
	ClearScreen();
}

//Handler for 'cpu' command.
VOID CpuHandler(LPSTR pstr)
{
	GotoHome();
	ChangeLine();
	PrintStr("Cpu Handler called.");
}

//Handler for 'support' command.
VOID SptHandler(LPSTR pstr)
{
	LPSTR strSupportInfo1 = "    For any technical support,send E-Mail to:";
	LPSTR strSupportInfo2 = "    garryxin@yahoo.com.cn.";
	LPSTR strSupportInfo3 = "    or join the QQ group : 38467832";

	PrintLine(strSupportInfo1);
	PrintLine(strSupportInfo2);
	PrintLine(strSupportInfo3);
	return;
}

//Default handler if no proper handler is located.
VOID  DefaultHandler(LPSTR pstr)
{
	LPSTR strPrompt = "You entered incorrect command name.";
	GotoHome();
	ChangeLine();
	PrintStr(strPrompt);
	return;
}

//Command analyzing routine,it analyzes user's input and search
//command array to find a proper handler,then call it.
//Default handler will be called if no proper command handler is
//located.
static VOID  DoCommand()
{
	DWORD wIndex = 0x0000;
	BOOL bResult = FALSE;        //If find the correct command object,then
	                             //This flag set to TRUE.
	CHAR tmpBuffer[36];
	DWORD dwIndex = 0;           //Used for 'for' loop.
	__KERNEL_THREAD_OBJECT* hKernelThread = NULL;

	CmdBuffer[BufferPtr] = 0x00; //Prepare the command string.
	BufferPtr = 0;

	while((' ' != CmdBuffer[wIndex]) && CmdBuffer[wIndex] && (wIndex < 32))
	{
		tmpBuffer[wIndex] = CmdBuffer[wIndex];
		wIndex ++;
	}
	tmpBuffer[wIndex] = 0;

	dwIndex = 0;
	//for(dwIndex = 0;dwIndex < CMD_OBJ_NUM;dwIndex ++)
	while(CmdObj[dwIndex].CmdStr)
	{
		if(StrCmp(&tmpBuffer[0],CmdObj[dwIndex].CmdStr))
		{
			CmdObj[dwIndex].CmdHandler(&CmdBuffer[wIndex]);  //Call the command handler.
			bResult = TRUE;      //Set the flag.
			break;
		}
		dwIndex ++;
	}
	if(bResult)
	{
		goto __END;
	}

	dwIndex = 0;  //Now,should search external command array.
	while(ExtCmdArray[dwIndex].lpszCmdName)
	{
		if(StrCmp(&tmpBuffer[0],ExtCmdArray[dwIndex].lpszCmdName))  //Found.
		{
			hKernelThread = KernelThreadManager.CreateKernelThread(
				(__COMMON_OBJECT*)&KernelThreadManager,
				0,
				KERNEL_THREAD_STATUS_READY,
				PRIORITY_LEVEL_NORMAL,
				ExtCmdArray[dwIndex].ExtCmdHandler,
				(LPVOID)&CmdBuffer[wIndex],
				NULL,
				NULL);
			if(!ExtCmdArray[dwIndex].bBackground)  //Should wait.
			{
				DeviceInputManager.SetFocusThread((__COMMON_OBJECT*)&DeviceInputManager,
					(__COMMON_OBJECT*)hKernelThread);  //Give the current input focus to this thread.
				hKernelThread->WaitForThisObject((__COMMON_OBJECT*)hKernelThread);
				KernelThreadManager.DestroyKernelThread(
					(__COMMON_OBJECT*)&KernelThreadManager,
					(__COMMON_OBJECT*)hKernelThread);  //Destroy it.
				//Set focus thread to shell.
				DeviceInputManager.SetFocusThread((__COMMON_OBJECT*)&DeviceInputManager,
					NULL);
			}
			bResult = TRUE;
			goto __END;
		}
		dwIndex ++;
	}

	if(!bResult)
	{
		DefaultHandler(NULL);        //Call the default command handler.
	}

__END:
	return;
}

//Print out command prompt.
static VOID  PrintPrompt()
{
	LPSTR pszSysName = "[system-view]";
	if(HostName[0])
	{
		PrintLine(&HostName[0]);
	}
	else
	{
	    PrintLine(pszSysName);
	}
	return;
}

//Shell thread's event handler.
static BOOL EventHandler(WORD wCommand,WORD wParam,DWORD dwParam)
{
	WORD wr = 0x0700;
	BYTE bt = 0x00;

	switch(wCommand)
	{
	case MSG_KEY_DOWN:
		bt = (BYTE)(dwParam);
		if(VK_RETURN == bt)
		{
			if(BufferPtr)
			{
				DoCommand();
			}
			PrintPrompt();
			break;
		}
		if(VK_BACKSPACE == bt)
		{
			if(0 != BufferPtr)
			{
				GotoPrev();
				BufferPtr --;
			}
			break;
		}
		else
		{
			if(MAX_BUFFER_LEN - 1 > BufferPtr)
			{
				CmdBuffer[BufferPtr] = bt;
				BufferPtr ++;
				wr += (BYTE)(dwParam);
				PrintCh(wr);
			}
		}
		break;
	case KERNEL_MESSAGE_TIMER:
	default:
		break;
	}
	return 0;
}

//
//Entry point of the text mode shell.
//
DWORD ShellEntryPoint(LPVOID pData)
{
	__KERNEL_THREAD_MESSAGE KernelThreadMessage;

	//Print out version and author information.
	//GotoHome();
	//ChangeLine();
	//PrintStr(VERSION_INFO);
	GotoHome();
	ChangeLine();

	PrintPrompt();
	while(TRUE)
	{
		if(GetMessage(&KernelThreadMessage))
		{
			if(KERNEL_MESSAGE_TERMINAL == KernelThreadMessage.wCommand)
			{
				break;
			}
			DispatchMessage(&KernelThreadMessage,EventHandler);
		}
	}

	//When reach here,it means the shell thread will terminate.We will reboot
	//the system in current version's implementation,since there is no interact
	//mechanism between user and computer in case of no shell.
	//NOTE:System clean up operations should be put here if necessary.
#ifdef __I386__
	BIOSReboot();
#endif
	return 0;
}
