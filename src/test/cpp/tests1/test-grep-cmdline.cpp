/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "grep/GrepTestSuite.hpp"

#include "grep/CGrepEnumKeys.h"

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

//! Grep 実行中は再入しない
TEST_F(GrepCommandLineTest, RejectsReentry)
{
	CEditApp::getInstance()->m_pcGrepAgent->m_bGrepRunning = true;
	EXPECT_EQ(0xffffffffu, Grep(L"HIT", L"*.txt", L"X"));
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
	folder.AddFile(L"s.txt", Encode(L"𠮷野家 𠮷\r\n", CP_UTF8));
	EXPECT_EQ(2u, Grep(L"𠮷", L"*.txt", L"X", std::format(L"-GCODE={}", int(CODE_UTF8))));
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

} // namespace grep_test
