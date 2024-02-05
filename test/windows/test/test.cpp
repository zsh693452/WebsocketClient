// test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "websocket.h"

void Callback(char *data, int size, int op, void *userdata)
{
	char buffer[1024] = {0};
	memcpy(buffer, data, size);

	printf("size=%d op=%d\ndata=%s\n", size, op, buffer);
}

int _tmain(int argc, _TCHAR* argv[])
{
	WS_Init();
	void *handle = WS_Create("120.25.223.112", 9001, FAMILY_IPV4, Callback, NULL);
	//void *handle = WS_Create("152.70.181.45", 443, FAMILY_IPV4);
	WS_RESULT rst = WS_Connect(handle, 1000 * 10);
	printf("connect rst=%d\n", rst);

	
	char sendstr[] = "{\"head\":{\"cmd\":1,\"role\":1,\"name\":\"123456\",\"remote\":\"\"}}";
	int sendrst = WS_Send(handle, sendstr, strlen(sendstr), WS_DATA_TEXT, 1000 * 5);
	printf("send rst=%d send %d bytes\n", sendrst, strlen(sendstr));


	char maxsendstr[1024] = {0};
	memset(maxsendstr, 'A', sizeof(maxsendstr) - 1);
	maxsendstr[1020] = 'D';
	maxsendstr[1021] = 'C';
	maxsendstr[1022] = 'B';
	sendrst = WS_Send(handle, maxsendstr, strlen(maxsendstr), 1000 * 5);
	printf("send max rst=%d send %d bytes\n", sendrst, strlen(maxsendstr));
	

	Sleep(1000 * 600);
	WS_Close(handle);

	return 0;
}

