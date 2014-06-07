//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct,22 2006
//    Module Name               : L_STDIO.H
//    Module Funciton           : 
//                                Standard I/O libary header file.
//                                Please note it's name is a prefix "L_".
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

typedef char *  va_list;
typedef unsigned int     size_t;

#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

#define va_start(ap,v)  ( ap = (va_list)&v + _INTSIZEOF(v) )
#define va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap)      ( ap = (va_list)0 )

#define MAX_BUFFER_SIZE 512

#define NOFLOAT  //Kernel does not support floating point number yet.

void* memcpy(void* dst,const void* src,size_t count);
void* memset(void* dst,int val,size_t count);
void* memzero(void* dst,size_t count); 

int sprintf(char* buf,const char* fmt,...);
