#include "FileAccess.h"

CFileAccess::CFileAccess()
: m_Sync()
{

}

CFileAccess::~CFileAccess()
{

}

void CFileAccess::BeginRead()
{
	m_Sync.BeginRead();
}
void CFileAccess::EndRead()
{
	m_Sync.EndRead();
}
bool CFileAccess::BeginWrite()
{
	return m_Sync.BeginWrite();
}
void CFileAccess::EndWrite()
{
	m_Sync.EndWrite();
}