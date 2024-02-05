#include "http.h"
#include <stdlib.h>

CHttpParser::CHttpParser()
{
	m_buffer = new zh_char[MAX_HTTP_PARSER_BUFF_SIZE];
	memset(m_buffer, 0, MAX_HTTP_PARSER_BUFF_SIZE);
	m_pread = m_buffer;
	m_bufferSize = MAX_HTTP_PARSER_BUFF_SIZE;
	m_readPos = 0;
}


CHttpParser::~CHttpParser()
{
	if (NULL != m_buffer)
	{
		delete[] m_buffer;
		m_buffer = NULL;
	}
}

zh_bool CHttpParser::ResponseHeaderCompleted(const zh_char *data, zh_int32 size)
{
	if (NULL == strstr(data, "\r\n\r\n"))
		return zh_false;

	return zh_true;
}

zh_int32 CHttpParser::ReadNextLine(zh_char *data, zh_int32 size)
{
	zh_int32 index = 0;

	for (;;)
	{
		// arrive max buffer size
		if (m_readPos >= m_bufferSize)
			break;

		// arrive max input buffer size
		if (index >= size)
			break;

		// all ++
		*(data + index) = *m_pread;
		m_pread++;
		m_readPos++;
		index++;

		if (m_readPos >= 2)
		{
			if ('\r' == *(m_pread - 2) && '\n' == *(m_pread - 1))
				break;
		}
	}

	return index;
}

zh_void CHttpParser::SetParseData(const zh_char *data, zh_int32 size)
{
	memcpy(m_buffer, data, size);

	m_readPos = 0;
	m_pread = m_buffer;
	m_bufferSize = size;
}

zh_int32 CHttpParser::GetResponseHeaders(HTTP_HEADER *header, zh_int32 headerSize, zh_int32 *statusCode)
{
	// first line 
	char line[512] = {0};
	zh_int32 lineSize = ReadNextLine(line, sizeof(line));
	if (0 == lineSize)
		return 0;

	// status code
	zh_int32 code = GetStatusCode(line, lineSize);
	if (NULL != statusCode) *statusCode = code;

	// other headers
	zh_int32 counter = 0;
	for (;;)
	{
		if (headerSize <= counter)
			break;

		memset(line, 0, sizeof(line));
		zh_int32 lineSize = ReadNextLine(line, sizeof(line));
		if (0 == lineSize)
			break;

		GetOneHeader(line, lineSize, header + counter);
		counter++;
	}

	return counter;
}


zh_int32 CHttpParser::GetStatusCode(const zh_char *line, zh_int32 lineSize)
{
	zh_int32 statusCode = -1;
	if (lineSize < 7) // 'HTTP/' + '\r\n'
		return statusCode;

	if ('H' == line[0] && 'T' == line[1] && 'T' == line[2] && 'P' == line[3] && '/' == line[4])
	{
		zh_bool bStartCopy = zh_false;
		zh_char szStatus[16] = {0};
		zh_int32 index = 0;
		for (zh_int32 i = 6; i < lineSize; i++)
		{
			if (' ' == line[i - 1] && (line[i] >= '0' && line[i] <= '9'))
				bStartCopy = zh_true;

			if (bStartCopy)
			{
				if (' ' == line[i])
				{
					statusCode = atoi(szStatus);
					break;
				}

				szStatus[index++] = line[i];
			}
		}
	}

	return statusCode;
}

zh_void CHttpParser::GetOneHeader(const zh_char *line, zh_int32 lineSize, HTTP_HEADER *header)
{
	zh_int32 i = 0;
	zh_int32 index = 0;
	zh_int32 maxNameSize = sizeof(header->name);
	zh_int32 maxValueSize = sizeof(header->value);

	for (i = 0; i < lineSize; i++)
	{
		if (':' == line[i] || ' ' == line[i])
			break;

		if (i >= maxNameSize)
			break;

		header->name[index++] = line[i];
	}

	index = 0;
	zh_bool bStart = zh_false;
	for (; i < lineSize; i++)
	{
		if (' ' != line[i] && ':' != line[i])
			bStart = zh_true;

		if (bStart)
		{
			if (i >= maxValueSize)
				break;

			if (i + 1 <= maxValueSize)
			{
				if ('\r' == line[i] && '\n' == line[i + 1])
					break;
			}

			header->value[index++] = line[i];
		}
	}
}