#ifndef _FUNCCOMMON_H_13B808254E46463CB77FD44A27DB5C87
#define _FUNCCOMMON_H_13B808254E46463CB77FD44A27DB5C87

#include "datatype.h"

#ifdef _WIN32
	#include "funcWin.h"
#endif


// socket
zh_int32 zh_socket(zh_int32 af, zh_int32 type, zh_int32 protocol);
zh_int32 zh_closesocket(zh_int32 s);
zh_int32 zh_connect(zh_int32 s, const struct sockaddr * name, zh_int32 namelen);
zh_int32 zh_select(zh_int32 nfds, fd_set * readfds, fd_set * writefds, fd_set * exceptfds, const struct timeval * timeout);
zh_int32 zh_getsockopt(zh_int32 s, zh_int32 level, zh_int32 optname, zh_char * optval, zh_int32 * optlen);
zh_int32 zh_ioctlsocket(zh_int32 s, zh_int32 cmd, zh_uint32 * argp);

// string
zh_int32 zh_trim(const zh_char *src, zh_char *buf, zh_int32 len);
zh_void zh_strlower(zh_char *str, zh_int32 len);
zh_uchar zh_reverseBits(zh_uchar c);

// system
zh_void zh_sleep(zh_uint32 ms);
zh_uint64 zh_GetTickCount64();
zh_int32 zh_GetUUID(zh_uchar *buf, zh_int32 bufsize);

// thread
#define zh_CreateThread(stackSize, func, args, threadId) zhCreateThread(stackSize, func, args, threadId)
#define zh_WaitForSingleObject(handle, time) zhWaitForSingleObject(handle, time)
#define zh_CloseHandle(handle) zhCloseHandle(handle);

// lock
#define zh_InitMutex(mutex) zhInitMutex(mutex)
#define zh_LockMutex(mutex) zhLockMutex(mutex)
#define zh_UnlockMutex(mutex) zhUnlockMutex(mutex)
#define zh_ReleaseMutex(mutex) zhReleaseMutex(mutex)


#endif
