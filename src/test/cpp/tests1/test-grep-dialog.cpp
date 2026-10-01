/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "grep/GrepTestSuite.hpp"

#include "dlg/CDlgGrep.h"
#include "dlg/CDlgGrepReplace.h"
#include "func/Funccode.h"
#include "env/DLLSHAREDATA.h"
#include "config/app_constants.h"

#include <initializer_list>

namespace grep_test {

/*!
	@brief GUI(Grep ダイアログ・Grep 置換ダイアログ)の経路のテスト

	編集ウィンドウに F_GREP_DIALOG / F_GREP_REPLACE_DLG を送り、ダイアログに値を入れて OK を押す。
	文書が無題・未編集なので、Grep は同じウィンドウで実行される(新しいプロセスを起動しない)。
*/
struct GrepDialogTest : public GrepTestSuite {
	TempFolder folder;
	CurrentDirectoryGuard currentDirectoryGuard;

	//! 基本のテストデータ(GrepCommandLineTest と同じ)
	void AddBasicFiles() const
	{
		folder.AddFile(L"a.txt", "HIT x HIT\r\nnone\r\nHIT\r\n");
		folder.AddFile(L"b.txt", "none\r\n");
		folder.AddFile(L"c.log", "HIT\r\n");
		folder.AddFile(LR"(sub\d.txt)", "HIT\r\n");
	}

	static void SetText(HWND hDlg, int id, std::wstring_view text)
	{
		::SetDlgItemTextW(hDlg, id, std::wstring(text).c_str());
	}

	static std::wstring GetText(HWND hDlg, int id)
	{
		std::wstring text(4096, L'\0');
		text.resize(::GetDlgItemTextW(hDlg, id, text.data(), int(text.size())));
		return text;
	}

	static void Check(HWND hDlg, int id, bool checked)
	{
		::CheckDlgButton(hDlg, id, checked ? BST_CHECKED : BST_UNCHECKED);
	}

	//! ラジオボタンのグループから 1 つを選ぶ
	static void SelectRadio(HWND hDlg, int id, std::initializer_list<int> group)
	{
		for (const auto item : group) {
			Check(hDlg, item, item == id);
		}
	}

	static void SelectOutputLineType(HWND hDlg, int id)
	{
		SelectRadio(hDlg, id, { IDC_RADIO_OUTPUTLINE, IDC_RADIO_OUTPUTMARKED, IDC_RADIO_NOHIT });
	}

	static void SelectOutputStyle(HWND hDlg, int id)
	{
		SelectRadio(hDlg, id, { IDC_RADIO_OUTPUTSTYLE1, IDC_RADIO_OUTPUTSTYLE2, IDC_RADIO_OUTPUTSTYLE3 });
	}

	//! 文字コードセットのコンボボックスで、項目データが code のものを選ぶ
	static void SelectCharset(HWND hDlg, ECodeType code)
	{
		const HWND hCombo = ::GetDlgItem(hDlg, IDC_COMBO_CHARSET);
		const auto count = int(::SendMessageW(hCombo, CB_GETCOUNT, 0, 0));
		for (int i = 0; i < count; ++i) {
			if (ECodeType(::SendMessageW(hCombo, CB_GETITEMDATA, i, 0)) == code) {
				::SendMessageW(hCombo, CB_SETCURSEL, i, 0);
				return;
			}
		}
		ADD_FAILURE() << "charset not found";
	}

	/*!
		共通の入力。前のテストの履歴・設定が残っていても結果が変わらないよう、すべての項目を明示する。
		出力は「該当部分」(一致した箇所ごとに数える)。
	*/
	void SetConditions(HWND hDlg, std::wstring_view key, std::wstring_view file) const
	{
		SetText(hDlg, IDC_COMBO_TEXT, key);
		SetText(hDlg, IDC_COMBO_FILE, file);
		SetText(hDlg, IDC_COMBO_FOLDER, folder.Path().native());
		SetText(hDlg, IDC_COMBO_EXCLUDE_FILE, L"");
		SetText(hDlg, IDC_COMBO_EXCLUDE_FOLDER, L"");
		Check(hDlg, IDC_CHK_WORD, false);
		Check(hDlg, IDC_CHK_LOHICASE, false);
		Check(hDlg, IDC_CHK_REGULAREXP, false);
		Check(hDlg, IDC_CHK_SUBFOLDER, true);
		SelectOutputLineType(hDlg, IDC_RADIO_OUTPUTMARKED);
		SelectOutputStyle(hDlg, IDC_RADIO_OUTPUTSTYLE1);
		Check(hDlg, IDC_CHECK_FILE_ONLY, false);
		Check(hDlg, IDC_CHECK_SEP_FOLDER, false);
		Check(hDlg, IDC_CHECK_BASE_PATH, false);
		SelectCharset(hDlg, CODE_AUTODETECT);
	}

	//! Grep ダイアログを開き、setup の後に OK を押す(受け付けられる前提)
	void AcceptGrepDialog(const std::function<void(HWND)>& setup)
	{
		dialog::ModalDialogCloser closer(L"Grep", [&setup](HWND hDlg) {
			setup(hDlg);
			SendDlgCommand(hDlg, IDOK);
		});
		FORWARD_WM_COMMAND(pcEditWnd->GetHwnd(), F_GREP_DIALOG, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);
	}

	/*!
		Grep ダイアログを開き、setup の後に OK を押す(入力エラーで受け付けられない前提)。
		表示されたメッセージの本文を返す。OK が拒否された後にキャンセルで閉じる。
	*/
	std::wstring RejectGrepDialog(const std::function<void(HWND)>& setup)
	{
		std::wstring shown;
		auto pUser32 = (MockUser32*)User32::getInstance();
		EXPECT_CALL(*pUser32, MessageBoxExW(_, _, _, _, _)).WillOnce(Invoke([&shown](HWND, LPCWSTR text, LPCWSTR, UINT, WORD) {
			shown = text ? text : L"";
			return IDOK;
		}));
		dialog::ModalDialogCloser closer(L"Grep", [&setup](HWND hDlg) {
			setup(hDlg);
			SendDlgCommand(hDlg, IDOK);
			SendDlgCommand(hDlg, IDCANCEL);
		});
		FORWARD_WM_COMMAND(pcEditWnd->GetHwnd(), F_GREP_DIALOG, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);
		return shown;
	}

	//! Grep ダイアログを開き、setup の後にキャンセルで閉じる(ボタンの動作の確認用)
	void CancelGrepDialog(const std::function<void(HWND)>& setup)
	{
		dialog::ModalDialogCloser closer(L"Grep", [&setup](HWND hDlg) {
			setup(hDlg);
			SendDlgCommand(hDlg, IDCANCEL);
		});
		FORWARD_WM_COMMAND(pcEditWnd->GetHwnd(), F_GREP_DIALOG, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);
	}
};

// ---------------------------------------------------------------------------
// OK で Grep が実行される
// ---------------------------------------------------------------------------

//! 基本: サブフォルダーを含めて一致した箇所ごとに数える
TEST_F(GrepDialogTest, Basic)
{
	AddBasicFiles();
	AcceptGrepDialog([this](HWND hDlg) { SetConditions(hDlg, L"HIT", L"*.txt"); });
	const auto text = GetDocumentText();
	EXPECT_TRUE(Contains(text, MatchCountText(4)));
	EXPECT_TRUE(Contains(text, L"d.txt"));
	EXPECT_FALSE(Contains(text, L"c.log"));
}

//! 結果出力: 該当行 / 否該当行
TEST_F(GrepDialogTest, OutputLineTypes)
{
	AddBasicFiles();
	AcceptGrepDialog([this](HWND hDlg) {
		SetConditions(hDlg, L"HIT", L"*.txt");
		SelectOutputLineType(hDlg, IDC_RADIO_OUTPUTLINE);
	});
	EXPECT_TRUE(Contains(GetDocumentText(), MatchCountText(3)));

	ResetDocument();
	AcceptGrepDialog([this](HWND hDlg) {
		SetConditions(hDlg, L"HIT", L"*.txt");
		SelectOutputLineType(hDlg, IDC_RADIO_NOHIT);
	});
	EXPECT_TRUE(Contains(GetDocumentText(), MatchCountText(2)));
}

//! ファイル毎最初のみ・出力形式・フォルダー毎・ベースフォルダー
TEST_F(GrepDialogTest, OutputOptions)
{
	AddBasicFiles();
	AcceptGrepDialog([this](HWND hDlg) {
		SetConditions(hDlg, L"HIT", L"*.txt");
		Check(hDlg, IDC_CHECK_FILE_ONLY, true);
	});
	EXPECT_TRUE(Contains(GetDocumentText(), MatchCountText(2)));

	for (const auto style : { IDC_RADIO_OUTPUTSTYLE2, IDC_RADIO_OUTPUTSTYLE3 }) {
		ResetDocument();
		AcceptGrepDialog([this, style](HWND hDlg) {
			SetConditions(hDlg, L"HIT", L"*.txt");
			SelectOutputStyle(hDlg, style);
			Check(hDlg, IDC_CHECK_SEP_FOLDER, true);
			Check(hDlg, IDC_CHECK_BASE_PATH, true);
		});
		EXPECT_TRUE(Contains(GetDocumentText(), MatchCountText(4))) << style;
	}
}

//! 単語単位・大文字小文字・正規表現(チェックボックスの操作を含む)
TEST_F(GrepDialogTest, SearchOptions)
{
	folder.AddFile(L"w.txt", "HIT HITS xHIT HIT. hit\r\n");
	AcceptGrepDialog([this](HWND hDlg) {
		SetConditions(hDlg, L"HIT", L"*.txt");
		Check(hDlg, IDC_CHK_WORD, true);
		Check(hDlg, IDC_CHK_LOHICASE, true);
	});
	EXPECT_TRUE(Contains(GetDocumentText(), MatchCountText(2)));

	ResetDocument();
	AcceptGrepDialog([this](HWND hDlg) {
		SetConditions(hDlg, L"H.T", L"*.txt");
		Check(hDlg, IDC_CHK_REGULAREXP, true);
		SendDlgCommand(hDlg, IDC_CHK_REGULAREXP);	// 正規表現のバージョン表示と「単語単位」の無効化
		Check(hDlg, IDC_CHK_LOHICASE, true);
	});
	EXPECT_TRUE(Contains(GetDocumentText(), MatchCountText(4)));
}

/*!
	除外ファイル・除外フォルダーに「,」を含む名前(Issue #2677 の再現手順そのもの)

	修正前は除外欄の連結で「,」の位置で分割され、除外が a / obj になり、b.txt / old が検索対象に加わっていた。
	そのため a,b.txt と obj,old\x.txt がヒットし、obj\x.txt が除外されていた(ヒット 5 件)。
*/
TEST_F(GrepDialogTest, ExcludePatternsWithComma)
{
	for (const auto name : { L"a,b.txt", L"b.txt", L"keep.txt", LR"(obj,old\x.txt)", LR"(obj\x.txt)", LR"(old\x.txt)" }) {
		folder.AddFile(name, "HIT\r\n");
	}
	AcceptGrepDialog([this](HWND hDlg) {
		SetConditions(hDlg, L"HIT", L"*.txt");
		SetText(hDlg, IDC_COMBO_EXCLUDE_FILE, LR"("a,b.txt")");
		SetText(hDlg, IDC_COMBO_EXCLUDE_FOLDER, LR"("obj,old")");
	});
	const auto text = GetDocumentText();

	// 結果: 除外したものがヒットせず、除外していない obj\x.txt はヒットする
	EXPECT_TRUE(Contains(text, MatchCountText(4)));
	EXPECT_TRUE(LineContaining(text, L"a,b.txt(").empty());
	EXPECT_TRUE(LineContaining(text, LR"(obj,old\x.txt()").empty());
	EXPECT_FALSE(LineContaining(text, LR"(\obj\x.txt()").empty());
	EXPECT_FALSE(LineContaining(text, LR"(\old\x.txt()").empty());

	// 条件の表示: 除外が分割されずに 1 つずつ表示される
	EXPECT_TRUE(Contains(LineContaining(text, LS(STR_GREP_EXCLUDE_FILE)), L"a,b.txt"));
	EXPECT_TRUE(Contains(LineContaining(text, LS(STR_GREP_EXCLUDE_FOLDER)), L"obj,old"));
}

/*!
	除外ファイルの欄の複数指定(引用符なし・引用符付きの混在)
*/
TEST_F(GrepDialogTest, ExcludeFields)
{
	AddBasicFiles();
	folder.AddFile(L"a,b.txt", "HIT\r\n");
	AcceptGrepDialog([this](HWND hDlg) {
		SetConditions(hDlg, L"HIT", L"*.txt");
		SetText(hDlg, IDC_COMBO_EXCLUDE_FILE, LR"(a.txt "a,b.txt")");
		SetText(hDlg, IDC_COMBO_EXCLUDE_FOLDER, L"sub");
	});
	EXPECT_TRUE(Contains(GetDocumentText(), MatchCountText(0)));
}

//! ファイルの欄が空なら「*.*」になる
TEST_F(GrepDialogTest, EmptyFileMeansAll)
{
	AddBasicFiles();
	AcceptGrepDialog([this](HWND hDlg) { SetConditions(hDlg, L"HIT", L""); });
	EXPECT_TRUE(Contains(GetDocumentText(), L"c.log"));
}

//! 複数のフォルダー
TEST_F(GrepDialogTest, MultipleFolders)
{
	folder.AddFile(LR"(p\x.txt)", "HIT\r\n");
	folder.AddFile(LR"(q\y.txt)", "HIT\r\n");
	AcceptGrepDialog([this](HWND hDlg) {
		SetConditions(hDlg, L"HIT", L"*.txt");
		SetText(hDlg, IDC_COMBO_FOLDER, std::format(L"{};{}", (folder.Path() / L"p").native(), (folder.Path() / L"q").native()));
	});
	EXPECT_TRUE(Contains(GetDocumentText(), MatchCountText(2)));
}

//! 入力した条件が履歴に残る
TEST_F(GrepDialogTest, History)
{
	AddBasicFiles();
	AcceptGrepDialog([this](HWND hDlg) { SetConditions(hDlg, L"HISTORY_KEY", L"*.hist.txt"); });
	const auto& keywords = GetDllShareData().m_sSearchKeywords;
	ASSERT_LT(0, int(keywords.m_aGrepFiles.size()));
	EXPECT_STREQ(L"*.hist.txt", keywords.m_aGrepFiles[0]);
	ASSERT_LT(0, int(keywords.m_aGrepFolders.size()));

	// %TEMP% が 8.3 形式の短い名前(CI の C:\Users\RUNNER~1\...)だと、表記が履歴と違うことがあるので、
	// 両方を長い名前に直して比べる
	const auto longPath = [](LPCWSTR path) {
		std::wstring buf(MAX_PATH, L'\0');
		buf.resize(::GetLongPathNameW(path, buf.data(), DWORD(buf.size())));
		return buf;
	};
	const std::wstring expected = longPath(folder.Path().c_str());
	const std::wstring actual = longPath(keywords.m_aGrepFolders[0]);
	ASSERT_FALSE(expected.empty());
	EXPECT_EQ(expected, actual);
}

// ---------------------------------------------------------------------------
// 入力エラー
// ---------------------------------------------------------------------------

//! フォルダーが空
TEST_F(GrepDialogTest, RejectsEmptyFolder)
{
	const auto shown = RejectGrepDialog([this](HWND hDlg) {
		SetConditions(hDlg, L"HIT", L"*.txt");
		SetText(hDlg, IDC_COMBO_FOLDER, L"");
	});
	EXPECT_EQ(std::wstring(LS(STR_DLGGREP4)), shown);
}

//! フォルダーが存在しない
TEST_F(GrepDialogTest, RejectsNonexistentFolder)
{
	const auto shown = RejectGrepDialog([this](HWND hDlg) {
		SetConditions(hDlg, L"HIT", L"*.txt");
		SetText(hDlg, IDC_COMBO_FOLDER, (folder.Path() / L"none").native());
	});
	EXPECT_EQ(std::wstring(LS(STR_DLGGREP5)), shown);
}

//! ファイルの欄: フォルダー部分のワイルドカード
TEST_F(GrepDialogTest, RejectsWildcardInFolderPart)
{
	const auto shown = RejectGrepDialog([this](HWND hDlg) { SetConditions(hDlg, L"HIT", LR"(sub*\a.txt)"); });
	EXPECT_EQ(std::wstring(LS(STR_DLGGREP2)), shown);
}

//! ファイルの欄: 絶対パス
TEST_F(GrepDialogTest, RejectsAbsoluteFile)
{
	const auto shown = RejectGrepDialog([this](HWND hDlg) { SetConditions(hDlg, L"HIT", LR"(C:\x\*.txt)"); });
	EXPECT_EQ(std::wstring(LS(STR_DLGGREP3)), shown);
}

//! 引用符の閉じ忘れ(C0 で追加した入力チェック)
TEST_F(GrepDialogTest, RejectsUnclosedQuote)
{
	const auto shown = RejectGrepDialog([this](HWND hDlg) {
		SetConditions(hDlg, L"HIT", L"*.txt");
		SetText(hDlg, IDC_COMBO_EXCLUDE_FILE, LR"("a,b.txt)");
	});
	EXPECT_EQ(std::wstring(LS(STR_DLGGREP_QUOTE_ERROR)), shown);
}

/*!
	正しくない正規表現

	CheckRegexpSyntax() の ::MessageBox も、テストでは MockUser32::MessageBoxExW() に届く。
*/
TEST_F(GrepDialogTest, RejectsInvalidRegularExpression)
{
	AddBasicFiles();
	std::wstring shownText;
	std::wstring shownCaption;
	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, _, _, _, _)).WillOnce(Invoke([&shownText, &shownCaption](HWND, LPCWSTR text, LPCWSTR caption, UINT, WORD) {
		shownText = text ? text : L"";
		shownCaption = caption ? caption : L"";
		return IDOK;
	}));
	dialog::ModalDialogCloser closer(L"Grep", [this](HWND hDlg) {
		SetConditions(hDlg, L"(", L"*.txt");
		Check(hDlg, IDC_CHK_REGULAREXP, true);
		SendDlgCommand(hDlg, IDOK);
		SendDlgCommand(hDlg, IDCANCEL);
	});
	FORWARD_WM_COMMAND(pcEditWnd->GetHwnd(), F_GREP_DIALOG, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);

	EXPECT_EQ(std::wstring(LS(STR_BREGONIG_TITLE)), shownCaption);
	EXPECT_TRUE(shownText.starts_with(LS(STR_REGEX_COMPILE_ERR_PREAMBLE)));
	EXPECT_FALSE(Contains(GetDocumentText(), L"a.txt"));
}

// ---------------------------------------------------------------------------
// ボタン・チェックボックスの動作(キャンセルで閉じる)
// ---------------------------------------------------------------------------

//! 「上階層へ」: 最後のフォルダーを 1 つ上にする。; を含むフォルダーは引用符で囲み直す
TEST_F(GrepDialogTest, FolderUpButton)
{
	const auto p = (folder.Path() / L"p").native();
	const auto qr = (folder.Path() / L"q;r").native();
	std::wstring single, multiple;
	CancelGrepDialog([&](HWND hDlg) {
		SetText(hDlg, IDC_COMBO_FOLDER, (folder.Path() / L"sub").native());
		SendDlgCommand(hDlg, IDC_BUTTON_FOLDER_UP);
		single = GetText(hDlg, IDC_COMBO_FOLDER);

		SetText(hDlg, IDC_COMBO_FOLDER, std::format(LR"({};"{}\s")", p, qr));
		SendDlgCommand(hDlg, IDC_BUTTON_FOLDER_UP);
		multiple = GetText(hDlg, IDC_COMBO_FOLDER);
	});
	EXPECT_EQ(folder.Path().native(), single);
	EXPECT_EQ(std::format(LR"({};"{}")", p, qr), multiple);
}

//! 「現フォルダー」: 無題ならカレントディレクトリ、ファイルを開いていればそのフォルダー
TEST_F(GrepDialogTest, CurrentFolderButton)
{
	AddBasicFiles();
	std::filesystem::current_path(folder.Path() / L"sub");
	std::wstring untitled;
	CancelGrepDialog([&](HWND hDlg) {
		SendDlgCommand(hDlg, IDC_BUTTON_CURRENTFOLDER);
		untitled = GetText(hDlg, IDC_COMBO_FOLDER);
	});
	EXPECT_EQ((folder.Path() / L"sub").native(), untitled);

	pcEditDoc->m_cDocFile.SetFilePath((folder.Path() / L"a.txt").c_str());
	std::wstring opened;
	CancelGrepDialog([&](HWND hDlg) {
		SendDlgCommand(hDlg, IDC_BUTTON_CURRENTFOLDER);
		opened = GetText(hDlg, IDC_COMBO_FOLDER);
	});
	EXPECT_EQ(folder.Path().native(), opened);
}

//! 「編集中のテキストから検索」: オン・オフで入力欄が切り替わる(ファイルを開いているときだけ有効)
TEST_F(GrepDialogTest, FromThisTextCheckbox)
{
	AddBasicFiles();
	pcEditDoc->m_cDocFile.SetFilePath((folder.Path() / L"a.txt").c_str());
	bool enabled = false;
	bool folderEnabledWhenOn = true;
	bool folderEnabledWhenOff = false;
	CancelGrepDialog([&](HWND hDlg) {
		enabled = ::IsWindowEnabled(::GetDlgItem(hDlg, IDC_CHK_FROMTHISTEXT)) != FALSE;
		Check(hDlg, IDC_CHK_FROMTHISTEXT, true);
		SendDlgCommand(hDlg, IDC_CHK_FROMTHISTEXT);
		folderEnabledWhenOn = ::IsWindowEnabled(::GetDlgItem(hDlg, IDC_COMBO_FOLDER)) != FALSE;
		Check(hDlg, IDC_CHK_FROMTHISTEXT, false);
		SendDlgCommand(hDlg, IDC_CHK_FROMTHISTEXT);
		folderEnabledWhenOff = ::IsWindowEnabled(::GetDlgItem(hDlg, IDC_COMBO_FOLDER)) != FALSE;
	});
	EXPECT_TRUE(enabled);
	EXPECT_FALSE(folderEnabledWhenOn);
	EXPECT_TRUE(folderEnabledWhenOff);
}

//! 「CP」: 文字コードセットの一覧にコードページが加わる
TEST_F(GrepDialogTest, CodePageCheckbox)
{
	int before = 0;
	int after = 0;
	CancelGrepDialog([&](HWND hDlg) {
		const HWND hCombo = ::GetDlgItem(hDlg, IDC_COMBO_CHARSET);
		before = int(::SendMessageW(hCombo, CB_GETCOUNT, 0, 0));
		Check(hDlg, IDC_CHECK_CP, true);
		SendDlgCommand(hDlg, IDC_CHECK_CP);
		after = int(::SendMessageW(hCombo, CB_GETCOUNT, 0, 0));
	});
	EXPECT_LT(before, after);
}

// ---------------------------------------------------------------------------
// 同じウィンドウで実行できないとき
// ---------------------------------------------------------------------------

/*!
	編集済みの文書では新しいウィンドウで Grep しようとする。
	ウィンドウ数が上限なら、メッセージを出して何もしない(新しいプロセスは起動しない)。
*/
TEST_F(GrepDialogTest, MaxWindowsWhenDocumentModified)
{
	AddBasicFiles();
	auto& nodes = GetDllShareData().m_sNodes;
	const auto savedCount = nodes.m_nEditArrNum;

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, _, _, _, _)).WillOnce(Return(IDOK));
	// 上限にしてから F_GREP_DIALOG を送ると、IsFuncEnable() が無効にしてダイアログ自体が開かない。
	// ダイアログが開いた後に上限にして、OK の後の Command_GREP() の上限チェックを通す。
	AcceptGrepDialog([this, &nodes](HWND hDlg) {
		SetConditions(hDlg, L"HIT", L"*.txt");
		nodes.m_nEditArrNum = MAX_EDITWINDOWS;
		pcEditDoc->m_cDocEditor.m_bIsDocModified = true;
	});

	nodes.m_nEditArrNum = savedCount;
	EXPECT_FALSE(Contains(GetDocumentText(), L"a.txt"));
}

// ---------------------------------------------------------------------------
// Grep 置換ダイアログ
// ---------------------------------------------------------------------------

//! 置換してバックアップを作る
TEST_F(GrepDialogTest, ReplaceWithBackup)
{
	AddBasicFiles();
	dialog::ModalDialogCloser closer(L"Grep置換", [this](HWND hDlg) {
		SetConditions(hDlg, L"HIT", L"a.txt");
		SetText(hDlg, IDC_COMBO_TEXT2, L"REP");
		Check(hDlg, IDC_CHK_PASTE, false);
		Check(hDlg, IDC_CHK_BACKUP, true);
		SendDlgCommand(hDlg, IDOK);
	});
	FORWARD_WM_COMMAND(pcEditWnd->GetHwnd(), F_GREP_REPLACE_DLG, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);

	EXPECT_EQ("REP x REP\r\nnone\r\nREP\r\n", folder.ReadFile(L"a.txt"));
	EXPECT_EQ("HIT x HIT\r\nnone\r\nHIT\r\n", folder.ReadFile(L"a.txt.skrold"));
}

// ---------------------------------------------------------------------------
// 文字コード・そのほかのイレギュラーケース
// ---------------------------------------------------------------------------

//! 自動判別(ダイアログの既定): 結果の行に文字コード名が付く
TEST_F(GrepDialogTest, CharsetAutoDetect)
{
	const auto text = JapaneseLines();
	folder.AddFile(L"sjis.txt", Encode(text, 932));
	folder.AddFile(L"utf8.txt", Encode(text, CP_UTF8));
	AcceptGrepDialog([this](HWND hDlg) { SetConditions(hDlg, L"テスト", L"*.txt"); });
	const auto result = GetDocumentText();
	EXPECT_TRUE(Contains(result, MatchCountText(6)));
	EXPECT_TRUE(Contains(LineContaining(result, L"sjis.txt"), CodeBracket(CODE_SJIS)));
	EXPECT_TRUE(Contains(LineContaining(result, L"utf8.txt"), CodeBracket(CODE_UTF8)));
}

//! 文字コードを固定する(UTF-8 なら SJIS のファイルは見つからない)
TEST_F(GrepDialogTest, CharsetFixed)
{
	const auto text = JapaneseLines();
	folder.AddFile(L"sjis.txt", Encode(text, 932));
	folder.AddFile(L"utf8.txt", Encode(text, CP_UTF8));
	AcceptGrepDialog([this](HWND hDlg) {
		SetConditions(hDlg, L"テスト", L"*.txt");
		SelectCharset(hDlg, CODE_UTF8);
	});
	EXPECT_TRUE(Contains(GetDocumentText(), MatchCountText(3)));
}

//! 「CP」をオンにしてコードページ(932)を選ぶ
TEST_F(GrepDialogTest, CharsetCodePage)
{
	folder.AddFile(L"sjis.txt", Encode(JapaneseLines(), 932));
	AcceptGrepDialog([this](HWND hDlg) {
		SetConditions(hDlg, L"テスト", L"*.txt");
		Check(hDlg, IDC_CHECK_CP, true);
		SendDlgCommand(hDlg, IDC_CHECK_CP);
		SelectCharset(hDlg, ECodeType(932));
	});
	EXPECT_TRUE(Contains(GetDocumentText(), MatchCountText(3)));
}

//! 検索文字列が空: ファイル名の一覧(自動判別の文字コード名付き)
TEST_F(GrepDialogTest, EmptyKeyListsFiles)
{
	AddBasicFiles();
	AcceptGrepDialog([this](HWND hDlg) { SetConditions(hDlg, L"", L"*.txt"); });
	const auto result = GetDocumentText();
	EXPECT_FALSE(LineContaining(result, L"d.txt").empty());
	EXPECT_TRUE(LineContaining(result, L"c.log").empty());
}

//! 「;」を含むフォルダー名は引用符で囲めば 1 つのフォルダーとして扱われる
TEST_F(GrepDialogTest, FolderWithSemicolon)
{
	folder.AddFile(LR"(q;r\y.txt)", "HIT\r\n");
	AcceptGrepDialog([this](HWND hDlg) {
		SetConditions(hDlg, L"HIT", L"*.txt");
		SetText(hDlg, IDC_COMBO_FOLDER, std::format(LR"("{}")", (folder.Path() / L"q;r").native()));
	});
	EXPECT_TRUE(Contains(GetDocumentText(), MatchCountText(1)));
}

//! キャンセルでは何も実行しない
TEST_F(GrepDialogTest, CancelDoesNothing)
{
	AddBasicFiles();
	CancelGrepDialog([this](HWND hDlg) { SetConditions(hDlg, L"HIT", L"*.txt"); });
	EXPECT_TRUE(GetDocumentText().empty());
	EXPECT_FALSE(CEditApp::getInstance()->m_pcGrepAgent->m_bGrepMode);
}

//! Grep 置換: 「クリップボードから貼り付け」でクリップボードが空なら OK を受け付けない
TEST_F(GrepDialogTest, ReplacePasteWithEmptyClipboard)
{
	AddBasicFiles();
	ASSERT_TRUE(::OpenClipboard(pcEditWnd->GetHwnd()));
	::EmptyClipboard();
	::CloseClipboard();

	std::wstring shown;
	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, _, _, _, _)).WillOnce(Invoke([&shown](HWND, LPCWSTR text, LPCWSTR, UINT, WORD) {
		shown = text ? text : L"";
		return IDOK;
	}));

	// 貼り付けできないと OK は受け付けられない(ダイアログは閉じない)ので、キャンセルで閉じる
	dialog::ModalDialogCloser closer(L"Grep置換", [this](HWND hDlg) {
		SetConditions(hDlg, L"HIT", L"a.txt");
		SetText(hDlg, IDC_COMBO_TEXT2, L"unused");
		Check(hDlg, IDC_CHK_PASTE, true);
		Check(hDlg, IDC_CHK_BACKUP, false);
		SendDlgCommand(hDlg, IDOK);
		SendDlgCommand(hDlg, IDCANCEL);
	});
	FORWARD_WM_COMMAND(pcEditWnd->GetHwnd(), F_GREP_REPLACE_DLG, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);

	EXPECT_EQ(std::wstring(LS(STR_DLGREPLC_CLIPBOARD)), shown);
	EXPECT_EQ("HIT x HIT\r\nnone\r\nHIT\r\n", folder.ReadFile(L"a.txt"));
}

//! Grep 置換: 自動判別で UTF-16(BOM 付き)のファイルを置換しても文字コードが保たれる
TEST_F(GrepDialogTest, ReplaceKeepsUtf16)
{
	const auto text = JapaneseLines();
	std::wstring replaced = text;
	for (size_t pos; (pos = replaced.find(L"テスト")) != std::wstring::npos; ) {
		replaced.replace(pos, 3, L"TEST");
	}
	folder.AddFile(L"u16.txt", EncodeUtf16(text, false, true));

	dialog::ModalDialogCloser closer(L"Grep置換", [this](HWND hDlg) {
		SetConditions(hDlg, L"テスト", L"u16.txt");
		SetText(hDlg, IDC_COMBO_TEXT2, L"TEST");
		Check(hDlg, IDC_CHK_PASTE, false);
		Check(hDlg, IDC_CHK_BACKUP, false);
		SendDlgCommand(hDlg, IDOK);
	});
	FORWARD_WM_COMMAND(pcEditWnd->GetHwnd(), F_GREP_REPLACE_DLG, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);

	EXPECT_EQ(EncodeUtf16(replaced, false, true), folder.ReadFile(L"u16.txt"));
}

} // namespace grep_test
