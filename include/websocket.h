#ifndef _WEBSOCKET_H_2936AC6A040648E7B0F7D83A09083848
#define _WEBSOCKET_H_2936AC6A040648E7B0F7D83A09083848

#ifdef _WIN32
	#include <windows.h>
	#define WSAPI WINAPI
#else
	#define WSAPI
#endif

typedef enum
{
	WS_OK = 0,
	WS_ERR_HANDLE = 1,
	WS_FAIL = 2
} WS_RESULT;

typedef enum 
{
	FAMILY_IPV4 = 0,
	FAMILY_IPV6 = 1
}WS_FAMILY;

typedef enum
{
	WS_DATA_TEXT = 0,
	WS_DATA_BIN = 1
}WS_DATA_TYPE;

typedef void (*WSDataCallback)(char *data, int size, int op, void *userdata);
typedef void (*WSDisconnectCallback)(void *userdata);

#ifdef __cplusplus
extern "C" {
#endif
	void WSAPI WS_Init();
	void * WSAPI WS_Create(const char *url, unsigned short port, WS_FAMILY family, WSDataCallback cb, void *cbUsrData);
	void WSAPI WS_Destroy(void *handle);
	WS_RESULT WSAPI WS_Open(void *handle, int timeout);
	void WSAPI WS_Close(void *handle);
	int WSAPI WS_Send(void *handle, char *data, int size, WS_DATA_TYPE type, int timeout);
	void WSAPI WS_SetDisconnectCB(void *handle, WSDisconnectCallback cb, void *cbUserData);

#ifdef __cplusplus
}
#endif


#endif

