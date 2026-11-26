/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"

#include <filesystem>
#include <format>
#include <fstream>
#include <string>
#include <string_view>
#include <tuple>

#include "cmd/CViewCommander.h"
#include "doc/logic/CDocLineMgr.h"
#include "grep/GrepMessageFormat.h"

namespace {

//! GetTagJumpTarget() の結果
struct STagJumpResult {
	bool			bFound = false;
	std::wstring	strFile;
	int				nLine = 0;
	int				nColumn = 0;
};

} // namespace

/*!
 * @brief CViewCommander::GetTagJumpTarget のテストのためのフィクスチャクラス
 *
 * ジャンプ先のファイルがあるかを見るので、一時フォルダーにファイルを作る。
 * base\a.py、base\(1)memo.txt、base\sub\a.txt
 */
class TagJumpTargetTest : public ::testing::Test {
protected:
	static inline std::filesystem::path base{};

	static void SetUpTestSuite()
	{
		base = std::filesystem::temp_directory_path() / L"sakura_test_tagjump";
		std::filesystem::create_directories(base / L"sub");
		for (const auto name : { L"a.py", L"(1)memo.txt", L"sub\\a.txt" }) {
			std::ofstream(base / name) << "x";
		}
	}

	static void TearDownTestSuite()
	{
		std::error_code ec;
		std::filesystem::remove_all(base, ec);
	}

	//! base の下のパス
	static std::wstring Path(std::wstring_view relPath)
	{
		return (base / relPath).native();
	}

	/*!
	 * text(行末は CRLF)を文書の行にして、nLineY 行目(0開始。負なら最後の行)を読む
	 */
	static STagJumpResult Read(std::wstring_view text, int nLineY = -1)
	{
		CDocLineMgr cDocLineMgr;
		while (!text.empty()) {
			const auto pos = text.find(L'\n');
			const auto len = pos == std::wstring_view::npos ? text.length() : pos + 1;
			cDocLineMgr.AddNewLine()->SetDocLineString(text.substr(0, len), false);
			text.remove_prefix(len);
		}
		if (nLineY < 0) {
			nLineY = cDocLineMgr.GetLineCount() + nLineY;
		}

		STagJumpResult result;
		result.bFound = CViewCommander::GetTagJumpTarget(cDocLineMgr, CLogicInt(nLineY), false, result.strFile, result.nLine, result.nColumn);
		return result;
	}

	/*!
	 * base の下の subFolder\fileName(subFolder が空なら base の直下)の 12 行 3 桁の一致を、gi の形式で書いた Grep の結果
	 *
	 * Grep と同じ CGrepResultWriter で書く。フォルダー名・相対パスは CGrepAgent と同じ決め方にする。
	 */
	static std::wstring GrepResult(const GrepInfo& gi, const wchar_t* pszCodeName, std::wstring_view subFolder, std::wstring_view fileName, std::wstring_view line)
	{
		const std::wstring baseFolder = base.native();
		const std::wstring relPath = subFolder.empty() ? std::wstring(fileName) : std::format(L"{}\\{}", subFolder, fileName);
		const std::wstring fullPath = Path(relPath);
		std::wstring folder = subFolder.empty() ? baseFolder : Path(subFolder);
		if (gi.bGrepSeparateFolder && gi.bGrepOutputBaseFolder) {
			folder = subFolder;
		}
		const std::wstring dispRelPath = gi.bGrepSeparateFolder ? std::wstring(fileName) : relPath;

		CNativeW cmemMessage(L"");
		bool bOutputBaseFolder = false;
		bool bOutputFolderName = false;
		CGrepResultWriter writer(cmemMessage, gi,
			{ .pszFullPath = fullPath.c_str(), .pszBaseFolder = baseFolder.c_str(), .pszFolder = folder.c_str(), .pszRelPath = dispRelPath.c_str(), .pszCodeName = pszCodeName },
			bOutputBaseFolder, bOutputFolderName);
		writer.AddMatch({
			.nLine = 12, .nColumn = 3,
			.pLine = line.data(), .nLineLen = static_cast<int>(line.length()), .nEolLen = 2,
			.pMatch = line.data(), .nMatchLen = 1
		});
		return cmemMessage.GetStringPtr();
	}

	//! 出力形式・ベースフォルダー表示・フォルダー毎に表示を決めた GrepInfo(該当行を出す)
	static GrepInfo Style(int nStyle, bool bBaseFolder, bool bSeparateFolder)
	{
		GrepInfo gi;
		gi.nGrepOutputStyle = nStyle;
		gi.nGrepOutputLineType = 1;
		gi.bGrepOutputBaseFolder = bBaseFolder;
		gi.bGrepSeparateFolder = bSeparateFolder;
		return gi;
	}

	//! 出力形式(ノーマル・WZ風)とベースフォルダー表示・フォルダー毎に表示の組み合わせ
	static constexpr std::tuple<int, bool, bool> styles[] = {
		{ 1, false, false }, { 1, true, false }, { 1, false, true }, { 1, true, true },
		{ 2, false, false }, { 2, true, false }, { 2, false, true }, { 2, true, true },
	};
};

/*!
 * @brief Grep の結果の行から、ファイルと行・桁を読む
 *  出力形式・ベースフォルダー表示・フォルダー毎に表示・文字コード名のすべての組み合わせで、一致の行を読む
 */
TEST_F(TagJumpTargetTest, GrepResultLine)
{
	for (const auto& [nStyle, bBaseFolder, bSeparateFolder] : styles) {
		for (const auto codeName : { L"", L"  [SJIS]", L"  [UTF-8]" }) {
			const auto text = GrepResult(Style(nStyle, bBaseFolder, bSeparateFolder), codeName, L"sub", L"a.txt", L"hello\r\n");
			SCOPED_TRACE(text);

			const auto result = Read(text);
			EXPECT_THAT(result.bFound, IsTrue());
			EXPECT_THAT(result.strFile, StrEq(Path(L"sub\\a.txt")));
			EXPECT_THAT(result.nLine, Eq(12));
			EXPECT_THAT(result.nColumn, Eq(3));
		}
	}
}

/*!
 * @brief Grep の結果の行から、ファイルと行・桁を読む
 *  フォルダー毎に表示で、ベースフォルダーの直下のファイル(フォルダーの見出しが「■」だけの行)
 */
TEST_F(TagJumpTargetTest, GrepResultLineInBaseFolder)
{
	for (const int nStyle : { 1, 2 }) {
		const auto text = GrepResult(Style(nStyle, true, true), L"", L"", L"a.py", L"hello\r\n");
		SCOPED_TRACE(text);

		const auto result = Read(text);
		EXPECT_THAT(result.bFound, IsTrue());
		EXPECT_THAT(result.strFile, StrEq(Path(L"a.py")));
		EXPECT_THAT(result.nLine, Eq(12));
		EXPECT_THAT(result.nColumn, Eq(3));
	}
}

/*!
 * @brief WZ風のファイル名の見出しの行から、ファイルを読む。行・桁は指定しない
 *  文字コード名に数字が無いもの(数字があるものは #2885 の 1)
 */
TEST_F(TagJumpTargetTest, WzStyleFileHeading)
{
	for (const auto& [bBaseFolder, bSeparateFolder] : { std::pair{ false, false }, std::pair{ true, false }, std::pair{ false, true }, std::pair{ true, true } }) {
		for (const auto codeName : { L"", L"  [SJIS]" }) {
			const auto text = GrepResult(Style(2, bBaseFolder, bSeparateFolder), codeName, L"sub", L"a.txt", L"hello\r\n");
			SCOPED_TRACE(text);

			// 最後の行(一致の行)の 1 つ上が、ファイル名の見出し
			const auto result = Read(text, -2);
			EXPECT_THAT(result.bFound, IsTrue());
			EXPECT_THAT(result.strFile, StrEq(Path(L"sub\\a.txt")));
			EXPECT_THAT(result.nLine, Eq(0));
			EXPECT_THAT(result.nColumn, Eq(0));
		}
	}
}

/*!
 * @brief 検索キーが空のときの、ファイル名だけの行から、ファイルを読む。行・桁は指定しない
 *  文字コード名に数字が無いもの(数字があるものは #2885 の 1)
 */
TEST_F(TagJumpTargetTest, FileNameOnlyLine)
{
	for (const auto codeName : { L"", L"  [SJIS]" }) {
		// ノーマル
		{
			const auto text = Path(L"sub\\a.txt") + codeName + L"\r\n";
			SCOPED_TRACE(text);

			// Windows はパスの末尾の空白を無視するので、IsFilePath() は文字コード名の前の空白までをパスとして読む
			const auto result = Read(text);
			EXPECT_THAT(result.bFound, IsTrue());
			EXPECT_THAT(result.strFile.substr(0, result.strFile.find_last_not_of(L' ') + 1), StrEq(Path(L"sub\\a.txt")));
			EXPECT_THAT(result.nLine, Eq(0));
		}
		// ノーマル・フォルダー毎に表示
		{
			const auto text = std::format(L"■\"{}\"\r\n・\"a.txt\"{}\r\n", Path(L"sub"), codeName);
			SCOPED_TRACE(text);

			const auto result = Read(text);
			EXPECT_THAT(result.bFound, IsTrue());
			EXPECT_THAT(result.strFile, StrEq(Path(L"sub\\a.txt")));
			EXPECT_THAT(result.nLine, Eq(0));
		}
	}
}

/*!
 * @brief 無題のウィンドウを Grep した結果の行から、:HWND: のタグと行・桁を読む
 */
TEST_F(TagJumpTargetTest, HwndTagLine)
{
	// ノーマル・ノーマル(ベースフォルダー表示)・16 進数の大文字
	for (const auto& [text, tag] : {
		std::pair{ L":HWND:[0000abcd]無題1(3,1): x\r\n", L":HWND:[0000abcd]" },
		std::pair{ L"・:HWND:[0000abcd]無題1(3,1): x\r\n", L":HWND:[0000abcd]" },
		std::pair{ L":HWND:[0000ABCD]無題1(3,1): x\r\n", L":HWND:[0000ABCD]" },
	}) {
		SCOPED_TRACE(text);

		const auto result = Read(text);
		EXPECT_THAT(result.bFound, IsTrue());
		EXPECT_THAT(result.strFile, StrEq(tag));
		EXPECT_THAT(result.nLine, Eq(3));
		EXPECT_THAT(result.nColumn, Eq(1));
	}
}

/*!
 * @brief Borland 形式のメッセージ(ファイル名の後に空白と行番号)から、ファイルと行を読む
 */
TEST_F(TagJumpTargetTest, BorlandMessage)
{
	const auto text = std::format(L"Error E2209 {} 12: Unable to open include file\r\n", Path(L"sub\\a.txt"));
	SCOPED_TRACE(text);

	const auto result = Read(text);
	EXPECT_THAT(result.bFound, IsTrue());
	EXPECT_THAT(result.strFile, StrEq(Path(L"sub\\a.txt")));
	EXPECT_THAT(result.nLine, Eq(12));
	EXPECT_THAT(result.nColumn, Eq(0));
}

/*!
 * @brief ジャンプ先が無い行・無い行番号では、見つからない。結果の変数は変えない
 */
TEST_F(TagJumpTargetTest, NotFound)
{
	for (const auto& [text, nLineY] : { std::pair{ L"hello world\r\n", 0 }, std::pair{ L"", 0 }, std::pair{ L"hello\r\n", 1 } }) {
		SCOPED_TRACE(text);

		CDocLineMgr cDocLineMgr;
		if (*text) {
			cDocLineMgr.AddNewLine()->SetDocLineString(std::wstring_view(text), false);
		}
		std::wstring strFile = L"unchanged";
		int nLine = -1;
		int nColumn = -1;
		EXPECT_THAT(CViewCommander::GetTagJumpTarget(cDocLineMgr, CLogicInt(nLineY), false, strFile, nLine, nColumn), IsFalse());
		EXPECT_THAT(strFile, StrEq(L"unchanged"));
		EXPECT_THAT(nLine, Eq(-1));
		EXPECT_THAT(nColumn, Eq(-1));
	}
}
