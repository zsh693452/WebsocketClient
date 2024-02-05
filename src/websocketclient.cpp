#include "websocket.h"
#include "websock.h"

#ifdef _WIN32
	#include <WinSock2.h>
	#include <stdio.h>
	#include <windows.h>
	#include <WS2tcpip.h>
#endif

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
		bInited = true;
	}
}

void * WSAPI WS_Create(const char *url, unsigned short port, WS_FAMILY family, WSDataCallback cb, void *cbUsrData)
{
	CWebsocket *ws = new CWebsocket(url, port, FAMILY_IPV4 == family ? AF_INET : AF_INET6);
	ws->SetCallback(cb, cbUsrData);
	return static_cast<void *>(ws);
}

void * WSAPI WS_Destroy()
{
	return NULL;
}

WS_RESULT WSAPI WS_Connect(void *handle, int timeout)
{
	if (NULL == handle)
		return WS_ERR_HANDLE;

	CWebsocket *ws = static_cast<CWebsocket *>(handle);
	if (0 != ws->Connect(timeout))
		return WS_FAIL;

	return WS_OK;
}

void WSAPI WS_Close(void *handle)
{
	if (NULL != handle)
	{
		CWebsocket *ws = static_cast<CWebsocket *>(handle);
		ws->Close();
	}
}

int WSAPI WS_Send(void *handle, char *data, int size, WS_DATA_TYPE type, int timeout)
{
	if (NULL == handle)
		return WS_ERR_HANDLE;

	CWebsocket *ws = static_cast<CWebsocket *>(handle);

	return ws->Send(data, size, type, timeout);
}