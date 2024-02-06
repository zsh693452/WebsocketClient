#ifndef _DATATYPE_H_939DA2ED7B88490FAC3520CC44A78FF7
#define _DATATYPE_H_939DA2ED7B88490FAC3520CC44A78FF7

// base type
#define zh_int32 int
#define zh_uint32 unsigned int
#define zh_char char
#define zh_uchar unsigned char
#define zh_short short
#define zh_ushort unsigned short
#define zh_void void
#define zh_int64 long long
#define zh_uint64 unsigned long long
#define zh_bool bool
#define zh_true true
#define zh_false false

#ifdef _WIN32
	#include <WinSock2.h>
	#include <stdio.h>
	#include <windows.h>
	#include <WS2tcpip.h>
#endif


#ifdef _WIN32
	#define sockaddr_in sockaddr_in
	#define sockaddr sockaddr
	#define zh_mutex CRITICAL_SECTION
	#define zh_thread_handle HANDLE
	#define zh_INFINITE INFINITE
#endif


#endif
