#ifndef _TCP_H_FE14377076094CD5AA11ED0115C4D000
#define _TCP_H_FE14377076094CD5AA11ED0115C4D000

#include "datatype.h"
#include "ISocket.h"

class CTCP : public ISocket
{
public:
	CTCP();
	~CTCP();

	zh_int32 CreateSocket(zh_int32 family);
	zh_int32 Connect(const zh_char *url, zh_ushort port, zh_int32 family, zh_int32 timeout);
	zh_int32 Close();
	zh_int32 Send(zh_char *data, zh_int32 size);
	zh_int32 Recv(zh_char *data, zh_int32 size);

protected:
	zh_int32 AsyncConnect(const zh_char *url, zh_ushort port, zh_int32 family, zh_int32 timeout);

private:
	zh_int32 m_sock;
	fd_set m_fdw;
	fd_set m_fdr;
};

#endif
