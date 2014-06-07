#ifndef __KAPI_H__
#include "..\HXOS\include\kapi.h"
#endif

//Entry point of tlon(turn light on) command.
DWORD TurnLightOn(LPVOID p)
{
	HANDLE hLED = NULL;
	CHAR   ctrl = 1;
	DWORD  dwWriteSize = 0;
	
	//Try to open LED file to write.
	hLED = CreateFile("\\\\.\\LED0",0,0,NULL);
	if(NULL == hLED)
	{
		PrintLine("  Can not open LED to write.\r\n");
		goto __TERMINAL;
	}
	
	//Try to write non-zero value to turn on the light.
	if(!WriteFile(hLED,1,&ctrl,&dwWriteSize))
	{
		PrintLine("  Failed to write to LED device.\r\n");
		goto __TERMINAL;
	}
	
	//Show out result.
	PrintLine("  LED light should be on.");
	
__TERMINAL:
	if(hLED)  //Should close it first.
	{
		CloseFile(hLED);
	}
	return 0;
}

//Entry point for tloff(turn light off) command.
//Entry point of tlon(turn light on) command.
DWORD TurnLightOff(LPVOID p)
{
	HANDLE hLED = NULL;
	CHAR   ctrl = 0;
	DWORD  dwWriteSize = 0;
	
	//Try to open LED file to write.
	hLED = CreateFile("\\\\.\\LED0",0,0,NULL);
	if(NULL == hLED)
	{
		PrintLine("  Can not open LED to write.\r\n");
		goto __TERMINAL;
	}
	
	//Try to write non-zero value to turn on the light.
	if(!WriteFile(hLED,1,&ctrl,&dwWriteSize))
	{
		PrintLine("  Failed to write to LED device.\r\n");
		goto __TERMINAL;
	}
	
	//Show out result.
	PrintLine("  LED light should be off.");
	
__TERMINAL:
	if(hLED)  //Should close it first.
	{
		CloseFile(hLED);
	}
	return 0;
}
