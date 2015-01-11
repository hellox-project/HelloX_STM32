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

//*******************
#include "..\include\debug.h"

#if defined(__I386__)
#ifndef __BIOS_H__
#include "..\arch\x86\BIOS.H"
#endif
#endif

#include "..\INCLUDE\MODMGR.H"
#include "..\include\console.h"
#include "..\lib\stdio.h"
#include "..\lib\string.h"

//*******************
#include "../include/debug.h"
//*******************

#define  DEF_PROMPT_STR   "[system-view]"
#define  ERROR_STR        "You entered incorrect command name."

//shell input pos
#define  SHELL_INPUT_START_X       (strlen(s_szPrompt))   // 
#define  SHELL_INPUT_START_Y       1 
#define  SHELL_INPUT_START_Y_FIRST 4 

//Host name array of the system.
#define MAX_HOSTNAME_LEN     16
CHAR    s_szPrompt[64]      = {0};

//Shell thread's handle.
__KERNEL_THREAD_OBJECT*  g_lpShellThread   = NULL;
//static HISOBJ            s_hHiscmdInoObj   = NULL;





//The following handlers are moved to shell1.cpp.
extern DWORD VerHandler(__CMD_PARA_OBJ* pCmdParaObj);          //Handles the version command.
extern DWORD MemHandler(__CMD_PARA_OBJ* pCmdParaObj);          //Handles the memory command.
extern DWORD SysInfoHandler(__CMD_PARA_OBJ* pCmdParaObj);      //Handles the sysinfo command.
extern DWORD HlpHandler(__CMD_PARA_OBJ* pCmdParaObj);
extern DWORD LoadappHandler(__CMD_PARA_OBJ* pCmdParaObj);
extern DWORD GUIHandler(__CMD_PARA_OBJ* pCmdParaObj);          //Handler for GUI command,resides in
extern DWORD FileWriteTest(__CMD_PARA_OBJ* pCmdParaObj); 
//shell2.cpp file.

static DWORD CpuHandler(__CMD_PARA_OBJ* pCmdParaObj);
static DWORD SptHandler(__CMD_PARA_OBJ* pCmdParaObj);
static DWORD ClsHandler(__CMD_PARA_OBJ* pCmdParaObj);
static DWORD RunTimeHandler(__CMD_PARA_OBJ* pCmdParaObj);
static DWORD SysNameHandler(__CMD_PARA_OBJ* pCmdParaObj);
static DWORD IoCtrlApp(__CMD_PARA_OBJ* pCmdParaObj);
static DWORD SysDiagApp(__CMD_PARA_OBJ* pCmdParaObj);
static DWORD Reboot(__CMD_PARA_OBJ* pCmdParaObj);
static DWORD Poweroff(__CMD_PARA_OBJ* pCmdParaObj);
static DWORD ComDebug(__CMD_PARA_OBJ* pCmdParaObj);
static DWORD DebugHandler(__CMD_PARA_OBJ* pCmdParaObj);

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
	{"debug"    ,    DebugHandler},
//	{"test"    ,    FileWriteTest},
	//The last element of this array must be NULL.
	{NULL       ,    NULL}
};

//Com interface debugging application.
static DWORD ComDebug(__CMD_PARA_OBJ* pCmdParaObj)
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

	return SHELL_CMD_PARSER_SUCCESS;
}

//
//sysname handler.
//This handler changes the system name,and save it to system config database.
//
static DWORD SaveSysName(__CMD_PARA_OBJ* pCmdParaObj)
{
	return SHELL_CMD_PARSER_SUCCESS;
}

static DWORD SysNameHandler(__CMD_PARA_OBJ* pCmdParaObj)
{
	//__CMD_PARA_OBJ*    pCmdObj = NULL;
	//pCmdParaObj = FormParameterObj(pszSysName);
	if(NULL == pCmdParaObj)
	{
		PrintLine("Not enough system resource to interpret the command.");
		goto __TERMINAL;
	}
	if((0 == pCmdParaObj->byParameterNum) || (0 == pCmdParaObj->Parameter[0][0]))
	{
		PrintLine("Invalid command parameter.");
		goto __TERMINAL;
	}

	if(StrLen(pCmdParaObj->Parameter[0]) >= MAX_HOSTNAME_LEN)
	{
		PrintLine("System name must not exceed 16 bytes.");
		goto __TERMINAL;
	}

	//SaveSysName(pCmdParaObj->Parameter[0]);
	//StrCpy(pCmdObj->Parameter[0],&s_szPrompt[0]);
	_hx_sprintf(s_szPrompt,"[%s]",pCmdParaObj->Parameter[0]);
	if(StrLen(s_szPrompt) <= 0)
	{
		StrCpy(DEF_PROMPT_STR,&s_szPrompt[0]);
	}

__TERMINAL:

	return SHELL_CMD_PARSER_SUCCESS;
}

//Handler for 'ioctrl' command.
DWORD IoCtrlApp(__CMD_PARA_OBJ* pCmdParaObj)
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
		return SHELL_CMD_PARSER_SUCCESS;
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

	return SHELL_CMD_PARSER_SUCCESS;

}

//
//System diag application's shell start code.
//
DWORD SysDiagApp(__CMD_PARA_OBJ* pCmdParaObj)
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
		return SHELL_CMD_PARSER_SUCCESS;
	}

	DeviceInputManager.SetFocusThread((__COMMON_OBJECT*)&DeviceInputManager,
		(__COMMON_OBJECT*)lpSysDiagThread);

	lpSysDiagThread->WaitForThisObject((__COMMON_OBJECT*)lpSysDiagThread);
	KernelThreadManager.DestroyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
		(__COMMON_OBJECT*)lpSysDiagThread);  //Destroy the kernel thread object.

	return SHELL_CMD_PARSER_SUCCESS;
}

//Entry point of reboot.
DWORD Reboot(__CMD_PARA_OBJ* pCmdParaObj)
{
	ClsHandler(NULL); //Clear screen first.
#ifdef __I386__
	BIOSReboot();
#endif
	return SHELL_CMD_PARSER_SUCCESS;
}

//Entry point of poweroff.
DWORD Poweroff(__CMD_PARA_OBJ* pCmdParaObj)
{
#ifdef __I386__
	BIOSPoweroff();
#endif
	return SHELL_CMD_PARSER_SUCCESS;
}

//Handler for 'runtime' command.
DWORD RunTimeHandler(__CMD_PARA_OBJ* pCmdParaObj)
{
	CHAR  Buffer[190] = {0};
	DWORD week = 0,day = 0,hour = 0,minute = 0,second = 0;
	DWORD* Array[5] = {0};

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

	Array[0] = &week;
	Array[1] = &day;
	Array[2] = &hour;
	Array[3] = &minute;
	Array[4] = &second;	
	FormString(Buffer,"System has running %d week(s), %d day(s), %d hour(s), %d minute(s), %d second(s).",(LPVOID*)Array);

	//Show out the result.
	/*sprintf(Buffer,"System has running %d week(s), %d day(s), %d hour(s), %d minute(s), %d second(s).",
		week,day,hour,minute,second);*/
	//sprintf(Buffer,"System has running %d,%d",(INT)week,(INT)day);//(INT)hour,(INT)minute,(INT)second
	//PrintLine(Buffer);
	CD_PrintString(Buffer,TRUE);

	return SHELL_CMD_PARSER_SUCCESS;
}

//Handler for 'cls' command.
DWORD ClsHandler(__CMD_PARA_OBJ* pCmdParaObj)
{
	ClearScreen();

	CD_SetCursorPos(0,SHELL_INPUT_START_Y);

	return SHELL_CMD_PARSER_SUCCESS;
}

//Handler for 'cpu' command.
DWORD CpuHandler(__CMD_PARA_OBJ* pCmdParaObj)
{
	/*GotoHome();
	ChangeLine();
	PrintStr("Cpu Handler called.");*/

	CD_PrintString("Cpu Handler called.",TRUE);

	return SHELL_CMD_PARSER_SUCCESS;
}

//Handler for 'support' command.
DWORD SptHandler(__CMD_PARA_OBJ* pCmdParaObj)
{
	LPSTR strSupportInfo1 = "    For any technical support,send E-Mail to:";
	LPSTR strSupportInfo2 = "    garryxin@yahoo.com.cn.";
	LPSTR strSupportInfo3 = "    or join the QQ group : 38467832";

	PrintLine(strSupportInfo1);
	PrintLine(strSupportInfo2);
	PrintLine(strSupportInfo3);

	return SHELL_CMD_PARSER_SUCCESS;
}

//*********************************
// For log service
//Author :	Erwin
//Email  :	erwin.wang@qq.com
//Date	 :  9th June, 2014
//********************************
DWORD DebugHandler(__CMD_PARA_OBJ* pCmdParaObj)
{
	/*char buf[256] = {'0'};
	int count = 0;
	while(TRUE)
	{
		count++;
		Sleep(1000);
		DebugManager.Logcat(&DebugManager, buf, 0);
		if(buf[0] != '0')
		{
			PrintLine(buf);
		}
		if(count == 10)break;
	}*/
	return SHELL_CMD_PARSER_SUCCESS;
}

static DWORD QueryCmdName(LPSTR pMatchBuf,INT nBufLen)
{
	static DWORD dwIndex   = 0;
	static DWORD dwExIndex = 0;

	if(pMatchBuf == NULL)
	{
		dwIndex    = 0;
		dwExIndex  = 0;
		return SHELL_QUERY_CONTINUE;
	}

	if(NULL != CmdObj[dwIndex].CmdStr)
	{
		strncpy(pMatchBuf,CmdObj[dwIndex].CmdStr,nBufLen);
		dwIndex ++;

		return SHELL_QUERY_CONTINUE;
	}

	if(NULL != ExtCmdArray[dwExIndex].lpszCmdName)
	{
		strncpy(pMatchBuf,ExtCmdArray[dwExIndex].lpszCmdName,nBufLen);
		dwExIndex ++;

		return SHELL_QUERY_CONTINUE;
	}

	dwIndex    = 0;
	dwExIndex  = 0;

	return SHELL_QUERY_CANCEL;	
}
//Command analyzing routine,it analyzes user's input and search
//command array to find a proper handler,then call it.
//Default handler will be called if no proper command handler is
//located.
static DWORD  CommandParser(LPCSTR pCmdBuf)
{
	__KERNEL_THREAD_OBJECT* hKernelThread = NULL;
	__CMD_PARA_OBJ*         lpCmdParamObj = NULL;
	DWORD   dwResult                      = SHELL_CMD_PARSER_INVALID;        //If find the correct command object,then
	DWORD   dwIndex                       = 0;          //Used for 'for' loop.
	

	lpCmdParamObj = FormParameterObj(pCmdBuf);
	if(NULL == lpCmdParamObj || lpCmdParamObj->byParameterNum < 1)    //Can not form a valid command parameter object.
	{
		CD_PrintString(pCmdBuf,TRUE);
		goto __END;
	}
	
	dwIndex = 0;
	while(CmdObj[dwIndex].CmdStr)
	{
		if(StrCmp(CmdObj[dwIndex].CmdStr,lpCmdParamObj->Parameter[0]))
		{			
			CmdObj[dwIndex].CmdHandler(lpCmdParamObj);  //Call the command handler.
			dwResult = SHELL_CMD_PARSER_SUCCESS;
			break;
		}
		dwIndex ++;
	}

	if(dwResult == SHELL_CMD_PARSER_SUCCESS)
	{
		goto __END;
	}
	
	dwIndex = 0;  //Now,should search external command array.
	while(ExtCmdArray[dwIndex].lpszCmdName)
	{		
		if(StrCmp(ExtCmdArray[dwIndex].lpszCmdName,lpCmdParamObj->Parameter[0]))  //Found.
		{	
			hKernelThread = KernelThreadManager.CreateKernelThread(
				(__COMMON_OBJECT*)&KernelThreadManager,
				0,
				KERNEL_THREAD_STATUS_READY,
				PRIORITY_LEVEL_NORMAL,
				ExtCmdArray[dwIndex].ExtCmdHandler,
				(LPVOID)lpCmdParamObj, //?
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
				DeviceInputManager.SetFocusThread((__COMMON_OBJECT*)&DeviceInputManager,NULL);
			}
			
			dwResult = SHELL_CMD_PARSER_SUCCESS;
			goto __END;
		}
		dwIndex ++;
	}
	
	//DefaultHandler(NULL); //Call the default command handler.	
__END:

	if(NULL != lpCmdParamObj)
	{
		ReleaseParameterObj(lpCmdParamObj);
	}
	
	return dwResult;		
}

//Entry point of the text mode shell.
//
DWORD ShellEntryPoint(LPVOID pData)
{			
	StrCpy(DEF_PROMPT_STR,&s_szPrompt[0]);

	CD_PrintString(VERSION_INFO,TRUE);
	CD_SetCursorPos(0,SHELL_INPUT_START_Y_FIRST);
	
	Shell_Msg_Loop(s_szPrompt,CommandParser,QueryCmdName);

	//When reach here,it means the shell thread will terminate.We will reboot
	//the system in current version's implementation,since there is no interact
	//mechanism between user and computer in case of no shell.
	//NOTE:System clean up operations should be put here if necessary.
#ifdef __I386__
	BIOSReboot();
#endif

	return 0;
}
