/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "grep/GrepTestSuite.hpp"

#include "grep/CGrepEnumKeys.h"
#include "types/CType.h"
#include "charset/CCodeBase.h"
#include "charset/CCodeFactory.h"
#include "mem/CMemory.h"
#include "mem/CNativeW.h"

#include <memory>
#include <sstream>
#include <vector>

namespace grep_test {

/*!
	@brief コマンドライン(-GREPMODE)の経路のテスト

	-GREPMODE で起動すると CCommandLine が引数を GrepInfo に解析し、
	CNormalProcess が編集ウィンドウを作って CGrepAgent::DoGrep() を呼ぶ。
	ここではプロセスの起動を省き、同じ解析結果で同じ DoGrep() を呼ぶ。
*/
struct GrepCommandLineTest : public GrepTestSuite {
	TempFolder folder;
	CurrentDirectoryGuard currentDirectoryGuard;

	/*!
		基本のテストデータ

		| ファイル | HIT の数 | HIT を含む行 | HIT を含まない行 |
		|---|---|---|---|
		| a.txt | 3 | 2 | 1 |
		| b.txt | 0 | 0 | 1 |
		| c.log | 1(対象外) | | |
		| sub\d.txt | 1 | 1 | 0 |
	*/
	void AddBasicFiles() const
	{
		folder.AddFile(L"a.txt", "HIT x HIT\r\nnone\r\nHIT\r\n");
		folder.AddFile(L"b.txt", "none\r\n");
		folder.AddFile(L"c.log", "HIT\r\n");
		folder.AddFile(LR"(sub\d.txt)", "HIT\r\n");
	}

	//! -GREPMODE の引数を組み立てる
	std::wstring Args(std::wstring_view key, std::wstring_view file, std::wstring_view opt, std::wstring_view extra = L"") const
	{
		return std::format(LR"(-GREPMODE -GKEY="{}" -GFILE="{}" -GFOLDER="{}" -GOPT={} {})", key, file, folder.Path().native(), opt, extra);
	}

	//! 引数を解析して Grep を実行する
	DWORD Grep(std::wstring_view key, std::wstring_view file, std::wstring_view opt, std::wstring_view extra = L"") const
	{
		return RunGrep(ParseGrepCommandLine(Args(key, file, opt, extra)));
	}
};

// ---------------------------------------------------------------------------
// 検索条件
// ---------------------------------------------------------------------------

//! 既定(該当部分): 一致した箇所ごとに数える。サブフォルダーは探さない
TEST_F(GrepCommandLineTest, CountsEachMatch)
{
	AddBasicFiles();
	EXPECT_EQ(3u, Grep(L"HIT", L"*.txt", L"X"));
	const auto text = GetDocumentText();
	EXPECT_TRUE(Contains(text, L"a.txt"));
	EXPECT_FALSE(Contains(text, L"d.txt"));
	EXPECT_FALSE(Contains(text, L"c.log"));
	EXPECT_TRUE(Contains(text, MatchCountText(3)));
}

//! S: サブフォルダーも探す
TEST_F(GrepCommandLineTest, SubFolder)
{
	AddBasicFiles();
	EXPECT_EQ(4u, Grep(L"HIT", L"*.txt", L"XS"));
	EXPECT_TRUE(Contains(GetDocumentText(), L"d.txt"));
}

//! P: 行単位で数える
TEST_F(GrepCommandLineTest, OutputLine)
{
	AddBasicFiles();
	EXPECT_EQ(3u, Grep(L"HIT", L"*.txt", L"XSP"));
}

//! N: 一致しなかった行を数える
TEST_F(GrepCommandLineTest, OutputNoHitLine)
{
	AddBasicFiles();
	EXPECT_EQ(2u, Grep(L"HIT", L"*.txt", L"XSN"));
	EXPECT_TRUE(Contains(GetDocumentText(), L"none"));
}

//! F: ファイルごとに最初の 1 件だけ
TEST_F(GrepCommandLineTest, FileOnly)
{
	AddBasicFiles();
	EXPECT_EQ(2u, Grep(L"HIT", L"*.txt", L"XSF"));
}

//! L: 英大文字小文字を区別する
TEST_F(GrepCommandLineTest, CaseSensitive)
{
	AddBasicFiles();
	EXPECT_EQ(3u, Grep(L"hit", L"*.txt", L"X"));
	ResetDocument();
	EXPECT_EQ(0u, Grep(L"hit", L"*.txt", L"XL"));
}

//! W: 単語単位
TEST_F(GrepCommandLineTest, WordOnly)
{
	folder.AddFile(L"w.txt", "HIT HITS xHIT HIT.\r\n");
	EXPECT_EQ(4u, Grep(L"HIT", L"*.txt", L"X"));
	ResetDocument();
	EXPECT_EQ(2u, Grep(L"HIT", L"*.txt", L"XW"));
}

//! R: 正規表現(該当部分・行単位・否該当行)
TEST_F(GrepCommandLineTest, RegularExpression)
{
	AddBasicFiles();
	EXPECT_EQ(3u, Grep(L"H.T", L"*.txt", L"XR"));
	ResetDocument();
	EXPECT_EQ(2u, Grep(L"H.T", L"*.txt", L"XRP"));
	ResetDocument();
	EXPECT_EQ(2u, Grep(L"H.T", L"*.txt", L"XRN"));
}

//! R: 正しくない正規表現では何もしない(CSearchStringPattern::SetPattern() はメッセージを出さない)
TEST_F(GrepCommandLineTest, InvalidRegularExpression)
{
	AddBasicFiles();
	EXPECT_EQ(0u, Grep(L"(", L"*.txt", L"XR"));
	EXPECT_FALSE(Contains(GetDocumentText(), L"a.txt"));
}

//! 空の検索キー: ファイル名の一覧になる
TEST_F(GrepCommandLineTest, EmptyKeyListsFiles)
{
	AddBasicFiles();
	Grep(L"", L"*.txt", L"XS");
	const auto text = GetDocumentText();
	EXPECT_TRUE(Contains(text, L"a.txt"));
	EXPECT_TRUE(Contains(text, L"b.txt"));
	EXPECT_TRUE(Contains(text, L"d.txt"));
	EXPECT_FALSE(Contains(text, L"c.log"));
}

// ---------------------------------------------------------------------------
// 出力形式
// ---------------------------------------------------------------------------

//! 1 / 2 / 3: 出力形式が変わってもヒット数は同じ
TEST_F(GrepCommandLineTest, OutputStyles)
{
	AddBasicFiles();
	for (const auto opt : { L"XS1", L"XS2", L"XS3", L"XS2B", L"XS2D", L"XS1BD", L"XS3BD" }) {
		ResetDocument();
		EXPECT_EQ(4u, Grep(L"HIT", L"*.txt", opt)) << opt;
	}
}

//! 出力形式と行単位・ファイル名のみの組み合わせ
TEST_F(GrepCommandLineTest, OutputStylesWithEmptyKey)
{
	AddBasicFiles();
	for (const auto opt : { L"XS1", L"XS2", L"XS3", L"XS2B", L"XS2D", L"XS3BD" }) {
		ResetDocument();
		Grep(L"", L"*.txt", opt);
		EXPECT_TRUE(Contains(GetDocumentText(), L"d.txt")) << opt;
	}
}

//! H: ヘッダー・フッターを出さない
TEST_F(GrepCommandLineTest, NoHeader)
{
	AddBasicFiles();
	Grep(L"HIT", L"*.txt", L"X");
	EXPECT_TRUE(Contains(GetDocumentText(), LS(STR_GREP_EXCLUDE_FILE)));
	ResetDocument();
	Grep(L"HIT", L"*.txt", L"XH");
	const auto text = GetDocumentText();
	EXPECT_FALSE(Contains(text, LS(STR_GREP_EXCLUDE_FILE)));
	EXPECT_FALSE(Contains(text, MatchCountText(3)));
	EXPECT_TRUE(Contains(text, L"a.txt"));
}

//! U: 標準出力に出し、文書には出さない
TEST_F(GrepCommandLineTest, Stdout)
{
	AddBasicFiles();
	StdoutCapture capture(folder.Path() / L"out.stdout");	// *.txt に一致しない名前にする
	ASSERT_TRUE(capture.IsOpen());
	Grep(L"HIT", L"*.txt", L"XU");
	const auto output = capture.Read();
	EXPECT_NE(std::string::npos, output.find("a.txt"));
	EXPECT_NE(std::string::npos, output.find("HIT"));
	EXPECT_FALSE(Contains(GetDocumentText(), L"a.txt"));
}

//! 10 件ごとにまとめて出力する経路(同じファイルで 12 件)
TEST_F(GrepCommandLineTest, ManyHits)
{
	std::string lines;
	for (int i = 0; i < 12; ++i) {
		lines += "HIT\r\n";
	}
	folder.AddFile(L"many.txt", lines);
	EXPECT_EQ(12u, Grep(L"HIT", L"*.txt", L"XP"));
}

// ---------------------------------------------------------------------------
// 対象ファイル・フォルダー
// ---------------------------------------------------------------------------

//! 除外ファイル・除外フォルダー
TEST_F(GrepCommandLineTest, ExcludeFileAndFolder)
{
	AddBasicFiles();
	EXPECT_EQ(1u, Grep(L"HIT", L"*.txt;!a.txt", L"XS"));
	ResetDocument();
	EXPECT_EQ(3u, Grep(L"HIT", L"*.txt;#sub", L"XS"));
}

/*!
	「,」を含む除外(ダイアログが C0 の修正後に組み立てる形 `#"obj,old";!"a,b.txt"`)

	Grep 実行側の解析の回帰テスト。ダイアログ側の連結の不具合(Issue #2677)は GrepDialogTest.ExcludePatternsWithComma で検出する。
*/
TEST_F(GrepCommandLineTest, ExcludePatternsWithComma)
{
	for (const auto name : { L"a,b.txt", L"b.txt", L"keep.txt", LR"(obj,old\x.txt)", LR"(obj\x.txt)", LR"(old\x.txt)" }) {
		folder.AddFile(name, "HIT\r\n");
	}
	EXPECT_EQ(4u, Grep(L"HIT", LR"(*.txt;#""obj,old"";!""a,b.txt"")", L"XS"));
	const auto text = GetDocumentText();
	EXPECT_TRUE(LineContaining(text, L"a,b.txt(").empty());
	EXPECT_TRUE(LineContaining(text, LR"(obj,old\x.txt()").empty());
	EXPECT_FALSE(LineContaining(text, LR"(\obj\x.txt()").empty());
}

//! 複数のフォルダー(; 区切り)と、; を含むフォルダー名(引用符付き)
TEST_F(GrepCommandLineTest, MultipleFolders)
{
	folder.AddFile(LR"(p\x.txt)", "HIT\r\n");
	folder.AddFile(LR"(q;r\y.txt)", "HIT\r\n");
	const auto p = (folder.Path() / L"p").native();
	const auto qr = (folder.Path() / L"q;r").native();
	const auto args = std::format(LR"(-GREPMODE -GKEY="HIT" -GFILE="*.txt" -GFOLDER="{};""{}""" -GOPT=X)", p, qr);
	EXPECT_EQ(2u, RunGrep(ParseGrepCommandLine(args)));
}

//! 存在しないフォルダーでもエラーにならず 0 件
TEST_F(GrepCommandLineTest, NonexistentFolder)
{
	const auto args = std::format(LR"(-GREPMODE -GKEY="HIT" -GFILE="*.txt" -GFOLDER="{}" -GOPT=X)", (folder.Path() / L"none").native());
	EXPECT_EQ(0u, RunGrep(ParseGrepCommandLine(args)));
}

//! ファイル指定の誤り: フォルダー部分のワイルドカード、絶対パスの検索対象
TEST_F(GrepCommandLineTest, InvalidFileKeys)
{
	AddBasicFiles();
	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, _, _, _, _)).Times(2).WillRepeatedly(Return(IDOK));
	EXPECT_EQ(0u, Grep(L"HIT", LR"(sub*\a.txt)", L"X"));
	ResetDocument();
	EXPECT_EQ(0u, Grep(L"HIT", LR"(C:\x\*.txt)", L"X"));
}

//! 開けないファイルはメッセージを結果に出して続ける
TEST_F(GrepCommandLineTest, FileOpenError)
{
	AddBasicFiles();
	const auto locked = folder.Path() / L"a.txt";
	HANDLE hLocked = ::CreateFileW(locked.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	ASSERT_NE(INVALID_HANDLE_VALUE, hLocked);
	EXPECT_EQ(1u, Grep(L"HIT", L"*.txt", L"XS"));
	::CloseHandle(hLocked);
	EXPECT_TRUE(Contains(GetDocumentText(), ResourcePrefix(STR_GREP_ERR_FILEOPEN)));
}

//! X が無いと、カレントディレクトリが検索したフォルダーに移る
TEST_F(GrepCommandLineTest, ChangesCurrentDirectoryWithoutX)
{
	AddBasicFiles();
	Grep(L"HIT", L"*.txt", L"");
	EXPECT_TRUE(std::filesystem::equivalent(std::filesystem::current_path(), folder.Path()));
}

// ---------------------------------------------------------------------------
// 文字コード
// ---------------------------------------------------------------------------

//! K(自動判別): SJIS と UTF-8 の両方で日本語が見つかる
TEST_F(GrepCommandLineTest, CharsetAutoDetect)
{
	// 「テストの文章です。日本語のファイル」: 判別しやすいよう長めにする
	folder.AddFile(L"sjis.txt", "\x83\x65\x83\x58\x83\x67\x82\xCC\x95\xB6\x8F\xCD\x82\xC5\x82\xB7\x81\x42\x93\xFA\x96\x7B\x8C\xEA\x82\xCC\x83\x74\x83\x40\x83\x43\x83\x8B\r\n");
	folder.AddFile(L"utf8.txt", "\xE3\x83\x86\xE3\x82\xB9\xE3\x83\x88\xE3\x81\xAE\xE6\x96\x87\xE7\xAB\xA0\xE3\x81\xA7\xE3\x81\x99\xE3\x80\x82\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E\xE3\x81\xAE\xE3\x83\x95\xE3\x82\xA1\xE3\x82\xA4\xE3\x83\xAB\r\n");
	EXPECT_EQ(2u, Grep(L"テスト", L"*.txt", L"XK"));
}

//! -GCODE で文字コードを固定する(UTF-8 固定なら SJIS のファイルは見つからない)
TEST_F(GrepCommandLineTest, CharsetFixed)
{
	folder.AddFile(L"utf8.txt", "\xE3\x83\x86\xE3\x82\xB9\xE3\x83\x88\r\n");
	folder.AddFile(L"sjis.txt", "\x83\x65\x83\x58\x83\x67\r\n");
	EXPECT_EQ(1u, Grep(L"テスト", L"*.txt", L"X", std::format(L"-GCODE={}", int(CODE_UTF8))));
}

// ---------------------------------------------------------------------------
// Grep 置換
// ---------------------------------------------------------------------------

//! 置換: ファイルが書き換わる
TEST_F(GrepCommandLineTest, Replace)
{
	AddBasicFiles();
	Grep(L"HIT", L"a.txt", L"X", LR"(-GREPR="REP")");
	EXPECT_EQ("REP x REP\r\nnone\r\nREP\r\n", folder.ReadFile(L"a.txt"));
	EXPECT_FALSE(folder.Exists(L"a.txt.skrold"));
	EXPECT_FALSE(folder.Exists(L"a.txt.skrnew"));
}

//! 置換: 一致しないファイルは書き換えない
TEST_F(GrepCommandLineTest, ReplaceNoHitKeepsFile)
{
	AddBasicFiles();
	EXPECT_EQ(0u, Grep(L"HIT", L"b.txt", L"X", LR"(-GREPR="REP")"));
	EXPECT_EQ("none\r\n", folder.ReadFile(L"b.txt"));
}

//! 置換 + O(バックアップ): 元のファイルが .skrold に残る。既存の .skrold は置き換わる
TEST_F(GrepCommandLineTest, ReplaceWithBackup)
{
	AddBasicFiles();
	folder.AddFile(L"a.txt.skrold", "old backup\r\n");
	Grep(L"HIT", L"a.txt", L"XO", LR"(-GREPR="REP")");
	EXPECT_EQ("REP x REP\r\nnone\r\nREP\r\n", folder.ReadFile(L"a.txt"));
	EXPECT_EQ("HIT x HIT\r\nnone\r\nHIT\r\n", folder.ReadFile(L"a.txt.skrold"));
}

//! 置換 + O: 既存の .skrold を消せないと、結果にメッセージを出す
TEST_F(GrepCommandLineTest, ReplaceBackupDeleteError)
{
	AddBasicFiles();
	folder.AddFile(L"a.txt.skrold", "old backup\r\n", FILE_ATTRIBUTE_READONLY);
	Grep(L"HIT", L"a.txt", L"XO", LR"(-GREPR="REP")");
	EXPECT_TRUE(Contains(GetDocumentText(), LS(STR_GREP_REP_ERR_DELETE)));
}

//! 置換: 読み取り専用のファイルは消せないので、結果にメッセージを出す
TEST_F(GrepCommandLineTest, ReplaceReadOnlyFile)
{
	folder.AddFile(L"ro.txt", "HIT\r\n", FILE_ATTRIBUTE_READONLY);
	Grep(L"HIT", L"ro.txt", L"X", LR"(-GREPR="REP")");
	EXPECT_TRUE(Contains(GetDocumentText(), LS(STR_GREP_REP_ERR_DELETE)));
	EXPECT_EQ("HIT\r\n", folder.ReadFile(L"ro.txt"));
}

//! 置換 + 正規表現(後方参照)
TEST_F(GrepCommandLineTest, ReplaceRegularExpression)
{
	folder.AddFile(L"r.txt", "HIT HAT\r\n");
	Grep(L"H(.)T", L"r.txt", L"XR", LR"(-GREPR="Z$1Z")");
	EXPECT_EQ("ZIZ ZAZ\r\n", folder.ReadFile(L"r.txt"));
}

//! 置換 + 行単位・ファイルごと最初のみ
TEST_F(GrepCommandLineTest, ReplaceWithOutputOptions)
{
	AddBasicFiles();
	Grep(L"HIT", L"a.txt", L"XP", LR"(-GREPR="REP")");
	EXPECT_EQ("REP x REP\r\nnone\r\nREP\r\n", folder.ReadFile(L"a.txt"));
	ResetDocument();
	folder.AddFile(L"a.txt", "HIT x HIT\r\nnone\r\nHIT\r\n");
	Grep(L"HIT", L"a.txt", L"XF", LR"(-GREPR="REP")");
	EXPECT_EQ("REP x REP\r\nnone\r\nREP\r\n", folder.ReadFile(L"a.txt"));
}

//! 置換 + C(クリップボードから貼り付け)
TEST_F(GrepCommandLineTest, ReplaceWithClipboard)
{
	AddBasicFiles();
	ASSERT_TRUE(::OpenClipboard(pcEditWnd->GetHwnd()));
	::EmptyClipboard();
	constexpr std::wstring_view clip = L"CLIP";
	HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE, (clip.size() + 1) * sizeof(wchar_t));
	auto p = static_cast<wchar_t*>(::GlobalLock(hGlobal));
	std::copy_n(clip.data(), clip.size(), p);
	p[clip.size()] = L'\0';
	::GlobalUnlock(hGlobal);
	::SetClipboardData(CF_UNICODETEXT, hGlobal);
	::CloseClipboard();

	Grep(L"HIT", L"a.txt", L"XC", LR"(-GREPR="unused")");
	EXPECT_EQ("CLIP x CLIP\r\nnone\r\nCLIP\r\n", folder.ReadFile(L"a.txt"));
}

// ---------------------------------------------------------------------------
// 状態
// ---------------------------------------------------------------------------

/*!
	Grep 実行中は再入しない

	Debug ビルドでは、再入の分岐の assert_warning が ::DebugBreak() を呼び、
	デバッガーが無いと止まるので飛ばす(Release ビルドで確かめる)。
*/
TEST_F(GrepCommandLineTest, RejectsReentry)
{
#ifdef _DEBUG
	GTEST_SKIP() << "assert_warning calls ::DebugBreak() in debug builds";
#else
	CEditApp::getInstance()->m_pcGrepAgent->m_bGrepRunning = true;
	EXPECT_EQ(0xffffffffu, Grep(L"HIT", L"*.txt", L"X"));
#endif
}

//! Grep 実行中は閉じられない。実行後は閉じられる
TEST_F(GrepCommandLineTest, OnBeforeClose)
{
	auto pGrepAgent = CEditApp::getInstance()->m_pcGrepAgent;
	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, _, _, _, _)).WillOnce(Return(IDOK));
	pGrepAgent->m_bGrepRunning = true;
	EXPECT_EQ(CALLBACK_INTERRUPT, pGrepAgent->OnBeforeClose());
	pGrepAgent->m_bGrepRunning = false;
	EXPECT_EQ(CALLBACK_CONTINUE, pGrepAgent->OnBeforeClose());
}

//! 保存すると Grep モードでなくなる
TEST_F(GrepCommandLineTest, OnAfterSaveLeavesGrepMode)
{
	AddBasicFiles();
	Grep(L"HIT", L"*.txt", L"X");
	auto pGrepAgent = CEditApp::getInstance()->m_pcGrepAgent;
	EXPECT_TRUE(pGrepAgent->m_bGrepMode);
	pGrepAgent->OnAfterSave(SSaveInfo());
	EXPECT_FALSE(pGrepAgent->m_bGrepMode);
}

// ---------------------------------------------------------------------------
// 文字コードの自動判別(イレギュラーケース)
// ---------------------------------------------------------------------------

/*!
	自動判別: 各文字コードのファイルを判別し、結果の行に文字コード名が付くこと

	B3 で書き換えた「文字コード名を結果に付ける」行(DoGrepFile() の自動判別の分岐)もここで通る。
*/
TEST_F(GrepCommandLineTest, CharsetAutoDetectEachEncoding)
{
	struct EncodedFile {
		const wchar_t* name;
		std::string bytes;
		ECodeType expected;
	};
	const auto text = JapaneseLines();
	const std::vector<EncodedFile> files{
		{ L"sjis.txt", Encode(text, 932), CODE_SJIS },
		{ L"euc.txt", Encode(text, 20932), CODE_EUC },
		{ L"jis.txt", Encode(text, 50220), CODE_JIS },
		{ L"utf8.txt", Encode(text, CP_UTF8), CODE_UTF8 },
		{ L"utf8bom.txt", std::string(Utf8Bom) + Encode(text, CP_UTF8), CODE_UTF8 },
		{ L"utf16le.txt", EncodeUtf16(text, false, true), CODE_UNICODE },
		{ L"utf16be.txt", EncodeUtf16(text, true, true), CODE_UNICODEBE },
	};
	for (const auto& file : files) {
		folder.AddFile(file.name, file.bytes);
	}

	EXPECT_EQ(DWORD(files.size() * 3), Grep(L"テスト", L"*.txt", L"XK"));
	const auto result = GetDocumentText();
	for (const auto& file : files) {
		SCOPED_TRACE(file.name);
		const auto line = LineContaining(result, file.name);
		ASSERT_FALSE(line.empty());
		EXPECT_TRUE(Contains(line, CodeBracket(file.expected))) << line;
	}
}

/*!
	自動判別: BOM の無い UTF-16

	【要実測】判別できるかは判別処理次第。初回の実行結果を見て、判別できなければ期待値を 0 にし、
	テスト名とコメントに【現状の制限】と書く。
*/
TEST_F(GrepCommandLineTest, CharsetAutoDetectUtf16WithoutBom)
{
	folder.AddFile(L"u16.txt", EncodeUtf16(JapaneseLines(), false, false));
	EXPECT_EQ(3u, Grep(L"テスト", L"*.txt", L"XK"));
}

/*!
	自動判別: ASCII だけ・空・NUL を含むファイルでも止まらずに続けること
*/
TEST_F(GrepCommandLineTest, CharsetAutoDetectEdgeFiles)
{
	folder.AddFile(L"ascii.txt", "HIT\r\n");
	folder.AddFile(L"empty.txt", "");
	folder.AddFile(L"binary.txt", std::string("\x00\x01HIT\x00\xFF\xFE", 8));

	const auto hits = Grep(L"HIT", L"*.txt", L"XK");
	EXPECT_LE(1u, hits);	// binary.txt で見つかるかは判別結果による
	const auto result = GetDocumentText();
	EXPECT_TRUE(Contains(LineContaining(result, L"ascii.txt"), L"HIT"));
	EXPECT_TRUE(LineContaining(result, L"empty.txt").empty());
	EXPECT_TRUE(Contains(result, MatchCountText(int(hits))));
}

/*!
	自動判別 + 空の検索キー(ファイル名の一覧): 一覧にも文字コード名が付くこと

	B3 で集約した DoGrepFile() の「ファイル名のみ出力」の分岐を通す。
*/
TEST_F(GrepCommandLineTest, FileListWithAutoDetect)
{
	const auto text = JapaneseLines();
	folder.AddFile(L"sjis.txt", Encode(text, 932));
	folder.AddFile(L"utf8.txt", Encode(text, CP_UTF8));
	folder.AddFile(L"utf16le.txt", EncodeUtf16(text, false, true));
	Grep(L"", L"*.txt", L"XK");
	const auto result = GetDocumentText();
	EXPECT_TRUE(Contains(LineContaining(result, L"sjis.txt"), CodeBracket(CODE_SJIS)));
	EXPECT_TRUE(Contains(LineContaining(result, L"utf8.txt"), CodeBracket(CODE_UTF8)));
	EXPECT_TRUE(Contains(LineContaining(result, L"utf16le.txt"), CodeBracket(CODE_UNICODE)));
}

//! 文字コードをコードページで指定する(-GCODE=932)
TEST_F(GrepCommandLineTest, CharsetCodePage)
{
	folder.AddFile(L"sjis.txt", Encode(JapaneseLines(), 932));
	folder.AddFile(L"utf8.txt", Encode(JapaneseLines(), CP_UTF8));
	EXPECT_EQ(3u, Grep(L"テスト", L"*.txt", L"X", L"-GCODE=932"));
}

//! 自動判別で置換しても、文字コードと BOM が保たれること
TEST_F(GrepCommandLineTest, ReplacePreservesEncoding)
{
	const auto text = JapaneseLines();
	std::wstring replaced = text;
	for (size_t pos; (pos = replaced.find(L"テスト")) != std::wstring::npos; ) {
		replaced.replace(pos, 3, L"TEST");
	}
	folder.AddFile(L"sjis.txt", Encode(text, 932));
	folder.AddFile(L"utf8bom.txt", std::string(Utf8Bom) + Encode(text, CP_UTF8));
	folder.AddFile(L"utf16le.txt", EncodeUtf16(text, false, true));
	folder.AddFile(L"utf16be.txt", EncodeUtf16(text, true, true));

	Grep(L"テスト", L"*.txt", L"XK", LR"(-GREPR="TEST")");

	EXPECT_EQ(Encode(replaced, 932), folder.ReadFile(L"sjis.txt"));
	EXPECT_EQ(std::string(Utf8Bom) + Encode(replaced, CP_UTF8), folder.ReadFile(L"utf8bom.txt"));
	EXPECT_EQ(EncodeUtf16(replaced, false, true), folder.ReadFile(L"utf16le.txt"));
	EXPECT_EQ(EncodeUtf16(replaced, true, true), folder.ReadFile(L"utf16be.txt"));
}

// ---------------------------------------------------------------------------
// そのほかのイレギュラーケース
// ---------------------------------------------------------------------------

//! 行末が LF だけ・CR だけ・最終行に行末が無い。置換しても行末は元のまま
TEST_F(GrepCommandLineTest, LineEndings)
{
	folder.AddFile(L"lf.txt", "HIT\nx\nHIT\n");
	folder.AddFile(L"cr.txt", "HIT\rx\rHIT\r");
	folder.AddFile(L"noeol.txt", "x\r\nHIT");
	EXPECT_EQ(5u, Grep(L"HIT", L"*.txt", L"XP"));

	ResetDocument();
	Grep(L"HIT", L"*.txt", L"X", LR"(-GREPR="REP")");
	EXPECT_EQ("REP\nx\nREP\n", folder.ReadFile(L"lf.txt"));
	EXPECT_EQ("REP\rx\rREP\r", folder.ReadFile(L"cr.txt"));
	EXPECT_EQ("x\r\nREP", folder.ReadFile(L"noeol.txt"));
}

//! サロゲートペアの文字を検索できる
TEST_F(GrepCommandLineTest, SurrogatePair)
{
	folder.AddFile(L"s.txt", Encode(L"𠮷野家 \U00020BB7\r\n", CP_UTF8));	// 𠮷
	EXPECT_EQ(2u, Grep(L"\U00020BB7", L"*.txt", L"X", std::format(L"-GCODE={}", int(CODE_UTF8))));	// 𠮷
}

/*!
	全角英字の大文字小文字を区別しない

	【要実測】skr_towlower() が全角英字を変換するかによる。変換しなければ期待値を 0 にし、【現状の制限】と書く。
*/
TEST_F(GrepCommandLineTest, FullWidthIgnoreCase)
{
	folder.AddFile(L"fw.txt", Encode(L"ＡＢＣ\r\n", CP_UTF8));
	EXPECT_EQ(1u, Grep(L"ａｂｃ", L"*.txt", L"X", std::format(L"-GCODE={}", int(CODE_UTF8))));
}

//! 長さ 0 に一致する正規表現でも無限ループにならない
TEST_F(GrepCommandLineTest, RegexZeroLengthMatchTerminates)
{
	AddBasicFiles();
	EXPECT_LE(1u, Grep(L"^", L"a.txt", L"XR"));
	ResetDocument();
	EXPECT_LE(1u, Grep(L"x*", L"a.txt", L"XR"));
}

//! 非常に長い行(10 万文字)
TEST_F(GrepCommandLineTest, VeryLongLine)
{
	folder.AddFile(L"long.txt", std::string(100000, 'x') + "HIT\r\n");
	EXPECT_EQ(1u, Grep(L"HIT", L"*.txt", L"XP"));
}

//! 検索キーに「"」を含む(コマンドラインでは "" と書く)
TEST_F(GrepCommandLineTest, KeyWithQuote)
{
	folder.AddFile(L"q.txt", "say \"HIT\"\r\nHIT\r\n");
	EXPECT_EQ(1u, Grep(LR"(""HIT"")", L"*.txt", L"X"));
}

/*!
	5MB を超えるファイル(進捗表示と UI 更新の間隔の分岐を通す)

	否該当行(N)にして結果の出力を 0 件にし、文書への追加で時間がかからないようにする。
*/
TEST_F(GrepCommandLineTest, LargeFile)
{
	const std::string line = std::string(94, 'x') + " HIT\r\n";	// 100 バイト
	constexpr size_t lineCount = 60000;							// 6MB
	std::string data;
	data.reserve(line.size() * lineCount);
	for (size_t i = 0; i < lineCount; ++i) {
		data += line;
	}
	folder.AddFile(L"large.txt", data);
	EXPECT_EQ(0u, Grep(L"HIT", L"large.txt", L"XN"));
}

// ---------------------------------------------------------------------------
// 追補: 旧ブランチのテストからの追加
// ---------------------------------------------------------------------------

//! 無効な :HWND: は 0 件で、結果にエラーを出す
TEST_F(GrepCommandLineTest, InvalidHwndTarget)
{
	EXPECT_EQ(0u, Grep(L"HIT", L":HWND:0", L"X"));
	EXPECT_TRUE(Contains(GetDocumentText(), L"HWND handle error."));
}

/*!
	【不具合】無効な :HWND: の後も、同じウィンドウで Grep できること(Issue #xxxx で修正予定)

	DoGrep() の :HWND: のエラー経路だけが m_bGrepRunning を戻さずに return するので、
	以後の Grep が再入とみなされる。後始末は GrepTestSuite::TearDown() がフラグを戻す。
*/
TEST_F(GrepCommandLineTest, DISABLED_InvalidHwndTargetResetsRunningFlag)
{
	AddBasicFiles();
	EXPECT_EQ(0u, Grep(L"HIT", L":HWND:0", L"X"));
	// フラグが残ったまま次の Grep を呼ぶと、再入の assert_warning で止まる(Debug ビルド)ので、ここで打ち切る
	ASSERT_FALSE(CEditApp::getInstance()->m_pcGrepAgent->m_bGrepRunning);
	ResetDocument();
	EXPECT_EQ(3u, Grep(L"HIT", L"*.txt", L"X"));
}

namespace {

//! 文書のタイプ別設定のうち、文字列の色分けとエスケープ方法を一時的に変える
class StringTypeGuard {
public:
	StringTypeGuard(STypeConfig& type, bool bDisp, EStringLiteralType stringType)
		: m_type(type)
		, m_oldDisp(type.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp)
		, m_oldStringType(type.m_nStringType)
	{
		m_type.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = bDisp;
		m_type.m_nStringType = decltype(m_oldStringType)(stringType);
	}

	~StringTypeGuard()
	{
		m_type.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = m_oldDisp;
		m_type.m_nStringType = m_oldStringType;
	}

	StringTypeGuard(const StringTypeGuard&) = delete;
	StringTypeGuard& operator=(const StringTypeGuard&) = delete;

private:
	STypeConfig& m_type;
	decltype(std::declval<STypeConfig&>().m_ColorInfoArr[0].m_bDisp) m_oldDisp;
	decltype(std::declval<STypeConfig&>().m_nStringType) m_oldStringType;
};

} // namespace

/*!
	見出しの検索キーは、出力先の文書のタイプ別設定に従ってエスケープされる(EscapeStringLiteral())

	C++ 風は \ と ' と " の前に \ を付け、PL/SQL 風は ' と " を重ねる。文字列の色分けが無効ならそのまま。
*/
TEST_F(GrepCommandLineTest, HeaderEscapesKeyByStringType)
{
	struct Case {
		bool bDisp;
		EStringLiteralType stringType;
		std::wstring_view expected;
	};
	const Case cases[] = {
		{ true,  STRING_LITERAL_CPP,   LR"(x\\y\'z)" },
		{ true,  STRING_LITERAL_PLSQL, LR"(x\y''z)" },
		{ false, STRING_LITERAL_CPP,   LR"(x\y'z)" },
	};
	for (const auto& c : cases) {
		ResetDocument();
		StringTypeGuard guard(pcEditDoc->m_cDocType.GetDocumentAttributeWrite(), c.bDisp, c.stringType);
		Grep(LR"(x\y'z)", L"*.txt", L"X");
		const std::wstring quoted = std::format(LR"("{}")", c.expected);	// 引用符を含む文字列はマクロの外で作る(C2017 の回避)
		EXPECT_TRUE(Contains(GetDocumentText(), quoted)) << quoted;
	}
}

/*!
	複数のフォルダー + ベースフォルダー表示(B): フォルダーごとに見出しが 1 回ずつ出る

	DoGrep() はフォルダーごとに「ベースフォルダーを出力したか」を戻してから DoGrepTree() を呼ぶ。
	D1・D2(出力処理の整理と並列化)の前に、出力の形を固定しておく。
*/
TEST_F(GrepCommandLineTest, MultipleFoldersBaseFolderHeader)
{
	folder.AddFile(LR"(p\x.txt)", "HIT\r\n");
	folder.AddFile(LR"(q\y.txt)", "HIT\r\n");
	const auto args = std::format(LR"(-GREPMODE -GKEY="HIT" -GFILE="*.txt" -GFOLDER="{};{}" -GOPT=XB)",
		(folder.Path() / L"p").native(), (folder.Path() / L"q").native());
	EXPECT_EQ(2u, RunGrep(ParseGrepCommandLine(args)));

	std::vector<std::wstring> headers;
	std::wistringstream lines(GetDocumentText());
	for (std::wstring line; std::getline(lines, line); ) {
		if (line.ends_with(L'\r')) {
			line.pop_back();
		}
		if (line.starts_with(L"■\"")) {
			headers.push_back(line);
		}
	}
	// 引用符を含む文字列はマクロの外で作る(C2017 の回避)
	const std::wstring pEnd = LR"(\p")";
	const std::wstring qEnd = LR"(\q")";
	ASSERT_EQ(2u, headers.size());
	EXPECT_TRUE(headers[0].ends_with(pEnd)) << headers[0];
	EXPECT_TRUE(headers[1].ends_with(qEnd)) << headers[1];
}

//! 置換: 書き込み用の .skrnew を作れないと、結果にメッセージを出し、元のファイルは変わらない
TEST_F(GrepCommandLineTest, ReplaceWriteOpenError)
{
	AddBasicFiles();
	folder.AddFolder(L"a.txt.skrnew");	// 同じ名前のフォルダーがあるとファイルを作れない
	Grep(L"HIT", L"a.txt", L"X", LR"(-GREPR="REP")");
	ASSERT_FALSE(ResourcePrefix(STR_GREP_ERR_FILEWRITE).empty());
	EXPECT_TRUE(Contains(GetDocumentText(), ResourcePrefix(STR_GREP_ERR_FILEWRITE)));
	EXPECT_EQ("HIT x HIT\r\nnone\r\nHIT\r\n", folder.ReadFile(L"a.txt"));
}

/*!
	置換: 元のファイルが共有読み取りで開かれていると消せないので、結果にメッセージを出し、元のファイルは変わらない

	【現状の制限】書き出した .skrnew は消されずに残る。
*/
TEST_F(GrepCommandLineTest, ReplaceLockedFile)
{
	AddBasicFiles();
	const auto path = folder.Path() / L"a.txt";
	const HANDLE hLocked = ::CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	ASSERT_NE(INVALID_HANDLE_VALUE, hLocked);
	Grep(L"HIT", L"a.txt", L"X", LR"(-GREPR="REP")");
	::CloseHandle(hLocked);

	EXPECT_TRUE(Contains(GetDocumentText(), ResourcePrefix(STR_GREP_REP_ERR_DELETE)));
	EXPECT_EQ("HIT x HIT\r\nnone\r\nHIT\r\n", folder.ReadFile(L"a.txt"));
	EXPECT_EQ("REP x REP\r\nnone\r\nREP\r\n", folder.ReadFile(L"a.txt.skrnew"));	// 【現状の制限】
}

/*!
	置換 + O(バックアップ): 元のファイルを .skrold に移せないと、結果にメッセージを出し、元のファイルは変わらない

	【現状の制限】書き出した .skrnew は消されずに残る。
*/
TEST_F(GrepCommandLineTest, ReplaceLockedFileWithBackup)
{
	AddBasicFiles();
	const auto path = folder.Path() / L"a.txt";
	const HANDLE hLocked = ::CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	ASSERT_NE(INVALID_HANDLE_VALUE, hLocked);
	Grep(L"HIT", L"a.txt", L"XO", LR"(-GREPR="REP")");
	::CloseHandle(hLocked);

	ASSERT_FALSE(ResourcePrefix(STR_GREP_REP_ERR_REPLACE).empty());
	EXPECT_TRUE(Contains(GetDocumentText(), ResourcePrefix(STR_GREP_REP_ERR_REPLACE)));
	EXPECT_EQ("HIT x HIT\r\nnone\r\nHIT\r\n", folder.ReadFile(L"a.txt"));
	EXPECT_FALSE(folder.Exists(L"a.txt.skrold"));
	EXPECT_TRUE(folder.Exists(L"a.txt.skrnew"));	// 【現状の制限】
}

//! Grep は隠し・読み取り専用・システムのファイルも検索する(除外のオプションはファイルツリーでだけ使われる)
TEST_F(GrepCommandLineTest, SearchesHiddenReadOnlySystemFiles)
{
	folder.AddFile(L"n.txt", "HIT\r\n");
	folder.AddFile(L"h.txt", "HIT\r\n", FILE_ATTRIBUTE_HIDDEN);
	folder.AddFile(L"r.txt", "HIT\r\n", FILE_ATTRIBUTE_READONLY);
	folder.AddFile(L"s.txt", "HIT\r\n", FILE_ATTRIBUTE_SYSTEM);
	EXPECT_EQ(4u, Grep(L"HIT", L"*.txt", L"X"));
}

/*!
	文字コード固定(UTF-8): BOM だけのファイルは 0 件。不正なバイト列を含むファイルも、正しい部分は検索できる

	【要実測】不正なバイト列の変換結果による。初回の結果で期待値を確定する。
*/
TEST_F(GrepCommandLineTest, FixedUtf8BomOnlyAndInvalidBytes)
{
	folder.AddFile(L"bomonly.txt", Utf8Bom);
	folder.AddFile(L"invalid.txt", std::string("abc\xFF\xFE" "def\r\n"));	// "\xFE" と "def" を分けて書く(続けると 16 進の一部になる)
	EXPECT_EQ(1u, Grep(L"def", L"*.txt", L"X", std::format(L"-GCODE={}", int(CODE_UTF8))));
	EXPECT_TRUE(LineContaining(GetDocumentText(), L"bomonly.txt").empty());
}

/*!
	-GCODE に範囲外の値: CCommandLine はそのまま持ち、Grep は止まらずに終わる

	【要実測】ファイルを開くときの文字コードの扱いによる。初回の結果で期待値を確定する。
	落ちる・止まる場合は不具合として別の Issue にする。
*/
TEST_F(GrepCommandLineTest, CharsetOutOfRange)
{
	AddBasicFiles();
	EXPECT_EQ(ECodeType(99999), ParseGrepCommandLine(Args(L"HIT", L"a.txt", L"X", L"-GCODE=99999")).nGrepCharSet);
	EXPECT_EQ(3u, Grep(L"HIT", L"a.txt", L"X", L"-GCODE=99999"));
}

// ---------------------------------------------------------------------------
// フォルダーの一覧(CGrepAgent の static 関数)
// ---------------------------------------------------------------------------

//! ChopYen: 末尾の \ を 1 つだけ取り除く。ルートの C:\ も C: になる
TEST(GrepFolderList, ChopYen)
{
	EXPECT_EQ(L"C:", CGrepAgent::ChopYen(L"C:\\"));
	EXPECT_EQ(L"C:\\work", CGrepAgent::ChopYen(L"C:\\work\\"));
	EXPECT_EQ(L"C:\\work\\", CGrepAgent::ChopYen(L"C:\\work\\\\"));
	EXPECT_EQ(L"C:\\work", CGrepAgent::ChopYen(L"C:\\work"));
	EXPECT_EQ(L"", CGrepAgent::ChopYen(L""));
}

//! CreateFolders: ; で分け、引用符を取り除く。存在しないフォルダーは長い名前に直せないのでそのまま
TEST(GrepFolderList, CreateFoldersSplitsAndUnquotes)
{
	TempFolder folder;
	const auto a = (folder.Path() / L"none_a").native();
	const auto b = (folder.Path() / L"none;b").native();
	const auto list = std::format(L"{};\"{}\"", a, b);
	std::vector<std::wstring> paths;
	CGrepAgent::CreateFolders(list.c_str(), paths);
	EXPECT_EQ((std::vector<std::wstring>{ a, b }), paths);
}

//! CreateFolders: 8.3 形式の短い名前を長い名前に直す
TEST(GrepFolderList, CreateFoldersResolvesShortName)
{
	TempFolder folder;
	const std::wstring longName = L"long_folder_name_for_grep";
	folder.AddFolder(longName);
	const auto longPath = folder.Path() / longName;

	std::wstring shortPath(MAX_PATH, L'\0');
	shortPath.resize(::GetShortPathNameW(longPath.c_str(), shortPath.data(), MAX_PATH));
	if (shortPath.empty() || std::filesystem::path(shortPath).filename() == longName) {
		GTEST_SKIP() << "8.3 short names are not available on this volume";
	}

	std::vector<std::wstring> paths;
	CGrepAgent::CreateFolders(shortPath.c_str(), paths);
	ASSERT_EQ(1u, paths.size());
	EXPECT_EQ(longName, std::filesystem::path(paths[0]).filename().native());
}

// ---------------------------------------------------------------------------
// 出力のスナップショット(旧ブランチのテストから)
// ---------------------------------------------------------------------------

/*!
	@brief Grep の出力をそのまま比べるテスト

	文書の表示のテキストは読まない。-GOPT=U で標準出力に出したバイト列を、
	文書の文字コードで戻して比べる。H でヘッダー・フッターを出さないので、
	出力は結果の行だけになる。
*/
struct GrepOutputSnapshotTest : public GrepCommandLineTest {
	//! 旧テストと同じデータ(1 行目の 7 文字目に needle)
	static constexpr std::string_view SNAPSHOT_TEXT = "alpha needle one\r\nbravo two\r\n";

	//! 長い名前のパス(出力のパスは CreateFolders() で長い名前になる)
	static std::wstring LongPath(const std::filesystem::path& path)
	{
		const DWORD cch = ::GetLongPathNameW(path.c_str(), nullptr, 0);
		if (cch == 0) {
			return path.native();
		}
		std::wstring buf(cch, L'\0');
		buf.resize(::GetLongPathNameW(path.c_str(), buf.data(), cch));
		return buf;
	}

	//! 標準出力のバイト列を文書の文字コードで戻す(AddTail() の逆)
	static std::wstring Decode(const std::string& bytes)
	{
		std::unique_ptr<CCodeBase> pcCodeBase(CCodeFactory::CreateCodeBase(pcEditDoc->GetDocumentEncoding(), 0));
		CMemory cmemSrc(bytes.data(), bytes.size());
		CNativeW cmemDst;
		pcCodeBase->CodeToUnicode(cmemSrc, &cmemDst);
		return std::wstring(cmemDst.GetStringPtr(), cmemDst.GetStringLength());
	}

	//! -GOPT に XHU を足して Grep し、標準出力を返す
	std::wstring GrepStdout(std::wstring_view key, std::wstring_view file, std::wstring_view opt, DWORD* pHitCount = nullptr) const
	{
		StdoutCapture capture(folder.Path() / L"out.stdout");	// 検索対象に一致しない名前にする
		EXPECT_TRUE(capture.IsOpen());
		const auto hitCount = Grep(key, file, std::format(L"XHU{}", opt));
		if (pHitCount) {
			*pHitCount = hitCount;
		}
		return Decode(capture.Read());
	}
};

//! 形式 1(ノーマル): 該当部分・該当行・否該当行
TEST_F(GrepOutputSnapshotTest, Style1LineTypes)
{
	folder.AddFile(L"snap.txt", SNAPSHOT_TEXT);
	const auto path = LongPath(folder.Path() / L"snap.txt");

	const std::wstring area = path + L"(1,7): needle\r\n";
	const std::wstring line = path + L"(1,7): alpha needle one\r\n";
	const std::wstring noHit = path + L"(2,1): bravo two\r\n";
	EXPECT_EQ(area, GrepStdout(L"needle", L"snap.txt", L"1"));
	EXPECT_EQ(line, GrepStdout(L"needle", L"snap.txt", L"1P"));
	EXPECT_EQ(noHit, GrepStdout(L"needle", L"snap.txt", L"1N"));
}

//! 形式 1: 1 行に 2 つ。該当部分なら 2 行、該当行なら 1 行
TEST_F(GrepOutputSnapshotTest, Style1MultipleHitsInLine)
{
	folder.AddFile(L"snap.txt", "needle x needle\r\n");
	const auto path = LongPath(folder.Path() / L"snap.txt");

	const std::wstring area = path + L"(1,1): needle\r\n" + path + L"(1,10): needle\r\n";
	const std::wstring line = path + L"(1,1): needle x needle\r\n";
	DWORD hitCount = 0;
	EXPECT_EQ(area, GrepStdout(L"needle", L"snap.txt", L"1", &hitCount));
	EXPECT_EQ(2u, hitCount);
	EXPECT_EQ(line, GrepStdout(L"needle", L"snap.txt", L"1P", &hitCount));
	EXPECT_EQ(1u, hitCount);
}

//! 形式 2(WZ 風): ファイルの見出しは 1 回、位置は (%6d,%-5d)
TEST_F(GrepOutputSnapshotTest, Style2HeaderOncePerFile)
{
	folder.AddFile(L"snap.txt", "needle\r\nx\r\nneedle\r\n");
	const auto path = LongPath(folder.Path() / L"snap.txt");

	const std::wstring expected = std::format(L"■\"{}\"\r\n", path)
		+ L"・(     1,1    ): needle\r\n"
		+ L"・(     3,1    ): needle\r\n";
	EXPECT_EQ(expected, GrepStdout(L"needle", L"snap.txt", L"2P"));
}

//! 形式 3(結果のみ): パスも位置も出さない
TEST_F(GrepOutputSnapshotTest, Style3LineTypes)
{
	folder.AddFile(L"snap.txt", SNAPSHOT_TEXT);

	EXPECT_EQ(L"needle\r\n", GrepStdout(L"needle", L"snap.txt", L"3"));
	EXPECT_EQ(L"alpha needle one\r\n", GrepStdout(L"needle", L"snap.txt", L"3P"));
}

//! B(ベースフォルダー表示): 見出しは 1 回、結果は「・」＋ベースからの相対パス。ファイルが先、サブフォルダーが後
TEST_F(GrepOutputSnapshotTest, BaseFolderRelativePath)
{
	folder.AddFile(L"snap.txt", SNAPSHOT_TEXT);
	folder.AddFile(LR"(sub\snap.txt)", SNAPSHOT_TEXT);
	const auto base = LongPath(folder.Path());

	// 形式 1 でも、B のときは結果の行の先頭に「・」が付く
	const std::wstring expected = std::format(L"■\"{}\"\r\n", base)
		+ L"・snap.txt(1,7): alpha needle one\r\n"
		+ LR"(・sub\snap.txt(1,7): alpha needle one)" + L"\r\n";
	EXPECT_EQ(expected, GrepStdout(L"needle", L"*.txt", L"1PSB"));
}

//! B+D(フォルダー毎に表示): ベースは ◎、フォルダーは ■、結果はファイル名
TEST_F(GrepOutputSnapshotTest, BaseAndSeparateFolderHeaders)
{
	folder.AddFile(L"snap.txt", SNAPSHOT_TEXT);
	folder.AddFile(LR"(sub\snap.txt)", SNAPSHOT_TEXT);
	const auto base = LongPath(folder.Path());

	DWORD hitCount = 0;
	const auto output = GrepStdout(L"needle", L"*.txt", L"2PSBD", &hitCount);
	EXPECT_EQ(2u, hitCount);

	const std::wstring baseHeader = std::format(L"◎\"{}\"\r\n", base);
	const std::wstring subHeader = L"■\"sub\"\r\n";
	const std::wstring hitLine = L"・(     1,7    ): alpha needle one\r\n";
	EXPECT_TRUE(output.starts_with(baseHeader)) << output;
	EXPECT_TRUE(Contains(output, subHeader)) << output;
	EXPECT_LT(output.find(hitLine), output.rfind(hitLine)) << output;	// 2 回出る
	EXPECT_LT(output.find(subHeader), output.rfind(hitLine)) << output;	// サブフォルダーの結果は見出しの後
}

//! 改行が混ざっても行番号がずれない(CRLF・LF・CR・改行なしの最終行)
TEST_F(GrepOutputSnapshotTest, MixedLineEndings)
{
	folder.AddFile(L"mixed.txt", "HIT\r\nx\nHIT\rHIT");
	const auto path = LongPath(folder.Path() / L"mixed.txt");

	const std::wstring expected = path + L"(1,1): HIT\r\n"
		+ path + L"(3,1): HIT\r\n"
		+ path + L"(4,1): HIT\r\n";
	DWORD hitCount = 0;
	EXPECT_EQ(expected, GrepStdout(L"HIT", L"mixed.txt", L"1P", &hitCount));
	EXPECT_EQ(3u, hitCount);
}

} // namespace grep_test
