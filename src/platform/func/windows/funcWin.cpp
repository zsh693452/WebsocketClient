#include "funcWin.h"
#include <rpc.h>
#include <string.h>

int zhsocket(int af, int type, int protocol)
{
	return socket(af, type, protocol);
}

int zhclosesocket(int s)
{
	return closesocket(s);
}

int zhconnect(int s, const struct sockaddr * name, int namelen)
{
	return connect(s, name, namelen);
}

int zhselect(int nfds, fd_set * readfds, fd_set * writefds, fd_set * exceptfds, const struct timeval * timeout)
{
	return select(nfds, readfds, writefds, exceptfds, timeout);
}

int zhgetsockopt(int s, int level, int optname, char * optval, int * optlen)
{
	return getsockopt(s, level, optname, optval, optlen);
}

int zhioctlsocket(int s, long cmd, u_long * argp)
{
	return ioctlsocket(s, cmd, argp);
}

int zhCreateThread(unsigned int stackSize, unsigned int (__stdcall *thread_func)(void *), void *args, unsigned int *threadId)
{
	return _beginthreadex(NULL, stackSize, thread_func, args, 0, threadId);
}

unsigned long long zhGetTickCount64()
{
	return GetTickCount64();
}

int zhGetUUID(unsigned char *buf, int bufsize)
{
	UUID uuid;
	RPC_STATUS hRst = UuidCreateSequential(&uuid);
	if (RPC_S_OK == hRst)
	{
		int cpysize = bufsize >= 16 ? 16 : bufsize;
		char *p = (char *)&uuid;
		memcpy(buf, p, cpysize);
		return cpysize;
	}

	return 0;
}

void zhSleep(unsigned int ms)
{
	Sleep(ms);
}