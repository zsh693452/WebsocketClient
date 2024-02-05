#include "tls.h"
#include "funcCommon.h"
#include "internaldef.h"
#include "openssl/ssl.h"
#include "openssl/err.h"

CTLS::CTLS()
{
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	ERR_load_crypto_strings();
	SSL_load_error_strings();

	SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
	SSL *ssl = SSL_new(ctx);
	m_ctx = static_cast<void *>(ctx);
	m_ssl = static_cast<void *>(ssl);
	m_sock = -1;
}

CTLS::~CTLS()
{

}

zh_int32 CTLS::CreateSocket(zh_int32 family)
{
	// 0 1 2 file description is use by system
	m_sock = zh_socket(family, SOCK_STREAM, 0);
	while(m_sock <= 2)
		m_sock = zh_socket(family, SOCK_STREAM, 0);

	zh_uint32 ul = 1;
	zh_ioctlsocket(m_sock, FIONBIO, &ul);

	return m_sock;
}

zh_int32 CTLS::Connect(const zh_char *url, zh_ushort port, zh_int32 family, zh_int32 timeout)
{
	zh_int32 rst = AsyncConnect(url, port, family, timeout);
	if (ErrSockSuccess != rst)
	{
		return rst;
	}
	SSL_set_fd(static_cast<SSL *>(m_ssl), m_sock);

	while (1)
	{
		rst = SSL_connect(static_cast<SSL *>(m_ssl));
		if (1 != rst)
		{
			zh_int32 error = SSL_get_error(static_cast<SSL *>(m_ssl), rst);
			if (SSL_ERROR_WANT_READ == error || SSL_ERROR_WANT_WRITE == error)
			{
				Sleep(100);
				continue;
			}

			return ErrSSLConnectFail;
		}
		else
		{
			break;
		}
	}
	
	return ErrSockSuccess;
}

zh_int32 CTLS::Close()
{
	if (-1 != m_sock && 0 != m_sock && 1 != m_sock && 2 != m_sock)
	{
		zh_closesocket(m_sock);
		m_sock = -1;
	}

	if (NULL != m_ssl)
	{
		SSL_free(static_cast<SSL *>(m_ssl));
		m_ssl = NULL;
	}
	
	if (NULL != m_ctx)
	{
		SSL_CTX_free(static_cast<SSL_CTX *>(m_ctx));
		m_ctx = NULL;
	}

	return 0;
}

zh_int32 CTLS::AsyncConnect(const zh_char *url, zh_ushort port, zh_int32 family, zh_int32 timeout)
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
					return ErrSockFail;
				}

				if (error != 0) 
				{
					if(!(error == EINTR || error == EAGAIN) )
					{
						return ErrSockFail;
					}			
				}
				else
				{
					break;
				}
			}
		}		
	}

	return ErrSockSuccess;	
}

zh_int32 CTLS::Send(zh_char *data, zh_int32 size)
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

		return SSL_write(static_cast<SSL *>(m_ssl), data, size);
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

zh_int32 CTLS::Recv(zh_char *data, zh_int32 size)
{
	FD_ZERO(&m_fdr);
	FD_SET(m_sock, &m_fdr);
	
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 30000;

	zh_int32 rst = select(m_sock + 1, &m_fdr, NULL, NULL, &tv);
	if(rst > 0) 
	{
		zh_int32 bytes = SSL_read(static_cast<SSL *>(m_ssl), data, size);
		if (bytes <= 0) 
		{
			zh_int32 errcode = SSL_get_error(static_cast<SSL *>(m_ssl), bytes);
			if (SSL_ERROR_WANT_READ == errcode)
			{
				return ErrSockNoData;	
			}
			else if (SSL_ERROR_ZERO_RETURN == errcode)
			{
				return ErrSockClose;
			}

			printf("recv errcode=%d errno=%d\n", errcode, errno);
			return ErrSockUnknown;
		}

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