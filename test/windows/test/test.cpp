// test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "websocket.h"
#include <signal.h>

void *g_handle = NULL;

void signal_callback_handler(int signum) {
	printf("Caught signal %d\n", signum);
	WS_Close(g_handle);
	WS_Destroy(g_handle);
}


void Callback(char *data, int size, int op, void *userdata)
{
	char buffer[1024] = {0};
	memcpy(buffer, data, size);

	printf("size=%d op=%d\ndata=%s\n", size, op, buffer);
}

int _tmain(int argc, _TCHAR* argv[])
{
	signal(SIGINT, signal_callback_handler);

	WS_Init();
	g_handle = WS_Create("120.25.223.112", 9001, FAMILY_IPV4, Callback, NULL);
	WS_RESULT rst = WS_Open(g_handle, 1000 * 10);
	printf("websocket connect rst = %d\n", rst);

	if (WS_OK != rst)
	{
		WS_Close(g_handle);
		WS_Destroy(g_handle);
		return 0;
	}

	char sendstr[] = "{\"head\":{\"cmd\":1,\"role\":1,\"name\":\"123456\",\"remote\":\"\"}}";
	int sendrst = WS_Send(g_handle, sendstr, strlen(sendstr), WS_DATA_TEXT, 1000 * 5);
	printf("send rst=%d send %d bytes\n", sendrst, strlen(sendstr));


	Sleep(1000 * 60 * 5);
	WS_Close(g_handle);
	WS_Destroy(g_handle);

	return 0;
}

