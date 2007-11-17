#pragma once

#include <string>

#define m_delete2(p) do{ if(p){ delete[] p; p=0; } }while(0)

class CMyString{
public:
	//コンストラクタ・デストラクタ
	CMyString(WCHAR wc)								: m_wstr(1,wc),          m_str_cache(NULL) { }
	CMyString(const WCHAR* szData=L"")				: m_wstr(szData),        m_str_cache(NULL) { }
	CMyString(const WCHAR* pData, size_t nLength)	: m_wstr(pData,nLength), m_str_cache(NULL) { }
	CMyString(const ACHAR* szData)					: m_wstr(L""), m_str_cache(NULL) { set(szData); }
	CMyString(const ACHAR* pData, size_t nLength)	: m_wstr(L""), m_str_cache(NULL) { set(pData,nLength); }
	CMyString(ACHAR wc)								: m_wstr(L""), m_str_cache(NULL) { ACHAR buf[2]={wc,0}; set(buf); }
	CMyString(const CMyString& rhs) : m_wstr(rhs.c_wstr()), m_str_cache(NULL) { }
	~CMyString();

	//演算子
	operator const wchar_t* () const{ return c_wstr(); }
	operator const char* () const{ return c_astr(); }
	CMyString& operator = (const CMyString& rhs){ set(rhs); return *this; }

	//設定
	void set(const wchar_t* wszData){ m_wstr=wszData; m_delete2(m_str_cache); }
	void set(const char* szData);
	void set(const char* szData, int nLength);
	void set(const CMyString& cszData){ set(cszData.c_wstr()); }

	//取得
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
	mutable char* m_str_cache; //c_str用キャッシュ。m_wstrが変更(set)されたらこれを解放し、NULLにしておくのがルール。
};

#include "CFilePath.h"


//std::string の TCHAR 対応用マクロ定義
#ifdef _UNICODE
#define tstring wstring
#else
#define tstring string
#endif
#define astring string
