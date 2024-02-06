#include "websocket.h"
#include "websock.h"
#include "autolock.h"
#include "funcCommon.h"

#ifdef _WIN32
	#include <WinSock2.h>
	#include <stdio.h>
	#include <windows.h>
	#include <WS2tcpip.h>
#endif

class Cleaner
{
public:
	Cleaner(){}
	Cleaner(zh_mutex *m)
	{
		m_mutex = m;
	}

	~Cleaner()
	{
		if (NULL != m_mutex)
			zh_ReleaseMutex(m_mutex);
	}

private:
	zh_mutex *m_mutex;
};

zh_mutex g_apimutex;
Cleaner clr(&g_apimutex);


static void SocketInit()
{
#ifdef _WIN32
	WSADATA data;
	WSAStartup(MAKEWORD(2, 2), &data);
#endif
}

void WSAPI WS_Init()
{
	static bool bInited = false;
	if (!bInited)
	{
		SocketInit();
		zh_InitMutex(&g_apimutex);
		bInited = true;
	}
}

void * WSAPI WS_Create(const char *url, unsigned short port, WS_FAMILY family, WSDataCallback cb, void *cbUsrData)
{
	CAutoLock lock(&g_apimutex);
	CWebsocket *ws = new CWebsocket(url, port, FAMILY_IPV4 == family ? AF_INET : AF_INET6);
	ws->SetCallback(cb, cbUsrData);
	return static_cast<void *>(ws);
}

void WSAPI WS_Destroy(void *handle)
{
	CAutoLock lock(&g_apimutex);
	CWebsocket *ws = static_cast<CWebsocket *>(handle);
	if (NULL != ws)
		delete ws;
}

WS_RESULT WSAPI WS_Open(void *handle, int timeout)
{
	if (NULL == handle)
		return WS_ERR_HANDLE;

	CAutoLock lock(&g_apimutex);
	CWebsocket *ws = static_cast<CWebsocket *>(handle);
	if (0 != ws->Connect(timeout))
		return WS_FAIL;

	return WS_OK;
}

void WSAPI WS_Close(void *handle)
{
	if (NULL != handle)
	{
		CAutoLock lock(&g_apimutex);
		CWebsocket *ws = static_cast<CWebsocket *>(handle);
		ws->Close();
	}
}

int WSAPI WS_Send(void *handle, char *data, int size, WS_DATA_TYPE type, int timeout)
{
	if (NULL == handle)
		return WS_ERR_HANDLE;

	CAutoLock lock(&g_apimutex);
	CWebsocket *ws = static_cast<CWebsocket *>(handle);
	return ws->Send(data, size, type, timeout);
}