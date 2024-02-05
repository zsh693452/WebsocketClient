#ifndef _PROTOCOL_H_ED84DADADD274B1E884363F48DFEF65E
#define _PROTOCOL_H_ED84DADADD274B1E884363F48DFEF65E

#include "datatype.h"
#include "ISocket.h"
#include "http.h"

typedef zh_void (*PayloadCallback)(zh_char *payload, zh_uint32 size, zh_int32 fin, zh_int32 op, zh_void *userdata);

#define URL_LEN 512
#define MAX_SEND_BUFF 1024
#define MAX_SEND_ONCE 65535

#ifdef _WIN32
	#define DECLARE_THREAD_FUNC(name, args) unsigned int WINAPI name(void *args);
	#define IMPLEMENT_THREAD_FUNC(class, name, args) unsigned int WINAPI class::name(void *args)
#endif

// define by little endian
typedef struct _FRAME_HEAD_
{
	zh_uchar opcode:4;
	zh_uchar rsv3:1;
	zh_uchar rsv2:1;
	zh_uchar rsv1:1;
	zh_uchar fin:1;
	zh_uchar payloadlen:7;
	zh_uchar mask:1;
} FRAME_HEAD, *PFRAME_HEAD;

typedef enum
{
	OP_ContinuationFrame = 0x00,
	OP_TextFrame = 0x01,
	OP_BinaryFrame = 0x02,
	OP_ConnectionClose = 0x08,
	OP_PingFrame = 0x09,
	OP_PongFrame = 0x0a
} Opcode;

typedef enum
{
	ResultOK = 0,
	ResultTimeout = 1,
	ResultFailed = 2,
	ResultHandshakeFailed = 3,
	ResultExceedMax = 4
} Result;

class CProtocol
{
public:
	CProtocol();
	CProtocol(zh_int32 ssl, const zh_char *url, zh_ushort port, zh_int32 family);
	~CProtocol();

	zh_void SetPayloadCB(PayloadCallback cb, zh_void *userdata);
	zh_int32 Connect(zh_int32 timeout);
	zh_int32 SendFrame(const zh_char *data, zh_uint32 size, Opcode op, zh_int32 fin, zh_int32 timeout);
	zh_int32 SendPing(const zh_char *data, zh_uint32 size, zh_int32 timeout);

protected:
	static DECLARE_THREAD_FUNC(ThreadRecv, args);
	zh_void Recving();
	zh_void StartRecv();
	zh_int32 SendAll(zh_char *data, zh_int32 size, zh_int32 timeout);

	zh_int32 Handshark();
	zh_int32 WaitHandshark(zh_int32 timeout);
	zh_bool HandsharkSuccess(zh_int32 code, HTTP_HEADER *header, zh_int32 headerSize);
	zh_int32 SecWebSocketAccept(const zh_char *key, zh_int32 KeySize, zh_char *value, zh_int32 valueSize);
	zh_bool FrameCompleted(zh_char *data, zh_uint32 size, zh_int32 *fin, zh_int32 *opcode, zh_int32 *maskKey, zh_uint32 *payloadLen, zh_uint32 *payloadOffset);
	
	

private:
	ISocket *m_isocket;
	CHttpParser m_httpParser;
	zh_char m_url[URL_LEN];
	zh_ushort m_port;
	zh_int32 m_family;
	zh_bool m_quit;
	zh_char m_base64Key[64];
	zh_char *m_sendBuf;
	zh_int32 m_sendBufSize;
	zh_char *m_recvBuf;
	zh_uint32 m_recvBufSize;
	PayloadCallback m_payloadCB;
	zh_void *m_userdata;
};

#endif

