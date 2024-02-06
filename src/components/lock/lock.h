#ifndef _LOCK_H_1058290B69054175975C0A498516B2B5
#define _LOCK_H_1058290B69054175975C0A498516B2B5

#include "datatype.h"

class CLock
{
public:
	CLock();
	CLock(zh_mutex *mutex);
	~CLock();

	zh_void Lock();
	zh_void Unlock();

private:
	zh_mutex *m_mutex;
};

#endif
