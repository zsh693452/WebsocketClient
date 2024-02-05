#ifndef _HTTP_H_6C72F18A927D4FB38F9242E40DA6501F
#define _HTTP_H_6C72F18A927D4FB38F9242E40DA6501F

#include "datatype.h"

#define MAX_HTTP_PARSER_BUFF_SIZE 1024 * 4

typedef struct _HTTP_HEADER_
{
	zh_char name[64];
	zh_char value[128];
} HTTP_HEADER, *PHTTP_HEADER;

class CHttpParser
{
public:
	CHttpParser();
	~CHttpParser();

	static zh_bool ResponseHeaderCompleted(const zh_char *data, zh_int32 size);
	
	zh_void SetParseData(const zh_char *data, zh_int32 size);
	zh_int32 GetResponseHeaders(HTTP_HEADER *header, zh_int32 headerSize, zh_int32 *statusCode);
	

private:
	zh_int32 ReadNextLine(zh_char *data, zh_int32 size);
	zh_int32 GetStatusCode(const zh_char *line, zh_int32 lineSize);
	zh_void GetOneHeader(const zh_char *line, zh_int32 lineSize, HTTP_HEADER *header);

private:
	zh_char *m_buffer;
	zh_char *m_pread;
	zh_int32 m_readPos;
	zh_int32 m_bufferSize;
};

#endif

