/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#pragma once

/*!
 * @file grep-test-util.h
 * @brief Grep 系テスト共通の補助ヘルパ
 *
 * test-grep.cpp / test-grep-irregular.cpp で重複定義されていた
 * テスト基盤（一時ディレクトリ RAII・オプション生成・ワーカー呼び出し）を共通化する。
 */

#include <atomic>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include "agent/CGrepAgent.h"
#include "agent/CSearchAgent.h"
#include "charset/CCodeFactory.h"
#include "extmodule/CBregexp.h"
#include "util/file.h"

// =============================================================================
// 一時ディレクトリ RAII
// =============================================================================

/*!
 * テスト用の一時ディレクトリを生成・自動削除する RAII クラス
 *
 *  - コンストラクタで一意な一時ディレクトリを作る
 *  - デストラクタで再帰削除する
 *  - WriteEncodedTextFile / WriteRawBytes 等のヘルパで配下にファイルを書き込む
 */
class GrepTempDir
{
public:
	explicit GrepTempDir(std::wstring_view prefix = L"grp")
	{
		// GetTempFilePath は一時ファイルを生成するので、それを消してから
		// 同名のディレクトリを掘り直すことで一意性を担保する。
		auto candidate = GetTempFilePath(prefix);
		std::filesystem::remove(candidate);
		std::filesystem::create_directories(candidate);
		m_root = candidate;
	}

	GrepTempDir(const GrepTempDir&) = delete;
	GrepTempDir& operator=(const GrepTempDir&) = delete;

	~GrepTempDir()
	{
		std::error_code ec;
		std::filesystem::remove_all(m_root, ec);
	}

	const std::filesystem::path& Root() const noexcept { return m_root; }

	std::filesystem::path Sub(std::wstring_view relative) const
	{
		return m_root / std::filesystem::path(relative);
	}

	void EnsureDir(std::wstring_view relative) const
	{
		std::filesystem::create_directories(Sub(relative));
	}

	/*!
	 * 任意の文字コードでテキストファイルを書き出す
	 *
	 * BOM 付き UTF-8 / UTF-16LE / UTF-16BE に対応するために
	 * withBom を指定可能にしている（自動判定経路のテストで使用）。
	 */
	std::filesystem::path WriteEncodedTextFile(
		std::wstring_view relative,
		ECodeType codeType,
		std::wstring_view text,
		bool withBom = false) const
	{
		const auto path = Sub(relative);
		std::filesystem::create_directories(path.parent_path());

		const auto encoded = CCodeFactory::ConvertToCode(codeType, text);
		if (encoded.result != RESULT_COMPLETE) {
			ADD_FAILURE() << "CCodeFactory::ConvertToCode failed for codeType="
				<< static_cast<int>(codeType);
			return path;
		}

		std::ofstream os(path, std::ios::binary | std::ios::trunc);
		if (!os) {
			ADD_FAILURE() << "failed to open file for write: " << path.string();
			return path;
		}

		if (withBom) {
			switch (codeType) {
			case CODE_UTF8:
				os.write("\xEF\xBB\xBF", 3);
				break;
			case CODE_UNICODE:
				os.write("\xFF\xFE", 2);
				break;
			case CODE_UNICODEBE:
				os.write("\xFE\xFF", 2);
				break;
			default:
				break;
			}
		}
		os.write(encoded.destination.data(),
			static_cast<std::streamsize>(encoded.destination.size()));
		return path;
	}

	// 生バイト列をそのままファイルに書き出す。
	std::filesystem::path WriteRawBytes(
		std::wstring_view relative, std::string_view bytes) const
	{
		const auto path = Sub(relative);
		std::filesystem::create_directories(path.parent_path());
		std::ofstream os(path, std::ios::binary | std::ios::trunc);
		os.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
		return path;
	}

private:
	std::filesystem::path m_root;
};

// =============================================================================
// 補助ヘルパ
// =============================================================================

/*!
 * スコープ終了時に任意の後始末を実行する軽量 RAII
 *
 * `std::unique_ptr<void, std::function<void(void*)>>` にダミーポインタを渡す
 * イディオムの置き換え（void* を使わず意図を型で表す）。
 */
template <class F>
class ScopeExit
{
public:
	explicit ScopeExit(F f) : m_f(std::move(f)) {}
	ScopeExit(const ScopeExit&) = delete;
	ScopeExit& operator=(const ScopeExit&) = delete;
	~ScopeExit() { m_f(); }
private:
	F m_f;
};

inline SSearchOption MakeSearchOption(bool regex, bool caseSensitive, bool wordOnly = false)
{
	SSearchOption opt;
	opt.Reset();
	opt.bRegularExp = regex;
	opt.bLoHiCase = caseSensitive;
	opt.bWordOnly = wordOnly;
	return opt;
}

// DoGrepFileWorker の戻り値に意味を持たせる定数
constexpr int GREP_RESULT_NO_HIT = 0;			// ヒットなし（ファイル読み取り不可含む）
constexpr int GREP_RESULT_CANCELLED = -1;		// キャンセルまたはエラー

inline SGrepOption MakeGrepOption(ECodeType charSet = CODE_AUTODETECT)
{
	// テストで共通に使う Grep 条件をまとめ、個別ケースでは必要な項目だけ上書きする。
	SGrepOption gopt;
	gopt.nGrepCharSet = charSet;
	gopt.nGrepOutputStyle = 1; // Normal
	gopt.nGrepOutputLineType = 1; // ヒット行を出力
	gopt.bGrepHeader = false;
	return gopt;
}

/*!
 * 1 ファイルに対する Grep ワーカーを呼び出してヒット数を返す
 *
 * CGrepAgent::DoGrepFileWorker は PR #2459 で並列 Grep の単位として
 * 切り出された関数。ここで直接呼ぶことで、ファイル列挙以降の検索処理を
 * 実ファイルに対して検証する。
 *
 *  @retval -1 キャンセル
 *  @retval >=0 ヒット数
 */
inline int RunGrepFileWorker(
	CGrepAgent& agent,
	const std::filesystem::path& path,
	std::wstring_view key,
	const SSearchOption& sSearchOption,
	const SGrepOption& sGrepOption,
	std::atomic<bool>& cancel)
{
	const std::wstring keyStr(key);
	const std::wstring fullPath = path.wstring();
	const std::wstring fileName = path.filename().wstring();
	const std::wstring baseFolder = path.parent_path().wstring();

	SGrepFileTask task;
	task.fullPath = fullPath;
	task.fileName = fileName;
	task.baseFolder = baseFolder;
	task.folder = baseFolder;
	task.relPath = fileName;

	CBregexp regexp;
	if (sSearchOption.bRegularExp) {
		if (!InitRegexp(nullptr, regexp, false)) {
			ADD_FAILURE() << "InitRegexp failed (bregonig.dll missing?)";
			return -1;
		}
		const DWORD flags = sSearchOption.bLoHiCase
			? CBregexp::optCaseSensitive
			: 0;
		if (!regexp.Compile(keyStr.c_str(), flags)) {
			ADD_FAILURE() << "regexp.Compile failed: " << path.string();
			return -1;
		}
	}

	CSearchStringPattern pattern;
	if (!pattern.SetPattern(nullptr, keyStr.c_str(), keyStr.size(),
			sSearchOption, sSearchOption.bRegularExp ? &regexp : nullptr)) {
		ADD_FAILURE() << "SetPattern failed";
		return -1;
	}

	CNativeW cmemMessage;
	CNativeW cUnicodeBuffer;
	cmemMessage.AllocStringBuffer(4000);
	cUnicodeBuffer.AllocStringBuffer(4000);

	return agent.DoGrepFileWorker(
		SGrepSearchParams{ keyStr.c_str(), sSearchOption, sGrepOption },
		task,
		sSearchOption.bRegularExp ? &regexp : nullptr,
		pattern,
		cancel,
		cmemMessage, cUnicodeBuffer);
}

// 同上だが、外側でキャンセル管理しない単純版
inline int RunGrepFileWorker(
	CGrepAgent& agent,
	const std::filesystem::path& path,
	std::wstring_view key,
	const SSearchOption& sSearchOption,
	const SGrepOption& sGrepOption)
{
	std::atomic<bool> cancel{ false };
	return RunGrepFileWorker(agent, path, key, sSearchOption, sGrepOption, cancel);
}
