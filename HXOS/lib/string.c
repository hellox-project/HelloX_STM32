//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,28 2004
//    Module Name               : string.cpp
//    Module Funciton           : 
//                                This module and string.h countains the
//                                string operation functions.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __HELLO_CHINA__
#include "..\INCLUDE\StdAfx.h"
#endif

#ifndef __STRING__
#include "string.h"
#endif

//------------------------------------------------------------------------
// Memory manipulating functions,memcpy,memset,...
//------------------------------------------------------------------------

void * memcpy (
        void * dst,
        const void * src,
        size_t count
        )
{
        void * ret = dst;

        //copy from lower addresses to higher addresses
        while (count--) {
                *(char *)dst = *(char *)src;
                dst = (char *)dst + 1;
                src = (char *)src + 1;
        }
        return(ret);
}

void * memset (
        void *dst,
        int val,
        size_t count
        )
{
        void *start = dst;

        while (count--) {
                *(char *)dst = (char)val;
                dst = (char *)dst + 1;
        }

        return(start);
}

void * memzero(
		void* dst,
		size_t count)
{
	return memset(dst,0,count);
}

//
//String operation functions implementation.
//
BOOL StrCmp(LPSTR strSrc,LPSTR strDes)  //Compare the two strings,if equal,returns
                                        //TRUE,otherwise,returns FALSE.
{
	//BOOL bResult = FALSE;
	WORD wIndex = 0x0000;

	if((NULL == strSrc) || (NULL == strDes))  //Parameter check.
	{
		return FALSE;
	}

	while(strSrc[wIndex] && strDes[wIndex] && (strSrc[wIndex] == strDes[wIndex]))
	{
		wIndex ++;
	}

	return strSrc[wIndex] == strDes[wIndex] ? TRUE : FALSE;
}

WORD StrLen(LPSTR strSrc)        //Get the string's length.
                                 //If the string's lenght is less than 
								 //MAX_STRING_LEN,returns the actual string's
								 //length,otherwise,returns MAX_STRING_LEN.
{
	WORD wStrLen = 0x00;

	if(NULL == strSrc)
	{
		return -1;
	}

	while(strSrc[wStrLen] && (MAX_STRING_LEN > wStrLen))
		wStrLen ++;

	return wStrLen;
}

BOOL Hex2Str(DWORD dwSrc,LPSTR strBuffer)  //Convert the hex format to string.
{
	BOOL bResult = FALSE;
	BYTE bt = 0x00;
	int  i;
	
	if(NULL == strBuffer)        //Parameter check.
		return bResult;

	for(i = 0;i < 8;i ++)
	{
		bt = (BYTE)dwSrc;   //LOBYTE(LOWORD(dwSrc));
		bt = bt & 0x0f;     //Get the low 4 bits.
		if(bt < 10)              //Should to convert to number.
		{
			bt += '0';
			strBuffer[7 - i] = bt;
		}
		else                     //Should to convert to character.
		{
			bt -= 10;
			bt += 'A';
			strBuffer[7 - i] = bt;
		}
		dwSrc = dwSrc >> 0x04;   //Continue to process the next 4 bits.
	}

	strBuffer[8] = 0x00;         //Add the string's terminal sign.
	return TRUE;
}

//
//Convert 32 bit int number to string.
//
BOOL Int2Str(DWORD dwNum,LPSTR pszResult)
{
	BOOL bResult = FALSE;
	BYTE bt;
	BYTE index = 0;
	BYTE sw;

	if(NULL == pszResult)
		return bResult;

	do{
		bt =  (BYTE)(dwNum % 10);
		bt += '0';
		pszResult[index++] = bt;
		dwNum /= 10;
	}while(dwNum);
	pszResult[index] = 0;        //Set the terminal sign.
	
	for(bt = 0;bt < index/2;bt ++)  //Inverse the string.
	{
		sw = pszResult[bt];
		pszResult[bt] = pszResult[index - bt -1];
		pszResult[index - bt - 1] = sw;
	}

	bResult = TRUE;
	return bResult;
}

//
//Print a string at a new line.
//
VOID PrintLine(LPSTR pszStr)
{
	CD_PrintString(pszStr,TRUE);
}

//
//Copy the first string, to the second string buffer.
//
VOID StrCpy(LPSTR strSrc,LPSTR strDes)
{
	DWORD dwIndex = 0;

	if((NULL == strSrc) || (NULL == strDes))  //Parameter check.
	{
		return;
	}
	
	while(strSrc[dwIndex])
	{
		strDes[dwIndex] = strSrc[dwIndex];
		dwIndex ++;
	}
	strDes[dwIndex] = 0;
}

//
//Convert the string's low character to uper character.
//Such as,the input string is "abcdefg",then,the output
//string would be "ABCDEFG".
//

VOID ConvertToUper(LPSTR pszSource)
{
	BYTE     bt        = 'a' - 'A';
	//DWORD    dwIndex   = 0x0000;
	DWORD    dwMaxLen  = MAX_STRING_LEN;

	if(NULL == pszSource)
	{
		return;
	}

	while(*pszSource)
	{
		if((*pszSource >= 'a') && ( *pszSource <= 'z'))
		{
			*pszSource -= bt;
		}
		pszSource ++;
		dwMaxLen --;
		if(0 == dwMaxLen)
			break;
	}

	return;
}

//
//Convert the string to hex number.
//If success,it returns TRUE,else,returns FALSE.
//
BOOL Str2Hex(LPSTR pszSrc,DWORD* pdwResult)
{
	BOOL     bResult  = FALSE;
	DWORD    dwResult = 0x00000000;
	if((NULL == pszSrc) || (NULL == pdwResult))  //Parameters check.
		return bResult;

	if(StrLen(pszSrc) > 8)                      //If the string's length is longer
		                                        //than the max hex number length.
		return bResult;

	ConvertToUper(pszSrc);                      //Convert to uper character.

	while(*pszSrc)
	{
		dwResult <<= 4;
		switch(*pszSrc)
		{
		case '0':
			dwResult += 0;
			break;
		case '1':
			dwResult += 1;
			break;
		case '2':
			dwResult += 2;
			break;
		case '3':
			dwResult += 3;
			break;
		case '4':
			dwResult += 4;
			break;
		case '5':
			dwResult += 5;
			break;
		case '6':
			dwResult += 6;
			break;
		case '7':
			dwResult += 7;
			break;
		case '8':
			dwResult += 8;
			break;
		case '9':
			dwResult += 9;
			break;
		case 'A':
			dwResult += 10;
			break;
		case 'B':
			dwResult += 11;
			break;
		case 'C':
			dwResult += 12;
			break;
		case 'D':
			dwResult += 13;
			break;
		case 'E':
			dwResult += 14;
			break;
		case 'F':
			dwResult += 15;
			break;
		default:
			bResult = FALSE;
			return bResult;
		}
		pszSrc ++;
	}

	bResult    = TRUE;
	*pdwResult = dwResult;

	return bResult;
}

//
//The implementation of FormString routine.
//This routine formats a string,and copy it into a buffer.
//It's function likes sprintf.
//
INT FormString(LPSTR lpszBuff,LPSTR lpszFmt,LPVOID* lppParam)
{
	DWORD        dwIndex        = 0;
	LPSTR        lpszTmp        = NULL;
	CHAR         Buff[12];

	if((NULL == lpszBuff) || (NULL == lpszBuff))
		return -1;

	lpszTmp = lpszBuff;
	while(*lpszFmt)
	{
		if('%' == *lpszFmt)    //Should process.
		{
			lpszFmt ++;        //Skip '%'.
			switch(*lpszFmt)
			{
			case 'd':    //Convert an integer to string.
				Int2Str(*((DWORD*)lppParam[dwIndex ++]),Buff);  //Convert to string.
				StrCpy(Buff,lpszTmp);
				lpszTmp += StrLen(Buff);
				lpszFmt ++;
				break;
			case 'c':    //Convert a character to string.
				*lpszTmp ++= *((BYTE*)lppParam[dwIndex ++]);
				lpszFmt ++;
				break;
			case 's':    //Append a string.
				StrCpy((LPSTR)lppParam[dwIndex],lpszTmp);
				lpszTmp += StrLen((LPSTR)lppParam[dwIndex ++]);
				lpszFmt ++;
				break;
			case 'x':    //Convert an integer to string in hex.
			case 'X':
				Hex2Str(*((DWORD*)lppParam[dwIndex ++]),Buff);  //Convert to string.
				StrCpy(Buff,lpszTmp);
				lpszTmp += StrLen(Buff);
				lpszFmt ++;
				break;
			default:     //Unsupported now.
				break;
			}
		}
		*lpszTmp = *lpszFmt;
		if(0 == *lpszTmp)    //Reach end.
			break;
		lpszTmp ++;
		lpszFmt ++;
	}

	*lpszTmp = 0;    //End sign.
	return (lpszTmp - lpszBuff);
}

//A helper routine used to convert a string from lowercase to capital.
//The string should be terminated by a zero,i.e,a C string.
VOID ToCapital(LPSTR lpszString)
{
	int nIndex = 0;

	if(NULL == lpszString)
	{
		return;
	}
	while(lpszString[nIndex++])
	{
		if((lpszString[nIndex] >= 'a') && (lpszString[nIndex] <= 'z'))
		{
			lpszString[nIndex] += 'A' - 'a';
		}
	}
}

//string comparation code.
int strcmp (
        const char * src,
        const char * dst
        )
{
        int ret = 0 ;
        while( ! (ret = *(unsigned char *)src - *(unsigned char *)dst) && *dst)
                ++src, ++dst;  
        if ( ret < 0 )
                ret = -1 ;
        else if ( ret > 0 )
                ret = 1 ;
        return( ret );
}

int memcmp(const void *buffer1,const void *buffer2,int count)
{
   if (!count)
      return(0);
   while ( --count && *(char *)buffer1 == *(char *)buffer2)
   {
      buffer1 = (char *)buffer1 + 1;
      buffer2 = (char *)buffer2 + 1;
   }
   return( *((unsigned char *)buffer1) - *((unsigned char *)buffer2) );
}

int strlen(const char * s)
{
   int i;
   for (i = 0; s[i]; i++) ;
   return i;
}

char *strcpy(char * dst, const char * src)
{
    char * cp = dst;
    while( *cp++ = *src++ )
            ;               /* Copy src over dst */
    return( dst );
}

char * strcat (
        char * dst,
        const char * src
        )
{
        char * cp = dst;
 
        while( *cp )
                cp++;                   /* find end of dst */
 
        while( *cp++ = *src++ ) ;       /* Copy src to end of dst */
 
        return( dst );                  /* return dst */
 
}

void strtrim(char * dst,int flag)
{
	char* pos   = dst;
	int   len   = 0;
	int   i     = 0;
	
	if(NULL == dst)
	{
		return;
	}

	len = strlen(dst);
	if(len <= 0)
	{
		return; 
	}
	if(flag&TRIM_LEFT)
	{
		while(i < len)
		{
			if(*pos != 0x20) 
			{
				break;
			}

			pos ++;
			i   ++;
		}

		//È«ÊÇ¿Õ¸ñ
		if(len == i)
		{
			dst[0] = 0;
			return;
		}

		if(i > 0) 
		{
			len -=  i;
			memcpy(dst,pos,len);
			dst[len] = 0;
		}
	}
	
	if(flag&TRIM_RIGHT)
	{
		for(i = len-1; i >= 0;i--)
		{
			if(dst[i] == 0x20)
			{
				dst[i] = 0;
			}
			else
			{
				break;
			}
		}
	}
	
}
