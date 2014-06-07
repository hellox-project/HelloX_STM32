//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 23 Feb, 2014
//    Module Name               : mem_scat.cpp
//    Module Funciton           : 
//                                This module contains hardware level system memory regions.Each memory region,
//                                for read and write,should be define an entry in SystemMemRegion array,thus it
//                                can be managed by operating system's memory management machanism.
//
//    Last modified Author      : Garry.Xin
//    Last modified Date        : 
//    Last modified Content     : 
//                                1. 
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "..\include\StdAfx.h"
#endif

__MEMORY_REGION SystemMemRegion[] = {
	{(LPVOID)0x20008000,0x00008000},//8K heap memory.
	//Please add more memory regions here.
	//The last entry must be NULL and zero,to indicate the end of this array.
	{NULL,0}
};
