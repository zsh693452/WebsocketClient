#ifndef _FUNCWIN_H_520A376649E34FD4875DB74212417702
#define _FUNCWIN_H_520A376649E34FD4875DB74212417702

#include <WinSock2.h>
#include <stdio.h>
#include <windows.h>
#include <WS2tcpip.h>
#include <process.h>

int zhsocket(int af, int type, int protocol);
int zhclosesocket(int s);
int zhconnect(int s, const struct sockaddr * name, int namelen);
int zhselect(int nfds, fd_set * readfds, fd_set * writefds, fd_set * exceptfds, const struct timeval * timeout);
int zhgetsockopt(int s, int level, int optname, char * optval, int * optlen);
int zhioctlsocket(int s, long cmd, u_long * argp);

void zhSleep(unsigned int ms);
unsigned long long zhGetTickCount64();
int zhGetUUID(unsigned char *buf, int bufsize);

int zhCreateThread(unsigned int stackSize, unsigned int (__stdcall *thread_func)(void *), void *args, unsigned int *threadId);
int zhWaitForSingleObject(HANDLE handle, unsigned int time);
int zhCloseHandle(HANDLE handle);

int zhInitMutex(CRITICAL_SECTION *cs);
int zhLockMutex(CRITICAL_SECTION *cs);
int zhUnlockMutex(CRITICAL_SECTION *cs);
int zhReleaseMutex(CRITICAL_SECTION *cs);



#endif
