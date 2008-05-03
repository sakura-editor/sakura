#include "stdafx.h"
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
	wchar_t* wszData=mbstowcs_new(szData,nLength);
	set(wszData);
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
	//�����_�ł�NULL���󂯕t���Ȃ�
	assert(rhs);
	_mbstotcs(m_tszPath, _countof(m_tszPath), rhs);
}
CFilePath::CFilePath(const wchar_t* rhs)
{
	//�����_�ł�NULL���󂯕t���Ȃ�
	assert(rhs);
	_wcstotcs(m_tszPath, _countof(m_tszPath), rhs);
}
*/
