/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_TESTS1_GREP_GREPTESTSUITE_HPP_
#define SAKURA_TESTS1_GREP_GREPTESTSUITE_HPP_
#pragma once

#include "window/EditorTestSuite.hpp"
#include "dlg/ModalDialogCloser.hpp"

#include "CEditApp.h"
#include "_main/CCommandLine.h"
#include "agent/CGrepAgent.h"
#include "basis/GrepInfo.h"
#include "charset/charset.h"
#include "doc/CEditDoc.h"
#include "window/CEditWnd.h"
#include "CSelectLang.h"
#include "sakura_rc.h"

#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <string>
#include <string_view>
#include <cstdio>

namespace grep_test {

/*!
	@brief テスト用の一時フォルダー

	%TEMP%\sakura_test_grep\<テストスイート名>.<テスト名> に作り、デストラクターで消す。
	読み取り専用などの属性を付けたファイルも消せるよう、削除の前に属性を戻す。
*/
class TempFolder {
public:
	TempFolder()
	{
		const auto info = ::testing::UnitTest::GetInstance()->current_test_info();
		m_path = std::filesystem::temp_directory_path() / L"sakura_test_grep" / std::format("{}.{}", info->test_suite_name(), info->name());
		Remove();
		std::filesystem::create_directories(m_path);
	}

	~TempFolder()
	{
		// デバッグ用: 環境変数 SAKURA_TEST_KEEP_TEMP があれば消さずに残し、場所を出力する
		if (::GetEnvironmentVariableW(L"SAKURA_TEST_KEEP_TEMP", nullptr, 0) != 0) {
			const auto u8 = m_path.u8string();
			std::printf("[kept] %.*s\n", int(u8.size()), reinterpret_cast<const char*>(u8.data()));
			return;
		}
		Remove();
	}

	TempFolder(const TempFolder&) = delete;
	TempFolder& operator=(const TempFolder&) = delete;

	//! ファイルをバイト列のまま書き出す(途中のフォルダーも作る)
	void AddFile(const std::filesystem::path& relPath, std::string_view bytes, DWORD attributes = FILE_ATTRIBUTE_NORMAL) const
	{
		const auto path = m_path / relPath;
		std::filesystem::create_directories(path.parent_path());
		{
			std::ofstream ofs(path, std::ios::binary);
			ofs.write(bytes.data(), std::streamsize(bytes.size()));
		}
		::SetFileAttributesW(path.c_str(), attributes);
	}

	//! フォルダーを作る
	void AddFolder(const std::filesystem::path& relPath) const
	{
		std::filesystem::create_directories(m_path / relPath);
	}

	//! ファイルをバイト列のまま読む
	std::string ReadFile(const std::filesystem::path& relPath) const
	{
		std::ifstream ifs(m_path / relPath, std::ios::binary);
		return std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
	}

	bool Exists(const std::filesystem::path& relPath) const
	{
		return std::filesystem::exists(m_path / relPath);
	}

	const std::filesystem::path& Path() const noexcept { return m_path; }

private:
	void Remove() const
	{
		std::error_code ec;
		if (std::filesystem::exists(m_path, ec)) {
			for (const auto& entry : std::filesystem::recursive_directory_iterator(m_path, ec)) {
				::SetFileAttributesW(entry.path().c_str(), FILE_ATTRIBUTE_NORMAL);
			}
		}
		std::filesystem::remove_all(m_path, ec);
	}

	std::filesystem::path m_path;
};

/*!
	@brief カレントディレクトリを元に戻す

	CGrepAgent::DoGrep() は -GOPT=X が無いと検索したフォルダーへ、
	CDlgGrep::GetData() はフォルダーの確認のために、カレントディレクトリを移す。
	一時フォルダーを消せなくなり、後続のテストにも影響するので必ず戻す。
*/
class CurrentDirectoryGuard {
public:
	CurrentDirectoryGuard() = default;
	~CurrentDirectoryGuard()
	{
		std::error_code ec;
		std::filesystem::current_path(m_saved, ec);
	}
	CurrentDirectoryGuard(const CurrentDirectoryGuard&) = delete;
	CurrentDirectoryGuard& operator=(const CurrentDirectoryGuard&) = delete;

private:
	std::filesystem::path m_saved = std::filesystem::current_path();
};

/*!
	@brief 標準出力をファイルに向ける(-GOPT=U の確認用)
*/
class StdoutCapture {
public:
	explicit StdoutCapture(std::filesystem::path path)
		: m_path(std::move(path))
		, m_hSaved(::GetStdHandle(STD_OUTPUT_HANDLE))
	{
		m_hFile = ::CreateFileW(m_path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		::SetStdHandle(STD_OUTPUT_HANDLE, m_hFile);
	}

	~StdoutCapture()
	{
		Restore();
	}

	StdoutCapture(const StdoutCapture&) = delete;
	StdoutCapture& operator=(const StdoutCapture&) = delete;

	//! 出力先のファイルを作れたか(Read() の後は false)
	bool IsOpen() const noexcept
	{
		return m_hFile != INVALID_HANDLE_VALUE;
	}

	//! 標準出力を戻して、書き込まれたバイト列を返す
	std::string Read()
	{
		Restore();
		std::ifstream ifs(m_path, std::ios::binary);
		return std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
	}

private:
	void Restore()
	{
		if (m_hFile != INVALID_HANDLE_VALUE) {
			::SetStdHandle(STD_OUTPUT_HANDLE, m_hSaved);
			::CloseHandle(m_hFile);
			m_hFile = INVALID_HANDLE_VALUE;
		}
	}

	std::filesystem::path m_path;
	HANDLE m_hSaved;
	HANDLE m_hFile = INVALID_HANDLE_VALUE;
};

//! 部分文字列を含むか
inline bool Contains(std::wstring_view text, std::wstring_view part)
{
	return text.find(part) != std::wstring_view::npos;
}

//! 文字列を指定のコードページのバイト列にする(Windows の変換を使う。932=SJIS、20932=EUC-JP、50220=JIS、CP_UTF8)
inline std::string Encode(std::wstring_view text, UINT codePage)
{
	const int len = ::WideCharToMultiByte(codePage, 0, text.data(), int(text.size()), nullptr, 0, nullptr, nullptr);
	std::string bytes(size_t(len), '\0');
	::WideCharToMultiByte(codePage, 0, text.data(), int(text.size()), bytes.data(), len, nullptr, nullptr);
	return bytes;
}

//! 文字列を UTF-16 のバイト列にする
inline std::string EncodeUtf16(std::wstring_view text, bool bigEndian, bool withBom)
{
	std::string bytes;
	if (withBom) {
		bytes += bigEndian ? "\xFE\xFF" : "\xFF\xFE";
	}
	for (const wchar_t ch : text) {
		const auto lo = char(ch & 0xFF);
		const auto hi = char((ch >> 8) & 0xFF);
		bytes += bigEndian ? hi : lo;
		bytes += bigEndian ? lo : hi;
	}
	return bytes;
}

//! UTF-8 の BOM
constexpr std::string_view Utf8Bom = "\xEF\xBB\xBF";

//! 文字コードの判別に使う日本語の文章(判別しやすいよう 3 行。各行に「テスト」が 1 つ)
inline std::wstring JapaneseLines()
{
	std::wstring text;
	for (int i = 0; i < 3; ++i) {
		text += L"テストの文章です。日本語のファイルを検索します。\r\n";
	}
	return text;
}

//! text の中で part を含む最初の行(見つからなければ空)
inline std::wstring LineContaining(const std::wstring& text, std::wstring_view part)
{
	const auto pos = text.find(part);
	if (pos == std::wstring::npos) {
		return {};
	}
	const auto lineHead = text.rfind(L'\n', pos);
	const auto begin = (lineHead == std::wstring::npos) ? 0 : lineHead + 1;
	const auto end = text.find(L'\n', pos);
	return text.substr(begin, (end == std::wstring::npos) ? std::wstring::npos : end - begin);
}

//! 文字コード名の「  [名前]」表記(Grep 結果の行に付くもの)
inline std::wstring CodeBracket(ECodeType code)
{
	return std::wstring(CCodeTypeName(code).Bracket());
}

//! 文字列リソースの「%s」「%d」より前の部分(書式付きのメッセージを探すため)
inline std::wstring ResourcePrefix(UINT id)
{
	std::wstring text = LS(id);
	if (const auto pos = text.find(L'%'); pos != std::wstring::npos) {
		text.resize(pos);
	}
	return text;
}

/*!
	@brief コマンドライン文字列を解析して GrepInfo を得る

	-GREPMODE で起動したときと同じ CCommandLine::ParseCommandLine() を通す。
*/
inline GrepInfo ParseGrepCommandLine(const std::wstring& args)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(args.c_str(), false);
	return cCommandLine.GetGrepInfoRef();
}

/*!
	@brief Grep のテストの共通基盤

	編集ウィンドウを 1 つ作り、その中で Grep を実行する。
	CViewCommander::Command_GREP() は、無題・未編集の文書なら結果を同じウィンドウに出す(新しいプロセスを起動しない)。
	メッセージボックスは MockUser32 で止まらずに返る。
*/
struct GrepTestSuite : public ::testing::Test, public window::EditorTestSuite, public window::UiaTestSuite {
	static inline std::unique_ptr<CCommandLine> pCommandLine = nullptr;

	static void SetUpTestSuite()
	{
		pCommandLine = std::make_unique<CCommandLine>();
		pCommandLine->ParseCommandLine(L"-PROF=", false);

		SetUpUiaTestSuite();

		SetUpEditor();
	}

	static void TearDownTestSuite()
	{
		TearDownEditor();

		User32::resetInstance();

		TearDownUiaTestSuite();

		pCommandLine = nullptr;
	}

	void SetUp() override
	{
		User32::setInstance<MockUser32>();
		ResetDocument();

		// デバッグ用: 環境変数 SAKURA_TEST_SHOW_WINDOW があれば編集ウィンドウを表示する(結果を目で見るため)
		if (::GetEnvironmentVariableW(L"SAKURA_TEST_SHOW_WINDOW", nullptr, 0) != 0) {
			::ShowWindow(pcEditWnd->GetHwnd(), SW_SHOWNORMAL);
		}
	}

	void TearDown() override
	{
		// デバッグ用: 失敗したテストでは、Grep の結果の文書を出力する
		if (HasFailure()) {
			const auto utf8 = Encode(GetDocumentText(), CP_UTF8);
			std::printf("----- document -----\n%s\n--------------------\n", utf8.c_str());
		}

		CEditApp::getInstance()->m_pcGrepAgent->m_bGrepMode = false;
		CEditApp::getInstance()->m_pcGrepAgent->m_bGrepRunning = false;

		MSG msg{};
		while (::PeekMessageW(&msg, nullptr, 0L, 0L, PM_REMOVE)) ;

		pcEditDoc->m_cDocFile.SetFilePath(L"");
		ResetDocument();

		TearDownUia();
	}

	//! 文書を空・未編集・無題に戻す
	static void ResetDocument()
	{
		pcEditDoc->InitDoc();
		pcEditDoc->InitAllView();
		pcEditDoc->m_cDocEditor.m_bIsDocModified = false;
	}

	//! 文書の全文(行末を含む)
	static std::wstring GetDocumentText()
	{
		std::wstring text;
		for (auto pLine = pcEditDoc->m_cDocLineMgr.GetDocLineTop(); pLine; pLine = pLine->GetNextLine()) {
			text.append(pLine->GetPtr(), pLine->GetLengthWithEOL());
		}
		return text;
	}

	//! GrepInfo で Grep を実行し、ヒット数を返す
	static DWORD RunGrep(const GrepInfo& gi)
	{
		return CEditApp::getInstance()->m_pcGrepAgent->DoGrep(&pcEditWnd->GetActiveView(), gi);
	}

	//! 「%d 個が検索されました」の文言
	static std::wstring MatchCountText(int count)
	{
		std::wstring text = LS(STR_GREP_MATCH_COUNT);
		if (const auto pos = text.find(L"%d"); pos != std::wstring::npos) {
			text.replace(pos, 2, std::to_wstring(count));
		}
		return text;
	}
};

} // namespace grep_test

#endif /* SAKURA_TESTS1_GREP_GREPTESTSUITE_HPP_ */
