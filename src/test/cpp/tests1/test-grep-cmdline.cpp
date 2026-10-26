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
#include "basis/EditInfo.h"
#include "config/system_constants.h"
#include "util/os/MockUser32.hpp"

#include <algorithm>
#include <cstdint>
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

//! リアルタイム表示: 結果を早めに出す経路を通っても、件数と結果は変わらない
TEST_F(GrepCommandLineTest, RealTimeView)
{
	AddBasicFiles();
	auto& search = GetDllShareData().m_Common.m_sSearch;
	const auto saved = search.m_bGrepRealTimeView;
	search.m_bGrepRealTimeView = TRUE;
	const auto hitCount = Grep(L"HIT", L"*.txt", L"XS");
	search.m_bGrepRealTimeView = saved;	// 後続のテストのために戻す

	EXPECT_THAT(hitCount, Eq(4u));
	EXPECT_THAT(Contains(GetDocumentText(), L"d.txt"), IsTrue());
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

/*!
	存在しないフォルダーを指定すると、メッセージを出して検索しない(Issue #2707)

	-GOPT=X の有無に依らない。X が無いと、検索後に存在しないフォルダーへ移ろうとして std::system_error になっていた。
*/
TEST_F(GrepCommandLineTest, NonexistentFolder)
{
	AddBasicFiles();
	auto pUser32 = (MockUser32*)User32::getInstance();
	const std::wstring message = LS(STR_DLGGREP5);
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(message), _, _, _)).Times(2).WillRepeatedly(Return(IDOK));

	const auto before = std::filesystem::current_path();
	const auto none = (folder.Path() / L"none").native();
	for (const auto opt : { L"X", L"" }) {
		ResetDocument();
		const auto args = std::format(LR"(-GREPMODE -GKEY="HIT" -GFILE="*.txt" -GFOLDER="{}" -GOPT={})", none, opt);
		DWORD hitCount = 1;
		EXPECT_NO_THROW(hitCount = RunGrep(ParseGrepCommandLine(args))) << opt;
		EXPECT_THAT(hitCount, Eq(0u)) << opt;
		EXPECT_THAT(GetDocumentText().empty(), IsTrue()) << opt;
	}
	EXPECT_THAT(std::filesystem::equivalent(std::filesystem::current_path(), before), IsTrue());
}

//! フォルダーの指定に 1 つでも存在しないもの(またはフォルダーでないもの)があれば、全体を検索しない
TEST_F(GrepCommandLineTest, InvalidFolderStopsBeforeSearch)
{
	AddBasicFiles();
	const auto base = folder.Path().native();
	const auto none = (folder.Path() / L"none").native();
	const auto file = (folder.Path() / L"a.txt").native();
	const std::vector<std::wstring> lists{
		std::format(L"{};{}", none, base),	// 先頭が存在しない
		std::format(L"{};{}", base, none),	// 2 つ目が存在しない
		file,								// ファイルはフォルダーではない
	};

	auto pUser32 = (MockUser32*)User32::getInstance();
	const std::wstring message = LS(STR_DLGGREP5);
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(message), _, _, _)).Times(int(lists.size())).WillRepeatedly(Return(IDOK));

	for (const auto& list : lists) {
		ResetDocument();
		const auto args = std::format(LR"(-GREPMODE -GKEY="HIT" -GFILE="*.txt" -GFOLDER="{}" -GOPT=X)", list);
		EXPECT_THAT(RunGrep(ParseGrepCommandLine(args)), Eq(0u)) << list;
		EXPECT_THAT(GetDocumentText().empty(), IsTrue()) << list;
	}
}

//! Grep 置換: フォルダーに存在しないものがあれば、ファイルを書き換える前に止まる
TEST_F(GrepCommandLineTest, InvalidFolderDoesNotReplace)
{
	AddBasicFiles();
	auto pUser32 = (MockUser32*)User32::getInstance();
	const std::wstring message = LS(STR_DLGGREP5);
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(message), _, _, _)).WillOnce(Return(IDOK));

	const auto list = std::format(L"{};{}", folder.Path().native(), (folder.Path() / L"none").native());
	const auto args = std::format(LR"(-GREPMODE -GKEY="HIT" -GFILE="a.txt" -GFOLDER="{}" -GOPT=X -GREPR="REP")", list);
	EXPECT_THAT(RunGrep(ParseGrepCommandLine(args)), Eq(0u));
	EXPECT_THAT(folder.ReadFile(L"a.txt"), Eq(std::string("HIT x HIT\r\nnone\r\nHIT\r\n")));
	EXPECT_THAT(folder.Exists(L"a.txt.skrnew"), IsFalse());
}

//! 末尾に \ が付いた実在するフォルダーは弾かない(FindFirstFile 系の確認では末尾の \ で失敗するため)
TEST_F(GrepCommandLineTest, FolderWithTrailingBackslash)
{
	AddBasicFiles();
	auto gi = ParseGrepCommandLine(Args(L"HIT", L"*.txt", L"X"));
	const auto withYen = folder.Path().native() + L"\\";
	gi.cmGrepFolder.SetString(withYen.c_str());
	EXPECT_THAT(RunGrep(gi), Eq(3u));
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
	　↓
	CGrepAgent::DoGrepの再入防止機構を変更し、ビルド種別に依らずテストできるよう改善済み。
*/
TEST_F(GrepCommandLineTest, RejectsReentry)
{
	// Grep実行中のフラグを立てる
	CEditApp::getInstance()->m_pcGrepAgent->m_bGrepRunning = true;

	EXPECT_THAT(([this] {
			// Grep実行中のフラグが立った状態でGrep開始要求する
			Grep(L"HIT", L"*.txt", L"X");
		}),
		ThrowsMessage<std::domain_error>(Eq("The DoGrep method is not reentrant"))
	);

	// Grep実行中のフラグはTearDownで落ちる
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

	//! 標準出力のバイト列を文書の文字コードで戻す(結果を標準出力に書くときの逆)
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

//! 入れ子のフォルダー: フォルダーごとにファイルが先、サブフォルダーが後(深さ優先)
TEST_F(GrepOutputSnapshotTest, NestedFoldersOrder)
{
	for (const auto name : { L"a.txt", L"b.txt", LR"(sub1\x.txt)", LR"(sub1\deep\y.txt)", LR"(sub2\z.txt)" }) {
		folder.AddFile(name, SNAPSHOT_TEXT);
	}
	const auto base = LongPath(folder.Path());

	const std::wstring expected = std::format(L"■\"{}\"\r\n", base)
		+ L"・a.txt(1,7): alpha needle one\r\n"
		+ L"・b.txt(1,7): alpha needle one\r\n"
		+ LR"(・sub1\x.txt(1,7): alpha needle one)" + L"\r\n"
		+ LR"(・sub1\deep\y.txt(1,7): alpha needle one)" + L"\r\n"
		+ LR"(・sub2\z.txt(1,7): alpha needle one)" + L"\r\n";
	EXPECT_THAT(GrepStdout(L"needle", L"*.txt", L"1PSB"), Eq(expected));
}

//! D(フォルダー毎に表示): 見出しはフォルダーごとに 1 回。ヒットの無いフォルダーには出ない
TEST_F(GrepOutputSnapshotTest, SeparateFolderHeaderOncePerFolder)
{
	folder.AddFile(LR"(sub1\a.txt)", SNAPSHOT_TEXT);
	folder.AddFile(LR"(sub1\b.txt)", SNAPSHOT_TEXT);
	folder.AddFile(LR"(nohit\c.txt)", "bravo two\r\n");
	folder.AddFile(LR"(sub2\d.txt)", SNAPSHOT_TEXT);

	DWORD hitCount = 0;
	const auto output = GrepStdout(L"needle", L"*.txt", L"2PSBD", &hitCount);

	const std::wstring sub1Header = L"■\"sub1\"\r\n";
	const std::wstring sub2Header = L"■\"sub2\"\r\n";
	const auto first = output.find(sub1Header);
	EXPECT_THAT(hitCount, Eq(3u));
	ASSERT_THAT(first, Ne(std::wstring::npos)) << output;
	EXPECT_THAT(output.find(sub1Header, first + 1), Eq(std::wstring::npos)) << output;	// 2 つのファイルで 1 回
	EXPECT_THAT(Contains(output, L"■\"nohit\""), IsFalse()) << output;
	EXPECT_THAT(first, Lt(output.find(sub2Header))) << output;
}

//! 検索キーにフォルダーを含む(sub\*.txt): 結果の相対パスにフォルダーが付く
TEST_F(GrepOutputSnapshotTest, KeyWithSubFolderPath)
{
	folder.AddFile(L"snap.txt", SNAPSHOT_TEXT);
	folder.AddFile(LR"(sub\snap.txt)", SNAPSHOT_TEXT);
	const auto base = LongPath(folder.Path());

	const std::wstring expected = std::format(L"■\"{}\"\r\n", base)
		+ LR"(・sub\snap.txt(1,7): alpha needle one)" + L"\r\n";
	EXPECT_THAT(GrepStdout(L"needle", LR"(sub\*.txt)", L"1PB"), Eq(expected));
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

/*!
	存在しないフォルダー + U(標準出力): メッセージボックスを出さず、同じ文言を標準出力に出す(Issue #2707)

	メッセージボックスだと、ボタンを押すまでバッチが止まる。X を付けないのは Issue の再現条件。
*/
TEST_F(GrepOutputSnapshotTest, NonexistentFolderToStdout)
{
	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, _, _, _, _)).Times(0);

	StdoutCapture capture(folder.Path() / L"out.stdout");	// 検索対象に一致しない名前にする
	ASSERT_TRUE(capture.IsOpen());
	const auto args = std::format(LR"(-GREPMODE -GKEY="HIT" -GFILE="*.txt" -GFOLDER="{}" -GOPT=U)", (folder.Path() / L"none").native());
	EXPECT_THAT(RunGrep(ParseGrepCommandLine(args)), Eq(0u));

	const std::wstring expected = std::wstring(LS(STR_DLGGREP5)) + L"\r\n";
	EXPECT_THAT(Decode(capture.Read()), Eq(expected));
}

// ---------------------------------------------------------------------------
// Grep 置換の細部
// ---------------------------------------------------------------------------

/*!
	@brief Grep 置換の細部のテスト

	置換した後のファイル、戻り値(置換した数)、標準出力の結果を比べる。
*/
struct GrepReplaceDetailTest : public GrepOutputSnapshotTest {
	//! クリップボードに文字列を置く(C: クリップボードから貼り付け)
	static void SetClipboardText(std::wstring_view text)
	{
		ASSERT_THAT(::OpenClipboard(pcEditWnd->GetHwnd()), IsTrue());
		::EmptyClipboard();
		HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE, (text.size() + 1) * sizeof(wchar_t));
		auto p = static_cast<wchar_t*>(::GlobalLock(hGlobal));
		std::copy_n(text.data(), text.size(), p);
		p[text.size()] = L'\0';
		::GlobalUnlock(hGlobal);
		::SetClipboardData(CF_UNICODETEXT, hGlobal);
		::CloseClipboard();
	}

	//! -GOPT に XHU を足し、-GREPR を付けて置換し、標準出力を返す
	std::wstring ReplaceStdout(std::wstring_view key, std::wstring_view file, std::wstring_view opt, std::wstring_view replace, DWORD* pHitCount = nullptr) const
	{
		StdoutCapture capture(folder.Path() / L"out.stdout");	// 検索対象に一致しない名前にする
		EXPECT_THAT(capture.IsOpen(), IsTrue());
		const auto hitCount = Grep(key, file, std::format(L"XHU{}", opt), std::format(LR"(-GREPR="{}")", replace));
		if (pHitCount) {
			*pHitCount = hitCount;
		}
		return Decode(capture.Read());
	}
};

//! 単語単位: 1 行に複数の単語を置換し、単語の一部(HITS・xHIT・HIT_X)は置換しない。結果は一致ごとに出す
TEST_F(GrepReplaceDetailTest, WordOnly)
{
	folder.AddFile(L"w.txt", "HIT HITS xHIT HIT.\r\nHIT_X HIT\r\n");
	const auto path = LongPath(folder.Path() / L"w.txt");

	const std::wstring expected = path + L"(1,1): HIT\r\n" + path + L"(1,15): HIT\r\n" + path + L"(2,7): HIT\r\n";
	DWORD hitCount = 0;
	EXPECT_THAT(ReplaceStdout(L"HIT", L"w.txt", L"1W", L"REP", &hitCount), Eq(expected));
	EXPECT_THAT(hitCount, Eq(3u));
	EXPECT_THAT(folder.ReadFile(L"w.txt"), Eq("REP HITS xHIT REP.\r\nHIT_X REP\r\n"));
}

//! 文字列: 1 行に複数・行末の直前・最終行(行末なし)を置換し、置換した数を返す
TEST_F(GrepReplaceDetailTest, StringMultipleInLine)
{
	folder.AddFile(L"s.txt", "aaaa HIT\r\nHITHIT\r\nxHIT");
	EXPECT_THAT(Grep(L"HIT", L"s.txt", L"X", LR"(-GREPR="REP")"), Eq(4u));
	EXPECT_THAT(folder.ReadFile(L"s.txt"), Eq("aaaa REP\r\nREPREP\r\nxREP"));
}

//! 文字列: 一致の後ろから次を探すので、重なる一致は置換しない
TEST_F(GrepReplaceDetailTest, StringDoesNotOverlap)
{
	folder.AddFile(L"s.txt", "aaaaa\r\n");
	EXPECT_THAT(Grep(L"aa", L"s.txt", L"X", LR"(-GREPR="b")"), Eq(2u));
	EXPECT_THAT(folder.ReadFile(L"s.txt"), Eq("bba\r\n"));
}

//! 文字列: L が無ければ大文字小文字を区別せずに置換し、L があれば区別する
TEST_F(GrepReplaceDetailTest, StringCase)
{
	folder.AddFile(L"s.txt", "hit Hit HIT\r\n");
	EXPECT_THAT(Grep(L"HIT", L"s.txt", L"X", LR"(-GREPR="REP")"), Eq(3u));
	EXPECT_THAT(folder.ReadFile(L"s.txt"), Eq("REP REP REP\r\n"));

	ResetDocument();
	folder.AddFile(L"s.txt", "hit Hit HIT\r\n");
	EXPECT_THAT(Grep(L"HIT", L"s.txt", L"XL", LR"(-GREPR="REP")"), Eq(1u));
	EXPECT_THAT(folder.ReadFile(L"s.txt"), Eq("hit Hit REP\r\n"));
}

//! 正規表現(普通): 1 行の一致をまとめて置換し、置換した数を足す
TEST_F(GrepReplaceDetailTest, RegexCountsEachMatch)
{
	folder.AddFile(L"r.txt", "HIT HAT\r\nHUT\r\n");
	EXPECT_THAT(Grep(L"H(.)T", L"r.txt", L"XR", LR"(-GREPR="Z$1Z")"), Eq(3u));
	EXPECT_THAT(folder.ReadFile(L"r.txt"), Eq("ZIZ ZAZ\r\nZUZ\r\n"));
}

//! 正規表現 + C(クリップボードから貼り付け): 一致を探して、前の部分と貼り付ける文字列をつなぐ
TEST_F(GrepReplaceDetailTest, PasteRegexMultipleInLine)
{
	folder.AddFile(L"p.txt", "aXbXc\r\n");
	SetClipboardText(L"Z");
	EXPECT_THAT(Grep(L"X", L"p.txt", L"XRC", LR"(-GREPR="unused")"), Eq(2u));
	EXPECT_THAT(folder.ReadFile(L"p.txt"), Eq("aZbZc\r\n"));
}

//! F(ファイル毎最初のみ): 結果に出すのは最初の 1 件だけで、置換はすべて行い、置換した数を返す
TEST_F(GrepReplaceDetailTest, FileOnlyReplacesAll)
{
	folder.AddFile(L"a.txt", "HIT x HIT\r\nnone\r\nHIT\r\n");
	const auto path = LongPath(folder.Path() / L"a.txt");

	DWORD hitCount = 0;
	EXPECT_THAT(ReplaceStdout(L"HIT", L"a.txt", L"1F", L"REP", &hitCount), Eq(path + L"(1,1): HIT\r\n"));
	EXPECT_THAT(hitCount, Eq(3u));
	EXPECT_THAT(folder.ReadFile(L"a.txt"), Eq("REP x REP\r\nnone\r\nREP\r\n"));
}

//! P(該当行): 1 行に複数の一致があっても結果は 1 行で、置換はすべて行う
TEST_F(GrepReplaceDetailTest, OutputLineOncePerLine)
{
	folder.AddFile(L"a.txt", "HIT x HIT\r\n");
	const auto path = LongPath(folder.Path() / L"a.txt");

	DWORD hitCount = 0;
	EXPECT_THAT(ReplaceStdout(L"HIT", L"a.txt", L"1P", L"REP", &hitCount), Eq(path + L"(1,1): HIT x HIT\r\n"));
	EXPECT_THAT(hitCount, Eq(2u));
	EXPECT_THAT(folder.ReadFile(L"a.txt"), Eq("REP x REP\r\n"));
}

//! 置換後の文字列が空: 一致を消す。ファイルが空になれば 0 バイトのファイルになる
TEST_F(GrepReplaceDetailTest, EmptyReplacement)
{
	folder.AddFile(L"s.txt", "aHITb HIT\r\n");
	EXPECT_THAT(Grep(L"HIT", L"s.txt", L"X", LR"(-GREPR="")"), Eq(2u));
	EXPECT_THAT(folder.ReadFile(L"s.txt"), Eq("ab \r\n"));

	ResetDocument();
	folder.AddFile(L"w.txt", "HIT x HIT\r\n");
	EXPECT_THAT(Grep(L"HIT", L"w.txt", L"XW", LR"(-GREPR="")"), Eq(2u));
	EXPECT_THAT(folder.ReadFile(L"w.txt"), Eq(" x \r\n"));

	ResetDocument();
	folder.AddFile(L"e.txt", "HIT");
	EXPECT_THAT(Grep(L"HIT", L"e.txt", L"X", LR"(-GREPR="")"), Eq(1u));
	EXPECT_THAT(folder.ReadFile(L"e.txt"), IsEmpty());
	EXPECT_THAT(folder.Exists(L"e.txt.skrnew"), IsFalse());
}

//! 文字列・単語単位: 置換後の文字列の $ や \ や & は、そのままの文字として入る
TEST_F(GrepReplaceDetailTest, ReplacementTakenLiterally)
{
	folder.AddFile(L"s.txt", "HIT\r\n");
	EXPECT_THAT(Grep(L"HIT", L"s.txt", L"X", LR"(-GREPR="a$1\n&b")"), Eq(1u));
	EXPECT_THAT(folder.ReadFile(L"s.txt"), Eq("a$1\\n&b\r\n"));
}

//! 置換後の文字列が検索語を含んでも、置換した後ろから次を探すので、もう一度は置換しない
TEST_F(GrepReplaceDetailTest, ReplacementContainsKey)
{
	folder.AddFile(L"s.txt", "HIT HIT\r\n");
	EXPECT_THAT(Grep(L"HIT", L"s.txt", L"X", LR"(-GREPR="HIT!")"), Eq(2u));
	EXPECT_THAT(folder.ReadFile(L"s.txt"), Eq("HIT! HIT!\r\n"));

	ResetDocument();
	folder.AddFile(L"w.txt", "HIT HIT\r\n");
	EXPECT_THAT(Grep(L"HIT", L"w.txt", L"XW", LR"(-GREPR="HIT!")"), Eq(2u));
	EXPECT_THAT(folder.ReadFile(L"w.txt"), Eq("HIT! HIT!\r\n"));
}

//! 置換後も内容が同じ(検索語と同じ文字列で置換): ファイルを書き直し、O ならバックアップも作る
TEST_F(GrepReplaceDetailTest, SameContentStillRewrites)
{
	folder.AddFile(L"s.txt", "HIT\r\n");
	EXPECT_THAT(Grep(L"HIT", L"s.txt", L"XO", LR"(-GREPR="HIT")"), Eq(1u));
	EXPECT_THAT(folder.ReadFile(L"s.txt"), Eq("HIT\r\n"));
	EXPECT_THAT(folder.ReadFile(L"s.txt.skrold"), Eq("HIT\r\n"));
}

//! 一致しようがない検索語(空・単語単位で空白だけ)は 0 件で、ファイルを書き換えない
TEST_F(GrepReplaceDetailTest, KeyThatCannotMatch)
{
	folder.AddFile(L"s.txt", "HIT x\r\n");
	EXPECT_THAT(Grep(L"", L"s.txt", L"X", LR"(-GREPR="REP")"), Eq(0u));
	ResetDocument();
	EXPECT_THAT(Grep(L" ", L"s.txt", L"XW", LR"(-GREPR="REP")"), Eq(0u));
	EXPECT_THAT(folder.ReadFile(L"s.txt"), Eq("HIT x\r\n"));
	EXPECT_THAT(folder.Exists(L"s.txt.skrnew"), IsFalse());
}

//! 正規表現(普通): 行末つきの検索語(HIT$)は、CRLF・LF・CR・行末なしのどれでも、行末を残して置換する
TEST_F(GrepReplaceDetailTest, RegexLineEndKeepsEol)
{
	folder.AddFile(L"crlf.txt", "HIT\r\nx HIT\r\n");
	folder.AddFile(L"lf.txt", "HIT\nx HIT\n");
	folder.AddFile(L"cr.txt", "HIT\rx HIT\r");
	folder.AddFile(L"noeol.txt", "HIT\r\nx HIT");
	EXPECT_THAT(Grep(L"HIT$", L"*.txt", L"XR", LR"(-GREPR="REP")"), Eq(8u));
	EXPECT_THAT(folder.ReadFile(L"crlf.txt"), Eq("REP\r\nx REP\r\n"));
	EXPECT_THAT(folder.ReadFile(L"lf.txt"), Eq("REP\nx REP\n"));
	EXPECT_THAT(folder.ReadFile(L"cr.txt"), Eq("REP\rx REP\r"));
	EXPECT_THAT(folder.ReadFile(L"noeol.txt"), Eq("REP\r\nx REP"));
}

//! N(否該当行)を指定しても、置換では P(該当行)として扱う(GrepInfo::Normalized())
TEST_F(GrepReplaceDetailTest, NoHitLineActsAsOutputLine)
{
	folder.AddFile(L"a.txt", "HIT x HIT\r\nnone\r\n");
	const auto path = LongPath(folder.Path() / L"a.txt");

	DWORD hitCount = 0;
	EXPECT_THAT(ReplaceStdout(L"HIT", L"a.txt", L"1N", L"REP", &hitCount), Eq(path + L"(1,1): HIT x HIT\r\n"));
	EXPECT_THAT(hitCount, Eq(2u));
	EXPECT_THAT(folder.ReadFile(L"a.txt"), Eq("REP x REP\r\nnone\r\n"));
}

//! O(バックアップ): 一致しないファイルは、既にある .skrold を消さず、上書きもしない
TEST_F(GrepReplaceDetailTest, BackupNoHitKeepsOldBackup)
{
	folder.AddFile(L"a.txt", "HIT\r\n");
	folder.AddFile(L"b.txt", "none\r\n");
	folder.AddFile(L"b.txt.skrold", "keep\r\n");
	EXPECT_THAT(Grep(L"HIT", L"*.txt", L"XO", LR"(-GREPR="REP")"), Eq(1u));
	EXPECT_THAT(folder.ReadFile(L"a.txt"), Eq("REP\r\n"));
	EXPECT_THAT(folder.ReadFile(L"a.txt.skrold"), Eq("HIT\r\n"));
	EXPECT_THAT(folder.ReadFile(L"b.txt"), Eq("none\r\n"));
	EXPECT_THAT(folder.ReadFile(L"b.txt.skrold"), Eq("keep\r\n"));
}

//! 単語単位: 検索語が 2 つの単語なら、どちらの単語も置換する(つながった語句ではない)
TEST_F(GrepReplaceDetailTest, WordOnlyMultipleWordsKey)
{
	folder.AddFile(L"w.txt", "foo bar baz foobar\r\n");
	EXPECT_THAT(Grep(L"foo bar", L"w.txt", L"XW", LR"(-GREPR="X")"), Eq(2u));
	EXPECT_THAT(folder.ReadFile(L"w.txt"), Eq("X X baz foobar\r\n"));
}

//! S(サブフォルダー) + O: サブフォルダーのファイルも置換してバックアップを作る。一致しないファイルには何も作らない
TEST_F(GrepReplaceDetailTest, SubFoldersWithBackup)
{
	folder.AddFile(L"a.txt", "HIT\r\n");
	folder.AddFile(LR"(sub\b.txt)", "HIT\r\n");
	folder.AddFile(LR"(sub\deep\c.txt)", "none\r\n");
	EXPECT_THAT(Grep(L"HIT", L"*.txt", L"XSO", LR"(-GREPR="REP")"), Eq(2u));
	EXPECT_THAT(folder.ReadFile(L"a.txt"), Eq("REP\r\n"));
	EXPECT_THAT(folder.ReadFile(LR"(sub\b.txt)"), Eq("REP\r\n"));
	EXPECT_THAT(folder.ReadFile(LR"(sub\b.txt.skrold)"), Eq("HIT\r\n"));
	EXPECT_THAT(folder.Exists(LR"(sub\deep\c.txt.skrold)"), IsFalse());
}

//! 文字コード固定の UTF-16LE(BOM なし): BOM を付けずに書き戻す
TEST_F(GrepReplaceDetailTest, Utf16WithoutBom)
{
	folder.AddFile(L"u.txt", std::string("H\0I\0T\0\r\0\n\0", 10));
	EXPECT_THAT(Grep(L"HIT", L"u.txt", L"X", std::format(LR"(-GREPR="REP" -GCODE={})", int(CODE_UNICODE))), Eq(1u));
	EXPECT_THAT(folder.ReadFile(L"u.txt"), Eq(std::string("R\0E\0P\0\r\0\n\0", 10)));
}

/*!
	ファイルの文字コード(SJIS)に無い文字で置換すると、「?」になる

	【現状の制限】変換できなかったことは結果に出ない。
 */
TEST_F(GrepReplaceDetailTest, ReplacementNotInCharset)
{
	folder.AddFile(L"s.txt", "HIT\r\n");
	const auto extra = std::format(LR"(-GREPR="{}" -GCODE={})", L"\U0001F600", int(CODE_SJIS));	// 😀
	EXPECT_THAT(Grep(L"HIT", L"s.txt", L"X", extra), Eq(1u));
	EXPECT_THAT(folder.ReadFile(L"s.txt"), Eq("?\r\n"));	// 【現状の制限】
}

/*!
	10MB を超えるファイル: 最初の一致より前の行が 10MB を超えると、一致の前に書き出し始める

	CWriteData::AppendBuffer() の「10MB 以上だったら出力してしまう」の分岐と、5MB 超の進み具合の表示の分岐を通す。
 */
TEST_F(GrepReplaceDetailTest, LargeFileFlushesBuffer)
{
	const std::string line = std::string(98, 'x') + "\r\n";	// 100 バイト
	constexpr size_t lineCount = 105000;					// 10.5MB
	std::string data;
	data.reserve(line.size() * lineCount + 5);
	for (size_t i = 0; i < lineCount; ++i) {
		data += line;
	}
	folder.AddFile(L"large.txt", data + "HIT\r\n");

	EXPECT_THAT(Grep(L"HIT", L"large.txt", L"X", LR"(-GREPR="REP")"), Eq(1u));
	const auto result = folder.ReadFile(L"large.txt");
	EXPECT_THAT(result.size(), Eq(data.size() + 5));
	EXPECT_THAT(result.ends_with(line + "REP\r\n"), IsTrue());
	EXPECT_THAT(result.compare(0, data.size(), data), Eq(0));	// 失敗したときに 10MB を出さないよう、比べた結果だけを見る
}

/*!
	置換に失敗したファイル(読み取り専用で消せない)は、置換した数に数えない

	今は数えるので DISABLED_ にしている(#2743 の 5)。ファイルは変わらない。
 */
TEST_F(GrepReplaceDetailTest, DISABLED_FailedReplaceIsNotCounted)
{
	folder.AddFile(L"ro.txt", "HIT\r\n", FILE_ATTRIBUTE_READONLY);
	EXPECT_THAT(Grep(L"HIT", L"ro.txt", L"X", LR"(-GREPR="REP")"), Eq(0u));
}

/*!
	O(バックアップ): 読み取り専用のファイルも、元のファイルを .skrold に移せるので置換できる

	O が無いと消せずに失敗する(ReplaceReadOnlyFile)。
 */
TEST_F(GrepReplaceDetailTest, ReadOnlyFileWithBackup)
{
	folder.AddFile(L"ro.txt", "HIT\r\n", FILE_ATTRIBUTE_READONLY);
	EXPECT_THAT(Grep(L"HIT", L"ro.txt", L"XO", LR"(-GREPR="REP")"), Eq(1u));
	EXPECT_THAT(folder.ReadFile(L"ro.txt"), Eq("REP\r\n"));
	EXPECT_THAT(folder.ReadFile(L"ro.txt.skrold"), Eq("HIT\r\n"));
}

// ---------------------------------------------------------------------------
// 除外ファイルの正規表現(-GOPT=E)
// ---------------------------------------------------------------------------

//! E: フルパスで照合し、サブフォルダーにも効き、大文字小文字を区別しない。結果の条件表示に出る
TEST_F(GrepCommandLineTest, ExceptFileRegexp)
{
	folder.AddFile(L"a.txt", "HIT\r\n");
	folder.AddFile(L"README", "HIT\r\n");
	folder.AddFile(L"app.log", "HIT\r\n");
	folder.AddFile(L"app.20260801.log", "HIT\r\n");
	folder.AddFile(LR"(sub\readme)", "HIT\r\n");
	const std::wstring files = LR"(*;!\\[^.\\]+$;!\\APP\.\d{8}\.log$)";
	EXPECT_THAT(Grep(L"HIT", files, L"XSE"), Eq(2u));
	const auto text = GetDocumentText();
	EXPECT_THAT(LineContaining(text, L"a.txt("), ::testing::Not(IsEmpty()));
	EXPECT_THAT(LineContaining(text, L"app.log("), ::testing::Not(IsEmpty()));
	EXPECT_THAT(LineContaining(text, L"app.20260801.log("), IsEmpty());
	EXPECT_THAT(Contains(text, LS(STR_GREP_EXCLUDE_FILE_REGEXP)), IsTrue());

	// E が無ければワイルドカードとして扱われ、何も除外されない
	ResetDocument();
	const std::wstring filesWithoutE = LR"(*;!\\[^.\\]+$)";
	EXPECT_THAT(Grep(L"HIT", filesWithoutE, L"XS"), Eq(5u));
}

//! E: 区切り文字を含む正規表現は引用符で囲む(コマンドラインでは "" と書く)
TEST_F(GrepCommandLineTest, ExceptFileRegexpQuoted)
{
	folder.AddFile(L"a1.txt", "HIT\r\n");
	folder.AddFile(L"a12.txt", "HIT\r\n");
	folder.AddFile(L"a123.txt", "HIT\r\n");
	const std::wstring files = LR"(*;!""\\a\d{2,3}\.txt$"")";	// 引用符を含むのでマクロの外で作る(C2017 の回避)
	EXPECT_THAT(Grep(L"HIT", files, L"XE"), Eq(1u));
}

//! E: 正しくない正規表現はエラーメッセージを出して何もしない
TEST_F(GrepCommandLineTest, ExceptFileRegexpInvalid)
{
	folder.AddFile(L"a.txt", "HIT\r\n");
	std::wstring shown;
	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, _, _, _, _)).WillOnce(Invoke([&shown](HWND, LPCWSTR text, LPCWSTR, UINT, WORD) {
		shown = text ? text : L"";
		return IDOK;
	}));
	EXPECT_THAT(Grep(L"HIT", L"*;!^[a$", L"XE"), Eq(0u));
	EXPECT_THAT(shown, StartsWith(LS(STR_GREP_ERR_EXCLUDE_REGEXP)));
	EXPECT_THAT(LineContaining(GetDocumentText(), L"a.txt("), IsEmpty());
}

// ---------------------------------------------------------------------------
// 編集中のテキストから検索(-GFILE=":HWND:<ウィンドウハンドル>")
// ---------------------------------------------------------------------------

/*!
	@brief 「編集中のテキストから検索」の対象にする、編集ウィンドウの代わり

	テストの編集ウィンドウ(pcEditWnd)はウィンドウを作らないので、:HWND: の対象にできない。
	編集ウィンドウと同じクラス名でメッセージ専用ウィンドウを作り、Grep が対象に送るメッセージ
	(MYWM_GETLINECOUNT・MYWM_GETFILEINFO・MYWM_GETLINEDATA)に CEditWnd と同じ約束で答える。
	IsSakuraMainWindow() はクラス名で判定するので、Grep の対象として受け付けられる。
*/
class FakeEditWindow {
public:
	//! path は文書のファイル名(MYWM_GETFILEINFO で返す)。lines は改行コードを含む各行
	FakeEditWindow(std::wstring path, std::vector<std::wstring> lines, ECodeType charCode = CODE_UTF8)
		: m_path(std::move(path))
		, m_lines(std::move(lines))
		, m_charCode(charCode)
	{
		WNDCLASSEXW wc{};
		wc.cbSize = sizeof(wc);
		wc.lpfnWndProc = WndProc;
		wc.hInstance = ::GetModuleHandleW(nullptr);
		wc.lpszClassName = GSTR_EDITWINDOWNAME;
		if (::RegisterClassExW(&wc)) {
			m_bRegistered = true;
			m_hWnd = ::CreateWindowExW(0, GSTR_EDITWINDOWNAME, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, wc.hInstance, this);
		}
	}

	~FakeEditWindow()
	{
		Destroy();
		if (m_bRegistered) {
			::UnregisterClassW(GSTR_EDITWINDOWNAME, ::GetModuleHandleW(nullptr));
		}
	}

	FakeEditWindow(const FakeEditWindow&) = delete;
	FakeEditWindow& operator=(const FakeEditWindow&) = delete;

	HWND GetHwnd() const noexcept { return m_hWnd; }

	//! -GFILE に渡す「:HWND:<16 進>」(GetHwndTitle() は %zx で読む)
	std::wstring Target() const { return std::format(L":HWND:{:x}", reinterpret_cast<std::uintptr_t>(m_hWnd)); }

	void Destroy()
	{
		if (m_hWnd) {
			::DestroyWindow(m_hWnd);
			m_hWnd = nullptr;
		}
	}

	//! この行(0 始まり)の要求にエラー(-2)を返す
	void SetErrorLine(size_t nLine) noexcept { m_nErrorLine = nLine; }

private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (uMsg == WM_NCCREATE) {
			const auto pcs = reinterpret_cast<const CREATESTRUCTW*>(lParam);
			::SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pcs->lpCreateParams));
		}
		if (const auto self = reinterpret_cast<const FakeEditWindow*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA))) {
			switch (uMsg) {
			case MYWM_GETLINECOUNT:
				return LRESULT(self->m_lines.size());
			case MYWM_GETFILEINFO:
				self->OnGetFileInfo();
				return 0;
			case MYWM_GETLINEDATA:
				return self->OnGetLineData(size_t(wParam), int(lParam));
			default:
				break;
			}
		}
		return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}

	void OnGetFileInfo() const
	{
		auto& info = GetDllShareData().m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO;
		info = EditInfo();
		info.m_szPath.Assign(m_path.c_str());
		info.m_nCharCode = m_charCode;
	}

	//! CEditWnd と同じ約束: 0 以上はオフセットより後ろの長さ(0 は EOF)、負はエラー。共有のワークバッファーに入るだけ写す
	LRESULT OnGetLineData(size_t nLine, int nOffset) const
	{
		if (nLine == m_nErrorLine || m_lines.size() < nLine) {
			return -2;
		}
		if (nLine == m_lines.size()) {
			return 0;
		}
		const auto& line = m_lines[nLine];
		if (nOffset < 0 || int(line.size()) < nOffset) {
			return -3;
		}
		const size_t rest = line.size() - size_t(nOffset);
		auto& work = GetDllShareData().m_sWorkBuffer;
		const size_t count = std::min(rest, work.GetWorkBufferCount<WCHAR>());
		::wmemcpy(work.GetWorkBuffer<WCHAR>(), line.data() + nOffset, count);
		return LRESULT(rest);
	}

	std::wstring m_path;
	std::vector<std::wstring> m_lines;
	ECodeType m_charCode;
	size_t m_nErrorLine = SIZE_MAX;
	bool m_bRegistered = false;
	HWND m_hWnd = nullptr;
};

/*!
	@brief 編集中のテキストから検索(:HWND:)のテスト

	文書のファイル名はテストのフォルダーの doc.txt にする(ファイルは作らない。作るテストは明記する)。
	結果のパスは、文書のファイル名をフォルダーとファイル名に分けたものから作られる。
*/
struct GrepHwndTargetTest : public GrepOutputSnapshotTest {
	std::wstring DocPath() const { return (folder.Path() / L"doc.txt").native(); }
};

//! 文書の各行を検索する。結果のパスは文書のファイル名
TEST_F(GrepHwndTargetTest, SearchesWindowText)
{
	FakeEditWindow window(DocPath(), { L"alpha HIT one\r\n", L"bravo two\r\n", L"HIT x HIT\r\n" });
	ASSERT_THAT(window.GetHwnd(), Ne(nullptr));

	const auto path = DocPath();
	const std::wstring expected = path + L"(1,7): HIT\r\n" + path + L"(3,1): HIT\r\n" + path + L"(3,7): HIT\r\n";
	DWORD hitCount = 0;
	EXPECT_THAT(GrepStdout(L"HIT", window.Target(), L"1", &hitCount), StrEq(expected));
	EXPECT_THAT(hitCount, Eq(3u));
}

//! 同じ名前のファイルがあっても、ファイルではなく文書(ウィンドウ)の内容を検索する
TEST_F(GrepHwndTargetTest, ReadsWindowNotFile)
{
	folder.AddFile(L"doc.txt", "none\r\n");
	FakeEditWindow window(DocPath(), { L"alpha HIT one\r\n" });
	ASSERT_THAT(window.GetHwnd(), Ne(nullptr));

	const std::wstring expected = DocPath() + L"(1,7): HIT\r\n";
	EXPECT_THAT(GrepStdout(L"HIT", window.Target(), L"1"), StrEq(expected));
}

//! 該当行(P): CRLF・LF・CR・改行なし(最後の行)のどれでも、改行コードを除いて出力する
TEST_F(GrepHwndTargetTest, LineEndings)
{
	FakeEditWindow window(DocPath(), { L"a HIT\r\n", L"b HIT\n", L"c HIT\r", L"d HIT" });
	ASSERT_THAT(window.GetHwnd(), Ne(nullptr));

	const auto path = DocPath();
	const std::wstring expected = path + L"(1,3): a HIT\r\n" + path + L"(2,3): b HIT\r\n" + path + L"(3,3): c HIT\r\n" + path + L"(4,3): d HIT\r\n";
	EXPECT_THAT(GrepStdout(L"HIT", window.Target(), L"1P"), StrEq(expected));
}

//! ベースフォルダー(B)・フォルダー毎(D)の見出しは、文書のフォルダーで出す
TEST_F(GrepHwndTargetTest, FolderHeaders)
{
	FakeEditWindow window(DocPath(), { L"alpha HIT one\r\n" });
	ASSERT_THAT(window.GetHwnd(), Ne(nullptr));

	const auto dir = folder.Path().native();
	const std::wstring line = L"・doc.txt(1,7): alpha HIT one\r\n";
	// B だけ: ベースフォルダーの見出し(■)が文書のフォルダー
	const std::wstring baseOnly = std::format(L"■\"{}\"\r\n", dir) + line;
	// D だけ: フォルダーの見出し(■)が文書のフォルダー(絶対パス)
	const std::wstring separateOnly = std::format(L"■\"{}\"\r\n", dir) + line;
	// B+D: ベース(◎)が文書のフォルダーで、フォルダーの見出しは空(■ だけ)
	const std::wstring baseAndSeparate = std::format(L"◎\"{}\"\r\n■\r\n", dir) + line;
	EXPECT_THAT(GrepStdout(L"HIT", window.Target(), L"1PB"), StrEq(baseOnly));
	EXPECT_THAT(GrepStdout(L"HIT", window.Target(), L"1PD"), StrEq(separateOnly));
	EXPECT_THAT(GrepStdout(L"HIT", window.Target(), L"1PBD"), StrEq(baseAndSeparate));
}

//! 自動判別(K)のときは、文書の文字コードを結果に付ける
TEST_F(GrepHwndTargetTest, CodeNameFromWindow)
{
	FakeEditWindow window(DocPath(), { L"alpha HIT one\r\n" }, CODE_EUC);
	ASSERT_THAT(window.GetHwnd(), Ne(nullptr));

	const std::wstring expected = DocPath() + L"(1,7)" + CodeBracket(CODE_EUC) + L": HIT\r\n";
	EXPECT_THAT(GrepStdout(L"HIT", window.Target(), L"1K"), StrEq(expected));
}

//! 検索キーが空なら、文書を読まずにファイル名だけを出して 1 件と数える
TEST_F(GrepHwndTargetTest, EmptyKeyListsDocumentName)
{
	FakeEditWindow window(DocPath(), { L"alpha HIT one\r\n" });
	ASSERT_THAT(window.GetHwnd(), Ne(nullptr));

	const std::wstring expected = DocPath() + L"\r\n";
	DWORD hitCount = 0;
	EXPECT_THAT(GrepStdout(L"", window.Target(), L"1", &hitCount), StrEq(expected));
	EXPECT_THAT(hitCount, Eq(1u));
}

//! 空の文書(0 行)は 0 件で、何も出さない
TEST_F(GrepHwndTargetTest, EmptyDocument)
{
	FakeEditWindow window(DocPath(), {});
	ASSERT_THAT(window.GetHwnd(), Ne(nullptr));

	DWORD hitCount = 1;
	EXPECT_THAT(GrepStdout(L"HIT", window.Target(), L"1", &hitCount), IsEmpty());
	EXPECT_THAT(hitCount, Eq(0u));
}

//! 一致しない文書は 0 件で、何も出さない
TEST_F(GrepHwndTargetTest, NoHit)
{
	FakeEditWindow window(DocPath(), { L"alpha one\r\n", L"bravo two\r\n" });
	ASSERT_THAT(window.GetHwnd(), Ne(nullptr));

	DWORD hitCount = 1;
	EXPECT_THAT(GrepStdout(L"HIT", window.Target(), L"1", &hitCount), IsEmpty());
	EXPECT_THAT(hitCount, Eq(0u));
}

/*!
	共有のワークバッファーより長い行: 何回かに分けて読んだものをつないでから検索する

	1 つ目の HIT はワークバッファーの境目をまたぐ。2 つ目は 3 回目に読む部分にある。
*/
TEST_F(GrepHwndTargetTest, KeyAcrossWorkBuffer)
{
	const size_t work = GetDllShareData().m_sWorkBuffer.GetWorkBufferCount<WCHAR>();
	const std::wstring longLine = std::wstring(work - 1, L'x') + L"HIT" + std::wstring(work, L'y') + L"HIT\r\n";
	FakeEditWindow window(DocPath(), { longLine });
	ASSERT_THAT(window.GetHwnd(), Ne(nullptr));

	const auto path = DocPath();
	const std::wstring expected = std::format(L"{}(1,{}): HIT\r\n{}(1,{}): HIT\r\n", path, work, path, work * 2 + 3);
	DWORD hitCount = 0;
	EXPECT_THAT(GrepStdout(L"HIT", window.Target(), L"1", &hitCount), StrEq(expected));
	EXPECT_THAT(hitCount, Eq(2u));
}

//! 行の読み出しがエラーを返すと、そこで打ち切る(エラーは出さず、それまでの結果を残す)
TEST_F(GrepHwndTargetTest, StopsAtLineDataError)
{
	FakeEditWindow window(DocPath(), { L"HIT a\r\n", L"HIT b\r\n", L"HIT c\r\n" });
	ASSERT_THAT(window.GetHwnd(), Ne(nullptr));
	window.SetErrorLine(1);

	const std::wstring expected = DocPath() + L"(1,1): HIT a\r\n";
	DWORD hitCount = 0;
	EXPECT_THAT(GrepStdout(L"HIT", window.Target(), L"1P", &hitCount), StrEq(expected));
	EXPECT_THAT(hitCount, Eq(1u));
}

//! 置換(-GREPR)を指定しても、:HWND: では検索だけを行い、文書と同じ名前のファイルを書き換えない
TEST_F(GrepHwndTargetTest, ReplaceOnlySearches)
{
	folder.AddFile(L"doc.txt", "HIT\r\n");
	FakeEditWindow window(DocPath(), { L"HIT a\r\n", L"HIT b\r\n" });
	ASSERT_THAT(window.GetHwnd(), Ne(nullptr));

	const auto replace = LR"(-GREPR="REP")";
	EXPECT_THAT(Grep(L"HIT", window.Target(), L"XH", replace), Eq(2u));
	EXPECT_THAT(folder.ReadFile(L"doc.txt"), StrEq("HIT\r\n"));
	EXPECT_THAT(folder.Exists(L"doc.txt.skrold"), IsFalse());
}

//! :HWND: の後ろが 16 進でなければ、0 件で結果にエラーを出す
TEST_F(GrepHwndTargetTest, NonHexHandle)
{
	EXPECT_THAT(Grep(L"HIT", L":HWND:zz", L"X"), Eq(0u));
	EXPECT_THAT(Contains(GetDocumentText(), L"HWND handle error."), IsTrue());
}

//! サクラエディタの編集ウィンドウでないウィンドウ(デスクトップ)は受け付けない
TEST_F(GrepHwndTargetTest, NotEditWindow)
{
	const auto target = std::format(L":HWND:{:x}", reinterpret_cast<std::uintptr_t>(::GetDesktopWindow()));
	EXPECT_THAT(Grep(L"HIT", target, L"X"), Eq(0u));
	EXPECT_THAT(Contains(GetDocumentText(), L"HWND handle error."), IsTrue());
}

//! 閉じたウィンドウのハンドルは受け付けない
TEST_F(GrepHwndTargetTest, ClosedWindow)
{
	FakeEditWindow window(DocPath(), { L"HIT\r\n" });
	ASSERT_THAT(window.GetHwnd(), Ne(nullptr));
	const auto target = window.Target();
	window.Destroy();

	EXPECT_THAT(Grep(L"HIT", target, L"X"), Eq(0u));
	EXPECT_THAT(Contains(GetDocumentText(), L"HWND handle error."), IsTrue());
}

// ---------------------------------------------------------------------------
// 中断と進み具合の表示
// ---------------------------------------------------------------------------

/*!
	@brief テスト用の中断の確認と進み具合の表示

	間引かない(IsCheckTime()・IsFileNameTime() はいつも true)。checkTime に false を渡すと、確かめる時機を来させない。
	IsCancelRequested() が cancelAt 回目に呼ばれたときから中断する(0 なら中断しない)。
*/
class FakeGrepProgress : public IGrepProgress {
public:
	explicit FakeGrepProgress(int cancelAt = 0, bool checkTime = true)
		: m_cancelAt(cancelAt)
		, m_checkTime(checkTime)
	{
	}

	bool IsCheckTime() override
	{
		return m_checkTime;
	}

	bool IsFileNameTime() override
	{
		return true;
	}

	bool IsCancelRequested() override
	{
		++checkCount;
		return 0 < m_cancelAt && m_cancelAt <= checkCount;
	}

	void ApplyRealTimeView() override
	{
		++applyCount;
	}

	void SetHitCount(int nHitCount) override
	{
		hitCounts.push_back(nHitCount);
	}

	void SetFileName(std::wstring_view fileName) override
	{
		fileNames.emplace_back(fileName);
	}

	void SetFolder(std::wstring_view folder) override
	{
		folders.emplace_back(folder);
	}

	int checkCount = 0;						//!< IsCancelRequested() が呼ばれた回数
	int applyCount = 0;						//!< ApplyRealTimeView() が呼ばれた回数
	std::vector<int> hitCounts;				//!< 表示したヒット数
	std::vector<std::wstring> fileNames;	//!< 表示したファイル名
	std::vector<std::wstring> folders;		//!< 表示したフォルダー

private:
	const int m_cancelAt;
	const bool m_checkTime;
};

/*!
	@brief 中断と進み具合の表示のテスト

	DoGrep() に FakeGrepProgress を渡し、「Grep実行中」のダイアログの代わりに使う。
	確かめる時機なら、中断の確認(IsCancelRequested())は、ファイルごとに 2 回(検索する前・ファイルを開いた後。検索キーが空なら検索する前の 1 回だけ)、
	Grep はファイルの 32 行ごと、Grep 置換は 1 行ごと、サブフォルダーに入る前に 1 回呼ばれる。
*/
struct GrepProgressTest : public GrepCommandLineTest {
	//! 引数を解析し、progress を渡して Grep を実行する
	DWORD GrepWith(FakeGrepProgress& progress, std::wstring_view key, std::wstring_view file, std::wstring_view opt, std::wstring_view extra = L"") const
	{
		return CEditApp::getInstance()->m_pcGrepAgent->DoGrep(&pcEditWnd->GetActiveView(), ParseGrepCommandLine(Args(key, file, opt, extra)), &progress);
	}

	//! 1 行に HIT が 1 つの行を lineCount 行並べたファイルを作る
	void AddLinesFile(const std::filesystem::path& relPath, int lineCount) const
	{
		std::string data;
		for (int i = 0; i < lineCount; ++i) {
			data += "HIT\r\n";
		}
		folder.AddFile(relPath, data);
	}
};

//! 最初のファイルの前に中断すると、どのファイルも探さず「中断しました」を出す
TEST_F(GrepProgressTest, CancelBeforeFirstFile)
{
	AddBasicFiles();
	FakeGrepProgress progress(1);
	EXPECT_THAT(GrepWith(progress, L"HIT", L"*.txt", L"X"), Eq(0u));
	const auto text = GetDocumentText();
	EXPECT_THAT(Contains(text, LS(STR_GREP_SUSPENDED)), IsTrue());
	EXPECT_THAT(Contains(text, L"a.txt"), IsFalse());
}

//! 2 つ目のファイルの前に中断すると、1 つ目のファイルの結果だけが残る
TEST_F(GrepProgressTest, CancelBetweenFiles)
{
	folder.AddFile(L"a.txt", "HIT\r\n");
	folder.AddFile(L"b.txt", "HIT\r\n");
	FakeGrepProgress progress(3);
	EXPECT_THAT(GrepWith(progress, L"HIT", L"*.txt", L"X"), Eq(1u));
	const auto text = GetDocumentText();
	EXPECT_THAT(Contains(text, LS(STR_GREP_SUSPENDED)), IsTrue());
	EXPECT_THAT(Contains(text, L"a.txt"), IsTrue());
	EXPECT_THAT(Contains(text, L"b.txt"), IsFalse());
	EXPECT_THAT(progress.checkCount, Eq(3));
}

//! ファイルの途中(32 行目)で中断すると、その前の 31 行の結果が残る
TEST_F(GrepProgressTest, CancelInsideFile)
{
	AddLinesFile(L"many.txt", 64);
	FakeGrepProgress progress(3);
	EXPECT_THAT(GrepWith(progress, L"HIT", L"many.txt", L"X"), Eq(31u));
	EXPECT_THAT(Contains(GetDocumentText(), LS(STR_GREP_SUSPENDED)), IsTrue());
}

//! サブフォルダーに入る前に中断すると、サブフォルダーは探さない
TEST_F(GrepProgressTest, CancelBeforeSubFolder)
{
	AddBasicFiles();
	FakeGrepProgress progress(5);
	EXPECT_THAT(GrepWith(progress, L"HIT", L"*.txt", L"XS"), Eq(3u));
	const auto text = GetDocumentText();
	EXPECT_THAT(Contains(text, LS(STR_GREP_SUSPENDED)), IsTrue());
	EXPECT_THAT(Contains(text, L"d.txt"), IsFalse());
}

//! Grep 置換をファイルの途中(2 行目)で中断すると、1 行目は数えるが、ファイルは書き換えず、一時ファイルも残さない
TEST_F(GrepProgressTest, CancelReplaceKeepsFile)
{
	folder.AddFile(L"r.txt", "HIT\r\nHIT\r\n");
	FakeGrepProgress progress(4);
	EXPECT_THAT(GrepWith(progress, L"HIT", L"r.txt", L"X", LR"(-GREPR="REP")"), Eq(1u));
	EXPECT_THAT(folder.ReadFile(L"r.txt"), StrEq("HIT\r\nHIT\r\n"));
	EXPECT_THAT(folder.Exists(L"r.txt.skrnew"), IsFalse());
	EXPECT_THAT(Contains(GetDocumentText(), LS(STR_GREP_SUSPENDED)), IsTrue());
}

//! ファイル名だけを返す(検索キーが空)ときは、ファイルごとにヒット数を表示する。最後はファイル名を消す
TEST_F(GrepProgressTest, ShowsHitCountOfFileList)
{
	AddBasicFiles();
	FakeGrepProgress progress;
	EXPECT_THAT(GrepWith(progress, L"", L"*.txt", L"X"), Eq(2u));
	EXPECT_THAT(progress.hitCounts, ::testing::ElementsAre(1, 2));
	ASSERT_THAT(progress.fileNames, ::testing::SizeIs(3));
	EXPECT_THAT(progress.fileNames.back(), StrEq(L" "));
}

//! ファイルの途中では、32 行ごとにリアルタイム表示を反映し、ファイル名・ヒット数・フォルダーを表示する
TEST_F(GrepProgressTest, ShowsProgressInsideFile)
{
	AddLinesFile(L"many.txt", 64);
	FakeGrepProgress progress;
	EXPECT_THAT(GrepWith(progress, L"HIT", L"many.txt", L"X"), Eq(64u));
	EXPECT_THAT(progress.checkCount, Eq(4));
	EXPECT_THAT(progress.applyCount, Eq(3));
	EXPECT_THAT(progress.hitCounts, ::testing::ElementsAre(31, 63));
	EXPECT_THAT(progress.fileNames, ::testing::ElementsAre(StrEq(L"many.txt"), StrEq(L"many.txt"), StrEq(L"many.txt"), StrEq(L" ")));
	EXPECT_THAT(progress.folders, ::testing::SizeIs(2));
}

//! 5MB を超えるファイルでは、読んだ割合をファイル名に付けて表示する(Grep・Grep 置換とも)
TEST_F(GrepProgressTest, ShowsReadPercentOfLargeFile)
{
	const std::string line = std::string(94, 'x') + " HIT\r\n";	// 100 バイト
	constexpr size_t lineCount = 60000;							// 6MB
	std::string data;
	data.reserve(line.size() * lineCount);
	for (size_t i = 0; i < lineCount; ++i) {
		data += line;
	}
	folder.AddFile(L"large.txt", data);

	FakeGrepProgress grep;
	EXPECT_THAT(GrepWith(grep, L"HIT", L"large.txt", L"XN"), Eq(0u));
	EXPECT_THAT(grep.fileNames, ::testing::Contains(StartsWith(L"large.txt (")));

	FakeGrepProgress replace;
	EXPECT_THAT(GrepWith(replace, L"NOHIT", L"large.txt", L"X", LR"(-GREPR="REP")"), Eq(0u));
	EXPECT_THAT(replace.fileNames, ::testing::Contains(StartsWith(L"large.txt (")));
}

//! 確かめる時機でなければ、中断の確認もリアルタイム表示の反映も進み具合の表示もしない(Grep 置換のファイルを開いた後だけは時機を見ずに確かめる)
TEST_F(GrepProgressTest, SkipsCheckUntilCheckTime)
{
	AddLinesFile(L"many.txt", 64);

	FakeGrepProgress grep(1, false);
	EXPECT_THAT(GrepWith(grep, L"HIT", L"many.txt", L"X"), Eq(64u));
	EXPECT_THAT(grep.checkCount, Eq(0));
	EXPECT_THAT(grep.applyCount, Eq(0));
	EXPECT_THAT(grep.hitCounts, ::testing::IsEmpty());

	FakeGrepProgress replace(1, false);
	EXPECT_THAT(GrepWith(replace, L"HIT", L"many.txt", L"X", LR"(-GREPR="REP")"), Eq(0u));
	EXPECT_THAT(replace.checkCount, Eq(1));
	EXPECT_THAT(Contains(GetDocumentText(), LS(STR_GREP_SUSPENDED)), IsTrue());
}

//! サブフォルダーから戻ると、元のフォルダーを表示する
TEST_F(GrepProgressTest, ShowsFolderAfterSubFolder)
{
	AddBasicFiles();
	FakeGrepProgress progress;
	EXPECT_THAT(GrepWith(progress, L"HIT", L"*.txt", L"XS"), Eq(4u));
	EXPECT_THAT(progress.folders, ::testing::ElementsAre(::testing::EndsWith(L"GrepProgressTest.ShowsFolderAfterSubFolder")));
}

// ---------------------------------------------------------------------------
// 結果の出力先
// ---------------------------------------------------------------------------

/*!
	@brief テスト用の結果の出力先

	足された文字列を、足された順にためる。
*/
class FakeGrepOutput : public IGrepOutputTarget {
public:
	void Append(const CNativeW& cmem) override
	{
		chunks.emplace_back(cmem.GetStringPtr(), cmem.GetStringLength());
	}

	bool IsDrawing() const override
	{
		return false;
	}

	//! 足された文字列をつないだもの
	std::wstring Text() const
	{
		std::wstring text;
		for (const auto& chunk : chunks) {
			text += chunk;
		}
		return text;
	}

	std::vector<std::wstring> chunks;	//!< 足された文字列
};

/*!
	@brief 結果の出力先のテスト

	DoGrep() に FakeGrepOutput を渡し、結果の文書の代わりに使う。
*/
struct GrepOutputTargetTest : public GrepCommandLineTest {
	//! output を渡して Grep を実行する(progress の指定が無ければダイアログを出す)
	DWORD GrepTo(FakeGrepOutput& output, const GrepInfo& gi, const std::optional<IGrepProgress*>& progress = std::nullopt) const
	{
		return CEditApp::getInstance()->m_pcGrepAgent->DoGrep(&pcEditWnd->GetActiveView(), gi, progress, &output);
	}
};

//! 見出し・結果・件数は、すべて出力先に足され、結果の文書には足されない
TEST_F(GrepOutputTargetTest, AppendsAllToTarget)
{
	AddBasicFiles();
	FakeGrepOutput output;
	EXPECT_THAT(GrepTo(output, ParseGrepCommandLine(Args(L"HIT", L"*.txt", L"XS"))), Eq(4u));
	EXPECT_THAT(GetDocumentText(), ::testing::IsEmpty());
	const auto text = output.Text();
	EXPECT_THAT(Contains(text, LS(STR_GREP_SEARCH_CONDITION)), IsTrue());
	EXPECT_THAT(Contains(text, L"d.txt"), IsTrue());
	EXPECT_THAT(Contains(text, MatchCountText(4)), IsTrue());
}

//! 出力先に足される文字列は、結果の文書に足される文字列と同じ
TEST_F(GrepOutputTargetTest, SameTextAsDocument)
{
	AddBasicFiles();
	FakeGrepOutput output;
	EXPECT_THAT(GrepTo(output, ParseGrepCommandLine(Args(L"HIT", L"*.txt", L"XSP"))), Eq(3u));
	EXPECT_THAT(Grep(L"HIT", L"*.txt", L"XSP"), Eq(3u));
	EXPECT_THAT(output.Text(), StrEq(GetDocumentText()));
}

//! 中断したときの「中断しました」も出力先に足される
TEST_F(GrepOutputTargetTest, SuspendedMessageToTarget)
{
	AddBasicFiles();
	FakeGrepOutput output;
	FakeGrepProgress progress(1);
	EXPECT_THAT(GrepTo(output, ParseGrepCommandLine(Args(L"HIT", L"*.txt", L"X")), &progress), Eq(0u));
	EXPECT_THAT(Contains(output.Text(), LS(STR_GREP_SUSPENDED)), IsTrue());
	EXPECT_THAT(GetDocumentText(), ::testing::IsEmpty());
}

//! 標準出力(U)で存在しないフォルダーを指定すると、メッセージを出力先に足して止まる
TEST_F(GrepOutputTargetTest, NonexistentFolderMessageToTarget)
{
	const auto args = std::format(LR"(-GREPMODE -GKEY="HIT" -GFILE="*.txt" -GFOLDER="{}" -GOPT=XU)", (folder.Path() / L"none").native());
	FakeGrepOutput output;
	EXPECT_THAT(GrepTo(output, ParseGrepCommandLine(args)), Eq(0u));
	EXPECT_THAT(output.Text(), StrEq(std::wstring(LS(STR_DLGGREP5)) + L"\r\n"));
}

/*!
	無効な :HWND: のエラーも出力先に足される

	このエラーの経路は m_bGrepRunning を戻さない(DISABLED_InvalidHwndTargetResetsRunningFlag の件)。
	後始末は GrepTestSuite::TearDown() が行うので、このテストでは DoGrep() を 1 回だけ呼ぶ。
*/
TEST_F(GrepOutputTargetTest, HwndErrorToTarget)
{
	FakeGrepOutput output;
	EXPECT_THAT(GrepTo(output, ParseGrepCommandLine(Args(L"HIT", L":HWND:0", L"X"))), Eq(0u));
	EXPECT_THAT(Contains(output.Text(), L"HWND handle error."), IsTrue());
	EXPECT_THAT(GetDocumentText(), ::testing::IsEmpty());
}

} // namespace grep_test
