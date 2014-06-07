//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 11,2011
//    Module Name               : shell1.cpp
//    Module Funciton           : 
//                                This module countains shell procedures.
//                                Some functions in shell.cpp originally are moved to this file to reduce the
//                                shell.cpp's size.
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
//#include "stat_s.h"

#include "..\lib\string.h"
#include "..\lib\stdio.h"

//Handler of version command.
VOID VerHandler(LPSTR pstr)
{
	GotoHome();
	ChangeLine();
	PrintStr(VERSION_INFO);
	GotoHome();
	ChangeLine();
	PrintStr(SLOGAN_INFO);
}

//Handler for memory,this routine print out the memory layout and memory usage status.
VOID MemHandler(LPSTR pstr)
{
	CHAR   buff[256];
	DWORD  dwFlags;

	DWORD  dwPoolSize;
	DWORD  dwFreeSize;
	DWORD  dwFreeBlocks;
	DWORD  dwAllocTimesSuccL;
	DWORD  dwAllocTimesSuccH;
	DWORD  dwAllocTimesL;
	DWORD  dwAllocTimesH;
	DWORD  dwFreeTimesL;
	DWORD  dwFreeTimesH;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	dwPoolSize      = AnySizeBuffer.dwPoolSize;
	dwFreeSize      = AnySizeBuffer.dwFreeSize;
	dwFreeBlocks    = AnySizeBuffer.dwFreeBlocks;
	dwAllocTimesSuccL  = AnySizeBuffer.dwAllocTimesSuccL;
	dwAllocTimesSuccH  = AnySizeBuffer.dwAllocTimesSuccH;
	dwAllocTimesL      = AnySizeBuffer.dwAllocTimesL;
	dwAllocTimesH      = AnySizeBuffer.dwAllocTimesH;
	dwFreeTimesL       = AnySizeBuffer.dwFreeTimesL;
	dwFreeTimesH       = AnySizeBuffer.dwFreeTimesH;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	PrintLine("    Free block list algorithm is adopted:");
	//Get and dump out memory usage status.
	sprintf(buff,"    Total memory size     : %d(0x%X)",dwPoolSize,dwPoolSize);
	PrintLine(buff);
	sprintf(buff,"    Free memory size      : %d(0x%X)",dwFreeSize,dwFreeSize);
	PrintLine(buff);
	sprintf(buff,"    Free memory blocks    : %d",dwFreeBlocks);
	PrintLine(buff);
	sprintf(buff,"    Alloc success times   : %d/%d",dwAllocTimesSuccH,dwAllocTimesSuccL);
	PrintLine(buff);
	sprintf(buff,"    Alloc operation times : %d/%d",dwAllocTimesH,dwAllocTimesL);
	PrintLine(buff);
	sprintf(buff,"    Free operation times  : %d/%d",dwFreeTimesH,dwFreeTimesL);
	PrintLine(buff);
}

//Local variables for sysinfo command.
LPSTR strHdr[] = {               //I have put the defination of this strings
	                             //in the function SysInfoHandler,but it do
	                             //not work,see the asm code,it generates the
	                             //incorrect asm code!Fuck Bill Gates!.
	"    EDI   :   0x",
	"    ESI   :   0x",
	"    EBP   :   0x",
	"    ESP   :   0x",
	"    EBX   :   0x",
	"    EDX   :   0x",
	"    ECX   :   0x",
	"    EAX   :   0x",
	"    CS-DS :   0x",
	"    FS-GS :   0x",
	"    ES-SS :   0x"};

#ifdef __I386__
static CHAR Buffer[] = {"Hello,China!"};
#endif

//Handler for sysinfo command.
VOID SysInfoHandler(LPSTR pstr)
{
#ifdef __I386__
	DWORD sysContext[11];
	DWORD bt;

	__asm{                       //Get the system information.
		pushad                   //Save all the general registers.
			                     //NOTICE: This operation only get
								 //the current status of system
								 //where this instruction is executed.
        push eax
        mov eax,dword ptr [esp + 0x04]
		mov dword ptr [ebp - 0x2c],eax    //Get the eax register's value.
		                                  //Fuck Bill Gates!!!!!
		mov eax,dword ptr [esp + 0x08]
		mov dword ptr [ebp - 0x28],eax    //Get the ecx value.
		mov eax,dword ptr [esp + 0x0c]
		mov dword ptr [ebp - 0x24],eax    //edx
		mov eax,dword ptr [esp + 0x10]
		mov dword ptr [ebp - 0x20],eax    //ebx
		mov eax,dword ptr [esp + 0x14]
		mov dword ptr [ebp - 0x1c],eax    //esp
		mov eax,dword ptr [esp + 0x18]
		mov dword ptr [ebp - 0x18],eax    //ebp
		mov eax,dword ptr [esp + 0x1c]
		mov dword ptr [ebp - 0x14],eax    //esi
		mov eax,dword ptr [esp + 0x20]
		mov dword ptr [ebp - 0x10],eax    //edi

		mov ax,cs
		shl eax,0x10
		mov ax,ds
		mov dword ptr [ebp - 0x0c],eax    //Get cs : ds.
		mov ax,fs
		shl eax,0x10
		mov ax,gs
		mov dword ptr [ebp - 0x08],eax    //Get fs : gs.
		mov ax,es
		shl eax,0x10
		mov ax,ss
		mov dword ptr [ebp - 0x04],eax   //Get es : ss.

		pop eax
		popad                    //Restore the stack frame.
	}

	//All system registers are got,then print out them.
	GotoHome();
	ChangeLine();
	PrintStr("    System context information(general registers and segment registers):");
	for(bt = 0;bt < 11;bt ++)
	{
		GotoHome();
		ChangeLine();
		PrintStr(strHdr[bt]);
		Hex2Str(sysContext[bt],Buffer);
		//Buffer[8] = 0x00;
		PrintStr(Buffer);
	}
	return;
#else   //Only x86 platform is supported yet.
	GotoHome();
	ChangeLine();
	PrintStr("    This operation can not supported on no-I386 platform.");
	return;
#endif
}

//Handler for help command.
VOID HlpHandler(LPSTR pstr)           //Command 'help' 's handler.
{
	LPSTR strHelpTitle   = "    The following commands are available currently:";
	LPSTR strHelpVer     = "    version      : Print out the version information.";
	LPSTR strHelpMem     = "    memory       : Print out current version's memory layout.";
	LPSTR strHelpSysInfo = "    sysinfo      : Print out the system context.";
	LPSTR strSysName     = "    sysname      : Change the system host name.";
	LPSTR strHelpHelp    = "    help         : Print out this screen.";
	LPSTR strSupport     = "    support      : Print out technical support information.";
	LPSTR strRunTime     = "    runtime      : Display the total run time since last reboot.";
	LPSTR strIoCtrlApp   = "    ioctrl       : Start IO control application.";
	LPSTR strSysDiagApp  = "    sysdiag      : System or hardware diag application.";
	LPSTR strFsApp       = "    fs           : File system operating application.";
	LPSTR strFdiskApp    = "    fdisk        : Hard disk operating application.";
	LPSTR strLoadappApp  = "    loadapp      : Load application module and execute it.";
	LPSTR strGUIApp      = "    gui          : Load GUI module and enter GUI mode.";
	LPSTR strReboot      = "    reboot       : Reboot the system.";
	LPSTR strCls         = "    cls          : Clear the whole screen.";

	PrintLine(strHelpTitle);              //Print out the help information line by line.
	PrintLine(strHelpVer);
	PrintLine(strHelpMem);
	PrintLine(strHelpSysInfo);
	PrintLine(strSysName);
	PrintLine(strHelpHelp);
	PrintLine(strSupport);
	PrintLine(strRunTime);
	PrintLine(strIoCtrlApp);
	PrintLine(strSysDiagApp);
	PrintLine(strFsApp);
	PrintLine(strFdiskApp);
	PrintLine(strLoadappApp);
	PrintLine(strGUIApp);
	PrintLine(strReboot);
	PrintLine(strCls);
}

//A helper routine used to load the specified binary application module into memory.
//  @hBinFile       : The handle of the module file;
//  @dwStartAddress : Load address.
//
static BOOL LoadBinModule(HANDLE hBinFile,DWORD dwStartAddress)
{
	BYTE*   pBuffer = (BYTE*)dwStartAddress;
	BYTE*   pTmpBuff = NULL;
	DWORD   dwReadSize = 0;
	BOOL    bResult    = FALSE;

	//Parameter check.
	if(dwStartAddress <= 0x00100000)  //End 1M space is reserved.
	{
		goto __TERMINAL;
	}

	//Allocate a temporary buffer to hold file content.
	pTmpBuff = (BYTE*)KMemAlloc(4096,KMEM_SIZE_TYPE_ANY);
	if(NULL == pTmpBuff)
	{
		goto __TERMINAL;
	}
	//Try to read the first 4K bytes from file.
	if(!ReadFile(hBinFile,
		4096,
		pTmpBuff,
		&dwReadSize))
	{
		goto __TERMINAL;
	}
	if(dwReadSize <= 4)  //Too short,invalid format.
	{
		goto __TERMINAL;
	}
	//Verify the validation of the bin file format.
	if(0xE9909090 != *(DWORD*)pTmpBuff)  //Invalid binary file format.
	{
		goto __TERMINAL;
	}
	//Format is ok,try to load it.
	memcpy(pBuffer,pTmpBuff,dwReadSize);  //Copy the first block into target.
	pBuffer += dwReadSize;     //Adjust the target pointer.
	while(dwReadSize == 4096)  //File size larger than 4k,continue to load it.
	{
		if(!ReadFile(hBinFile,
			4096,
			pBuffer,
			&dwReadSize))
		{
			goto __TERMINAL;
		}
		pBuffer += dwReadSize;  //Move target pointer.
	}
	bResult = TRUE;

__TERMINAL:
	if(NULL != pTmpBuff)  //Should release the memory.
	{
		KMemFree(pTmpBuff,KMEM_SIZE_TYPE_ANY,0);
	}
	return bResult;
}

//A helper routine used to launch the loaded binary module.
static BOOL ExecuteBinModule(DWORD dwStartAddress,LPVOID pParams)
{
	__KERNEL_THREAD_OBJECT*   hKernelThread  = NULL;
	BOOL                      bResult        = FALSE;

	if(dwStartAddress <= 0x00100000) //Low end 1M memory is reserved.
	{
		goto __TERMINAL;
	}
	//Create a kernel thread to run the binary module.
	hKernelThread = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		(__KERNEL_THREAD_ROUTINE)dwStartAddress,
		pParams,
		NULL,
		NULL);
	if(NULL == hKernelThread)  //Can not create the thread.
	{
		goto __TERMINAL;
	}
	//Switch input focus to the thread.
	DeviceInputManager.SetFocusThread(
		(__COMMON_OBJECT*)&DeviceInputManager,
		(__COMMON_OBJECT*)hKernelThread);
	hKernelThread->WaitForThisObject((__COMMON_OBJECT*)hKernelThread);  //Block shell to wait module over.
	//Destroy the module's kernel thread.
	KernelThreadManager.DestroyKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		(__COMMON_OBJECT*)hKernelThread);
	//Switch back input focus to shell.
	DeviceInputManager.SetFocusThread(
		(__COMMON_OBJECT*)&DeviceInputManager,
		NULL);
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

//Handler for loadapp command.
VOID LoadappHandler(LPSTR strCmdParam)
{
	__CMD_PARA_OBJ*    pCmdObj = NULL;
	HANDLE             hBinFile  = NULL;
	CHAR               FullPathName[64];  //Full name of binary file.
	DWORD              dwStartAddr;       //Load address of the module.

	pCmdObj = FormParameterObj(strCmdParam);
	if(NULL == pCmdObj)
	{
		PrintLine("Not enough system resource to interpret the command.");
		goto __TERMINAL;
	}
	if(pCmdObj->byParameterNum < 2)
	{
		PrintLine("Please specify both app module name and load address.");
		goto __TERMINAL;
	}
	if((0 == pCmdObj->Parameter[0][0]) || (0 == pCmdObj->Parameter[1][0]))
	{
		PrintLine("Invalid parameter(s).");
		goto __TERMINAL;
	}
	//Construct the full path and name.
	strcpy(FullPathName,"C:\\PTHOUSE\\");
	strcat(FullPathName,pCmdObj->Parameter[0]);
	if(!Str2Hex(pCmdObj->Parameter[1],&dwStartAddr))
	{
		PrintLine("Invalid load address.");
		goto __TERMINAL;
	}
	//Try to open the binary file.
	hBinFile = CreateFile(
		FullPathName,
		FILE_ACCESS_READ,
		0,
		NULL);
	if(NULL == hBinFile)
	{
		PrintLine("Can not open the specified file in OS root directory.");
		goto __TERMINAL;
	}
	//Try to load and execute it.
	if(!LoadBinModule(hBinFile,dwStartAddr))
	{
		PrintLine("Can not load the specified binary file.");
		goto __TERMINAL;
	}
	if(!ExecuteBinModule(dwStartAddr,NULL))
	{
		PrintLine("Can not execute the binary module.");
		goto __TERMINAL;
	}
__TERMINAL:
	if(NULL != hBinFile)  //Destroy it.
	{
		CloseFile(hBinFile);
	}
	if(NULL != pCmdObj)
		KMemFree((LPVOID)pCmdObj,KMEM_SIZE_TYPE_4K,4096);
	return;
}

//Handler for GUI command,it only call LoadappHandler by given
//the GUI module's name and it's start address after loaded into
//memory.
VOID GUIHandler(LPSTR pstr)
{
	LoadappHandler("hcngui.bin 160000");  //hcngui.bin is the GUI module's name,it's
	                                      //start address after loaded into memory
	                                      //is 0x160000.
}
