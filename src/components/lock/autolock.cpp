#include "autolock.h"
#include "funcCommon.h"

CAutoLock::CAutoLock()
{
	m_mutex = NULL;
}

CAutoLock::CAutoLock(zh_mutex *mutex)
{
	m_mutex = mutex;
	zh_LockMutex(m_mutex);
}

CAutoLock::~CAutoLock()
{
	if (NULL != m_mutex)
	{
		zh_UnlockMutex(m_mutex);
	}
}