//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Dec 1,2013(It's 7 years from the first ARCH file was created for x86...)
//    Module Name               : ARCH_STM.CPP
//    Module Funciton           : 
//                                This module countains CPU and hardware platfrom specific code,this
//                                file is for STM32 series chipset and edited under MDK.
//                                This is a flagship for Hello China to migrate to other platforms than
//                                x86.
//
//    Last modified Author      :
//    Last modified Date        : 
//    Last modified Content     :
//                                
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "..\..\INCLUDE\StdAfx.h"
#endif

//Include STM32 chip headers if necessary.
#ifdef __STM32__
#include <stm32f10x.h>
#endif

#include "ARCH.H"

#ifdef __STM32__  //Only available under STM32 chipset.

//Hardware initialization code,low level hardware should be initialized in
//this routine.It will be called before OS initialization process.
BOOL HardwareInitialize()
{
	int hz = 1000 / SYSTEM_TIME_SLICE;
	//Initialize systick.
	SysTick_Config(72000000 / hz);
	return TRUE;
}

//
//Enable Virtual Memory Management mechanism.This routine will be called in
//process of OS initialization if __CFG_SYS_VMM flag is defined.
//
VOID EnableVMM()
{
}

//
//This routine initializes a kernel thread's context.
//This routine's action depends on different platform.
//
VOID InitKernelThreadContext(__KERNEL_THREAD_OBJECT* lpKernelThread,
							 __KERNEL_THREAD_WRAPPER lpStartAddr)
{
	DWORD*        lpStackPtr = NULL;

	if((NULL == lpKernelThread) || (NULL == lpStartAddr))  //Invalid parameters.
	{
		return;
	}

//Define a macro to make the code readable.
#define __PUSH(stackptr,val) \
	do{  \
	(DWORD*)(stackptr) --; \
	*((DWORD*)stackptr) = (DWORD)(val); \
	}while(0)

	lpStackPtr = (DWORD*)lpKernelThread->lpInitStackPointer;

  //Push registers of the thread into stack,to simulate a interrupt stack frame.
	__PUSH(lpStackPtr,0x01000000);          //xPSR.
	__PUSH(lpStackPtr,(DWORD)lpStartAddr);  //PC.
	__PUSH(lpStackPtr,0xFFFFFFFF);          //LR.
	__PUSH(lpStackPtr,0x12121212);          //R12.
	__PUSH(lpStackPtr,0x03030303);          //R3.
	__PUSH(lpStackPtr,0x02020202);          //R2.
	__PUSH(lpStackPtr,0x01010101);          //R1.
	__PUSH(lpStackPtr,lpKernelThread);      //R0,should be the thread's handle,very important.
	
	__PUSH(lpStackPtr,0x11111111);          //R11.
	__PUSH(lpStackPtr,0x10101010);          //R10.
	__PUSH(lpStackPtr,0x09090909);          //R9.
	__PUSH(lpStackPtr,0x08080808);          //R8.
	__PUSH(lpStackPtr,0x07070707);          //R7.
	__PUSH(lpStackPtr,0x06060606);          //R6.
	__PUSH(lpStackPtr,0x05050505);          //R5.
	__PUSH(lpStackPtr,0x04040404);          //R4.

	//Save context.
	lpKernelThread->lpKernelThreadContext = (__KERNEL_THREAD_CONTEXT*)lpStackPtr;
	return;
}

//Get system TimeStamp counter.
VOID __GetTsc(__U64* lpResult)
{
#ifdef __I386__
	__asm{
		push eax
		push edx
		push ebx
		rdtsc    //Read time stamp counter.
		mov ebx,dword ptr [ebp + 0x08]
		mov dword ptr [ebx],eax
		mov dword ptr [ebx + 0x04],edx
		pop ebx
		pop edx
		pop eax
	}
#else
#endif
}


#define CLOCK_PER_MICROSECOND 1024  //Assume the CPU's clock is 1G Hz.

VOID __MicroDelay(DWORD dwmSeconds)
{
	return;
}

VOID __outd(WORD wPort,DWORD dwVal)  //Write one double word to a port.
{
}

DWORD __ind(WORD wPort)    //Read one double word from a port.
{
	return 0;
}

VOID __outb(UCHAR _bt,WORD _port)  //Send bt to port.
{
}

UCHAR __inb(WORD _port)  //Receive a byte from port.
{
	return 0;
}

WORD __inw(WORD wPort)
{
	return 0;
}

VOID __outw(WORD wVal,WORD wPort)
{
}

VOID __inws(BYTE* pBuffer,DWORD dwBuffLen,WORD wPort)
{
}

VOID __outws(BYTE* pBuffer,DWORD dwBuffLen,WORD wPort)
{
}
#endif //__STM32__
