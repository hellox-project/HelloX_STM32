//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun,24 2006
//    Module Name               : EXTCMD.CPP
//    Module Funciton           : 
//                                This module countains Hello China's External command's
//                                implementation.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "..\include\StdAfx.h"
#include "EXTCMD.H"
#include "HYPERTRM.H"  //For hypertrm application.
#include "FDISK.H"
#include "FS.H"        //For fs application.
#include "HEDIT.H"     //For hedit application.
#include "fibonacci.h" //A test application to calculate Fibonacci sequence.

//External global routines.
extern DWORD TurnLightOn(LPVOID p);
extern DWORD TurnLightOff(LPVOID p);

__EXTERNAL_COMMAND ExtCmdArray[] = {
	{"fibonacci",NULL,FALSE,Fibonacci},
	{"hypertrm",NULL,FALSE,Hypertrm},
	{"hyptrm2",NULL,FALSE,Hyptrm2},
	{"fdisk",NULL,FALSE,fdiskEntry},
	{"fs",NULL,FALSE,fsEntry},
	{"hedit",NULL,FALSE,heditEntry},
	//Add your external command/application entry here.
	//{"yourcmd",NULL,FALSE,cmdentry},
	//The last entry of this array must be the following one,
	//to indicate the terminator of this array.
  {"tlon",NULL,FALSE,TurnLightOn},
  {"tloff",NULL,FALSE,TurnLightOff},
	{NULL,NULL,FALSE,NULL}
};

