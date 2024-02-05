#include "tcp.h"
#include "funcCommon.h"
#include "internaldef.h"
#include <errno.h>


CTCP::CTCP()
{
	m_sock = -1;
}

CTCP::~CTCP()
{

}

zh_int32 CTCP::CreateSocket(zh_int32 family)
{
	// 0 1 2 file description is use by system
	m_sock = zh_socket(family, SOCK_STREAM, 0);
	while(m_sock <= 2)
		m_sock = zh_socket(family, SOCK_STREAM, 0);

	zh_uint32 ul = 1;
	zh_ioctlsocket(m_sock, FIONBIO, &ul);

	return m_sock;
}

zh_int32 CTCP::Connect(const zh_char *url, zh_ushort port, zh_int32 family, zh_int32 timeout)
{
	zh_int32 rst = AsyncConnect(url, port, family, timeout);
	if (ErrSockSuccess != rst)
	{
		return rst;
	}
	return ErrSockSuccess;
}

zh_int32 CTCP::Close()
{
	if (-1 != m_sock && 0 != m_sock && 1 != m_sock && 2 != m_sock)
	{
		zh_closesocket(m_sock);
		m_sock = -1;
	}

	return 0;
}

zh_int32 CTCP::AsyncConnect(const zh_char *url, zh_ushort port, zh_int32 family, zh_int32 timeout)
{
	sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = family;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = inet_addr(url);

	zh_uint64 endtime = zh_GetTickCount64() + static_cast<zh_uint64>(timeout);
	if (-1 == zh_connect(m_sock, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)))
	{
		int times = 300;
		while (true)
		{
			if (zh_GetTickCount64() > endtime)
			{
				printf("connect timeout\n");
				return ErrSockTimeout;
			}

			struct timeval tv;
			tv.tv_sec = 0;
			tv.tv_usec = 100000;

			FD_ZERO(&m_fdw);
			FD_SET(m_sock, &m_fdw);
			int rst = zh_select(m_sock + 1, NULL, &m_fdw, NULL, &tv);
			if(rst > 0) 
			{
				zh_int32 error = 1;
				int len = sizeof(zh_int32);
				if (0 != zh_getsockopt(m_sock, SOL_SOCKET, SO_ERROR, (char *)&error, &len))
				{
					printf("connect ErrSockFail\n");
					return ErrSockFail;
				}

				if (error != 0) 
				{
					if(!(error == EINTR || error == EAGAIN) )
					{
						printf("connect ErrSockFail 1\n");
						return ErrSockFail;
					}			
				}
				else
				{
					printf("connect break\n");
					break;
				}
			}
		}		
	}

	return ErrSockSuccess;	
}

zh_int32 CTCP::Send(zh_char *data, zh_int32 size)
{
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 30000;

	FD_ZERO(&m_fdw);
	FD_SET(m_sock, &m_fdw);

	zh_int32 rst = select(m_sock + 1, NULL, &m_fdw, NULL, &tv);
	if (rst > 0)
	{
		if (!FD_ISSET(m_sock, &m_fdw)) 
		{
			return ErrSockNoData;
		}

		return send(m_sock, data, size, 0);
	}
	else if (0 == rst)
	{
		return ErrSockNoData;
	}
	else
	{
		return ErrSockError;
	}
	
}

zh_int32 CTCP::Recv(zh_char *data, zh_int32 size)
{
	FD_ZERO(&m_fdr);
	FD_SET(m_sock, &m_fdr);
	
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 30000;

	zh_int32 rst = select(m_sock + 1, &m_fdr, NULL, NULL, &tv);
	if(rst > 0) 
	{
		zh_int32 bytes = recv(m_sock, data, size, 0);
		if (bytes < 0)
		{
			printf("recv bytes %d errno=%d\n", errno);
			return ErrSockUnknown;
		}
		if (0 == bytes)
			return ErrSockClose;

		return bytes;
	}
	else if (rst < 0) 
	{
		return ErrSockError;
	}
	else 
	{
		return ErrSockNoData;
	}
}