/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "util/window.h"

#include "dlg/ModalDialogCloser.hpp"
#include "env/ShareDataTestSuite.hpp"
#include "window/EditorTestSuite.hpp"

#include "dlg/CDlgCancel.h"
#include "dlg/CDlgCompare.h"
#include "dlg/CDlgFileUpdateQuery.h"
#include "dlg/CDlgPluginOption.h"
#include "dlg/CDlgPrintSetting.h"
#include "dlg/CDlgTagJumpList.h"
#include "dlg/CDlgTagsMake.h"
#include "dlg/CDlgWinSize.h"
#include "dlg/CDlgWindowList.h"
#include "macro/CKeyMacroMgr.h"
#include "macro/CMacroFactory.h"
#include "macro/CWSHManager.h"
#include "outline/CDlgFileTree.h"
#include "typeprop/CDlgKeywordSelect.h"
#include "typeprop/CDlgSameColor.h"
#include "typeprop/CDlgTypeAscertain.h"
#include "typeprop/CDlgTypeList.h"

#include "_main/CCommandLine.h"
#include "_main/CControlTray.h"
#include "plugin/CJackManager.h"
#include "plugin/CPluginManager.h"
#include "prop/CPropCommon.h"
#include "typeprop/CPropTypes.h"

#include "config/system_constants.h"

#include "tests1_rc.h"

using namespace std::literals::string_literals;
using namespace std::literals::string_view_literals;

void extract_zip_resource(WORD id, const std::optional<std::filesystem::path>& optOutDir);

namespace window {

struct TrayWndTest : public ::testing::Test, public env::ShareDataTestSuite {
	using CControlTrayHolder = std::unique_ptr<CControlTray>;

	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpTestSuite()
	{
		SetUpShareData();
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownTestSuite()
	{
		TearDownShareData();
	}

	CControlTrayHolder pcTrayWnd = nullptr;

	/*!
	 * テストが起動される直前に毎回呼ばれる関数
	 */
	void SetUp() override {
		// テストクラスをインスタンス化する
		pcTrayWnd = std::make_unique<CControlTray>();
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override {
		// テストクラスのインスタンスを破棄する
		pcTrayWnd = nullptr;
	}
};

TEST_F(TrayWndTest, OnGetTypeSetting001)
{
	// 受け取りバッファに値を設定
	GetDllShareData().m_sWorkBuffer.m_TypeConfig.m_nIdx = int(-1);

	HWND hWndTray = nullptr;
	int index = 0;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_GET_TYPESETTING, index, 0), IsTrue());

	// 結果確認
	EXPECT_THAT(GetDllShareData().m_sWorkBuffer.m_TypeConfig.m_nIdx, Eq(index));
}

TEST_F(TrayWndTest, OnGetTypeSetting102)
{
	// 取得の空振り(上限値オーバー)
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_GET_TYPESETTING, GetDllShareData().m_nTypesCount, 0), IsFalse());
}

TEST_F(TrayWndTest, OnAddTypeSetting001)
{
	// テキストの前に追加
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_ADD_TYPESETTING, 1, 0), IsTrue());
}

TEST_F(TrayWndTest, OnAddTypeSetting002)
{
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_GET_TYPESETTING, 1, 0), IsTrue());

	auto typeName = std::format(L"設定{}", 2);
	if (typeName != GetDllShareData().m_sWorkBuffer.m_TypeConfig.m_szTypeName) {
		// 受け渡しバッファに値を設定
		::wcscpy_s(GetDllShareData().m_sWorkBuffer.m_TypeConfig.m_szTypeName, typeName.c_str());

		EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_SET_TYPESETTING, 1, 0), IsTrue());
	}

	// 重複する名前を追加
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_ADD_TYPESETTING, 1, 0), IsTrue());

	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_GET_TYPESETTING, 1, 0), IsTrue());

	typeName = std::format(L"設定{}", 3);
	EXPECT_THAT(GetDllShareData().m_sWorkBuffer.m_TypeConfig.m_szTypeName, StrEq(typeName));
}

TEST_F(TrayWndTest, OnAddTypeSetting101)
{
	// 追加の空振り(基本の前には入れない)
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_ADD_TYPESETTING, 0, 0), IsFalse());
}

TEST_F(TrayWndTest, OnAddTypeSetting102)
{
	// 追加の空振り(上限値オーバー、「指定したインデックスの前」なので他と上限が違う)
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_ADD_TYPESETTING, GetDllShareData().m_nTypesCount + 1, 0), IsFalse());
}

TEST_F(TrayWndTest, OnAddTypeSetting103)
{
	// 元の数をバックアップして最大数にする
	const auto defaultCount = GetDllShareData().m_nTypesCount;
	GetDllShareData().m_nTypesCount = int(MAX_TYPES);

	// 追加の空振り(もう追加できない)
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_ADD_TYPESETTING, GetDllShareData().m_nTypesCount - 1, 0), IsFalse());

	// 数を元に戻す
	GetDllShareData().m_nTypesCount = defaultCount;
}

TEST_F(TrayWndTest, OnSetTypeSetting001)
{
	// 受け渡しバッファに値を設定
	::wcscpy_s(GetDllShareData().m_sWorkBuffer.m_TypeConfig.m_szTypeName, L"テスト");

	// 更新してみる
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_SET_TYPESETTING, 1, 0), IsTrue());

	// 受け取りバッファに値を設定
	::wcscpy_s(GetDllShareData().m_sWorkBuffer.m_TypeConfig.m_szTypeName, L"");

	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_GET_TYPESETTING, 1, 0), IsTrue());

	EXPECT_THAT(GetDllShareData().m_sWorkBuffer.m_TypeConfig.m_szTypeName, StrEq(L"テスト"));
}

TEST_F(TrayWndTest, OnSetTypeSetting002)
{
	// 受け渡しバッファに値を設定
	::wcscpy_s(GetDllShareData().m_sWorkBuffer.m_TypeConfig.m_szTypeName, L"テスト");

	// 更新してみる
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_SET_TYPESETTING, 0, 0), IsTrue());

	// 受け取りバッファに値を設定
	::wcscpy_s(GetDllShareData().m_sWorkBuffer.m_TypeConfig.m_szTypeName, L"");

	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_GET_TYPESETTING, 0, 0), IsTrue());

	EXPECT_THAT(GetDllShareData().m_sWorkBuffer.m_TypeConfig.m_szTypeName, StrEq(L"テスト"));
}

TEST_F(TrayWndTest, OnSetTypeSetting102)
{
	// 更新の空振り(上限値オーバー)
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_SET_TYPESETTING, GetDllShareData().m_nTypesCount, 0), IsFalse());
}

TEST_F(TrayWndTest, OnDelTypeSetting001)
{
	// 削除してみる
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_DEL_TYPESETTING, 8, 0), IsTrue());
}

TEST_F(TrayWndTest, OnDelTypeSetting101)
{
	// 削除の空振り(0は削除させない)
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_DEL_TYPESETTING, 0, 0), IsFalse());
}

TEST_F(TrayWndTest, OnDelTypeSetting102)
{
	// 削除の空振り(上限値オーバー)
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_DEL_TYPESETTING, GetDllShareData().m_nTypesCount, 0), IsFalse());
}

TEST_F(TrayWndTest, OnChangeSetting001)
{
	// 英語にする
	::wcscpy_s(GetDllShareData().m_Common.m_sWindow.m_szLanguageDll, L"sakura_lang_en_US.dll");

	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_CHANGESETTING, 0, int(PM_CHANGESETTING_ALL)), 0);

	// 日本語に戻す
	::wcscpy_s(GetDllShareData().m_Common.m_sWindow.m_szLanguageDll, L"");

	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_CHANGESETTING, 0, int(PM_CHANGESETTING_ALL)), 0);
}

struct EditWndTest : public ::testing::Test, public window::EditorTestSuite, public window::UiaTestSuite {
	static constexpr HINSTANCE unusedArg1 = nullptr;

	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpTestSuite()
	{
		SetUpEditor();

		CKeyMacroMgr::declare();
		CWSHMacroManager::declare();

		SetUpUia();
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownTestSuite()
	{
		TearDownUia();

		CMacroFactory::getInstance()->Unregister(CWSHMacroManager::Creator);
		CMacroFactory::getInstance()->Unregister(CKeyMacroMgr::Creator);

		TearDownEditor();
	}

	std::unique_ptr<CMacroManagerBase> mgr = nullptr;

	/*!
	 * テストが実行された直前に毎回呼ばれる関数
	 */
	void SetUp() override
	{
		mgr = std::unique_ptr<CMacroManagerBase>(CMacroFactory::getInstance()->Create(L"mac"));
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override
	{
		mgr = nullptr;
	}

	/*!
	 * ダイアログを閉じるスレッドを開始する
	 *
	 * @param dialogTitle タイトル
	 * @return ダイアログを閉じるためのスレッド
	 */
	std::jthread StartWindowCloser(std::wstring_view dialogTitle)
	{
		return std::jthread([this, title = std::wstring(dialogTitle)] {
			const auto hWndFound = WaitForWindow(MAKEINTRESOURCEW(dialog::ModalDialogCloser::DIALOG_CLASS), title);
			EmulateInvokeButton(hWndFound, L"キャンセル");
		});
	}
};

/*!
 * キャンセルダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgCancel001)
{
	CDlgCancel cDlgCancel;

	const auto hWnd = pcEditWnd->GetHwnd();

	cDlgCancel.DoModeless(unusedArg1, hWnd, IDD_GREPRUNNING);

	cDlgCancel.CloseDialog(0);
}

/*!
 * 検索ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgFind001)
{
	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L"SearchDialog()"), IsTrue());
	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());

	pcEditWnd->m_cDlgFind.CloseDialog(0);
}

/*!
 * アウトライン解析ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgFuncList001)
{
	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L"Outline(0)"), IsTrue());
	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());

	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L"Outline(1)"), IsTrue());
	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());

	pcEditWnd->m_cDlgFuncList.CloseDialog(0);
}

/*!
 * 置換ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgReplace001)
{
	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L"ReplaceDialog()"), IsTrue());
	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());

	pcEditWnd->m_cDlgReplace.CloseDialog(0);
}

/*!
 * バージョン情報ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgAbout101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L"About()"), IsTrue());
	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());
}

/*!
 * ファイル比較ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgCompare101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	CDlgCompare cDlgCompare;
	const auto hWnd = pcEditWnd->GetHwnd();
	const LPARAM unusedArg2 = 0;
	HWND hWndCompareWnd = nullptr;
	cDlgCompare.DoModal(unusedArg1, hWnd, unusedArg2, L"", &hWndCompareWnd);
}

/*!
 * コントロールコード入力ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgCtrlCode101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L"CtrlCodeDialog()"), IsTrue());
	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());
}

/*!
 * Diff差分ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgDiff101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L"DiffDialog()"), IsTrue());
	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());
}

/*!
 * 外部コマンド実行ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgExec101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L"ExecCommandDialog()"), IsTrue());
	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());
}

/*!
 * 履歴とお気に入りの管理ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgFavorite101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L"OptionFavorite()"), IsTrue());
	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());
}

/*!
 * ファイルツリーダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgFileTree101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	CDlgFileTree cDlgFileTree;
	const auto hWnd = pcEditWnd->GetHwnd();
	auto& cDlgFuncList = pcEditWnd->m_cDlgFuncList;
	cDlgFileTree.DoModal(unusedArg1, hWnd, LPARAM(&cDlgFuncList));
}

/*!
 * 更新通知及び確認ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgFileUpdateQuery101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	CDlgFileUpdateQuery cDlgFileUpdateQuery(L"", false);
	const auto hWnd = pcEditWnd->GetHwnd();
	cDlgFileUpdateQuery.DoModal(unusedArg1, hWnd, IDD_FILEUPDATEQUERY, 0 );
}

/*!
 * Grepダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgGrep101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	pcEditWnd->GetActiveView().GetCommander().HandleCommand(F_GREP_DIALOG, true, 0, 0, 0, 0);
}

/*!
 * Grep置換ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgGrepReplace101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	pcEditWnd->GetActiveView().GetCommander().HandleCommand(F_GREP_REPLACE_DLG, true, 0, 0, 0, 0);
}

/*!
 * 1行入力ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgInputBox101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	// マクロ関数を呼ぶためにWSHマクロマネージャーを使う
	mgr = std::unique_ptr<CMacroManagerBase>(CMacroFactory::getInstance()->Create(L"js"));

	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L"InputBox('test1', 'test2', 0)"), IsTrue());
	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());
}

/*!
 * 指定行へジャンプダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgJump101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	pcEditWnd->GetActiveView().GetCommander().HandleCommand(F_JUMP_DIALOG, true, 0, 0, 0, 0);
}

/*!
 * 強調キーワード選択ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgKeywordSelect101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	CDlgKeywordSelect cDlgKeywordSelect;
	const auto hWnd = pcEditWnd->GetHwnd();
	std::array<int, 10> nSet{};
	cDlgKeywordSelect.DoModal(unusedArg1, hWnd, nSet.data());
}

/*!
 * ファイルを開くダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgOpenFileLegacy101)
{
	// Vistaスタイルのファイルダイアログを無効にする
	GetDllShareData().m_Common.m_sEdit.m_bVistaStyleFileDialog = false;

	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L"FileOpen('', 99, 0, '無題1')"), IsTrue());
	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());

	// 設定を元に戻す
	GetDllShareData().m_Common.m_sEdit.m_bVistaStyleFileDialog = true;
}

/*!
 * ファイルを開くダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgOpenFile101)
{
	// Vistaスタイルのファイルダイアログを有効にする
	GetDllShareData().m_Common.m_sEdit.m_bVistaStyleFileDialog = true;

	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L"FileOpen('', 99, 0, '無題1')"), IsTrue());
	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());
}

/*!
 * プラグイン設定ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgPluginOption001)
{
	// プラグイン設定フォルダー
	const auto pluginPath = GetIniFileName().remove_filename().append(L"plugins");

	// プラグイン定義を展開する
	extract_zip_resource(IDR_ZIPRES1, pluginPath);

	constexpr int pluginId = 1;

	auto& sPlugin = GetDllShareData().m_Common.m_sPlugin;
	sPlugin.m_bEnablePlugin = true;

	auto& pluginRec = sPlugin.m_PluginTable[pluginId];
	pluginRec.m_nCmdNum = pluginId;
	::wcscpy_s(pluginRec.m_szId, L"TestWshPlugin");
	::wcscpy_s(pluginRec.m_szName, L"test-plugin");

	// ジャック初期化
	CJackManager::getInstance();

	// プラグイン読み込み
	CPluginManager::getInstance()->LoadAllPlugin();

	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	CDlgPluginOption cDlgPluginOption;
	const auto hWnd = pcEditWnd->GetHwnd();
	const auto propPlugin = std::make_unique<CPropPlugin>();
	cDlgPluginOption.DoModal(unusedArg1, hWnd, propPlugin.get(), pluginId);

	cDlgPluginOption.DoModal(unusedArg1, hWnd, propPlugin.get(), 0);

	// プラグイン読み込み解除
	CPluginManager::getInstance()->UnloadAllPlugin();

	if (const auto pluginPath = GetIniFileName().remove_filename().append(L"plugins"); fexist(pluginPath)) {
		std::filesystem::remove_all(pluginPath);
	}
}

/*!
 * 印刷設定ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgPrintSetting101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	CDlgPrintSetting cDlgPrintSetting;
	const auto hWnd = pcEditWnd->GetHwnd();
	int nCurrentPrintSetting = -1;
	int nLineNumberColumns = 10;
	cDlgPrintSetting.DoModal(unusedArg1, hWnd, &nCurrentPrintSetting, GetDllShareData().m_PrintSettingArr, nLineNumberColumns);
}

/*!
 * プロファイルマネージャーダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgProfileMgr101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	// プロファイルマネージャーを表示するには、CCommandLineのインスタンスが必要。	👈バグです。
	CCommandLine cmd;

	pcEditWnd->GetActiveView().GetCommander().HandleCommand(F_PROFILEMGR, true, 0, 0, 0, 0);
}

/*!
 * プロパティ情報ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgProperty101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L"PropertyFile()"), IsTrue());
	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());
}

/*!
 * 文字色／背景色統一ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgSameColor101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	CDlgSameColor cDlgSameColor;
	const auto hWnd = pcEditWnd->GetHwnd();
	const WORD wID = 1;
	auto m_nCurrentColorType = 1;
	auto& m_Types = pcEditDoc->m_cDocType.GetDocumentAttributeWrite();
	COLORREF cr = m_Types.m_ColorInfoArr[m_nCurrentColorType].m_sColorAttr.m_cTEXT;
	cDlgSameColor.DoModal(unusedArg1, hWnd, wID, &m_Types, cr);
}

/*!
 * 文字コードセット設定ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgSetCharSet101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L"ChgCharSet(99, 0)"), IsTrue());
	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());
}

/*!
 * タグジャンプダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgTagJumpList101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	bool bDirectTagJump = false;
	CDlgTagJumpList cDlgTagJumpList(bDirectTagJump);
	const auto hWnd = pcEditWnd->GetHwnd();
	cDlgTagJumpList.DoModal(unusedArg1, hWnd, 0L);
}

/*!
 * タグファイル作成ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgTagsMake101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	CDlgTagsMake cDlgTagsMake;
	const auto hWnd = pcEditWnd->GetHwnd();
	LPARAM lParam = 0;
	std::filesystem::path path = L"";

	cDlgTagsMake.DoModal(unusedArg1, hWnd, lParam, path.c_str());
}

/*!
 * タイプ別設定インポート確認ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgTypeAscertain101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	CDlgTypeAscertain cDlgTypeAscertain;
	const auto hWnd = pcEditWnd->GetHwnd();
	CDlgTypeAscertain::SAscertainInfo sAscertainInfo{};
	cDlgTypeAscertain.DoModal(unusedArg1, hWnd, &sAscertainInfo);
}

/*!
 * ファイルタイプ一覧ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgTypeList101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	CDlgTypeList cDlgTypeList;
	const auto hWnd = pcEditWnd->GetHwnd();
	CDlgTypeList::SResult sResult = { CTypeConfig(0), false };
	cDlgTypeList.DoModal(unusedArg1, hWnd, &sResult);
}

/*!
 * ウインドウサイズダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgWinSize101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	CDlgWinSize cDlgWinSize;
	const auto hWnd = pcEditWnd->GetHwnd();
	EWinSizeMode eSaveWinSize = WINSIZEMODE_DEF;
	EWinSizeMode eSaveWinPos = WINSIZEMODE_DEF;
	int nWinSizeType = 0;
	RECT rc = {};
	cDlgWinSize.DoModal(unusedArg1, hWnd, eSaveWinSize, eSaveWinPos, nWinSizeType, rc);
}

/*!
 * ウインドウ一覧ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgWindowList101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer([](HWND hWndDlg){
		// キャンセルボタンが存在しないので強制的に閉じる
		::EndDialog(hWndDlg, 0);
	});

	CDlgWindowList cDlgWindowList;
	const auto hWnd = pcEditWnd->GetHwnd();
	cDlgWindowList.DoModal(unusedArg1, hWnd, 0);
}

/*!
 * 補完ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowHokanMgr001)
{
	// データなしだとスカる
	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L"Complete()"), IsTrue());
	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());

	// とりあえずデータを入れる
	const auto& text = L"test\n\ntes";
	pcEditWnd->GetActiveView().GetCommander().HandleCommand(F_ADDTAIL_W, false, LPARAM(std::data(text)), LPARAM(std::size(text)), 0L, 0L);

	// キャレットを強引に移動させる
	auto& cCaret = pcEditWnd->GetActiveView().GetCaret();
	cCaret.SetCaretLayoutPos(CLayoutPoint(15, 2));
	cCaret.SetCaretLogicPos(CLogicPoint(3, 2));

	// データありで補完実行
	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L"Complete()"), IsTrue());
	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());

	pcEditWnd->m_cHokanMgr.CloseDialog(0);
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon001)
{
	// 表示された共通設定を閉じるためのスレッドを起動する
	std::jthread t = StartWindowCloser(LS(STR_PROPCOMMON));

	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L"OptionCommon()"), IsTrue());
	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon002)
{
	// 表示された共通設定を閉じるためのスレッドを起動する
	std::jthread t = StartWindowCloser(LS(STR_PROPCOMMON));

	CEditApp::getInstance()->OpenPropertySheet(static_cast<int>(ID_PROPCOM_PAGENUM_GENERAL));
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon003)
{
	// 表示された共通設定を閉じるためのスレッドを起動する
	std::jthread t = StartWindowCloser(LS(STR_PROPCOMMON));

	CEditApp::getInstance()->OpenPropertySheet(static_cast<int>(ID_PROPCOM_PAGENUM_WIN));
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon004)
{
	// 表示された共通設定を閉じるためのスレッドを起動する
	std::jthread t = StartWindowCloser(LS(STR_PROPCOMMON));

	CEditApp::getInstance()->OpenPropertySheet(static_cast<int>(ID_PROPCOM_PAGENUM_MAINMENU));
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon005)
{
	// 表示された共通設定を閉じるためのスレッドを起動する
	std::jthread t = StartWindowCloser(LS(STR_PROPCOMMON));

	CEditApp::getInstance()->OpenPropertySheet(static_cast<int>(ID_PROPCOM_PAGENUM_TOOLBAR));
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon006)
{
	// 表示された共通設定を閉じるためのスレッドを起動する
	std::jthread t = StartWindowCloser(LS(STR_PROPCOMMON));

	CEditApp::getInstance()->OpenPropertySheet(static_cast<int>(ID_PROPCOM_PAGENUM_TAB));
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon007)
{
	// 表示された共通設定を閉じるためのスレッドを起動する
	std::jthread t = StartWindowCloser(LS(STR_PROPCOMMON));

	CEditApp::getInstance()->OpenPropertySheet(static_cast<int>(ID_PROPCOM_PAGENUM_STATUSBAR));
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon008)
{
	// 表示された共通設定を閉じるためのスレッドを起動する
	std::jthread t = StartWindowCloser(LS(STR_PROPCOMMON));

	CEditApp::getInstance()->OpenPropertySheet(static_cast<int>(ID_PROPCOM_PAGENUM_EDIT));
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon009)
{
	// 表示された共通設定を閉じるためのスレッドを起動する
	std::jthread t = StartWindowCloser(LS(STR_PROPCOMMON));

	CEditApp::getInstance()->OpenPropertySheet(static_cast<int>(ID_PROPCOM_PAGENUM_FILE));
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon010)
{
	// 表示された共通設定を閉じるためのスレッドを起動する
	std::jthread t = StartWindowCloser(LS(STR_PROPCOMMON));

	CEditApp::getInstance()->OpenPropertySheet(static_cast<int>(ID_PROPCOM_PAGENUM_FILENAME));
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon011)
{
	// 表示された共通設定を閉じるためのスレッドを起動する
	std::jthread t = StartWindowCloser(LS(STR_PROPCOMMON));

	CEditApp::getInstance()->OpenPropertySheet(static_cast<int>(ID_PROPCOM_PAGENUM_BACKUP));
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon012)
{
	// 表示された共通設定を閉じるためのスレッドを起動する
	std::jthread t = StartWindowCloser(LS(STR_PROPCOMMON));

	CEditApp::getInstance()->OpenPropertySheet(static_cast<int>(ID_PROPCOM_PAGENUM_FORMAT));
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon013)
{
	// 表示された共通設定を閉じるためのスレッドを起動する
	std::jthread t = StartWindowCloser(LS(STR_PROPCOMMON));

	CEditApp::getInstance()->OpenPropertySheet(static_cast<int>(ID_PROPCOM_PAGENUM_GREP));
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon014)
{
	// 表示された共通設定を閉じるためのスレッドを起動する
	std::jthread t = StartWindowCloser(LS(STR_PROPCOMMON));

	CEditApp::getInstance()->OpenPropertySheet(static_cast<int>(ID_PROPCOM_PAGENUM_KEYBOARD));
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon015)
{
	// 表示された共通設定を閉じるためのスレッドを起動する
	std::jthread t = StartWindowCloser(LS(STR_PROPCOMMON));

	CEditApp::getInstance()->OpenPropertySheet(static_cast<int>(ID_PROPCOM_PAGENUM_CUSTMENU));
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon016)
{
	// 表示された共通設定を閉じるためのスレッドを起動する
	std::jthread t = StartWindowCloser(LS(STR_PROPCOMMON));

	CEditApp::getInstance()->OpenPropertySheet(static_cast<int>(ID_PROPCOM_PAGENUM_KEYWORD));
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon017)
{
	// 表示された共通設定を閉じるためのスレッドを起動する
	std::jthread t = StartWindowCloser(LS(STR_PROPCOMMON));

	CEditApp::getInstance()->OpenPropertySheet(static_cast<int>(ID_PROPCOM_PAGENUM_HELPER));
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon018)
{
	// 表示された共通設定を閉じるためのスレッドを起動する
	std::jthread t = StartWindowCloser(LS(STR_PROPCOMMON));

	CEditApp::getInstance()->OpenPropertySheet(static_cast<int>(ID_PROPCOM_PAGENUM_MACRO));
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon019)
{
	// 表示された共通設定を閉じるためのスレッドを起動する
	std::jthread t = StartWindowCloser(LS(STR_PROPCOMMON));

	CEditApp::getInstance()->OpenPropertySheet(static_cast<int>(ID_PROPCOM_PAGENUM_PLUGIN));
}

/*!
 * タイプ別設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropType001)
{
	// 表示されたタイプ別設定を閉じるためのスレッドを起動する
	std::jthread t = StartWindowCloser(LS(STR_PROPTYPE));

	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L"OptionType()"), IsTrue());
	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());
}

/*!
 * タイプ別設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropType002)
{
	// 表示されたタイプ別設定を閉じるためのスレッドを起動する
	std::jthread t = StartWindowCloser(LS(STR_PROPTYPE));

	CEditApp::getInstance()->OpenPropertySheetTypes(static_cast<int>(ID_PROPTYPE_PAGENUM_SCREEN), GetDocument()->m_cDocType.GetDocumentType());
}

/*!
 * タイプ別設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropType003)
{
	// 表示されたタイプ別設定を閉じるためのスレッドを起動する
	std::jthread t = StartWindowCloser(LS(STR_PROPTYPE));

	CEditApp::getInstance()->OpenPropertySheetTypes(static_cast<int>(ID_PROPTYPE_PAGENUM_COLOR), GetDocument()->m_cDocType.GetDocumentType());
}

/*!
 * タイプ別設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropType004)
{
	// 表示されたタイプ別設定を閉じるためのスレッドを起動する
	std::jthread t = StartWindowCloser(LS(STR_PROPTYPE));

	CEditApp::getInstance()->OpenPropertySheetTypes(static_cast<int>(ID_PROPTYPE_PAGENUM_WINDOW), GetDocument()->m_cDocType.GetDocumentType());
}

/*!
 * タイプ別設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropType005)
{
	// 表示されたタイプ別設定を閉じるためのスレッドを起動する
	std::jthread t = StartWindowCloser(LS(STR_PROPTYPE));

	CEditApp::getInstance()->OpenPropertySheetTypes(static_cast<int>(ID_PROPTYPE_PAGENUM_SUPPORT), GetDocument()->m_cDocType.GetDocumentType());
}

/*!
 * タイプ別設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropType006)
{
	// 表示されたタイプ別設定を閉じるためのスレッドを起動する
	std::jthread t = StartWindowCloser(LS(STR_PROPTYPE));

	CEditApp::getInstance()->OpenPropertySheetTypes(static_cast<int>(ID_PROPTYPE_PAGENUM_REGEX), GetDocument()->m_cDocType.GetDocumentType());
}

/*!
 * タイプ別設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropType007)
{
	// 表示されたタイプ別設定を閉じるためのスレッドを起動する
	std::jthread t = StartWindowCloser(LS(STR_PROPTYPE));

	CEditApp::getInstance()->OpenPropertySheetTypes(static_cast<int>(ID_PROPTYPE_PAGENUM_KEYHELP), GetDocument()->m_cDocType.GetDocumentType());
}

/*!
 * @brief CFontAutoDeleterのテスト
 */
TEST( CFontAutoDeleter, test )
{
	CFontAutoDeleter deleter;
	ASSERT_EQ(nullptr, deleter.GetFont());

	if (const auto hGdiFont = GetStockFont(DEFAULT_GUI_FONT)) {
		if (LOGFONT lf = {};
			::GetObject(hGdiFont, sizeof(lf), &lf)) {
			if (const auto hFont = ::CreateFontIndirect(&lf)) {
				deleter.SetFont(nullptr, hFont, nullptr);
				ASSERT_EQ(hFont, deleter.GetFont());
			}
		}
	}

	ASSERT_NE(nullptr, deleter.GetFont());
	if (const auto hFont = deleter.GetFont()) {
		CFontAutoDeleter other(deleter);
		ASSERT_NE(hFont, other.GetFont());

		other.ReleaseOnDestroy();
		ASSERT_EQ(nullptr, other.GetFont());

		CFontAutoDeleter another(std::move(deleter));
		ASSERT_EQ(hFont, another.GetFont());
		ASSERT_EQ(nullptr, deleter.GetFont());
	}
}

} // namespace window
