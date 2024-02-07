#include "protocol.h"
#include "funcCommon.h"
#include "tcp.h"
#include "tls.h"
#include "internaldef.h"
#include "http.h"
#include "sha1.h"
#include "base64.h"

#define MAX_RECVBUF_SIZE 1024 * 1024


CProtocol::CProtocol(zh_int32 ssl, const zh_char *url, zh_ushort port, zh_int32 family)
{
	zh_InitMutex(&m_netMutex);

	if (1 == ssl)
		m_isocket = new CTLS(&m_netMutex);
	else
		m_isocket = new CTCP();

	BufferAlloc();
	memset(m_url, 0, sizeof(m_url));
	if (strlen(url) >= sizeof(m_url))
		memcpy(m_url, url, sizeof(m_url) - 1);
	else
		strcpy(m_url, url);

	m_payloadCB = NULL;
	m_disconnectCB = NULL;
	m_payloadUserdata = NULL;
	m_disconnectUserdata = NULL;
	m_hThreadRecv = NULL;
	m_quit = zh_true;
	m_port = port;
	m_family = family;
	m_isocket->CreateSocket(family);
}

CProtocol::~CProtocol()
{
	Disconnect();

	if (NULL == m_isocket)
	{
		m_isocket->Close();

		delete m_isocket;
		m_isocket = NULL;
	}

	BufferFree();

	zh_ReleaseMutex(&m_netMutex);
}

zh_void CProtocol::BufferAlloc()
{
	m_sendBuf = new zh_char[MAX_SEND_BUFF];
	m_sendBufSize = MAX_SEND_BUFF;
	memset(m_sendBuf, 0, m_sendBufSize);

	m_recvBuf = new zh_char[MAX_RECVBUF_SIZE];
	m_recvBufSize = MAX_RECVBUF_SIZE;
	memset(m_recvBuf, 0, m_recvBufSize);
}

zh_void CProtocol::BufferFree()
{
	if (NULL != m_sendBuf)
	{
		delete[] m_sendBuf;
		m_sendBuf = NULL;
	}

	if (NULL != m_recvBuf)
	{
		delete[] m_recvBuf;
		m_recvBuf = NULL;
	}
}

zh_int32 CProtocol::Connect(zh_int32 timeout)
{
	zh_uint64 startTick = zh_GetTickCount64();

	zh_int32 rst = m_isocket->Connect(m_url, m_port, m_family, timeout);
	if (ErrSockSuccess != rst)
	{
		m_isocket->Close();

		if (ErrSockTimeout == rst)
			return ResultTimeout;
		
		return ResultFailed;
	}
	
	zh_uint64 endTick = zh_GetTickCount64();
	zh_int32 time = static_cast<zh_int32>(endTick - startTick);

	// send handshake
	if (0 != Handshake())
	{
		m_isocket->Close();
		return ResultHandshakeFailed;
	}

	// recv data to check handshake result
	if (0 == WaitHandshake(timeout - time))
	{
		StartRecv();
		return ResultOK;
	}

	m_isocket->Close();
	return ResultHandshakeFailed;
}

zh_void CProtocol::Disconnect()
{
	m_quit = zh_true;
	if (NULL != m_hThreadRecv)
	{
		zh_WaitForSingleObject(m_hThreadRecv, zh_INFINITE);
		zh_CloseHandle(m_hThreadRecv);
		m_hThreadRecv = NULL;
	}

	m_isocket->Close();
}

IMPLEMENT_THREAD_FUNC(CProtocol, ThreadRecv, args)
{
	CProtocol *ws = static_cast<CProtocol *>(args);
	zh_int32 reason = ws->Recving();
	if (ws->m_disconnectCB) 
		ws->m_disconnectCB(ws->m_disconnectUserdata);
	
	return 0;
}

zh_void CProtocol::StartRecv()
{
	m_quit = zh_false;
	zh_uint32 threadid = 0;
	zh_CreateThread(0, ThreadRecv, this, &threadid);
}

zh_int32 CProtocol::Recving()
{
	zh_char buffer[1024 * 4] = {0};
	zh_uint32 offset = 0;
	zh_uint64 pingTick = 0;
	zh_int32 quitReason = ReasonUsrQuit;
	const zh_char *ping = "zsh_software_ping";
	const zh_uint64 pingTimeout = 1000 * 30;

	while (!m_quit)
	{
		// send ping
		if (pingTimeout < (GetTickCount64() - pingTick))
		{
			if (ResultOK == SendPing(ping, strlen(ping), 1000 * 2))
				pingTick = GetTickCount64();
		}

		zh_int32 bytes = this->m_isocket->Recv(buffer, sizeof(buffer));
		if (ErrSockClose == bytes || ErrSockUnknown == bytes || ErrSockFail == bytes)
		{
			quitReason = ReasonPeerClose;
			break;
		}
		else if (ErrSockNoData == bytes)
		{
			zh_sleep(10);
			continue;
		}

		if (offset + static_cast<zh_uint32>(bytes) > m_recvBufSize)
		{
			// error callback
			LOGE("buffer full!\n");
			offset = 0;
			break;
		}

		zh_int32 fin = 0;
		zh_int32 op = -1;
		zh_int32 maskKey = 0;
		zh_uint32 payloadOffset = 0;
		zh_uint32 payloadLen = 0;
		memcpy(m_recvBuf + offset, buffer, bytes);
		offset += bytes;
		
		// frame check
		if (FrameCompleted(m_recvBuf, offset, &fin, &op, &maskKey, &payloadLen, &payloadOffset))
		{
			LOGI("fin=%d op=%d mask key=0x%04x payloadlen=%u offset=%u\n", fin, op, maskKey, payloadLen, payloadOffset);
			offset = 0;

			if (NULL != m_payloadCB)
			{
				m_payloadCB(m_recvBuf + payloadOffset, payloadLen, fin, op, m_payloadUserdata);
			}
		}
	}

	m_quit = zh_true;
	LOGE("thread quit!\n");

	return quitReason;
}

zh_int32 CProtocol::SendAll(zh_char *data, zh_int32 size, zh_int32 timeout)
{
	zh_int32 bytes = 0;
	zh_int32 offset = 0;
	zh_int32 total = size;
	zh_int32 rst = ResultFailed;

	zh_uint64 deadline = zh_GetTickCount64() + static_cast<zh_uint64>(timeout);
	while (1)
	{
		if (zh_GetTickCount64() >= deadline)
			return ResultTimeout;

		if (offset == size)
		{
			rst = ResultOK;
			break;
		}

		int sendbytes = m_isocket->Send(data + offset, total);
		if (ErrSockError == sendbytes)
		{
			rst = ResultFailed;
			break;
		}
		else if (ErrSockNoData == sendbytes) 
		{
			continue;
		}

		offset += sendbytes;
		total -= sendbytes;
	}

	return rst;
}

zh_int32 CProtocol::Handshake()
{
	zh_uchar uuid[16] = {0};
	if (0 == zh_GetUUID(uuid, sizeof(uuid)))
		return ResultFailed;

	zh_uchar base64[64] = {0};
	zh_uint32 base64Len = b64_encode(uuid, sizeof(uuid), base64);

	zh_char buffer[1024] = {0};
	const zh_char *head_method = "GET / HTTP/1.1";
	const zh_char *head_host = "Host:";
	const zh_char *head_upgrade = "Upgrade:websocket";
	const zh_char *head_connection = "Connection:Upgrade";
	const zh_char *head_key = "Sec-WebSocket-Key:";
	const zh_char *head_origin = "Origin:";
	const zh_char *head_version = "Sec-WebSocket-Version:13";

	int offset = 0;
	offset += sprintf(buffer + offset, "%s\r\n", head_method);
	offset += sprintf(buffer + offset, "%s%s\r\n", head_host, m_url);
	offset += sprintf(buffer + offset, "%s\r\n", head_upgrade);
	offset += sprintf(buffer + offset, "%s\r\n", head_connection);
	offset += sprintf(buffer + offset, "%s%s\r\n", head_key, base64);
	offset += sprintf(buffer + offset, "%s%s\r\n", head_origin, m_url);
	offset += sprintf(buffer + offset, "%s\r\n\r\n", head_version);

	memset(m_base64Key, 0, sizeof(m_base64Key));
	strcpy(m_base64Key, reinterpret_cast<zh_char *>(base64));

	printf("handshake len=%d\n%s\n", strlen(buffer), buffer);
	return SendAll(buffer, strlen(buffer), 1000 * 5);
}

zh_int32 CProtocol::WaitHandshake(zh_int32 timeout)
{
	char buffer[1024 * 4] = {0};
	zh_int32 rst = 1;
	zh_uint64 time = zh_GetTickCount64() + timeout;
	while (1)
	{
		if (zh_GetTickCount64() >= time)
		{
			rst = 1;
			break;
		}

		zh_char buff[1024] = {0};
		zh_int32 offset = 0;
		zh_int32 bytes = m_isocket->Recv(buff, sizeof(buff));
		if (ErrSockClose == bytes || ErrSockError == bytes || ErrSockUnknown == bytes || ErrSockFail == bytes)
		{
			rst = 1;
			break;
		}
		else if (ErrSockNoData == bytes) 
		{
			zh_sleep(20);
			continue;
		}

		if (offset + bytes > sizeof(buffer))
		{
			rst = 1;
			break;
		}

		memcpy(buffer + offset, buff, bytes);
		offset += bytes;

		if (CHttpParser::ResponseHeaderCompleted(buffer, offset))
		{
			zh_int32 code = -1;
			HTTP_HEADER headers[16] = {0};
			m_httpParser.SetParseData(buffer, offset);
			zh_int32 size = m_httpParser.GetResponseHeaders(headers, sizeof(headers) / sizeof(HTTP_HEADER), &code);
			if (0 == size)
			{
				rst = 1;
				break;
			}

			if (HandshakeSuccess(code, headers, size))
			{
				printf("handshake response\n%s\n", buffer);
				rst = 0;
				break;
			}
		}
	}

	return rst;
}

zh_bool CProtocol::HandshakeSuccess(zh_int32 code, HTTP_HEADER *header, zh_int32 headerSize)
{
	// status code 
	if (101 != code)
		return zh_false;

	zh_bool bUpgraded = zh_false;
	zh_bool bAccept = zh_false;
	for (int i = 0; i < headerSize; i++)
	{
		if (NULL != strstr(header[i].name, "Upgrade"))
		{
			zh_char buf[16] = {0};
			zh_trim(header[i].value, buf, sizeof(buf));
			zh_strlower(buf, strlen(buf));

			// upgrade value
			if (0 == strncmp(buf, "websocket", 9))
				bUpgraded = zh_true;			
		}
		else if (NULL != strstr(header[i].name, "Sec-WebSocket-Accept"))
		{
			zh_char buf[128] = {0};
			zh_trim(header[i].value, buf, sizeof(buf));

			// Sec-WebSocket-Accept
			zh_char check[128] = {0};
			SecWebSocketAccept(m_base64Key, strlen(m_base64Key), check, sizeof(check));

			if ((0 == strncmp(buf, check, strlen(check))) && (strlen(check) == strlen(buf)))
				bAccept = zh_true;
		}
	}

	return bUpgraded && bAccept;
}

zh_int32 CProtocol::SecWebSocketAccept(const zh_char *key, zh_int32 KeySize, zh_char *value, zh_int32 valueSize)
{
	const zh_char *magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	zh_char src[512] = {0};
	memcpy(src, key, KeySize);
	memcpy(src + KeySize, magic, strlen(magic));

	zh_uchar sha1[SHA1_BLOCK_SIZE] = {0};
	SHA1_CTX ctx;
	sha1_init(&ctx);
	sha1_update(&ctx, reinterpret_cast<zh_uchar *>(src), strlen(src));
	sha1_final(&ctx, sha1);

	zh_uchar base64[128] = {0};
	zh_uint32 size = b64_encode(sha1, sizeof(sha1), base64);
	
	zh_int32 copysize = valueSize < static_cast<int>(size) ? valueSize - 1 : static_cast<int>(size);
	memcpy(value, base64, copysize);
	return copysize;
}

zh_bool CProtocol::FrameCompleted(zh_char *data, zh_uint32 size, zh_int32 *fin, zh_int32 *opcode, zh_int32 *maskKey, zh_uint32 *payloadLen, zh_uint32 *payloadOffset)
{
	if (size <= sizeof(FRAME_HEAD))
		return zh_false;

	zh_int32 payloadField = 0;
	zh_int32 maskKeyField = 0;

	// parse length
	FRAME_HEAD *frameHead = reinterpret_cast<FRAME_HEAD *>(data);
	zh_ushort len = static_cast<zh_ushort>(frameHead->payloadlen);
	if (126 == len)
	{
		if (size < 4)
			return zh_false;

		zh_ushort *p = reinterpret_cast<zh_ushort *>(data + 2);
		len = ntohs(*p);
		payloadField += 2;
	} 
	else if (127 == len)
	{
		payloadField += 8;
	}

	if (frameHead->mask) maskKeyField = 4;
	if (size < sizeof(FRAME_HEAD) + payloadField + maskKeyField)
		return false;

	if (NULL != payloadOffset) *payloadOffset = static_cast<zh_uint32>(sizeof(FRAME_HEAD) + payloadField + maskKeyField);
	if (NULL != fin) *fin = static_cast<zh_int32>(frameHead->fin);
	if (NULL != opcode) *opcode = static_cast<zh_int32>(frameHead->opcode);
	if (NULL != payloadLen) *payloadLen = static_cast<zh_uint32>(len);
	if (frameHead->mask > 0 && NULL != maskKey)
		memcpy(maskKey, data + 2 + payloadField, 4);

	return true;
}

zh_void CProtocol::SetPayloadCB(PayloadCallback cb, zh_void *userdata)
{
	m_payloadCB = cb;
	m_payloadUserdata = userdata;
}

zh_void CProtocol::SetDisconnectCB(DisconnectCallback cb, zh_void *userdata)
{
	m_disconnectCB = cb;
	m_disconnectUserdata = userdata;
}

zh_int32 CProtocol::SendFrame(const zh_char *data, zh_uint32 size, Opcode op, zh_int32 fin, zh_int32 timeout)
{
	const zh_int32 maxSendSize = MAX_SEND_ONCE;
	if (size > maxSendSize)
		return ResultExceedMax;

	zh_int32 sendsize = 0;
	FRAME_HEAD *head = reinterpret_cast<FRAME_HEAD *>(m_sendBuf);
	head->fin = fin;
	head->rsv1 = 0;
	head->rsv2 = 0;
	head->rsv3 = 0;
	head->mask = 1; // must be 1
	head->opcode = op;
	sendsize += sizeof(FRAME_HEAD);

	if (size <= 125)
	{
		head->payloadlen = size;
	}
	else
	{
		zh_ushort extendLen = 0;
		head->payloadlen = 126;

		if (size > maxSendSize)
			extendLen = maxSendSize;
		else
			extendLen = static_cast<zh_ushort>(size);

		extendLen = htons(extendLen);
		memcpy(m_sendBuf + sendsize, &extendLen, 2);

		// extend payload size
		sendsize += 2;
	}


	zh_uchar rnd[4] = {0};
	zh_GetUUID(rnd, 4);
	memcpy(m_sendBuf + sendsize, rnd, 4);
	sendsize +=4;

	zh_char *payload = m_sendBuf + sendsize;
	for (int i = 0; i < size; i++)
	{
		int j = i % 4;
		payload[i] = data[i] ^ rnd[j];
	}

	sendsize += size;

	return SendAll(m_sendBuf, sendsize, timeout);
}

zh_int32 CProtocol::SendPing(const zh_char *data, zh_uint32 size, zh_int32 timeout)
{
	return SendFrame(data, size, OP_PingFrame, 1, timeout);
}