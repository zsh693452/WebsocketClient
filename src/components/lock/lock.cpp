#include "lock.h"
#include "funcCommon.h"

CLock::CLock()
{
	m_mutex = NULL;
}

CLock::CLock(zh_mutex *mutex)
{
	m_mutex = mutex;
	zh_InitMutex(m_mutex);
}

CLock::~CLock()
{
	if (NULL != m_mutex)
	{
		zh_ReleaseMutex(m_mutex);
	}
}

zh_void CLock::Lock()
{
	zh_LockMutex(m_mutex);
}

zh_void CLock::Unlock()
{
	zh_UnlockMutex(m_mutex);
}