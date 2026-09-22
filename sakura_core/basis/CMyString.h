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
using SFilePath = StaticString<_MAX_PATH, false>;

/*!
 * @brief 長めの文字列を格納する文字列バッファ
 *
 * 上限値の根拠が不明瞭なので、検討の余地がある。
 */
using SFilePathLong = StaticString<MAX_GREP_PATH, false>;

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

/*!
 * @brief コマンドライン文字列の組み立てに使う文字列バッファ
 *
 * かつてWindowsには、コマンドライン長 1023文字 の制約があった。
 *
 * 現代のWindowsは 32,767文字 まで指定可能。
 * あえて制約を残す設計も多いが、1023文字では短すぎる。
 */
// TODO: いつか削除する
class CCommandLineString{
private:
	static_assert(
		1024 == static_cast<int>(MAX_CMDLEN),
		"CCommandLineString designed 1024 chars buffer."
	);

	using SCmdLine = StaticString<MAX_CMDLEN>;

public:
	CCommandLineString() = default;

	template <typename... Args>
	void AppendF(
		_In_z_ _Printf_format_string_ LPCWSTR format,
		const Args&... args
	)
	{
		// 残りサイズを計算する
		const auto availableSize = std::size(m_szCmdLine) - length();

		// 書き込んでよい固定長バッファを作る
		auto buffer = std::span{ m_pHead, availableSize };

		// 書式付き文字列をバッファに書き込む
		const auto written = cxx::_sprintf_s(
			buffer,
			format,
			std::as_const(args)...
		);

		// エラーは想定していない
		assert(0 <= written);

		m_pHead += written;
	}

	/*!
	 * @brief C String(NUL終端文字列)を取得する
	 *
	 * @return C String(NUL終端文字列)
	 */
	constexpr LPCWSTR c_str() const noexcept
	{
		return m_szCmdLine.c_str();
	}

	/*!
	 * @brief 文字列長を取得する
	 *
	 * @return 文字列長（NUL終端を含まない）
	 */
	constexpr size_t length() const noexcept
	{
		return m_pHead - m_szCmdLine.c_str();
	}

	// 固定長バッファなのでsizeはバッファサイズを返すべき。(std::stringは可変長。)
	// TODO: いつか削除する
	size_t size() const
	{
		return m_pHead - m_szCmdLine;
	}

	// インスタンスごとに変わる値ではないので static メソッドとすべき。
	// TODO: いつか削除する
	size_t max_size() const
	{
		// FIXME: 固定長バッファのmax_size()は要素数を返すが、このメソッドは -1 している。
		return int(std::size(m_szCmdLine)) - 1;
	}

private:
	SCmdLine	m_szCmdLine;
	WCHAR*		m_pHead = m_szCmdLine.data();
};

#endif /* SAKURA_CMYSTRING_009A2525_6B06_4C1B_B089_C1B8A424A565_H_ */
