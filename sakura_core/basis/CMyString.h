#pragma once

#include <string>

#define m_delete2(p) do{ if(p){ delete[] p; p=0; } }while(0)

class CMyString{
public:
	//�R���X�g���N�^�E�f�X�g���N�^
	CMyString(WCHAR wc)								: m_wstr(1,wc),          m_str_cache(NULL) { }
	CMyString(const WCHAR* szData=L"")				: m_wstr(szData),        m_str_cache(NULL) { }
	CMyString(const WCHAR* pData, size_t nLength)	: m_wstr(pData,nLength), m_str_cache(NULL) { }
	CMyString(const ACHAR* szData)					: m_wstr(L""), m_str_cache(NULL) { set(szData); }
	CMyString(const ACHAR* pData, size_t nLength)	: m_wstr(L""), m_str_cache(NULL) { set(pData,nLength); }
	CMyString(ACHAR wc)								: m_wstr(L""), m_str_cache(NULL) { ACHAR buf[2]={wc,0}; set(buf); }
	CMyString(const CMyString& rhs) : m_wstr(rhs.c_wstr()), m_str_cache(NULL) { }
	~CMyString();

	//���Z�q
	operator const wchar_t* () const{ return c_wstr(); }
	operator const char* () const{ return c_astr(); }
	CMyString& operator = (const CMyString& rhs){ set(rhs); return *this; }

	//�ݒ�
	void set(const wchar_t* wszData){ m_wstr=wszData; m_delete2(m_str_cache); }
	void set(const char* szData);
	void set(const char* szData, int nLength);
	void set(const CMyString& cszData){ set(cszData.c_wstr()); }

	//�擾
	const wchar_t* c_wstr() const{ return m_wstr.c_str(); }
	const char* c_astr() const;
	int wlength() const{ return wcslen(c_wstr()); }
	int alength() const{ return strlen(c_astr()); }


	//TCHAR
#ifdef _UNICODE
	const TCHAR* c_tstr() const{ return c_wstr(); }
#else
	const TCHAR* c_tstr() const{ return c_astr(); }
#endif

private:
	std::wstring m_wstr;
	mutable char* m_str_cache; //c_str�p�L���b�V���Bm_wstr���ύX(set)���ꂽ�炱���������ANULL�ɂ��Ă����̂����[���B
};

#include "CFilePath.h"


//std::string �� TCHAR �Ή��p�}�N����`
#ifdef _UNICODE
#define tstring wstring
#else
#define tstring string
#endif
#define astring string
