/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

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

/*!
 * @brief ファイルパスを格納する文字列バッファ
 *
 * _MAX_PATH == 260
 *
 * 文字列長 _MAX_PATH - 1 までを格納できる固定長バッファ。
 *
 * 4096程度に拡張すべきだが、色々事情があり対応保留。
 */
using SFilePath = StaticString<_MAX_PATH>;

/*!
 * @brief 長めの文字列を格納する文字列バッファ
 *
 * 上限値の根拠が不明瞭なので、検討の余地がある。
 */
using SFilePathLong = StaticString<MAX_GREP_PATH>;

/*!
 * @brief ドキュメントパスの格納に使っているクラス
 *
 * SFilePathなので最大259文字。
 */
// TODO: いつか削除する
class CFilePath : public SFilePath {
private:
	using Base = SFilePath;
	using Me = CFilePath;

public:
	// コンストラクタは流用
	using Base::Base;

	/*!
	 * @brief 文字列をコピーして構築する
	 *
	 * explicitを付けないのはC++の作法に照らして適切でない。
	 *
	 * @param pszPath [in, opt] コピーする文字列
	 */
	constexpr /* implicit */ CFilePath(_In_opt_z_ LPCWSTR pszPath)
	{
		Assign(pszPath);
	}

	[[nodiscard]] bool IsValidPath() const noexcept { return !empty(); }
	[[nodiscard]] std::wstring GetDirPath() const
	{
		std::filesystem::path path{ *this };
		return path.remove_filename();
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
		m_pHead+=auto_vsprintf_s(m_pHead, std::size(m_szCmdLine)-(m_pHead-m_szCmdLine),szFormat,v);
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
		return int(std::size(m_szCmdLine)) - 1;
	}
private:
	WCHAR	m_szCmdLine[1024];
	WCHAR*	m_pHead;
};
#endif /* SAKURA_CMYSTRING_009A2525_6B06_4C1B_B089_C1B8A424A565_H_ */
