//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep,18 2006
//    Module Name               : ARCH_X86.CPP
//    Module Funciton           : 
//                                This module countains CPU specific code,in this file,
//                                Intel X86 series CPU's specific code is included.
//
//    Last modified Author      :
//    Last modified Date        : 29 JAN,2009
//    Last modified Content     :
//                                1. __inb,__inw,__ind,__inbs,__inws and __inds are added.
//                                2. __outb,__outw,__outd are added.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "..\..\INCLUDE\StdAfx.h"
#endif

#include "ARCH.H"

#ifdef __I386__  //Only available in x86 based PC platform.

//Architecture related initialization code,this routine will be called in the
//begining of system initialization.
//This routine must be in GLOBAL scope since it will be called by other routines.
BOOL HardwareInitialize()
{
	//x86 related hardware should be initialized here.Return FALSE if there is
	//error that can lead fault of system.
	return TRUE;
}

//
//This routine switches the current executing path to the new one identified
//by lpContext.
//
__declspec(naked) VOID __SwitchTo(__KERNEL_THREAD_CONTEXT* lpContext)
{
	__asm{
		push ebp
		mov ebp,esp
		mov esp,dword ptr [ebp + 0x08]  //Restore ESP.
		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx

		mov al,0x20  //Dismiss interrupt controller.
		out 0x20,al
		out 0xa0,al

		pop eax
		iretd
	}
}

//
//These three global variables are used as temp variables
//by __SaveAndSwitch routine.
//
static DWORD dwTmpEip = 0;
static DWORD dwTmpEax = 0;
static DWORD dwTmpEbp = 0;

//
//This routine saves current kernel thread's context,and switch
//to the new kernel thread.
//
__declspec(naked) VOID __SaveAndSwitch(__KERNEL_THREAD_CONTEXT** lppOldContext,
									   __KERNEL_THREAD_CONTEXT** lppNewContext)
{
	__asm{
		mov dwTmpEbp,esp
		pop dwTmpEip
		mov dwTmpEax,eax //Save EAX.
		pushfd           //Save EFlags.
		xor eax,eax
		mov ax,cs
		push eax         //Save CS.
		push dwTmpEip    //Save EIP.
		push dwTmpEax    //Save EAX.
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp

		//Now,we have build the target stack frame,then save it.
		mov ebp,dwTmpEbp
		mov ebx,dword ptr [ebp + 0x04]
		mov dword ptr [ebx],esp  //Save old stack frame.

		//Restore the new thread's context and switch to it.
		mov ebx,dword ptr [ebp + 0x08]
		mov esp,dword ptr [ebx]  //Restore new stack.
		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		pop eax
		iretd
	}
}

//
//Enable Virtual Memory Management mechanism.This routine will be called in
//process of OS initialization if __CFG_SYS_VMM flag is defined.
//
VOID EnableVMM()
{
	__asm{
		push eax
		mov eax,PD_START
		mov cr3,eax
		mov eax,cr0
		or eax,0x80000000
		mov cr0,eax
		pop eax
	}
}

//
//This routine initializes a kernel thread's context.
//This routine's action depends on different platform.
//
VOID InitKernelThreadContext(__KERNEL_THREAD_OBJECT* lpKernelThread,
							 __KERNEL_THREAD_WRAPPER lpStartAddr)
{
	DWORD*        lpStackPtr = NULL;
	DWORD         dwStackSize = 0;

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

	__PUSH(lpStackPtr,lpKernelThread);       //Push lpKernelThread to stack.
	__PUSH(lpStackPtr,NULL);                 //Push a new return address,simulate a call.
	__PUSH(lpStackPtr,INIT_EFLAGS_VALUE);    //Push EFlags.
	__PUSH(lpStackPtr,0x00000008);           //Push CS.
	__PUSH(lpStackPtr,lpStartAddr);  //Push start address.
	__PUSH(lpStackPtr,0);                   //Push eax.
	__PUSH(lpStackPtr,0);
	__PUSH(lpStackPtr,0);
	__PUSH(lpStackPtr,0);
	__PUSH(lpStackPtr,0);
	__PUSH(lpStackPtr,0);
	__PUSH(lpStackPtr,0);

	//Save context.
	lpKernelThread->lpKernelThreadContext = (__KERNEL_THREAD_CONTEXT*)lpStackPtr;
	return;
}

//Get time stamp counter.
VOID __GetTsc(__U64* lpResult)
{
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
}

#define CLOCK_PER_MICROSECOND 1024  //Assume the CPU's clock is 1G Hz.

VOID __MicroDelay(DWORD dwmSeconds)
{
	__U64    u64CurrTsc;
	__U64    u64TargetTsc;

	__GetTsc(&u64TargetTsc);
	u64CurrTsc.dwHighPart = 0;
	u64CurrTsc.dwLowPart  = dwmSeconds;
	//u64Mul(&u64CurrTsc,CLOCK_PER_MICROSECOND);
	u64RotateLeft(&u64CurrTsc,10);
	u64Add(&u64TargetTsc,&u64CurrTsc,&u64TargetTsc);
	while(TRUE)
	{
		__GetTsc(&u64CurrTsc);
		if(MoreThan(&u64CurrTsc,&u64TargetTsc))
		{
			break;
		}
	}
	return;
}

VOID __outd(WORD wPort,DWORD dwVal)  //Write one double word to a port.
{
	__asm{
		push eax
		push edx
		mov dx,wPort
		mov eax,dwVal
		out dx,eax
		pop edx
		pop eax
	}
}

DWORD __ind(WORD wPort)    //Read one double word from a port.
{
	DWORD    dwRet       = 0;
	__asm{
		push eax
		push edx
		mov dx,wPort
		in eax,dx
		mov dwRet,eax
		pop edx
		pop eax
	}
	return dwRet;
}

VOID __outb(UCHAR _bt,WORD _port)  //Send bt to port.
{
	__asm{
		push eax
		push edx
		mov al,_bt
		mov dx,_port
		out dx,al
		pop edx
		pop eax
	}
}

UCHAR __inb(WORD _port)  //Receive a byte from port.
{
	UCHAR uRet;
	__asm{
		xor eax,eax
		push edx
		mov dx,_port
		in al,dx
		pop edx
		mov uRet,al
	}
	return uRet;
}

WORD __inw(WORD wPort)
{
	WORD    wRet       = 0;
	__asm{
		push eax
		push edx
		mov dx,wPort
		in ax,dx
		mov wRet,ax
		pop edx
		pop eax
	}
	return wRet;
}

VOID __outw(WORD wVal,WORD wPort)
{
	__asm{
		push eax
		push edx
		mov ax,wVal
		mov dx,wPort
		out dx,ax
		pop edx
		pop eax
	}
}

__declspec(naked) VOID __inws(BYTE* pBuffer,DWORD dwBuffLen,WORD wPort)
{
#ifdef __I386__
	__asm{
		push ebp
		mov ebp,esp
		push ecx
		push edx
		push edi
		mov edi,dword ptr [ebp + 0x08]
		mov ecx,dword ptr [ebp + 0x0c]
		shr ecx,0x01
		mov dx,  word ptr [ebp + 0x10]
		cld
		rep insw
		pop edi
		pop edx
		pop ecx
		leave
		retn
	}
#else
#endif
}

__declspec(naked) VOID __outws(BYTE* pBuffer,DWORD dwBuffLen,WORD wPort)
{
#ifdef __I386__
	__asm{
		push ebp
		mov ebp,esp
		push ecx
		push edx
		push esi
		mov esi,dword ptr [ebp + 0x08]
		mov ecx,dword ptr [ebp + 0x0c]
		shr ecx,0x01
		mov dx,  word ptr [ebp + 0x10]
		rep outsw
		pop esi
		pop edx
		pop ecx
		leave
		retn
	}
#else
#endif
}

#endif //__I386__
