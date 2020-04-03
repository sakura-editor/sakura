/*! @file */
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
#pragma once

#include <string>
#include "util/string_ex.h"
#include "util/StaticType.h"

#define m_delete2(p) { if(p){ delete[] p; p=0; } }

#define astring string

//共通マクロ
#define _FT _T

//共通型
typedef StaticString<WCHAR,_MAX_PATH> SFilePath;
typedef StaticString<WCHAR, MAX_GREP_PATH> SFilePathLong;
class CFilePath : public StaticString<WCHAR,_MAX_PATH>{
private:
	typedef StaticString<WCHAR,_MAX_PATH> Super;
public:
	CFilePath() : Super() { }
	CFilePath(const WCHAR* rhs) : Super(rhs) { }

	bool IsValidPath() const{ return At(0)!=L'\0'; }
	std::wstring GetDirPath() const
	{
		WCHAR	szDirPath[_MAX_PATH];
		WCHAR	szDrive[_MAX_DRIVE];
		WCHAR	szDir[_MAX_DIR];
		_wsplitpath( this->c_str(), szDrive, szDir, NULL, NULL );
		wcscpy( szDirPath, szDrive);
		wcscat( szDirPath, szDir );
		return szDirPath;
	}
	//拡張子を取得する
	LPCWSTR GetExt( bool bWithoutDot = false ) const
	{
		const WCHAR* head = c_str();
		const WCHAR* p = wcschr(head,L'\0') - 1;
		while(p>=head){
			if(*p==L'.')break;
			if(*p==L'\\')break;
			if(*p==L'/')break;
			p--;
		}
		if(p>=head && *p==L'.'){
			return bWithoutDot ? p+1 : p;	//bWithoutDot==trueならドットなしを返す
		}else{
			return wcschr(head,L'\0');
		}
	}
};

//$$ 仮
class CCommandLineString{
public:
	CCommandLineString()
	{
		m_szCmdLine[0] = L'\0';
		m_pHead = m_szCmdLine;
	}
	void AppendF(const WCHAR* szFormat, ...)
	{
		va_list v;
		va_start(v,szFormat);
		m_pHead+=auto_vsprintf_s(m_pHead,_countof(m_szCmdLine)-(m_pHead-m_szCmdLine),szFormat,v);
		va_end(v);
	}
	const WCHAR* c_str() const
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
	WCHAR	m_szCmdLine[1024];
	WCHAR*	m_pHead;
};
