#ifndef _AUTOLOCK_H_AF8EF13CCAF947D1A4043C29BC47AC04
#define _AUTOLOCK_H_AF8EF13CCAF947D1A4043C29BC47AC04

#include "datatype.h"

class CAutoLock
{
public:
	CAutoLock();
	CAutoLock(zh_mutex *mutex);
	~CAutoLock();

private:
	zh_mutex *m_mutex;
};

#endif
