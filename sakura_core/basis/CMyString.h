/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CMYSTRING_009A2525_6B06_4C1B_B089_C1B8A424A565_H_
#define SAKURA_CMYSTRING_009A2525_6B06_4C1B_B089_C1B8A424A565_H_
#pragma once

#include <string>
#include "util/string_ex.h"
#include "util/StaticType.h"
#include "config/maxdata.h"

//共通型
using SFilePath = StaticString<_MAX_PATH>;
using SFilePathLong = StaticString<MAX_GREP_PATH>;
class CFilePath : public StaticString<_MAX_PATH>{
private:
	using Super = StaticString<_MAX_PATH>;
public:
	CFilePath() = default;
	CFilePath(const WCHAR* rhs) : Super(rhs) { }

	[[nodiscard]] bool IsValidPath() const{ return At(0)!=L'\0'; }
	[[nodiscard]] std::wstring GetDirPath() const
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
	[[nodiscard]] LPCWSTR GetExt( bool bWithoutDot = false ) const
	{
		// 文字列の末尾アドレスを取得
		const WCHAR* tail = c_str() + Length();

		// 文字列末尾から逆方向に L'.' を検索
		if (const auto *p = ::wcsrchr(c_str(), L'.')) {
			// L'.'で始まる文字列がパス区切りを含まない場合のみ「拡張子あり」と看做す
			if (const bool hasExt = !::wcspbrk(p, L"\\/"); hasExt && !bWithoutDot) {
				return p;
			}
			else if (hasExt && p < tail) {
				return p + 1;		//bWithoutDot==trueならドットなしを返す
			}
		}

		// 文字列末尾のアドレスを返す
		return tail;
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
#endif /* SAKURA_CMYSTRING_009A2525_6B06_4C1B_B089_C1B8A424A565_H_ */
