//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 20, 2014
//    Module Name               : ktmgr2.h
//    Module Funciton           : 
//                                This is the second part of ktmgr.h file,since it's master part's
//                                size is too large.
//                                Semaphore and Mail Box objects are defined in this file.
//
//                                ************
//                                This file is the most important file of Hello China.
//                                ************
//    Last modified Author      : Garry
//    Last modified Date        : Jun 20,2014
//    Last modified Content     : 
//                                1. 
//    Lines number              :
//***********************************************************************/

#ifndef __KTMGR2_H__
#define __KTMGR2_H__

//Definition of Semaphore objects.
BEGIN_DEFINE_OBJECT(__SEMAPHORE)
    INHERIT_FROM_COMMON_OBJECT
	INHERIT_FROM_COMMON_SYNCHRONIZATION_OBJECT
	DWORD                dwMaxSem;    //Maximal semaphore counter,default is 1.
    DWORD                dwCurrSem;   //Current value of semaphore,default same as dwMaxSem.
	__PRIORITY_QUEUE*    lpWaitingQueue;  //Kernel thread(s) waiting for the semaphore.
	//Change default sem counters,this routine should be called before any WaitForThisObject or
	//WaitForThisObjectEx operations since it can not wake up already blocked kernel thread.
	BOOL                 (*SetSemaphoreCount)(__COMMON_OBJECT*,DWORD,DWORD); 
	BOOL                 (*ReleaseSemaphore)(__COMMON_OBJECT*,DWORD* pdwPrevCount);
	DWORD                (*WaitForThisObjectEx)(__COMMON_OBJECT*,DWORD dwMillionSecond,DWORD* pdwWait);
END_DEFINE_OBJECT(__SEMAPHORE)

//Initializer and Uninitializer of semaphore object.
BOOL SemInitialize(__COMMON_OBJECT* pSemaphore);
VOID SemUninitialize(__COMMON_OBJECT* pSemaphore);

//Definition of mailbox's message.
typedef struct tag_MB_MESSAGE{
	LPVOID               pMessage;    //Point to any object.
	DWORD                dwPriority;  //Message priority in message box.
}__MB_MESSAGE;

//Definition of mail box object.
BEGIN_DEFINE_OBJECT(__MAIL_BOX)
    INHERIT_FROM_COMMON_OBJECT
	INHERIT_FROM_COMMON_SYNCHRONIZATION_OBJECT
	__MB_MESSAGE*        pMessageArray;    //Array contains the message sent to this mail box.
    DWORD                dwMaxMessageNum;  //The mail box's volume.
	DWORD                dwCurrMessageNum; //Current message number in mailbox.
	DWORD                dwMessageHeader;  //Pointing to message array's header.
	DWORD                dwMessageTail;    //Pointing to message array's tail.
	__PRIORITY_QUEUE*    lpSendingQueue;   //Kernel thread(s) try to send mail.
	__PRIORITY_QUEUE*    lpGettingQueue;   //Kernel thread(s) try to get mail.

	//Set mailbox's size,default is 1.This routine should be called before any SendMail or
	//GetMail operations since it can not wake up any already blocked kernel thread.
	BOOL           (*SetMailboxSize)(__COMMON_OBJECT*,DWORD);
	DWORD          (*SendMail)(__COMMON_OBJECT*,LPVOID pMessage,
		                      DWORD dwPriority,DWORD dwMillionSecond,DWORD* pdwWait);  //Send message to mailbox.
	DWORD          (*GetMail)(__COMMON_OBJECT*,LPVOID* ppMessage,
		                     DWORD dwMillionSecond,DWORD* pdwWait);  //Get a mail from mail box.
END_DEFINE_OBJECT(__MAIL_BOX)

//Initializer and Uninitializer of mail box object.
BOOL MailboxInitialize(__COMMON_OBJECT* pMailbox);
VOID MailboxUninitialize(__COMMON_OBJECT* pMailbox);

#endif //__KTMGR2_H__
