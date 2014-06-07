//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Feb,25 2005
//    Module Name               : dim.cpp
//    Module Funciton           : 
//                                This module countains the implementation code of Device
//                                Input Manager.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "..\INCLUDE\StdAfx.h"
#endif

//
//The implementation of Initialize routine.
//

static BOOL DimInitialize(__COMMON_OBJECT* lpThis,
						  __COMMON_OBJECT* lpFocusThread,
						  __COMMON_OBJECT* lpShellThread)
{
	__DEVICE_INPUT_MANAGER*    lpInputMgr = NULL;

	if(NULL == lpThis)    //Parameter check.
	{
		return FALSE;
	}

	lpInputMgr = (__DEVICE_INPUT_MANAGER*)lpThis;

	lpInputMgr->lpFocusKernelThread = (__KERNEL_THREAD_OBJECT*)lpFocusThread;
	lpInputMgr->lpShellKernelThread = (__KERNEL_THREAD_OBJECT*)lpShellThread;

	return TRUE;
}


//
//The implementation of SendDeviceMessage.
//

static DWORD SendDeviceMessage(__COMMON_OBJECT*    lpThis,
							   __DEVICE_MESSAGE*   lpDevMsg,
							   __COMMON_OBJECT*    lpTarget)
{
	__DEVICE_INPUT_MANAGER*    lpInputMgr     = NULL;
	//__KERNEL_THREAD_OBJECT*    lpTargetThread = NULL;
	__KERNEL_THREAD_MESSAGE*   lpThreadMsg    = NULL;
	DWORD                      dwFlags        = 0;

	if((NULL == lpThis) || (NULL == lpDevMsg))    //Parameter check.
	{
		return DEVICE_MANAGER_FAILED;
	}

	lpInputMgr   = (__DEVICE_INPUT_MANAGER*)lpThis;
	lpThreadMsg  = (__KERNEL_THREAD_MESSAGE*)lpDevMsg;

	if(lpTarget != NULL)
	{
		KernelThreadManager.SendMessage(lpTarget,lpThreadMsg);
		return DEVICE_MANAGER_SUCCESS;
	}

	if(lpInputMgr->lpFocusKernelThread != NULL)
	{
		if(KERNEL_THREAD_STATUS_TERMINAL == lpInputMgr->lpFocusKernelThread->dwThreadStatus)    
			//The current focus
			//kernel thread is
			//terminal,so we must
			//reset the current
			//focus kernel thread
			//pointer,and send the
			//device message to shell
			//kernel thread.
		{
			//ENTER_CRITICAL_SECTION();
			__ENTER_CRITICAL_SECTION(NULL,dwFlags);
			lpInputMgr->lpFocusKernelThread = NULL;
			//LEAVE_CRITICAL_SECTION();
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

			if(NULL != lpInputMgr->lpShellKernelThread)
			{
				KernelThreadManager.SendMessage((__COMMON_OBJECT*)(lpInputMgr->lpShellKernelThread),
					lpThreadMsg);
				return DEVICE_MANAGER_SUCCESS;
			}
			else    //The current shell kernel thread is not exists.
			{
				return DEVICE_MANAGER_NO_SHELL_THREAD;
			}
		}           //The current status of the focus kernel thread is not TERMINAL.
		else
		{
			KernelThreadManager.SendMessage((__COMMON_OBJECT*)lpInputMgr->lpFocusKernelThread,
				lpThreadMsg);
			return DEVICE_MANAGER_SUCCESS;
		}
	}
	else            //The current focus kernel thread is not exists.
	{
		if(NULL != lpInputMgr->lpShellKernelThread)
		{
			KernelThreadManager.SendMessage((__COMMON_OBJECT*)lpInputMgr->lpShellKernelThread,
				lpThreadMsg);
			return DEVICE_MANAGER_SUCCESS;
		}
		else
		{
			return DEVICE_MANAGER_NO_SHELL_THREAD;
		}
	}
}

//
//The implementation of SetFocusThread routine.
//

static __COMMON_OBJECT* SetFocusThread(__COMMON_OBJECT*  lpThis,
									   __COMMON_OBJECT*  lpFocusThread)
{
	__DEVICE_INPUT_MANAGER*    lpInputMgr = NULL;
	__KERNEL_THREAD_OBJECT*    lpRetVal   = NULL;
	DWORD                      dwFlags    = 0;
	
	if(NULL == lpThis)    //Parameter check.
	{
		return (__COMMON_OBJECT*)lpRetVal;
	}
	lpInputMgr = (__DEVICE_INPUT_MANAGER*)lpThis;

	//ENTER_CRITICAL_SECTION();
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpRetVal = lpInputMgr->lpFocusKernelThread;
	lpInputMgr->lpFocusKernelThread = (__KERNEL_THREAD_OBJECT*)lpFocusThread;
	//LEAVE_CRITICAL_SECTION();
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	return (__COMMON_OBJECT*)lpRetVal;
}

//
//The implementation of SetShellThread routine.
//

static __COMMON_OBJECT* SetShellThread(__COMMON_OBJECT*  lpThis,
									   __COMMON_OBJECT*  lpShellThread)
{
	__DEVICE_INPUT_MANAGER*    lpInputMgr = NULL;
	__KERNEL_THREAD_OBJECT*    lpRetVal   = NULL;
	DWORD                      dwFlags    = 0;
	
	if(NULL == lpThis)    //Parameter check.
		return (__COMMON_OBJECT*)lpRetVal;

	lpInputMgr = (__DEVICE_INPUT_MANAGER*)lpThis;

	lpRetVal = lpInputMgr->lpShellKernelThread;

	//ENTER_CRITICAL_SECTION();
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpInputMgr->lpShellKernelThread = (__KERNEL_THREAD_OBJECT*)lpShellThread;
	//LEAVE_CRITICAL_SECTION();
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	return (__COMMON_OBJECT*)lpRetVal;
}


/************************************************************************
*************************************************************************
*************************************************************************
*************************************************************************
************************************************************************/

//
//The definition of Global Object DeviceInputManager.
//

__DEVICE_INPUT_MANAGER DeviceInputManager = {
	NULL,                                     //lpFocusKernelThread.
	NULL,                                     //lpShellKernelThread.
	SendDeviceMessage,                        //SendDeviceMessage routine.
	SetFocusThread,                           //SetFocusThread routine.
	SetShellThread,                           //SetShellThread routine.
	DimInitialize                             //Initialize routine.
};


