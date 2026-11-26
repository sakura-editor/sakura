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

//! cmd::tagjump::GetTagJumpTarget() の結果
struct STagJumpResult {
	bool			bFound = false;
	std::wstring	strFile;
	int				nLine = 0;
	int				nColumn = 0;
};

} // namespace

/*!
 * @brief cmd::tagjump::GetTagJumpTarget のテストのためのフィクスチャクラス
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
		if (const auto target = cmd::tagjump::GetTagJumpTarget(cDocLineMgr, CLogicInt(nLineY), false)) {
			result = { true, target->strFile, target->nLine, target->nColumn };
		}
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
 *  出力形式・ベースフォルダー表示・フォルダー毎に表示・文字コード名のすべての組み合わせで、一致の行を読む。
 *  本文に「): 」「]: 」があっても、ファイル名の後ろの行・桁を読む
 */
TEST_F(TagJumpTargetTest, GrepResultLine)
{
	for (const auto& [nStyle, bBaseFolder, bSeparateFolder] : styles) {
		for (const auto codeName : { L"", L"  [SJIS]", L"  [UTF-8]" }) {
			for (const auto line : { L"hello\r\n", L"def f(x): pass\r\n", L"a[0]: b\r\n" }) {
				const auto text = GrepResult(Style(nStyle, bBaseFolder, bSeparateFolder), codeName, L"sub", L"a.txt", line);
				SCOPED_TRACE(text);

				const auto result = Read(text);
				EXPECT_THAT(result.bFound, IsTrue());
				EXPECT_THAT(result.strFile, StrEq(Path(L"sub\\a.txt")));
				EXPECT_THAT(result.nLine, Eq(12));
				EXPECT_THAT(result.nColumn, Eq(3));
			}
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
 *  文字コード名の数字([UTF-8] の 8)を行番号として読まない
 */
TEST_F(TagJumpTargetTest, WzStyleFileHeading)
{
	for (const auto& [bBaseFolder, bSeparateFolder] : { std::pair{ false, false }, std::pair{ true, false }, std::pair{ false, true }, std::pair{ true, true } }) {
		for (const auto codeName : { L"", L"  [SJIS]", L"  [UTF-8]" }) {
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
 *  文字コード名・コードページ名の数字([UTF-8] の 8、[CP932] の 932)を行番号として読まない
 */
TEST_F(TagJumpTargetTest, FileNameOnlyLine)
{
	for (const auto codeName : { L"", L"  [SJIS]", L"  [UTF-8]", L"  [CP932]" }) {
		// ノーマル
		{
			const auto text = Path(L"sub\\a.txt") + codeName + L"\r\n";
			SCOPED_TRACE(text);

			const auto result = Read(text);
			EXPECT_THAT(result.bFound, IsTrue());
			EXPECT_THAT(result.strFile, StrEq(Path(L"sub\\a.txt")));
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
 * @brief 「(」で始まるファイル名の行を、WZ風の行として読まない
 */
TEST_F(TagJumpTargetTest, FileNameStartingWithParen)
{
	for (const auto codeName : { L"", L"  [UTF-8]" }) {
		const auto text = GrepResult(Style(1, true, false), codeName, L"", L"(1)memo.txt", L"x\r\n");
		SCOPED_TRACE(text);

		const auto result = Read(text);
		EXPECT_THAT(result.bFound, IsTrue());
		EXPECT_THAT(result.strFile, StrEq(Path(L"(1)memo.txt")));
		EXPECT_THAT(result.nLine, Eq(12));
		EXPECT_THAT(result.nColumn, Eq(3));
	}
}

/*!
 * @brief 無題のウィンドウを Grep した結果の行から、:HWND: のタグと行・桁を読む
 */
TEST_F(TagJumpTargetTest, HwndTagLine)
{
	// ノーマル・ノーマル(ベースフォルダー表示)・16 進数の大文字・文字コード名と本文の「): 」
	for (const auto& [text, tag] : {
		std::pair{ L":HWND:[0000abcd]無題1(3,1): x\r\n", L":HWND:[0000abcd]" },
		std::pair{ L"・:HWND:[0000abcd]無題1(3,1): x\r\n", L":HWND:[0000abcd]" },
		std::pair{ L":HWND:[0000ABCD]無題1(3,1): x\r\n", L":HWND:[0000ABCD]" },
		std::pair{ L":HWND:[0000abcd]無題1(3,1)  [SJIS]: def f(x): pass\r\n", L":HWND:[0000abcd]" },
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
 * @brief 「[」と「]」の間が 16 進数でないタグを、:HWND: のタグとして読まない
 */
TEST_F(TagJumpTargetTest, HwndTagWithNonHexIsNotTag)
{
	for (const auto text : { L":HWND:[zzzz]無題1(3,1): x\r\n", L":HWND:[]無題1(3,1): x\r\n" }) {
		SCOPED_TRACE(text);

		EXPECT_THAT(Read(text).bFound, IsFalse());
	}
}

/*!
 * @brief 見出しの中の :HWND: のタグ(ファイル名の見出し・ファイル毎の見出し・「・"」の行)から、タグと行・桁を読む
 */
TEST_F(TagJumpTargetTest, HwndTagInHeading)
{
	for (const auto& [text, nLineY, nLine] : {
		std::tuple{ L"◆\":HWND:[0000abcd]無題1\"\r\n", 0, 0 },
		std::tuple{ L"◆\":HWND:[0000abcd]無題1\"  [SJIS]\r\n・(     3,1    ): x\r\n", 1, 3 },
		std::tuple{ L"■\":HWND:[0000abcd]無題1\"  [SJIS]\r\n・(     3,1    ): x\r\n", 1, 3 },
		std::tuple{ L"・\":HWND:[0000abcd]無題1\"\r\n", 0, 0 },
	}) {
		SCOPED_TRACE(text);

		const auto result = Read(text, nLineY);
		EXPECT_THAT(result.bFound, IsTrue());
		EXPECT_THAT(result.strFile, StrEq(L":HWND:[0000abcd]"));
		EXPECT_THAT(result.nLine, Eq(nLine));
	}
}

/*!
 * @brief 読めない見出し(閉じる「"」が無い・パスが長すぎる・ファイルが無い)では、見つからない
 */
TEST_F(TagJumpTargetTest, BrokenHeadingIsNotFound)
{
	const std::wstring baseFolder = base.native();
	const std::wstring longName(250, L'x');
	for (const auto& text : {
		std::wstring(L"◆\"abc\r\n"),
		std::wstring(L"・\"abc\r\n"),
		std::wstring(L"■\"abc\r\n"),
		std::format(L"■\"{}\r\n・sub\\a.txt(1,1): x\r\n", baseFolder),
		std::format(L"◎\"{}\r\n■\"sub\"\r\n・a.txt(1,1): x\r\n", baseFolder),
		std::format(L"・{}(1,1): x\r\n", std::wstring(260, L'x')),
		std::format(L"■\"{}\"\r\n・\"{}\"  [SJIS]\r\n", baseFolder, longName),
		std::format(L"◎\"{}\"\r\n■\"{}\"\r\n・a.txt(1,1): x\r\n", baseFolder, longName),
		std::format(L"◎\"{}\"\r\n■\"sub\"\r\n・nofile.txt(1,1): x\r\n", baseFolder),
	}) {
		SCOPED_TRACE(text);

		EXPECT_THAT(Read(text).bFound, IsFalse());
	}
}

/*!
 * @brief パスの後ろが文字コード名だけでない行(閉じる「]」が無い・「]」の後ろに文字がある)は、文字コード名だけの行として扱わない
 */
TEST_F(TagJumpTargetTest, NotOnlyCodeNameAfterPath)
{
	for (const auto codeName : { L"  [SJIS", L"  [SJIS]x" }) {
		const auto text = std::format(L"■\"{}\"{}\r\n", Path(L"sub\\a.txt"), codeName);
		SCOPED_TRACE(text);

		const auto result = Read(text);
		EXPECT_THAT(result.bFound, IsTrue());
		EXPECT_THAT(result.strFile, StrEq(Path(L"sub\\a.txt")));
		EXPECT_THAT(result.nLine, Eq(0));
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
 * @brief ジャンプ先が無い行・無い行番号では、見つからない
 */
TEST_F(TagJumpTargetTest, NotFound)
{
	for (const auto& [text, nLineY] : { std::pair{ L"hello world\r\n", 0 }, std::pair{ L"", 0 }, std::pair{ L"hello\r\n", 1 } }) {
		SCOPED_TRACE(text);

		CDocLineMgr cDocLineMgr;
		if (*text) {
			cDocLineMgr.AddNewLine()->SetDocLineString(std::wstring_view(text), false);
		}
		EXPECT_THAT(cmd::tagjump::GetTagJumpTarget(cDocLineMgr, CLogicInt(nLineY), false).has_value(), IsFalse());
	}
}
