#ifndef _INTERNALDEF_H_780FE8256F3348BFB0A30FA54ECE7A05
#define _INTERNALDEF_H_780FE8256F3348BFB0A30FA54ECE7A05


typedef enum
{
	ErrSockSuccess = 0,
	ErrSockTimeout = -2,
	ErrSockFail = -3,
	ErrSSLConnectFail = -4,
	ErrSockNoData = -5,
	ErrSockError = -6,
	ErrSockClose = -7,
	ErrSockUnknown = -8
} SocketError;


#define LOGI(format, ...) printf(format, ##__VA_ARGS__)
#define LOGW(format, ...) printf(format, ##__VA_ARGS__)
#define LOGE(format, ...) printf(format, ##__VA_ARGS__)

#endif
