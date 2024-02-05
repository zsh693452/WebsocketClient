#ifndef _ISOCKET_H_2936AC6A040648E7B0F7D83A09083848
#define _ISOCKET_H_2936AC6A040648E7B0F7D83A09083848

#include "datatype.h"

class ISocket
{
public:
	ISocket(){};
	virtual ~ISocket(){};
	virtual zh_int32 CreateSocket(zh_int32 family) = 0;
	virtual zh_int32 Connect(const zh_char *url, zh_ushort port, zh_int32 family, zh_int32 timeout) = 0;
	virtual zh_int32 Close() = 0;
	virtual zh_int32 Send(zh_char *data, zh_int32 size) = 0;
	virtual zh_int32 Recv(zh_char *data, zh_int32 size) = 0;
};

#endif

