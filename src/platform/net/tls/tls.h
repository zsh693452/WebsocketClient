#ifndef _TLS_H_C4FBD99F46DD482DB8D7B5FAC30C94AD
#define _TLS_H_C4FBD99F46DD482DB8D7B5FAC30C94AD

#include "datatype.h"
#include "ISocket.h"

class CTLS : public ISocket
{
public:
	CTLS();
	~CTLS();

	zh_int32 CreateSocket(zh_int32 family);
	zh_int32 Connect(const zh_char *url, zh_ushort port, zh_int32 family, zh_int32 timeout);
	zh_int32 Close();
	zh_int32 Send(zh_char *data, zh_int32 size);
	zh_int32 Recv(zh_char *data, zh_int32 size);

protected:
	zh_int32 AsyncConnect(const zh_char *url, zh_ushort port, zh_int32 family, zh_int32 timeout);

private:
	void *m_ssl;
	void *m_ctx;
	zh_int32 m_sock;
	fd_set m_fdw;
	fd_set m_fdr;
};

#endif
