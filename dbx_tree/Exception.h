#pragma once

#define throwException(Format, ...) throw CException(__FILE__, __LINE__, __FUNCTION__, Format, __VA_ARGS__)

class CException
{
private:
	char * m_Message;
	char * m_File;
	int    m_Line;
	char * m_Function;

	int    m_SysError;
	char * m_SysMessage;

public:
	CException(CException & Other);
	CException(char * File, int Line, char * Function, char * Format, ...);
	~CException();

	void ShowMessage();
};