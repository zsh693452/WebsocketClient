#include "websock.h"
#include "funcCommon.h"

#define TCPSEND_TIMEOUT 1000 * 5

CWebsocket::CWebsocket()
{

}

CWebsocket::CWebsocket(const zh_char *url, zh_ushort port, zh_int32 family)
{
#ifdef _SSL
	zh_int32 ssl = 1;
#else
	zh_int32 ssl = 0;
#endif

	m_protocol = new CProtocol(ssl, url, port, family);
	m_protocol->SetPayloadCB(CWebsocket::PayloadCB, this);

	m_payloadOffset = 0;
	m_payloadBuffSize = MAX_PAYLOAD_BUFF_SIZE;
	m_payloadBuff = new zh_char[MAX_PAYLOAD_BUFF_SIZE];
	memset(m_payloadBuff, 0, m_payloadBuffSize);

	m_userdata = NULL;
	m_cb = NULL;
}

CWebsocket::~CWebsocket()
{
	if (NULL != m_protocol)
	{
		delete m_protocol;
		m_protocol = NULL;
	}

	if (NULL != m_payloadBuff)
	{
		delete[] m_payloadBuff;
		m_payloadBuff = NULL;
	}
}

zh_int32 CWebsocket::Connect(zh_int32 timeout)
{
	zh_int32 rst = m_protocol->Connect(timeout);
	if (ResultOK != rst)
	{
		if (ResultTimeout == rst)
			return WebsocketResult_Timeout;

		return WebsocketResult_Failed;
	}

	return WebsocketResult_OK;
}

zh_void CWebsocket::Close()
{
	
}

zh_int32 CWebsocket::Send(const zh_char *data, zh_int32 size, zh_int32 op, zh_int32 timeout)
{
	zh_uint32 total = static_cast<zh_uint32>(size);
	zh_uint32 offset = 0;
	zh_int32 fin = 0;
	zh_uint64 startTick = GetTickCount64();
	Opcode curop = OP_TextFrame;
	if (1 == op) curop = OP_BinaryFrame;

	do 
	{
		if (GetTickCount64() > startTick + static_cast<zh_uint64>(timeout))
			return 1; 

		zh_uint32 sendsize = total > MAX_SEND_ONCE ? MAX_SEND_ONCE : total;
		if ((sendsize < MAX_SEND_ONCE) || (MAX_SEND_ONCE == sendsize && sendsize == total))
			fin = 1;

		if (ResultOK != m_protocol->SendFrame(data + offset, sendsize, curop, fin, 1000 * 5))
		{
			return ResultFailed;
		}

		total -= sendsize;
		offset += sendsize;
		curop = OP_ContinuationFrame;
	} while (total);

	return 0;
}

zh_void CWebsocket::PayloadCB(zh_char *payload, zh_uint32 size, zh_int32 fin, zh_int32 op, zh_void *userdata)
{
	CWebsocket *p = static_cast<CWebsocket *>(userdata);

	// store payload of frame
	memcpy(p->m_payloadBuff + p->m_payloadOffset, payload, size);
	p->m_payloadOffset += size;

	// copy out
	if (1 == fin)
	{
		// callback
		if (NULL != p->m_cb)
		{
			p->m_cb(p->m_payloadBuff, p->m_payloadOffset, op, p->m_userdata);
			p->m_payloadOffset = 0;
		}
	}
}

zh_void CWebsocket::SetCallback(WebsocketDataCallback cb, zh_void *userdata)
{
	m_cb = cb;
	m_userdata = userdata;
}