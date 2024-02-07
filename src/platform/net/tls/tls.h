#ifndef _TLS_H_C4FBD99F46DD482DB8D7B5FAC30C94AD
#define _TLS_H_C4FBD99F46DD482DB8D7B5FAC30C94AD

#include "datatype.h"
#include "ISocket.h"

class CTLS : public ISocket
{
public:
	CTLS(zh_mutex *mutex);
	~CTLS();

	zh_int32 CreateSocket(zh_int32 family);
	zh_int32 Connect(const zh_char *url, zh_ushort port, zh_int32 family, zh_int32 timeout);
	zh_int32 Close();
	zh_int32 Send(zh_char *data, zh_int32 size);
	zh_int32 Recv(zh_char *data, zh_int32 size);

protected:
	zh_int32 AsyncConnect(const zh_char *url, zh_ushort port, zh_int32 family, zh_int32 timeout);

private:
	zh_void SSLInit();

private:
	void *m_ssl;
	void *m_ctx;
	zh_int32 m_sock;
	fd_set m_fdw;
	fd_set m_fdr;

	// for non-blocking, if SSL_Read or SSL_Write return <=0 
	// and error code is SSL_ERROR_WANT_READ or SSL_ERROR_WANT_WRITE, 
	// we should call SSL_Read or SSL_Write until reading completed 
	// or writing completed  
	zh_mutex *m_mutex;
	zh_bool m_bReadable;
	zh_bool m_bWritable;
};

#endif
