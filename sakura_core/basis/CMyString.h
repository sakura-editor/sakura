/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#ifndef SAKURA_CMYSTRING_43EB58CF_8782_43AC_AC82_A22DFC99E063_H_
#define SAKURA_CMYSTRING_43EB58CF_8782_43AC_AC82_A22DFC99E063_H_

#include <string>
#include "util/string_ex.h"

#define m_delete2(p) { if(p){ delete[] p; p=0; } }

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
	void set(const wchar_t* wszData, int nLength){ m_wstr.assign(wszData, nLength); m_delete2(m_str_cache); }
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

//std::string の TCHAR 対応用マクロ定義
#ifdef _UNICODE
#define tstring wstring
#else
#define tstring string
#endif
#define astring string



//共通マクロ
#define _FT _T
#include "util/StaticType.h"

//共通型
typedef StaticString<TCHAR,_MAX_PATH> SFilePath;
typedef StaticString<TCHAR, MAX_GREP_PATH> SFilePathLong;
class CFilePath : public StaticString<TCHAR,_MAX_PATH>{
private:
	typedef StaticString<TCHAR,_MAX_PATH> Super;
public:
	CFilePath() : Super() { }
	CFilePath(const TCHAR* rhs) : Super(rhs) { }

	bool IsValidPath() const{ return At(0)!=_T('\0'); }
	std::tstring GetDirPath() const
	{
		TCHAR	szDirPath[_MAX_PATH];
		TCHAR	szDrive[_MAX_DRIVE];
		TCHAR	szDir[_MAX_DIR];
		_tsplitpath( this->c_str(), szDrive, szDir, NULL, NULL );
		_tcscpy( szDirPath, szDrive);
		_tcscat( szDirPath, szDir );
		return szDirPath;
	}
	//拡張子を取得する
	LPCTSTR GetExt( bool bWithoutDot = false ) const
	{
		const TCHAR* head = c_str();
		const TCHAR* p = auto_strchr(head,_T('\0')) - 1;
		while(p>=head){
			if(*p==_T('.'))break;
			if(*p==_T('\\'))break;
			if(*p==_T('/'))break;
			p--;
		}
		if(p>=head && *p==_T('.')){
			return bWithoutDot ? p+1 : p;	//bWithoutDot==trueならドットなしを返す
		}else{
			return auto_strchr(head,_T('\0'));
		}
	}
};


//$$ 仮
class CCommandLineString{
public:
	CCommandLineString()
	{
		m_szCmdLine[0] = _T('\0');
		m_pHead = m_szCmdLine;
	}
	void AppendF(const TCHAR* szFormat, ...)
	{
		va_list v;
		va_start(v,szFormat);
		m_pHead+=auto_vsprintf_s(m_pHead,_countof(m_szCmdLine)-(m_pHead-m_szCmdLine),szFormat,v);
		va_end(v);
	}
	const TCHAR* c_str() const
	{
		return m_szCmdLine;
	}
	size_t size() const
	{
		return m_pHead - m_szCmdLine;
	}
	size_t max_size() const
	{
		return _countof(m_szCmdLine) - 1;
	}
private:
	TCHAR	m_szCmdLine[1024];
	TCHAR*	m_pHead;
};

#endif /* SAKURA_CMYSTRING_43EB58CF_8782_43AC_AC82_A22DFC99E063_H_ */
/*[EOF]*/
