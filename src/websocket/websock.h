#ifndef _WEBSOCK_H_46D29AA08A594CEC838F0FBA1B23BAEA
#define _WEBSOCK_H_46D29AA08A594CEC838F0FBA1B23BAEA

#include "datatype.h"
#include "internaldef.h"
#include "protocol.h"

#define URL_LEN 512
#define MAX_PAYLOAD_BUFF_SIZE 1024 * 8

typedef enum
{
	WebsocketResult_OK = 0,
	WebsocketResult_Failed = 1,
	WebsocketResult_Timeout = 2
} WebsocketResult;

typedef zh_void (*WebsocketDataCallback)(zh_char *data, zh_int32 size, zh_int32 op, zh_void *userdata);
typedef zh_void (*WebsocketDisconnectCallback)(zh_void *userdata);

class CWebsocket
{
public:
	CWebsocket();
	CWebsocket(const zh_char *url, zh_ushort port, zh_int32 family);
	~CWebsocket();

	zh_int32 Connect(zh_int32 timeout);
	zh_void Close();
	zh_int32 Send(const zh_char *data, zh_int32 size, zh_int32 op, zh_int32 timeout);

	zh_void SetCallbackPayload(WebsocketDataCallback cb, zh_void *userdata);
	zh_void SetCallbackDisconnect(WebsocketDisconnectCallback cb, zh_void *userdata);

	static zh_void PayloadCB(zh_char *payload, zh_uint32 size, zh_int32 fin, zh_int32 op, zh_void *userdata);

private:
	CProtocol *m_protocol;
	zh_char *m_payloadBuff;
	zh_uint32 m_payloadOffset;
	zh_uint32 m_payloadBuffSize;
	zh_void *m_payloadUserdata;
	zh_bool m_bConntected;
	WebsocketDataCallback m_payloadCB;
};

#endif



