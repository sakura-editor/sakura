#include "StdAfx.h"
#include "CMyString.h"
#include "charset/charcode.h"
#include "charset/CharPointer.h"

void CMyString::set(const char* szData)
{
	wchar_t* wszData=mbstowcs_new(szData);
	set(wszData);
	delete[] wszData;
}

void CMyString::set(const char* szData, int nLength)
{
	int nLen;
	wchar_t* wszData=mbstowcs_new(szData,nLength,&nLen);
	set(wszData,nLen);
	delete[] wszData;
}

const char* CMyString::c_astr() const
{
	if(!m_str_cache)
		m_str_cache=wcstombs_new(m_wstr.c_str());
	return m_str_cache;
}

CMyString::~CMyString()
{
	m_delete2(m_str_cache);
}

/*
CFilePath::CFilePath(const char* rhs)
{
	//現時点ではNULLを受け付けない
	assert(rhs);
	_mbstotcs(m_tszPath, _countof(m_tszPath), rhs);
}
CFilePath::CFilePath(const wchar_t* rhs)
{
	//現時点ではNULLを受け付けない
	assert(rhs);
	_wcstotcs(m_tszPath, _countof(m_tszPath), rhs);
}
*/
