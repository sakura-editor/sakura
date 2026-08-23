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

#include "dlg/CDlgAbout.h"
#include "dlg/CDlgCancel.h"
#include "dlg/CDlgCompare.h"
#include "dlg/CDlgCtrlCode.h"
#include "dlg/CDlgDiff.h"
#include "dlg/CDlgExec.h"
#include "dlg/CDlgFavorite.h"
#include "dlg/CDlgFileUpdateQuery.h"
#include "dlg/CDlgInput1.h"
#include "dlg/CDlgPluginOption.h"
#include "dlg/CDlgPrintSetting.h"
#include "dlg/CDlgProfileMgr.h"
#include "dlg/CDlgProperty.h"
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
#include "util/shell.h"

#include <fstream>
#include <gtest/gtest-spi.h>

#include "config/system_constants.h"
#include "config/app_constants.h"
#include "env/CommonSetting.h"

#include "sakura.hh"

#include "eval_outputs.hpp"

#include "tests1_rc.h"

using namespace std::literals::string_literals;
using namespace std::literals::string_view_literals;

void extract_zip_resource(WORD id, const std::optional<std::filesystem::path>& optOutDir);

struct MockShell32 final : public Shell32
{
	MOCK_CONST_METHOD1(ShellExecuteExW, BOOL (SHELLEXECUTEINFOW*));
};

struct MockCDlgInput1 final : public CDlgInput1
{
	MOCK_METHOD5(DoModal, BOOL(
		_In_opt_ HWND hWndOwner,
		_In_z_ LPCWSTR pszTitle,
		_In_z_ LPCWSTR pszMessage,
		_Out_writes_z_(cchBuffer) LPWSTR pBuffer,
		size_t cchBuffer
	));
};

namespace dialog {

constexpr auto& title1 = L"検索";
constexpr auto& title2 = L"置換";

struct ModalDialogCloserTestPeer {
	static void NotifyCreate(HWND hWnd, LPCWSTR title)
	{
		CREATESTRUCTW createStruct{};
		createStruct.lpszName = title;
		createStruct.lpszClass = WC_DIALOG;
		CBT_CREATEWND createWnd{ &createStruct, HWND_TOP };
		ModalDialogCloser::CBTProc(HCBT_CREATEWND, std::bit_cast<WPARAM>(hWnd), LPARAM(&createWnd));
	}

	static void ActivateDialog(HWND hWnd)
	{
		ModalDialogCloser::CBTProc(HCBT_ACTIVATE, std::bit_cast<WPARAM>(hWnd), 0);
	}

	static void RunFallback(HWND hWnd)
	{
		ModalDialogCloser::TimerProc(hWnd, WM_TIMER, ModalDialogCloser::TIMER_ID_FIRST_IDLE, 0);
	}
};

/*!
 * @brief テスト用テンポラリダイアログ
 *
 * @note CDialogのデストラクターが意図していると思われる機能「不要になったタイミングでまだ表示中ならウィンドウを閉じて安全に破棄される」を実現する。
 * @note そのうち消す
 */
class TestDialog
{
private:
	using HwndHolder = cxx::ResourceHolder<&::DestroyWindow>;

	using Me = TestDialog;

	static INT_PTR CALLBACK DlgProc(
		HWND hWndDlg,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam
	);

	static LRESULT CALLBACK SubclassProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		UINT_PTR uIdSubclass,
		DWORD_PTR dwRefData
	);

public:
	HWND DoModeless(
		int dialogTemplateId,
		_In_opt_ HWND hWndOwnerOrParent = nullptr
	);

private:
	INT_PTR	DispatchDlgEvent(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT	DispatchEvent(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	HWND			m_hWnd = nullptr;

	HwndHolder		m_Holder = nullptr;
};

/*!
 * @brief ダイアログプロシージャ
 *
 * @param hWndDlg [in] 宛先ウィンドウのハンドル
 * @param uMsg [in] メッセージコード
 * @param wParam [in, opt] 第1パラメーター
 * @param lParam [in, opt] 第2パラメーター
 *
 * @retval 0以外 メッセージは処理済み。システムはDWLP_MSGRESULTを参照する。
 * @retval 0     メッセージは未処理。システムに処理させる。
 */
/* static */ INT_PTR CALLBACK TestDialog::DlgProc(
	HWND hWndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
)
{
	// GetWindowLong/SetWindowLongする都合、NULLはマズい。
	if (!hWndDlg) return FALSE;	// システムに処理させる

	// WM_INITDIALOGが来たらHWNDにクラスデータを紐づける
	if (auto pcDlg = std::bit_cast<Me*>(lParam); WM_INITDIALOG == uMsg && pcDlg) {
		// ウィンドウハンドルを格納する
		pcDlg->m_hWnd = hWndDlg;

		// lParamをユーザーデータに格納する。（格納方法は改善検討要。）
		::SetWindowLongPtrW(hWndDlg, DWLP_USER, lParam);

		// ダイアログをカスタマイズする
		::SetWindowSubclass(hWndDlg, &SubclassProc, 0, DWORD_PTR(lParam));
	}

	// HWNDに紐づいたクラスを取り出す。
	if (auto pcDlg = std::bit_cast<Me*>(::GetWindowLongPtrW(hWndDlg, DWLP_USER))) {
		// HWNDに紐づいたクラスにメッセージを処理させる
		const auto ret = pcDlg->DispatchDlgEvent(hWndDlg, uMsg, wParam, lParam);

		if (WM_DESTROY == uMsg) {
			// ユーザーデータの紐付けを解除する
			::SetWindowLongPtrW(hWndDlg, DWLP_USER, 0L);

			// ウィンドウハンドルの紐付けを解除する
			pcDlg->m_hWnd = nullptr;
		}

		// 処理結果を返す。0以外ならシステムはDWLP_MSGRESULTを参照する。
		return ret;
	}

	return FALSE;	// システムに処理させる
}

/*!
 * @brief ウィンドウプロシージャ
 *
 * @param hWnd [in] 宛先ウィンドウのハンドル
 * @param uMsg [in] メッセージコード
 * @param wParam [in, opt] 第1パラメーター
 * @param lParam [in, opt] 第2パラメーター
 * @param uIdSubclass [in] サブクラスID
 * @param dwRefData [in] サブクラスデータ
 *
 * @returns  メッセージ処理結果。値の意味はメッセージ依存。
 */
/* static */ LRESULT CALLBACK TestDialog::SubclassProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	UINT_PTR uIdSubclass,
	DWORD_PTR dwRefData
)
{
	LRESULT ret = 0;

	// サブクラスデータを取り出す。（格納方法は改善検討要。）
	auto pcDlg = std::bit_cast<Me*>(dwRefData);
	if (pcDlg) {
		// サブクラスデータにメッセージを処理させる
		ret = pcDlg->DispatchEvent(hWnd, uMsg, wParam, lParam);
	}

	if (WM_DESTROY == uMsg) {
		::RemoveWindowSubclass(hWnd, &SubclassProc, uIdSubclass);
		return 0;
	}

	if (pcDlg) {
		return ret;
	}

	//あとはデフォルトに任せる
	return ::DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

/*!
 * @brief モードレスダイアログの表示
 *
 * @param dialogTemplateId [in] ダイアログテンプレート
 * @param hWndOwner [in, opt] オーナーウィンドウのハンドル
 */
HWND TestDialog::DoModeless(
	int dialogTemplateId,
	_In_opt_ HWND hWndOwnerOrParent
)
{
	return ::CreateDialogParamW(
		CSelectLang::getLangRsrcInstance(),
		MAKEINTRESOURCE(dialogTemplateId),
		hWndOwnerOrParent,
		DlgProc,
		LPARAM(this)
	);
}

/*!
 * @brief ダイアログのメッセージ配送
 *
 * @param hWndDlg [in] 宛先ウィンドウのハンドル
 * @param uMsg [in] メッセージコード
 * @param wParam [in, opt] 第1パラメーター
 * @param lParam [in, opt] 第2パラメーター
 *
 * @retval 0以外 メッセージは処理済み。システムはDWLP_MSGRESULTを参照する。
 * @retval 0     メッセージは未処理。システムに処理させる。
 *
 * @sa SetDlgMsgResult
 */
INT_PTR TestDialog::DispatchDlgEvent(
	HWND hWndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
)
{
	if (WM_INITDIALOG == uMsg) {
		m_Holder = hWndDlg;

		return TRUE;	// システムが設定した初期フォーカスを受け入れる
	}

	return FALSE;	// システムに処理させる
}

/*!
 * @brief 拡張ウィンドウのメッセージ配送
 *
 * @param hWndDlg [in] 宛先ウィンドウのハンドル
 * @param uMsg [in] メッセージコード
 * @param wParam [in, opt] 第1パラメーター
 * @param lParam [in, opt] 第2パラメーター
 *
 * @returns  メッセージ処理結果。値の意味はメッセージ依存。
 *
 * @note 戻り値0は「処理済み」を示すことが多いが、実装時に確認すること。
 */
LRESULT TestDialog::DispatchEvent(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
)
{
	if (WM_DESTROY == uMsg) {
		m_Holder = nullptr;

		return 0L;
	}

	//あとはデフォルトに任せる
	return ::DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

TEST(ModalDialogCloserTest, RunsActionOnActivateOnlyOnce)
{
	TestDialog dlg1;
	const auto hWnd = dlg1.DoModeless(IDD_FIND);
	int actionCount = 0;
	{
		dialog::ModalDialogCloser closer(title1, [&actionCount] (HWND) { ++actionCount; });
		dialog::ModalDialogCloserTestPeer::NotifyCreate(hWnd, title1);
		dialog::ModalDialogCloserTestPeer::ActivateDialog(hWnd);
		dialog::ModalDialogCloserTestPeer::ActivateDialog(hWnd);
		dialog::ModalDialogCloserTestPeer::RunFallback(hWnd);
	}

	EXPECT_THAT(actionCount, Eq(1));
}

TEST(ModalDialogCloserTest, KeepsPendingEntryWhenTitleDoesNotMatch)
{
	TestDialog dlg1;
	TestDialog dlg2;
	const auto otherHwnd = dlg1.DoModeless(IDD_FIND);
	const auto expectedHwnd = dlg2.DoModeless(IDD_REPLACE);
	HWND handledHwnd = nullptr;
	{
		dialog::ModalDialogCloser closer(title1, [&handledHwnd] (HWND hWnd) { handledHwnd = hWnd; });
		dialog::ModalDialogCloserTestPeer::NotifyCreate(otherHwnd, title2);
		dialog::ModalDialogCloserTestPeer::ActivateDialog(otherHwnd);
		dialog::ModalDialogCloserTestPeer::NotifyCreate(expectedHwnd, title1);
		dialog::ModalDialogCloserTestPeer::ActivateDialog(expectedHwnd);

		while (!dialog::ModalDialogCloser::IsHandled()) {
			::BlockingHook(nullptr);
			::Sleep(10);
		}
	}

	EXPECT_THAT(handledHwnd, Eq(expectedHwnd));
}

TEST(ModalDialogCloserTest, HandlesMultipleEntriesInRegistrationOrder)
{
	TestDialog dlg1;
	TestDialog dlg2;
	const auto firstHwnd = dlg1.DoModeless(IDD_FIND);
	const auto secondHwnd = dlg2.DoModeless(IDD_REPLACE);
	std::vector<HWND> handledWindows;
	const auto func = [&handledWindows] (HWND hWnd) { handledWindows.emplace_back(hWnd); };
	{
		dialog::ModalDialogCloser firstCloser(title1, func);
		dialog::ModalDialogCloser secondCloser(title2, func);
		dialog::ModalDialogCloserTestPeer::NotifyCreate(firstHwnd, title1);
		dialog::ModalDialogCloserTestPeer::NotifyCreate(secondHwnd, title2);
		dialog::ModalDialogCloserTestPeer::ActivateDialog(firstHwnd);
		dialog::ModalDialogCloserTestPeer::ActivateDialog(secondHwnd);

		while (!dialog::ModalDialogCloser::IsHandled()) {
			::BlockingHook(nullptr);
			::Sleep(10);
		}
	}

	EXPECT_THAT(handledWindows, testing::SizeIs(2));	// ウィンドウのスタンバイ状態を確認するよう変えたので順序は保証されない
}

/*!
 * @brief ダイアログのメッセージ配送
 */
TEST(CDialog, DlgProc101)
{
	EXPECT_THAT(CDlgInput1::DlgProc(nullptr, WM_NULL, 0L, 0L), IsFalse());
}

/*!
 * @brief カスタムウィンドウのメッセージ配送
 */
TEST(CDialog, SubclassProc101)
{
	CDlgInput1::SubclassProc(nullptr, WM_NULL, 0L, 0L, 0L, 0L);
}

} // namespace dialog

namespace env {

/*!
 * ShellExecuteExWの呼出(非モック)
 */
TEST(Shell32, ShellExecuteExW001)
{
	const auto path = GetIniFileName().replace_filename(L"test.bat");

	std::error_code ec;
	std::filesystem::remove(path, ec);

	{
		std::wofstream fos(path);
		fos << L"echo test" << std::endl;
	}

	SHELLEXECUTEINFOW execInfo{ sizeof(SHELLEXECUTEINFOW) };
	execInfo.fMask = SEE_MASK_DEFAULT;
	execInfo.lpVerb = L"open";
	execInfo.lpFile = path.c_str();
	execInfo.lpParameters = nullptr;
	execInfo.lpDirectory = path.parent_path().c_str();
	execInfo.nShow = SW_SHOWNORMAL;

	EXPECT_THAT(Shell32::getInstance()->ShellExecuteExW(&execInfo), IsTrue());
	Shell32::resetInstance();

	std::filesystem::remove(path, ec);
}

/*!
 * ShellExecuteExWの呼出(モック利用)
 */
TEST(Shell32, ShellExecuteExW101)
{
	Shell32::setInstance<MockShell32>();
	auto pShell32 = (MockShell32*)Shell32::getInstance();
	EXPECT_CALL(*pShell32, ShellExecuteExW(_))
		.Times(1)
		.WillOnce(Return(TRUE));

	SHELLEXECUTEINFOW execInfo{ sizeof(SHELLEXECUTEINFOW) };
	EXPECT_THAT(Shell32::getInstance()->ShellExecuteExW(&execInfo), IsTrue());

	Shell32::resetInstance();
}

} //namespace env

namespace testing {

void RequestForeignWindowClose(HWND hWnd);

} // namespace testing

namespace window {

struct TrayWndTest : public ::testing::Test, public env::ShareDataTestSuite, public window::UiaTestSuite {
	using CControlTrayHolder = std::unique_ptr<CControlTray>;

	static inline const std::filesystem::path dummyPath = GetIniFileName().replace_filename(L"dummy.txt");

	static inline std::unique_ptr<CCommandLine> pCommandLine = nullptr;

	static inline CControlTrayHolder pcTrayWnd = nullptr;

	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpTestSuite()
	{
		// コマンドラインオブジェクトを用意する
		pCommandLine = std::make_unique<CCommandLine>();
		pCommandLine->ParseCommandLine(L"-PROF=", false);

		SetUpUiaTestSuite();

		SetUpShareData();

		// トレイウィンドウをインスタンス化する
		pcTrayWnd = std::make_unique<CControlTray>();

		pcTrayWnd->m_hIcons.Create(G_AppInstance());

		pcTrayWnd->m_cMenuDrawer.Create(
			CSelectLang::getLangRsrcInstance(),
			pcTrayWnd->GetTrayHwnd(),
			&pcTrayWnd->m_hIcons
		);

		//プロパティ管理
		pcTrayWnd->m_pcPropertyManager = new CPropertyManager();
		pcTrayWnd->m_pcPropertyManager->Create(
			pcTrayWnd->GetTrayHwnd(),
			&pcTrayWnd->m_hIcons,
			&pcTrayWnd->m_cMenuDrawer
		);

		std::error_code ec;
		std::filesystem::remove(dummyPath, ec);

		{
			std::wofstream fos(dummyPath);
			fos << L"ダミーファイルです" << std::endl;
		}
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownTestSuite()
	{
		std::error_code ec;
		std::filesystem::remove(dummyPath, ec);

		// トレイウィンドウのインスタンスを破棄する
		pcTrayWnd = nullptr;

		TearDownShareData();

		TearDownUiaTestSuite();

		pCommandLine = nullptr;
	}

	/*!
	 * テストが実行される直前に毎回呼ばれる関数
	 */
	void SetUp() override
	{
		User32::setInstance<MockUser32>();

		CDlgOpenFile::setInstance<MockCDlgOpenFile>();
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override {
		// キューに溜まったメッセージは全部捨てる
		MSG msg{};
		while (::PeekMessageW(&msg, nullptr, 0L, 0L, PM_REMOVE)) ;

		CDlgOpenFile::resetInstance();

		User32::resetInstance();

		TearDownUia();
	}
};

TEST_F(TrayWndTest, OpenNewEditor101)
{
	// 開いているファイルの数を上限値に設定する
	GetDllShareData().m_sNodes.m_nEditArrNum = MAX_EDITWINDOWS;

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(strprintf(LS(STR_MAXWINDOW), MAX_EDITWINDOWS)), _, _, _))
		.Times(1)
		.WillOnce(Return(IDOK));

	SLoadInfo sLoadInfo{};
	EXPECT_THAT(CControlTray::OpenNewEditor(nullptr, HWND(nullptr), sLoadInfo), IsFalse());

	// 設定を元に戻す
	GetDllShareData().m_sNodes.m_nEditArrNum = 0;
}

TEST_F(TrayWndTest, OpenNewEditor102)
{
	// 開いているファイルの数を上限値に設定する
	GetDllShareData().m_sNodes.m_nEditArrNum = MAX_EDITWINDOWS;

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(strprintf(LS(STR_MAXWINDOW), MAX_EDITWINDOWS)), _, _, _))
		.Times(1)
		.WillOnce(Return(IDOK));

	EditInfo fi{};
	EXPECT_THAT(CControlTray::OpenNewEditor2(nullptr, HWND(nullptr), &fi, false), IsFalse());

	// 設定を元に戻す
	GetDllShareData().m_sNodes.m_nEditArrNum = 0;
}

TEST_F(TrayWndTest, TerminateApplication101)
{
	GetDllShareData().m_Common.m_sGeneral.m_bExitConfirm = false;

	GetDllShareData().m_sNodes.m_nEditArrNum = 0;

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, _, _, _, _))
		.Times(0);

	const HWND hWndFrom = nullptr;
	CControlTray::TerminateApplication(hWndFrom);

	// 設定を元に戻す
	GetDllShareData().m_sNodes.m_nEditArrNum = 0;

	GetDllShareData().m_Common.m_sGeneral.m_bExitConfirm = false;
}

TEST_F(TrayWndTest, TerminateApplication102)
{
	GetDllShareData().m_Common.m_sGeneral.m_bExitConfirm = true;

	GetDllShareData().m_sNodes.m_nEditArrNum = 1;

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(L"現在開いている編集用のウィンドウをすべて閉じて終了しますか?"), _, _, _))
		.Times(1)
		.WillOnce(Return(IDNO));

	const HWND hWndFrom = nullptr;
	CControlTray::TerminateApplication(hWndFrom);

	// 設定を元に戻す
	GetDllShareData().m_sNodes.m_nEditArrNum = 0;

	GetDllShareData().m_Common.m_sGeneral.m_bExitConfirm = false;
}

TEST_F(TrayWndTest, DISABLED_OnCreate101)	// パラメーター不正の考慮がないので呼べない
{
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, WM_CREATE, 0L, 0L), IsTrue());	// 戻り値は反転される
}

TEST_F(TrayWndTest, DISABLED_OnDestroy101)	// パラメーター不正の考慮がないので呼べない
{
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, WM_DESTROY, 0L, 0L), IsFalse());
}

TEST_F(TrayWndTest, DISABLED_OnClose101)	// パラメーター不正の考慮がないので呼べない
{
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, WM_CLOSE, 0L, 0L), IsFalse());
}

TEST_F(TrayWndTest, DISABLED_OnQueryEndSession101)	// パラメーター不正の考慮がないので呼べない
{
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, WM_QUERYENDSESSION, 0L, 0L), IsTrue());
}

TEST_F(TrayWndTest, OnEndSession101)
{
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, WM_ENDSESSION, FALSE, 0L), IsFalse());
}

TEST_F(TrayWndTest, OnHelp101)
{
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, WM_HELP, 0L, 0L), IsTrue());

	HELPINFO hi{};
	hi.iContextType = HELPINFO_WINDOW;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, WM_HELP, 0L, LPARAM(&hi)), IsTrue());

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());

	hi.iContextType = HELPINFO_MENUITEM;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, WM_HELP, 0L, LPARAM(&hi)), IsTrue());
}

TEST_F(TrayWndTest, OnCommand101)
{
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, WM_COMMAND, 0L, 0L), IsFalse());
}

TEST_F(TrayWndTest, OnTimer101)
{
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, WM_TIMER, 2 /* IDT_EDITCHECK */, 0L), IsFalse());
}

TEST_F(TrayWndTest, OnMenuChar101)
{
	HWND hWndTray = nullptr;
	pcTrayWnd->DispatchEvent(hWndTray, WM_MENUCHAR, 0L, 0L);
}

TEST_F(TrayWndTest, OnExitMenuLoop101)
{
	HWND hWndTray = nullptr;
	pcTrayWnd->DispatchEvent(hWndTray, WM_EXITMENULOOP, 0L, 0L);
}

TEST_F(TrayWndTest, OnHotKey101)
{
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, WM_HOTKEY, 0L, 0L), IsFalse());
}

TEST_F(TrayWndTest, OnTaskbarReCreated101)
{
	const UINT uMsgTaskbarCreated = ::RegisterWindowMessageW(L"TaskbarCreated");

	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, uMsgTaskbarCreated, 0L, 0L), IsFalse());
}

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

TEST_F(TrayWndTest, OnDeleteMe101)
{
	pcTrayWnd->m_bCreatedTrayIcon = true;

	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_DELETE_ME, 0L, 0L), IsFalse());

	pcTrayWnd->m_bCreatedTrayIcon = false;
}

TEST_F(TrayWndTest, OnHtmlHelp101)
{
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_HTMLHELP, WPARAM(hWndTray), 0L), IsFalse());
}

/*!
 * 開いているエディターをアクティブにする
 */
TEST_F(TrayWndTest, ActivateOpenedEditor101)
{
	// 開いているファイルの数を1にする
	GetDllShareData().m_sNodes.m_nEditArrNum = 1;

	HWND hWndTray = nullptr;
	pcTrayWnd->ExecCommand(hWndTray, IDM_SELWINDOW);

	// 開いているファイルの数を0に戻す
	GetDllShareData().m_sNodes.m_nEditArrNum = 0;
}

/*!
 * Grepダイアログを表示して実行する
 */
TEST_F(TrayWndTest, DoGrep101)
{
	// 検索条件
	CSearchKeywordManager().AddToSearchKeyArr(LR"(localhost)");

	// 検索フォルダー
	CSearchKeywordManager().AddToGrepFolderArr(LR"(C:\WINDOWS\System32\Drivers)");

	// 検索ファイル
	CSearchKeywordManager().AddToGrepFileArr(LR"(*.*)");

	// 除外フォルダー
	CSearchKeywordManager().AddToExcludeFolderArr(LR"(en-US;DriverData;UMDF;udc;mde;wd;)");

	// 除外ファイル
	CSearchKeywordManager().AddToExcludeFileArr(LR"(*.sys;*.dll;*.exe;*.mui;*.nls;*.chm;*.dat;*.tmp;*.wdf)");

	// 開いているファイルの数を上限値に設定する
	GetDllShareData().m_sNodes.m_nEditArrNum = MAX_EDITWINDOWS;

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(strprintf(LS(STR_MAXWINDOW), MAX_EDITWINDOWS)), _, _, _))
		.Times(1)
		.WillOnce(Return(IDOK));

	// 表示されたモーダルダイアログをOKボタンで閉じる
	dialog::ModalDialogCloser closer(L"Grep", [] (HWND hWndDlg) {
		SendDlgCommand(hWndDlg, IDC_COMBO_TEXT, CBN_DROPDOWN);
		SendDlgCommand(hWndDlg, IDC_COMBO_FILE, CBN_DROPDOWN);
		SendDlgCommand(hWndDlg, IDC_COMBO_FOLDER, CBN_DROPDOWN);
		SendDlgCommand(hWndDlg, IDC_COMBO_EXCLUDE_FILE, CBN_DROPDOWN);
		SendDlgCommand(hWndDlg, IDC_COMBO_EXCLUDE_FOLDER, CBN_DROPDOWN);

		SendDlgCommand(hWndDlg, IDOK);
	});

	HWND hWndTray = nullptr;
	pcTrayWnd->ExecCommand(hWndTray, F_GREP_DIALOG);

	// 設定を元に戻す
	GetDllShareData().m_sSearchKeywords.m_aSearchKeys.clear();
	GetDllShareData().m_sSearchKeywords.m_aGrepFolders.clear();
	GetDllShareData().m_sSearchKeywords.m_aGrepFiles.clear();
	GetDllShareData().m_sSearchKeywords.m_aExcludeFolders.clear();
	GetDllShareData().m_sSearchKeywords.m_aExcludeFiles.clear();

	GetDllShareData().m_sNodes.m_nEditArrNum = 0;
}

/*!
 * 編集の全終了のテスト
 */
TEST_F(TrayWndTest, ExitAllEdtors101)
{
	HWND hWndTray = nullptr;
	pcTrayWnd->ExecCommand(hWndTray, F_EXITALLEDITORS);
}

/*!
 * サクラエディタの全終了のテスト
 */
TEST_F(TrayWndTest, ExitAll101)
{
	HWND hWndTray = nullptr;
	pcTrayWnd->ExecCommand(hWndTray, F_EXITALL);
}

TEST_F(TrayWndTest, HelpContents101)
{
	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_COMMAND, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	HWND hWndTray = nullptr;
	pcTrayWnd->ExecCommand(hWndTray, F_HELP_CONTENTS);
}

TEST_F(TrayWndTest, HelpSearch101)
{
	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_KEY, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	HWND hWndTray = nullptr;
	pcTrayWnd->ExecCommand(hWndTray, F_HELP_SEARCH);
}

/*!
 * 開くダイアログを表示する
 */
TEST_F(TrayWndTest, OpenFile101)
{
	// 開いているファイルの数を上限値に設定する
	GetDllShareData().m_sNodes.m_nEditArrNum = MAX_EDITWINDOWS;

	const auto& path = dummyPath;
	MockCDlgOpenFile::gm_Files.emplace_back(path.native());

	auto& cDlgOpenFile = static_cast<MockCDlgOpenFile&>(*CDlgOpenFile::getInstance());
	EXPECT_CALL(cDlgOpenFile, DoModalOpenDlg(_, _, _))
		.Times(1)
		.WillOnce(testing::DoDefault());

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(strprintf(LS(STR_MAXWINDOW), MAX_EDITWINDOWS)), _, _, _))
		.Times(1)
		.WillOnce(Return(IDOK));

	HWND hWndTray = nullptr;
	pcTrayWnd->ExecCommand(hWndTray, F_FILEOPEN);

	// 設定を元に戻す
	GetDllShareData().m_sNodes.m_nEditArrNum = 0;
}

/*!
 * トレイダブルクリックのテスト
 * トレイアイコンをダブルクリックすると新規エディターが開く
 */
TEST_F(TrayWndTest, OpenNewEditor103)
{
	// 開いているファイルの数を上限値に設定する
	GetDllShareData().m_sNodes.m_nEditArrNum = MAX_EDITWINDOWS;

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(strprintf(LS(STR_MAXWINDOW), MAX_EDITWINDOWS)), _, _, _))
		.Times(1)
		.WillOnce(Return(IDOK));

	// トレイアイコンダブルクリックイベントを発生させる
	HWND hWndTray = nullptr;
	pcTrayWnd->DispatchEvent(hWndTray, MYWM_NOTIFYICON, 0L, WM_LBUTTONDBLCLK);

	// 設定を元に戻す
	GetDllShareData().m_sNodes.m_nEditArrNum = 0;
}

/*!
 * 新規作成のテスト
 */
TEST_F(TrayWndTest, OpenNewEditor104)
{
	// 開いているファイルの数を上限値に設定する
	GetDllShareData().m_sNodes.m_nEditArrNum = MAX_EDITWINDOWS;

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(strprintf(LS(STR_MAXWINDOW), MAX_EDITWINDOWS)), _, _, _))
		.Times(1)
		.WillOnce(Return(IDOK));

	HWND hWndTray = nullptr;
	pcTrayWnd->ExecCommand(hWndTray, F_FILENEW);

	// 設定を元に戻す
	GetDllShareData().m_sNodes.m_nEditArrNum = 0;
}

/*!
 * 最近使ったファイルを開くのテスト
 */
TEST_F(TrayWndTest, SelectAndOpenFilesFromMruFile101)
{
	// 開いているファイルの数を上限値に設定する
	GetDllShareData().m_sNodes.m_nEditArrNum = MAX_EDITWINDOWS;

	GetDllShareData().m_Common.m_sFile.m_bRestoreCurPosition = true;

	GetDllShareData().m_sHistory.m_nMRUArrNum = 1;

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(strprintf(LS(STR_MAXWINDOW), MAX_EDITWINDOWS)), _, _, _))
		.Times(1)
		.WillOnce(Return(IDOK));

	HWND hWndTray = nullptr;
	pcTrayWnd->ExecCommand(hWndTray, IDM_SELMRU);

	// 設定を元に戻す
	GetDllShareData().m_sNodes.m_nEditArrNum = 0;

	GetDllShareData().m_sHistory.m_nMRUArrNum = 0;

	GetDllShareData().m_Common.m_sFile.m_bRestoreCurPosition = true;
}

/*!
 * 最近使ったファイルを開くのテスト
 */
TEST_F(TrayWndTest, SelectAndOpenFilesFromMruFile102)
{
	// 開いているファイルの数を上限値に設定する
	GetDllShareData().m_sNodes.m_nEditArrNum = MAX_EDITWINDOWS;

	GetDllShareData().m_Common.m_sFile.m_bRestoreCurPosition = false;

	GetDllShareData().m_sHistory.m_nMRUArrNum = 1;

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(strprintf(LS(STR_MAXWINDOW), MAX_EDITWINDOWS)), _, _, _))
		.Times(1)
		.WillOnce(Return(IDOK));

	HWND hWndTray = nullptr;
	pcTrayWnd->ExecCommand(hWndTray, IDM_SELMRU);

	// 設定を元に戻す
	GetDllShareData().m_sNodes.m_nEditArrNum = 0;

	GetDllShareData().m_sHistory.m_nMRUArrNum = 0;

	GetDllShareData().m_Common.m_sFile.m_bRestoreCurPosition = true;
}

/*!
 * 最近使ったフォルダーからファイルを選択して開くのテスト
 */
TEST_F(TrayWndTest, SelectAndOpenFilesFromMruFolder101)
{
	GetDllShareData().m_sHistory.m_nOPENFOLDERArrNum = 1;

	MockCDlgOpenFile::gm_Files.clear();

	auto& cDlgOpenFile = static_cast<MockCDlgOpenFile&>(*CDlgOpenFile::getInstance());
	EXPECT_CALL(cDlgOpenFile, DoModalOpenDlg(_, _, _))
		.Times(1)
		.WillOnce(testing::DoDefault());

	HWND hWndTray = nullptr;
	pcTrayWnd->ExecCommand(hWndTray, IDM_SELOPENFOLDER);

	GetDllShareData().m_sHistory.m_nOPENFOLDERArrNum = 0;
}

/*!
 * すべて上書き保存のテスト
 */
TEST_F(TrayWndTest, SaveAllFiles101)
{
	HWND hWndTray = nullptr;
	pcTrayWnd->ExecCommand(hWndTray, F_FILESAVEALL);
}

/*!
 * バージョン情報ダイアログを表示する
 */
TEST_F(TrayWndTest, ShowDlgAbout001)
{
	// 表示されたモーダルダイアログをOKボタンで閉じる
	dialog::ModalDialogCloser closer(L"バージョン情報", [this] (HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndDlg, IDC_BUTTON_HELP);

		SendDlgCommand(hWndDlg, IDC_BUTTON_COPY);

		SendDlgCommand(hWndDlg, IDOK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	HWND hWndTray = nullptr;
	pcTrayWnd->ExecCommand(hWndTray, F_ABOUT);
}

/*!
 * 履歴とお気に入りの管理ダイアログを表示する
 */
TEST_F(TrayWndTest, ShowDlgFavorite001)
{
	// 表示されたモーダルダイアログをOKボタンで閉じる
	dialog::ModalDialogCloser closer(L"履歴とお気に入りの管理", IDOK);

	HWND hWndTray = nullptr;
	pcTrayWnd->ExecCommand(hWndTray, F_FAVORITE);
}

/*!
 * タイプ別設定一覧ダイアログの表示テスト
 */
/*!
 * タイプ別設定一覧ダイアログの表示テスト
 */
TEST_F(TrayWndTest, ShowDlgTypeList101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(L"タイプ別設定一覧");

	// タイプ別設定一覧ダイアログを表示する
	HWND hWndTray = nullptr;
	pcTrayWnd->ExecCommand(hWndTray, F_TYPE_LIST);
}

/*!
 * ウインドウ一覧ダイアログの表示テスト
 */
TEST_F(TrayWndTest, ShowDlgWindowList001)
{
	// 表示されたモーダルダイアログをOKボタンで閉じる
	dialog::ModalDialogCloser closer(L"ウィンドウ一覧", [] (HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndDlg, IDC_BUTTON_HELP);

		SendDlgCommand(hWndDlg, IDC_BUTTON_SAVE);
		SendDlgCommand(hWndDlg, IDC_BUTTON_CLOSE);

		CMyRect rc{};
		::GetClientRect(hWndDlg, &rc);
		FORWARD_WM_SIZE(hWndDlg, SIZE_RESTORED, rc.right, rc.bottom, ::SendMessageW);

		SendDlgCommand(hWndDlg, IDOK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	using target = CDlgWindowList;
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_DLGWINLIST, 0L, 0L), IsFalse());
}

/*!
 * ウィンドウ一覧ダイアログを表示する
 */
TEST_F(TrayWndTest, ShowDlgWindowList101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(L"ウィンドウ一覧");

	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_DLGWINLIST, 0L, 0L), IsFalse());
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(TrayWndTest, ShowPropCommon001)
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

	// 表示された共通設定をOKボタンで閉じる
	dialog::PropertySheetCloser closer(PSBTN_OK);

	// 共通設定プロパティーシートを表示する
	HWND hWndTray = nullptr;
	pcTrayWnd->ExecCommand(hWndTray, F_OPTION);

	std::error_code ec;
	std::filesystem::remove_all(pluginPath, ec);
}

/*!
 * タイプ別設定プロパティーシートの表示テスト
 */
TEST_F(TrayWndTest, ShowPropType001)
{
	// 表示されたモーダルダイアログをOKボタンで閉じる
	dialog::ModalDialogCloser closer1(L"タイプ別設定一覧", IDOK);

	// 表示されたタイプ別設定をOKボタンで閉じる
	dialog::PropertySheetCloser closer2(PSBTN_OK);

	// タイプ別設定一覧ダイアログを表示する
	HWND hWndTray = nullptr;
	pcTrayWnd->ExecCommand(hWndTray, F_TYPE_LIST);
}

/*!
 * タイプ別設定プロパティーシートの表示テスト
 */
TEST_F(TrayWndTest, ShowPropType101)
{
	// 表示されたモーダルダイアログをOKボタンで閉じる
	dialog::ModalDialogCloser closer1(L"タイプ別設定一覧", IDOK);

	// 表示されたタイプ別設定をキャンセルボタンで閉じる
	dialog::PropertySheetCloser closer2;

	// タイプ別設定一覧ダイアログを表示する
	HWND hWndTray = nullptr;
	pcTrayWnd->ExecCommand(hWndTray, F_TYPE_LIST);
}

/*!
 * トレイメニューのテスト
 *
 * 左クリックメニューからコマンドを実行する
 * 実行するコマンドは何でもよいので履歴とお気に入りの管理ダイアログを出しておく
 */
TEST_F(TrayWndTest, ShowTrayMenu001)
{
	// 表示されたモーダルダイアログをOKボタンで閉じる
	dialog::ModalDialogCloser closer(L"履歴とお気に入りの管理", IDOK);

	// ポップアップメニュー項目を選択させる
	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, TrackPopupMenu(_, _, _, _, _, _, _))
		.Times(1)
		.WillOnce(Return<int>(F_FAVORITE));

	// トレイアイコン左クリックメニューを表示させる
	HWND hWndTray = nullptr;
	pcTrayWnd->DispatchEvent(hWndTray, MYWM_NOTIFYICON, 0L, WM_LBUTTONDOWN);
	pcTrayWnd->DispatchEvent(hWndTray, MYWM_NOTIFYICON, 0L, WM_LBUTTONUP);
}

/*!
 * トレイコンテキストメニューのテスト
 *
 * 右クリックメニューからコマンドを実行する
 * 実行するコマンドは何でもよいのでバージョン情報ダイアログを出しておく
 */
TEST_F(TrayWndTest, ShowContextMenu001)
{
	// 表示されたモーダルダイアログをOKボタンで閉じる
	dialog::ModalDialogCloser closer(L"バージョン情報", IDOK);

	// ポップアップメニュー項目を選択させる
	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, TrackPopupMenu(_, _, _, _, _, _, _))
		.Times(1)
		.WillOnce(Return<int>(F_ABOUT));

	// トレイアイコン右クリックメニューを表示させる
	HWND hWndTray = nullptr;
	pcTrayWnd->DispatchEvent(hWndTray, MYWM_NOTIFYICON, 0L, WM_RBUTTONDOWN);
	pcTrayWnd->DispatchEvent(hWndTray, MYWM_NOTIFYICON, 0L, WM_RBUTTONUP);
}

struct EditWndTest : public ::testing::Test, public window::EditorTestSuite, public window::UiaTestSuite {
	static constexpr HINSTANCE unusedArg1 = nullptr;

	static inline const std::filesystem::path backupAgentTargetPath = GetIniFileName().replace_filename(L"backup-agent-target.txt");
	static inline const std::filesystem::path backupPath = backupAgentTargetPath.parent_path() / (backupAgentTargetPath.stem().native() + L".bak");
	static inline const std::filesystem::path colorizeExportPath = GetIniFileName().replace_filename(L"基本.col");
	static inline const std::filesystem::path custmenuExportPath = GetIniFileName().replace_filename(L"カスタムメニュー.mnu");
	static inline const std::filesystem::path dummyPath = GetIniFileName().replace_filename(L"dummy.txt");
	static inline const std::filesystem::path fileTreeExportPath = GetIniFileName().replace_filename(L"ファイルツリー.ini");
	static inline const std::filesystem::path keybindExportPath = GetIniFileName().replace_filename(L"キー割り当て.key");
	static inline const std::filesystem::path keywordExportPath = GetIniFileName().replace_filename(L"強調キーワード.kwd");
	static inline const std::filesystem::path keywordHelpExportPath = GetIniFileName().replace_filename(L"キーワードヘルプ.txt");
	static inline const std::filesystem::path mainmenuExportPath = GetIniFileName().replace_filename(L"メインメニュー.ini");
	static inline const std::filesystem::path regexKeywordExportPath = GetIniFileName().replace_filename(L"テキスト.rkw");
	static inline const std::filesystem::path typeConfigExportPath = GetIniFileName().replace_filename(L"基本.ini");

	static inline std::unique_ptr<CCommandLine> pCommandLine = nullptr;

	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpTestSuite()
	{
		// コマンドラインオブジェクトを用意する
		pCommandLine = std::make_unique<CCommandLine>();
		pCommandLine->ParseCommandLine(L"-PROF=", false);

		SetUpUiaTestSuite();

		SetUpEditor();

		CKeyMacroMgr::declare();
		CWSHMacroManager::declare();

		std::error_code ec;
		std::filesystem::remove(backupAgentTargetPath, ec);
		std::filesystem::remove(backupPath, ec);
		std::filesystem::remove(colorizeExportPath, ec);
		std::filesystem::remove(custmenuExportPath, ec);
		std::filesystem::remove(dummyPath, ec);
		std::filesystem::remove(fileTreeExportPath, ec);
		std::filesystem::remove(keybindExportPath, ec);
		std::filesystem::remove(keywordExportPath, ec);
		std::filesystem::remove(keywordHelpExportPath, ec);
		std::filesystem::remove(mainmenuExportPath, ec);
		std::filesystem::remove(regexKeywordExportPath, ec);
		std::filesystem::remove(typeConfigExportPath, ec);

		{
			std::wofstream fos(backupAgentTargetPath);
			fos << L"line1" << std::endl;
		}

		{
			std::wofstream fos(dummyPath);
			fos << L"ダミーファイルです" << std::endl;
		}
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownTestSuite()
	{
		std::error_code ec;
		std::filesystem::remove(backupAgentTargetPath, ec);
		std::filesystem::remove(backupPath, ec);
		std::filesystem::remove(colorizeExportPath, ec);
		std::filesystem::remove(custmenuExportPath, ec);
		std::filesystem::remove(dummyPath, ec);
		std::filesystem::remove(fileTreeExportPath, ec);
		std::filesystem::remove(keybindExportPath, ec);
		std::filesystem::remove(keywordExportPath, ec);
		std::filesystem::remove(keywordHelpExportPath, ec);
		std::filesystem::remove(mainmenuExportPath, ec);
		std::filesystem::remove(regexKeywordExportPath, ec);
		std::filesystem::remove(typeConfigExportPath, ec);

		CMacroFactory::getInstance()->Unregister(CWSHMacroManager::Creator);
		CMacroFactory::getInstance()->Unregister(CKeyMacroMgr::Creator);

		TearDownEditor();

		TearDownUiaTestSuite();

		pCommandLine = nullptr;
	}

	std::unique_ptr<CMacroManagerBase> mgr = nullptr;

	/*!
	 * テストが実行される直前に毎回呼ばれる関数
	 */
	void SetUp() override
	{
		User32::setInstance<MockUser32>();

		mgr = std::unique_ptr<CMacroManagerBase>(CMacroFactory::getInstance()->Create(L"mac"));

		CDlgOpenFile::setInstance<MockCDlgOpenFile>();

		CDlgInput1::setInstance<MockCDlgInput1>();
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override
	{
		// 強制的に「編集なし」にする
		pcEditDoc->m_cDocEditor.m_bIsDocModified = false;

		// 強制的に「Grepモード」を解除する
		CEditApp::getInstance()->m_pcGrepAgent->m_bGrepMode = false;

		// キューに溜まったメッセージは全部捨てる
		MSG msg{};
		while (::PeekMessageW(&msg, nullptr, 0L, 0L, PM_REMOVE)) ;

		// ファイルを閉じたことにする
		pcEditDoc->m_cDocFile.SetFilePath(L"");

		CDiffManager::resetInstance();

		CDlgInput1::resetInstance();

		CDlgOpenFile::resetInstance();

		mgr = nullptr;

		Comdlg32::resetInstance();

		Shell32::resetInstance();

		User32::resetInstance();

		TearDownUia();
	}

	/*!
	 * マクロ経由でコマンドを実行する
	 */
	bool ExecMacroCommand(std::wstring_view macroScript) {
		return mgr->LoadKeyMacroStr(unusedArg1, std::data(macroScript))
			&& mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0);
	}

	/*!
	 * 共通設定プロパティーシートを表示する
	 */
	void ShowPropCommon(PropComSheetOrder pageNum) const
	{
		CEditApp::getInstance()->OpenPropertySheet( static_cast<int>(pageNum) );
	}

	/*!
	 * タイプ別設定プロパティーシートを表示する
	 */
	void ShowPropType(PropTypeSheetOrder pageNum) const
	{
		CEditApp::getInstance()->OpenPropertySheetTypes(static_cast<int>(pageNum), GetDocument()->m_cDocType.GetDocumentType());
	}
};

TEST_F(EditWndTest, DISABLED_OnCreate101)	// パラメーター不正の考慮がないので呼べない
{
	HWND hWndEdit = nullptr;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_CREATE, 0L, 0L), IsTrue());	// 戻り値は反転される
}

TEST_F(EditWndTest, DISABLED_OnDestroy101)	// パラメーター不正の考慮がないので呼べない
{
	HWND hWndEdit = nullptr;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_DESTROY, 0L, 0L), IsFalse());
}

TEST_F(EditWndTest, DISABLED_OnMove101)	// パラメーター不正の考慮がないので呼べない
{
	HWND hWndEdit = nullptr;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_MOVE, 0L, 0L), IsFalse());
}

TEST_F(EditWndTest, DISABLED_OnSize101)	// パラメーター不正の考慮がないので呼べない
{
	HWND hWndEdit = nullptr;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_SIZE, SIZE_RESTORED, 0L), IsFalse());
}

TEST_F(EditWndTest, DISABLED_OnSetFocus101)	// パラメーター不正の考慮がないので呼べない
{
	HWND hWndEdit = nullptr;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_SETFOCUS, 0L, 0L), IsFalse());
}

TEST_F(EditWndTest, OnEnable101)
{
	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(LS(STR_ERR_DLGDRPTGT1)), _, _, _))
		.Times(1)
		.WillOnce(Return(IDOK));

	HWND hWndEdit = nullptr;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_ENABLE, TRUE, 0L), IsFalse());
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_ENABLE, FALSE, 0L), IsFalse());
}

TEST_F(EditWndTest, OnSetText101)
{
	HWND hWndEdit = nullptr;
	pcEditWnd->DispatchEvent(hWndEdit, WM_SETTEXT, 0L, LPARAM(L"title-from-test"));
}

TEST_F(EditWndTest, DISABLED_OnClose101)	// パラメーター不正の考慮がないので呼べない
{
	HWND hWndEdit = nullptr;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_CLOSE, 0L, 0L), IsFalse());
}

TEST_F(EditWndTest, DISABLED_OnQueryEndSession101)	// パラメーター不正の考慮がないので呼べない
{
	HWND hWndEdit = nullptr;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_QUERYENDSESSION, 0L, 0L), IsTrue());
}

TEST_F(EditWndTest, OnShowWindow101)
{
	HWND hWndEdit = nullptr;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_SHOWWINDOW, FALSE, 0L), IsFalse());
}

TEST_F(EditWndTest, OnPaintIcon101)
{
	HWND hWndEdit = nullptr;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_PAINTICON, 0L, 0L), IsFalse());
}

TEST_F(EditWndTest, OnIconEraseBkgnd101)
{
	HWND hWndEdit = nullptr;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_ICONERASEBKGND, 0L, 0L), IsFalse());
}

TEST_F(EditWndTest, OnWindowPosChanged101)
{
	HWND hWndEdit = nullptr;
	WINDOWPOS wp{};
	wp.flags = SWP_SHOWWINDOW;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_WINDOWPOSCHANGED, 0L, LPARAM(&wp)), IsFalse());
}

TEST_F(EditWndTest, OnWindowPosChanged102)
{
	HWND hWndEdit = nullptr;
	WINDOWPOS wp{};
	wp.flags = SWP_HIDEWINDOW;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_WINDOWPOSCHANGED, 0L, LPARAM(&wp)), IsFalse());
}

TEST_F(EditWndTest, OnHScroll101)
{
	HWND hWndEdit = nullptr;
	pcEditWnd->DispatchEvent(hWndEdit, WM_HSCROLL, MAKEWPARAM(SB_LINELEFT, 0), 0L);
}

TEST_F(EditWndTest, OnVScroll101)
{
	HWND hWndEdit = nullptr;
	pcEditWnd->DispatchEvent(hWndEdit, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0L);
}

TEST_F(EditWndTest, OnMenuSelect101)
{
	HWND hWndEdit = nullptr;
	pcEditWnd->DispatchEvent(hWndEdit, WM_MENUSELECT, 0L, 0L);
}

TEST_F(EditWndTest, DISABLED_OnNotify101)	// パラメーター不正の考慮がないので呼べない
{
	HWND hWndEdit = nullptr;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_NOTIFY, 0L, 0L), IsFalse());
}

TEST_F(EditWndTest, OnHelp101)
{
	HWND hWndEdit = nullptr;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_HELP, 0L, 0L), IsTrue());

	HELPINFO hi{};
	hi.iContextType = HELPINFO_WINDOW;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_HELP, 0L, LPARAM(&hi)), IsTrue());

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());

	hi.iContextType = HELPINFO_MENUITEM;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_HELP, 0L, LPARAM(&hi)), IsTrue());

	User32::resetInstance();

	EXPECT_THAT(User32::getInstance()->WinHelpW(hWndEdit, nullptr, HELP_CONTEXT, 0L), IsFalse());
}

#if defined(_MSC_VER) &&  defined(_DEBUG)

TEST_F(EditWndTest, DISABLED_OnCommand101)
{
	HWND hWndEdit = nullptr;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_COMMAND, 0L, 0L), IsFalse());
}

#endif // if defined(_MSC_VER) &&  defined(_DEBUG)

TEST_F(EditWndTest, OnCommand102)
{
	MockCDlgOpenFile::gm_Files.clear();

	auto& cDlgOpenFile = static_cast<MockCDlgOpenFile&>(*CDlgOpenFile::getInstance());
	EXPECT_CALL(cDlgOpenFile, DoModalOpenDlg(_, _, _))
		.Times(1)
		.WillOnce(testing::DoDefault());

	GetDllShareData().m_sHistory.m_nOPENFOLDERArrNum = 1;

	// ファイルを開くダイアログを表示する
	HWND hWndEdit = nullptr;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_COMMAND, MAKEWPARAM(IDM_SELOPENFOLDER, 0), 0L), IsFalse());

	GetDllShareData().m_sHistory.m_nOPENFOLDERArrNum = 0;
}

TEST_F(EditWndTest, DISABLED_OnSysCommand101)	// パラメーター不正の考慮がないので呼べない
{
	HWND hWndEdit = nullptr;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_SYSCOMMAND, SC_CLOSE, 0L), IsFalse());
}

TEST_F(EditWndTest, OnTimer101)
{
	HWND hWndEdit = nullptr;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_TIMER, 2 /* IDT_EDITCHECK */, 0L), IsFalse());
}

TEST_F(EditWndTest, OnExitMenuLoop101)
{
	HWND hWndEdit = nullptr;
	pcEditWnd->DispatchEvent(hWndEdit, WM_EXITMENULOOP, 0L, 0L);
}

TEST_F(EditWndTest, DISABLED_OnInitMenuPopup101)	// パラメーター不正の考慮がないので呼べない
{
	HWND hWndEdit = nullptr;
	pcEditWnd->DispatchEvent(hWndEdit, WM_INITMENUPOPUP, 0L, 0L);
}

TEST_F(EditWndTest, DISABLED_OnDropFiles101)	// パラメーター不正の考慮がないので呼べない
{
	HWND hWndEdit = nullptr;
	pcEditWnd->DispatchEvent(hWndEdit, WM_DROPFILES, 0L, 0L);
}

TEST_F(EditWndTest, DISABLED_OnThemeChanged101)	// パラメーター不正の考慮がないので呼べない
{
	HWND hWndEdit = nullptr;
	pcEditWnd->DispatchEvent(hWndEdit, WM_THEMECHANGED, 0L, 0L);
}

TEST_F(EditWndTest, OnMenuChar101)
{
	HWND hWndEdit = nullptr;
	pcEditWnd->DispatchEvent(hWndEdit, WM_MENUCHAR, 0L, 0L);
}

#if defined(_MSC_VER) &&  defined(_DEBUG)

TEST_F(EditWndTest, OnCopy101)
{
	HWND hWndEdit = nullptr;
	pcEditWnd->DispatchEvent(hWndEdit, WM_COPY, 0L, 0L);
}

TEST_F(EditWndTest, OnPaste101)
{
	HWND hWndEdit = nullptr;
	pcEditWnd->DispatchEvent(hWndEdit, WM_PASTE, 0L, 0L);
}

#endif // if defined(_MSC_VER) &&  defined(_DEBUG)

TEST_F(EditWndTest, OnMyWmGetLineData101)
{
	HWND hWndEdit = nullptr;
	const auto invalidLine = GetDocument()->m_cDocLineMgr.GetLineCount() + CLogicInt(1);
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, MYWM_GETLINEDATA, WPARAM(invalidLine), 0L), Eq(-2));
}

TEST_F(EditWndTest, OnMyWmUipiCheck101)
{
	HWND hWndEdit = nullptr;
	constexpr LPARAM expected = 0x1234;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, MYWM_UIPI_CHECK, 0L, expected), Eq(expected));
}

TEST_F(EditWndTest, OnMyWmAllowActivate101)
{
	HWND hWndEdit = nullptr;
	pcEditWnd->DispatchEvent(hWndEdit, MYWM_ALLOWACTIVATE, 0L, 0L);
}

TEST_F(EditWndTest, DISABLED_OnMyWmChangeSetting101)	// 副作用が大きいので無効
{
	HWND hWndEdit = nullptr;
	pcEditWnd->DispatchEvent(hWndEdit, MYWM_CHANGESETTING, 0L, int(PM_CHANGESETTING_ALL));
}

TEST_F(EditWndTest, OnMyWmSaveEditState101)
{
	HWND hWndEdit = nullptr;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, MYWM_SAVEEDITSTATE, 0L, 0L), IsFalse());
}

TEST_F(EditWndTest, OnMyWmSetActivePane101)
{
	HWND hWndEdit = nullptr;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, MYWM_SETACTIVEPANE, WPARAM(-1), 0L), IsFalse());
}

TEST_F(EditWndTest, OnMyWmSetCaretPos101)
{
	HWND hWndEdit = nullptr;
	GetDllShareData().m_sWorkBuffer.m_LogicPoint = CLogicPoint(0, 0);
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, MYWM_SETCARETPOS, 0L, 0L), IsFalse());
}

TEST_F(EditWndTest, OnMyWmGetCaretPos101)
{
	HWND hWndEdit = nullptr;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, MYWM_GETCARETPOS, 0L, 0L), IsFalse());
}

TEST_F(EditWndTest, OnMyWmGetLineCount101)
{
	HWND hWndEdit = nullptr;
	const auto lineCount = pcEditWnd->DispatchEvent(hWndEdit, MYWM_GETLINECOUNT, 0L, 0L);
	EXPECT_THAT(lineCount, Ge(0));
}

#if defined(_MSC_VER) &&  defined(_DEBUG)

TEST_F(EditWndTest, OnMyWmAddStringLenW101)
{
	HWND hWndEdit = nullptr;
	auto* pWork = GetDllShareData().m_sWorkBuffer.GetWorkBuffer<EDIT_CHAR>();
	::wcscpy_s(pWork, 4, L"abc");
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, MYWM_ADDSTRINGLEN_W, 3, 0L), IsFalse());
}

#endif // if defined(_MSC_VER) &&  defined(_DEBUG)

TEST_F(EditWndTest, OnLButtonDown101)
{
	HWND hWndEdit = nullptr;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(10, 20)), IsFalse());
}

TEST_F(EditWndTest, OnMouseWheel101)
{
	HWND hWndEdit = nullptr;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_MOUSEWHEEL, MAKEWPARAM(0, WHEEL_DELTA), MAKELPARAM(10, 20)), IsFalse());
}

TEST_F(EditWndTest, OnLButtonUp101)
{
	HWND hWndEdit = nullptr;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_LBUTTONUP, 0L, MAKELPARAM(10, 20)), IsFalse());
}

TEST_F(EditWndTest, OnNCLButtonDown101)
{
	HWND hWndEdit = nullptr;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_NCLBUTTONDOWN, 0L, MAKELPARAM(10, 20)), IsFalse());
}

TEST_F(EditWndTest, OnNCLButtonUp101)
{
	HWND hWndEdit = nullptr;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_NCLBUTTONUP, 0L, MAKELPARAM(10, 20)), IsFalse());
}

TEST_F(EditWndTest, OnLButtonDblClk101)
{
	HWND hWndEdit = nullptr;
	EXPECT_THAT(pcEditWnd->DispatchEvent(hWndEdit, WM_LBUTTONDBLCLK, 0L, MAKELPARAM(10, 20)), IsFalse());
}

#if defined(_MSC_VER) &&  defined(_DEBUG)

/*!
 * コマンド：コマンドプロンプトを開く
 */
TEST_F(EditWndTest, Command_OPEN_COMMAND_PROMPT101)
{
	const auto& targetPath = dummyPath;

	EXPECT_THAT(ExecMacroCommand(std::format(L"FileOpen('{}', 99, 0, '無題1')", targetPath.native())), IsTrue());

	Shell32::setInstance<MockShell32>();
	auto pShell32 = (MockShell32*)Shell32::getInstance();
	EXPECT_CALL(*pShell32, ShellExecuteExW(_))
		.Times(1)
		.WillOnce(Return(FALSE));

	HWND hWnd = nullptr;
	FORWARD_WM_COMMAND(hWnd, F_OPEN_COMMAND_PROMPT, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);
}

/*!
 * Diffコンポーネントのテスト
 *
 * 将来的に要らなくなるはず。
 */
TEST_F(EditWndTest, DiffComponents001)
{
	auto pcDocLineMgr = &pcEditDoc->m_cDocLineMgr;

	auto pcDiffLineSetter = std::make_unique<CDiffLineSetter>(pcDocLineMgr->GetDocLineTop());
	ASSERT_THAT(pcDiffLineSetter, NotNull());

	auto pcDiffLineMgr = std::make_unique<CDiffLineMgr>(pcDocLineMgr);
	ASSERT_THAT(pcDiffLineMgr, NotNull());
}

/*!
 * 上書き保存時バックアップのテスト
 */
TEST_F(EditWndTest, FileSaveWithBackupAgent001)
{
	const auto& targetPath = backupAgentTargetPath;

	auto& sBackup = GetDllShareData().m_Common.m_sBackup;
	const CommonSetting_Backup backupOld = sBackup;

	sBackup.m_bBackUp = true;
	sBackup.m_bBackUpDialog = false;
	sBackup.SetBackupType(1);
	sBackup.m_bBackUpFolder = false;
	sBackup.m_bBackUpPathAdvanced = false;
	sBackup.m_bBackUpDustBox = false;

	// ファイルを開く
	EXPECT_THAT(ExecMacroCommand(std::format(L"FileOpen('{}', 99, 0, '無題1')", targetPath.native())), IsTrue());

	// 編集済みにする
	pcEditDoc->m_cDocEditor.m_bIsDocModified = true;

	// 上書き保存してバックアップを作らせる
	EXPECT_THAT(ExecMacroCommand(L"FileSave()"), IsTrue());

	// バックアップが作られたことを確認する
	EXPECT_THAT(fexist(backupPath), IsTrue());

	auto backupAgent = std::make_unique<CBackupAgent>();

	sBackup.m_bBackUpFolder = true;
	sBackup.m_szBackUpFolder = L"%COMDESKTOP%";

	SFilePath newPath;
	backupAgent->FormatBackUpPath(newPath, std::size(newPath), backupPath.c_str());
	EXPECT_THAT(newPath, StrEq(LR"(C:\Users\Public\Desktop\backup-agent-target.bak)"));

	backupAgent = nullptr;

	sBackup = backupOld;
}

/*!
 * CEditDoc::GetDataObjectのテスト
 */
TEST_F(EditWndTest, GetDocDataObject001)
{
	const auto& targetPath = dummyPath;

	cxx::com_pointer<IDataObject> pDataObject;
	EXPECT_HRESULT_SUCCEEDED(GetDocument()->GetDataObject(&pDataObject));
	EXPECT_THAT(pDataObject, IsNull());

	EXPECT_THAT(ExecMacroCommand(std::format(L"FileOpen('{}', 99, 0, '無題1')", targetPath.native())), IsTrue());

	EXPECT_HRESULT_SUCCEEDED(GetDocument()->GetDataObject(&pDataObject));
	EXPECT_THAT(pDataObject, NotNull());

	const auto cfFormat = static_cast<CLIPFORMAT>(::RegisterClipboardFormatW(CFSTR_FILENAMEW));

	FORMATETC format = { cfFormat, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM medium = {};
	EXPECT_THAT(pDataObject->GetData(&format, &medium), S_OK);
	EXPECT_THAT(medium.tymed, TYMED_HGLOBAL);

	ASSERT_THAT(medium.hGlobal, NotNull());

	cxx::GlobalWString memory{ medium.hGlobal };
	EXPECT_THAT(memory.wstring(), StrEq(targetPath.native()));

	// 独自仕様のチェック
	format.cfFormat = CF_UNICODETEXT;
	
	EXPECT_THAT(pDataObject->GetData(&format, &medium), S_OK);
	EXPECT_THAT(medium.tymed, TYMED_HGLOBAL);

	ASSERT_THAT(medium.hGlobal, NotNull());
	memory = medium.hGlobal;

	EXPECT_THAT(memory.wstring(), StrEq(targetPath.native()));
}

TEST_F(EditWndTest, HelpContents101)
{
	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_COMMAND, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_HELP_CONTENTS, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);
}

TEST_F(EditWndTest, HelpSearch101)
{
	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_KEY, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_HELP_SEARCH, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);
}

/*!
 * バージョン情報ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgAbout001)
{
	// 表示されたモーダルダイアログを閉じる
	dialog::ModalDialogCloser closer(L"バージョン情報", [this] (HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		std::vector<INPUT> inputs{};
		RECT rc{};

		EXPECT_THAT(::GetWindowRect(::GetDlgItem(hWndDlg, IDC_STATIC_URL_UR), &rc), IsTrue());
		inputs.emplace_back(MakeMouseInputMove((rc.left + rc.right) / 2, (rc.top + rc.bottom) / 2));

		EXPECT_THAT(::GetWindowRect(::GetDlgItem(hWndDlg, IDC_STATIC_URL_GIT), &rc), IsTrue());
		inputs.emplace_back(MakeMouseInputMove((rc.left + rc.right) / 2, (rc.top + rc.bottom) / 2));

		EXPECT_THAT(SendInput(inputs), Eq(std::size(inputs)));

		SendDlgCommand(hWndDlg, IDOK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り

	using target = CDlgAbout;
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_ABOUT, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);
}

/*!
 * バージョン情報ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgAbout101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(L"バージョン情報");

	using target = CDlgAbout;
	EXPECT_THAT(ExecMacroCommand(L"About()"), IsTrue());
}

#endif // if defined(_MSC_VER) &&  defined(_DEBUG)

/*!
 * キャンセルダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgCancel001)
{
	// 表示されたモーダルダイアログを閉じる
	dialog::ModalDialogCloser closer(L"Grep実行中", [](HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		// ボタンID以外でOnCommandを空振りさせる
		SendDlgCommand(hWndDlg, IDC_STATIC_CURPATH);

		SendDlgCommand(hWndDlg, IDCANCEL);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り

	CDlgCancel cDlgCancel;
	const auto hWnd = pcEditWnd->GetHwnd();
	cDlgCancel.DoModeless(unusedArg1, hWnd, IDD_GREPRUNNING);

	// キューに溜まるメッセージを処理する
	RunMessageLoop();
}

/*!
 * ファイル比較ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgCompare001)
{
	// 表示されたモーダルダイアログをOKボタンで閉じる
	dialog::ModalDialogCloser closer(L"ファイル内容比較", [this] (HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndDlg, IDC_BUTTON_HELP);

		CMyRect rc{};
		::GetClientRect(hWndDlg, &rc);
		FORWARD_WM_SIZE(hWndDlg, SIZE_RESTORED, rc.right, rc.bottom, ::SendMessageW);

		SendDlgCommand(hWndDlg, IDOK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	CDlgCompare cDlgCompare;
	const auto hWnd = pcEditWnd->GetHwnd();
	const LPARAM unusedArg2 = 0;
	HWND hWndCompareWnd = nullptr;
	cDlgCompare.DoModal(unusedArg1, hWnd, unusedArg2, L"", &hWndCompareWnd);
}

/*!
 * ファイル比較ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgCompare101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(L"ファイル内容比較");

	CDlgCompare cDlgCompare;
	const auto hWnd = pcEditWnd->GetHwnd();
	const LPARAM unusedArg2 = 0;
	HWND hWndCompareWnd = nullptr;
	cDlgCompare.DoModal(unusedArg1, hWnd, unusedArg2, L"", &hWndCompareWnd);
}

#if defined(_MSC_VER) &&  defined(_DEBUG)

/*!
 * コントロールコード入力ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgCtrlCode001)
{
	// 表示されたモーダルダイアログをOKボタンで閉じる
	dialog::ModalDialogCloser closer(L"コントロールコード", [this] (HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndDlg, IDC_BUTTON_HELP);

		SendDlgCommand(hWndDlg, IDOK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, HLP000255)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	using target = CDlgCtrlCode;
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_CTRL_CODE_DIALOG, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);
}

/*!
 * コントロールコード入力ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgCtrlCode101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(L"コントロールコード");

	using target = CDlgCtrlCode;
	EXPECT_THAT(ExecMacroCommand(L"CtrlCodeDialog()"), IsTrue());
}

/*!
 * Diff差分ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgDiff001)
{
	// 表示されたモーダルダイアログをOKボタンで閉じる
	dialog::ModalDialogCloser closer(L"DIFF差分表示", [this] (HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndDlg, IDC_BUTTON_HELP);

		SendDlgCommand(hWndDlg, IDC_BUTTON_DIFF_DST);

		apiwrap::CheckDlgButton(hWndDlg, IDC_RADIO_DIFF_DST2, true);
		SendDlgCommand(hWndDlg, IDC_RADIO_DIFF_DST2);

		apiwrap::CheckDlgButton(hWndDlg, IDC_RADIO_DIFF_DST1, true);
		SendDlgCommand(hWndDlg, IDC_RADIO_DIFF_DST1);

		apiwrap::CheckDlgButton(hWndDlg, IDC_RADIO_DIFF_FILE2, true);
		SendDlgCommand(hWndDlg, IDC_RADIO_DIFF_FILE2);

		apiwrap::CheckDlgButton(hWndDlg, IDC_RADIO_DIFF_FILE1, true);
		SendDlgCommand(hWndDlg, IDC_RADIO_DIFF_FILE1);

		CMyRect rc{};
		::GetClientRect(hWndDlg, &rc);
		FORWARD_WM_SIZE(hWndDlg, SIZE_RESTORED, rc.right, rc.bottom, ::SendMessageW);

		SendDlgCommand(hWndDlg, IDOK);
	});

	auto& cDlgOpenFile = static_cast<MockCDlgOpenFile&>(*CDlgOpenFile::getInstance());
	EXPECT_CALL(cDlgOpenFile, DoModal_GetOpenFileName(_, _))
		.Times(1)
		.WillOnce(testing::DoDefault());

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	using target = CDlgDiff;
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_DIFF_DIALOG, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);
}

/*!
 * Diff差分ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgDiff101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(L"DIFF差分表示");

	using target = CDlgDiff;
	EXPECT_THAT(ExecMacroCommand(L"DiffDialog()"), IsTrue());
}

/*!
 * 外部コマンド実行ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgExec001)
{
	// 表示されたモーダルダイアログを閉じる
	dialog::ModalDialogCloser closer(L"ファイル名を指定して実行", [] (HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndDlg, IDC_BUTTON_HELP);

		// プログラムを起動させに行く。
		::SetDlgItemTextW(hWndDlg, IDC_COMBO_m_szCommand, L"ctags.exe --version");
		::CheckDlgButtonBool(hWndDlg, IDC_CHECK_GETSTDOUT, true);
		::CheckDlgButtonBool(hWndDlg, IDC_RADIO_EDITWINDOW, true);
		::CheckDlgButtonBool(hWndDlg, IDC_CHECK_CUR_DIR, true);
		::SetDlgItemTextW(hWndDlg, IDC_COMBO_CUR_DIR, GetExeFileName().parent_path().c_str());

		SendDlgCommand(hWndDlg, IDOK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	using target = CDlgExec;
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_EXECMD_DIALOG, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);
}

/*!
 * 外部コマンド実行ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgExec101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(L"ファイル名を指定して実行");

	using target = CDlgExec;
	EXPECT_THAT(ExecMacroCommand(L"ExecCommandDialog()"), IsTrue());
}

/*!
 * 履歴とお気に入りの管理ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgFavorite001)
{
	// 表示されたモーダルダイアログをOKボタンで閉じる
	dialog::ModalDialogCloser closer(L"履歴とお気に入りの管理", [this] (HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndDlg, IDC_BUTTON_HELP);

		// 初期表示の履歴項目は「編集不可」なので空振る
		SendDlgCommand(hWndDlg, IDC_BUTTON_ADD_FAVORITE);

		SendDlgCommand(hWndDlg, IDC_BUTTON_DELETE_NOFAVORATE);
		SendDlgCommand(hWndDlg, IDC_BUTTON_DELETE_NOTFOUND);
		SendDlgCommand(hWndDlg, IDC_BUTTON_DELETE_SELECTED);

		SendDlgCommand(hWndDlg, IDC_BUTTON_CLEAR);

		const auto hWndTab = ::GetDlgItem(hWndDlg, IDC_TAB_FAVORITE);

		NMHDR nmhdr{};
		nmhdr.hwndFrom = hWndTab;
		nmhdr.idFrom = IDC_TAB_FAVORITE;
		nmhdr.code = TCN_SELCHANGE;
		FORWARD_WM_NOTIFY(hWndDlg, IDC_TAB_FAVORITE, &nmhdr,  ::SendMessageW);

		CMyRect rc{};
		::GetClientRect(hWndDlg, &rc);
		FORWARD_WM_SIZE(hWndDlg, SIZE_RESTORED, rc.right, rc.bottom, ::SendMessageW);

		SendDlgCommand(hWndDlg, IDOK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, _, _, _, _))
		.Times(3)
		.WillOnce(Return(IDOK))		// 最近使ったファイルの履歴のお気に入り以外を削除します。\nよろしいですか？
		.WillOnce(Return(IDOK))		// 最近使ったファイルの存在しないパスを削除します。\nよろしいですか？
		.WillOnce(Return(IDOK));	// 最近使ったファイルの履歴を削除します。\nよろしいですか？

	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	using target = CDlgFavorite;
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_FAVORITE, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);
}

/*!
 * 履歴とお気に入りの管理ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgFavorite101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(L"履歴とお気に入りの管理");

	using target = CDlgFavorite;
	EXPECT_THAT(ExecMacroCommand(L"OptionFavorite()"), IsTrue());
}

#endif // if defined(_MSC_VER) &&  defined(_DEBUG)

/*!
 * ファイルツリーダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgFileTree001)
{
	// 表示されたモードレスダイアログを閉じる
	dialog::ModalDialogCloser closer1(L"x", [](HWND hWndDlg) {
		SendDlgCommand(hWndDlg, IDC_BUTTON_SETTING);
		SendDlgCommand(hWndDlg, IDCANCEL);
	});

	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer2(L"ファイルツリー設定", [](HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndDlg, IDC_BUTTON_HELP);

		SendDlgCommand(hWndDlg, IDC_BUTTON_EXPORT);
		// 保存先ファイル名の入力はモックで実現する

		SendDlgCommand(hWndDlg, IDC_BUTTON_IMPORT);
		// 開くファイル名の入力はモックで実現する

		SendDlgCommand(hWndDlg, IDOK);

		::EndDialog(hWndDlg, IDOK);
	});

	const auto& exportPath = fileTreeExportPath;
	MockCDlgOpenFile::gm_Files.emplace_back(exportPath.native());

	auto& cDlgOpenFile = static_cast<MockCDlgOpenFile&>(*CDlgOpenFile::getInstance());
	EXPECT_CALL(cDlgOpenFile, DoModal_GetSaveFileName(_))
		.Times(1)
		.WillOnce(testing::DoDefault());
	EXPECT_CALL(cDlgOpenFile, DoModal_GetOpenFileName(_, _))
		.Times(1)
		.WillOnce(testing::DoDefault());

	auto pUser32 = (MockUser32*)User32::getInstance();
	const auto exportMsg = std::wstring{ LS(STR_IMPEXP_OK_EXPORT) } + exportPath.native();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(exportMsg), _, _, _))
		.WillRepeatedly(Return(IDOK));

	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	using target = CDlgFileTree;
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_OUTLINE, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);

	// キューに溜まるメッセージを処理する
	RunMessageLoop();
}

/*!
 * ファイルツリーダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgFileTree101)
{
	// 表示されたモードレスダイアログを閉じる
	dialog::ModalDialogCloser closer1(L"x", [](HWND hWndDlg) {
		SendDlgCommand(hWndDlg, IDC_BUTTON_SETTING);
		SendDlgCommand(hWndDlg, IDCANCEL);
	});

	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer2(L"ファイルツリー設定", [](HWND hWndDlg) {
		::EndDialog(hWndDlg, IDCANCEL);
	});

	using target = CDlgFileTree;
	EXPECT_THAT(ExecMacroCommand(L"Outline(0)"), IsTrue());

	// キューに溜まるメッセージを処理する
	RunMessageLoop();
}

/*!
 * 更新通知及び確認ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgFileUpdateQuery101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(L"ファイルが更新されました", [this] (HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		SendDlgCommand(hWndDlg, IDCANCEL);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り

	CDlgFileUpdateQuery cDlgFileUpdateQuery(L"", false);
	const auto hWnd = pcEditWnd->GetHwnd();
	cDlgFileUpdateQuery.DoModal(unusedArg1, hWnd, IDD_FILEUPDATEQUERY, 0 );
}

#if defined(_MSC_VER) &&  defined(_DEBUG)

/*!
 * 検索ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgFind001)
{
	// 検索条件
	CSearchKeywordManager().AddToSearchKeyArr(LR"(localhost)");

	GetDllShareData().m_Common.m_sSearch.m_bAutoCloseDlgFind = false;

	// 表示されたモーダルダイアログを閉じる
	dialog::ModalDialogCloser closer(L"検索", [](HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndDlg, IDC_BUTTON_HELP);

		// ボタンID以外でOnCommandを空振りさせる
		SendDlgCommand(hWndDlg, IDC_STATIC_CURPATH);

		apiwrap::CheckDlgButton(hWndDlg, IDC_CHK_REGULAREXP, false);
		SendDlgCommand(hWndDlg, IDC_CHK_REGULAREXP);
		apiwrap::CheckDlgButton(hWndDlg, IDC_CHK_REGULAREXP, true);
		SendDlgCommand(hWndDlg, IDC_CHK_REGULAREXP);

		// 検索系ボタンの押下(空振りさせる)
		SendDlgCommand(hWndDlg, IDC_BUTTON_SEARCHNEXT);
		SendDlgCommand(hWndDlg, IDC_BUTTON_SEARCHPREV);
		SendDlgCommand(hWndDlg, IDC_BUTTON_SETMARK);

		SendDlgCommand(hWndDlg, IDC_COMBO_TEXT, CBN_DROPDOWN);

		// 検索条件をセット
		apiwrap::SetDlgItemTextW(hWndDlg, IDC_COMBO_TEXT, L"test");

		// 前方検索
		SendDlgCommand(hWndDlg, IDC_BUTTON_SEARCHNEXT);

		// 後方検索
		SendDlgCommand(hWndDlg, IDC_BUTTON_SEARCHPREV);

		// 該当行をマーク
		SendDlgCommand(hWndDlg, IDC_BUTTON_SETMARK);

		SendDlgCommand(hWndDlg, IDCANCEL);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, _, _, _, _))
		.Times(4)
		.WillOnce(Return(IDOK))		// 検索条件を指定してください。
		.WillOnce(Return(IDOK))		// 検索条件を指定してください。
		.WillOnce(Return(IDOK))		// 前方(↓) に文字列 'test' が１つも見つかりません。
		.WillOnce(Return(IDOK));	// 後方(↓) に文字列 'test' が１つも見つかりません。

	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	using target = CDlgFind;
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_SEARCH_DIALOG, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);

	// キューに溜まるメッセージを処理する
	RunMessageLoop();

	// 設定を元に戻す
	GetDllShareData().m_sSearchKeywords.m_aSearchKeys.clear();
	GetDllShareData().m_Common.m_sSearch.m_bAutoCloseDlgFind = true;
}

/*!
 * 検索ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgFind101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(L"検索");

	using target = CDlgFind;
	EXPECT_THAT(ExecMacroCommand(L"SearchDialog()"), IsTrue());

	// キューに溜まるメッセージを処理する
	RunMessageLoop();
}

/*!
 * アウトライン解析ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgFuncList101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer1(L"x", [] (HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));
		

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndDlg, IDC_BUTTON_HELP);

		SendDlgCommand(hWndDlg, IDC_BUTTON_MENU);

		SendDlgCommand(hWndDlg, IDC_BUTTON_COPY);

		SendDlgCommand(hWndDlg, IDC_BUTTON_WINSIZE);

		SendDlgCommand(hWndDlg, IDC_CHECK_bAutoCloseDlgFuncList);

		SendDlgCommand(hWndDlg, IDC_BUTTON_SETTING);

		SendDlgCommand(hWndDlg, IDC_COMBO_nSortType, CBN_SELENDOK);

		// タイマーを空振りさせる
		FORWARD_WM_TIMER(hWndDlg, 1, ::SendMessageW);
		FORWARD_WM_TIMER(hWndDlg, 2, ::SendMessageW);
		FORWARD_WM_TIMER(hWndDlg, 3, ::SendMessageW);
		FORWARD_WM_TIMER(hWndDlg, 4, ::SendMessageW);

		CMyRect rc{};
		::GetClientRect(hWndDlg, &rc);
		FORWARD_WM_SIZE(hWndDlg, SIZE_RESTORED, rc.right, rc.bottom, ::SendMessageW);

		// OKボタンを押下するとOnJump経由でGetDataが呼ばれる
		SendDlgCommand(hWndDlg, IDOK);

		SendDlgCommand(hWndDlg, IDCANCEL);
	});

	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer2(L"ファイルツリー設定");

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, TrackPopupMenu(_, _, _, _, _, _, _))
		.Times(1)
		.WillOnce(Return(450));	// 更新

	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	using target = CDlgFuncList;
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_OUTLINE, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);

	// キューに溜まるメッセージを処理する
	RunMessageLoop();
}

/*!
 * Grepダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgGrep001)
{
	// 表示されたモーダルダイアログを閉じる
	dialog::ModalDialogCloser closer(L"Grep", [] (HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndDlg, IDC_BUTTON_HELP);

		apiwrap::CheckDlgButton(hWndDlg, IDC_CHK_REGULAREXP, false);
		SendDlgCommand(hWndDlg, IDC_CHK_REGULAREXP);
		apiwrap::CheckDlgButton(hWndDlg, IDC_CHK_REGULAREXP, true);
		SendDlgCommand(hWndDlg, IDC_CHK_REGULAREXP);

		apiwrap::CheckDlgButton(hWndDlg, IDC_CHECK_CP, true);
		SendDlgCommand(hWndDlg, IDC_CHECK_CP);

		apiwrap::CheckDlgButton(hWndDlg, IDC_CHK_FROMTHISTEXT, true);
		SendDlgCommand(hWndDlg, IDC_CHK_FROMTHISTEXT);
		apiwrap::CheckDlgButton(hWndDlg, IDC_CHK_FROMTHISTEXT, false);
		SendDlgCommand(hWndDlg, IDC_CHK_FROMTHISTEXT);

		SendDlgCommand(hWndDlg, IDC_BUTTON_CURRENTFOLDER);
		SendDlgCommand(hWndDlg, IDC_BUTTON_FOLDER_UP);

		// OKボタンを押下するとGrepが動いてしまうのでコメントアウトする。
		// （Grep置換と異なり、動かしても危険はないが余分にテスト時間がかかってしまう）
		//SendDlgCommand(hWndDlg, IDOK);

		SendDlgCommand(hWndDlg, IDCANCEL);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	using target = CDlgGrep;
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_GREP_DIALOG, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);
}

/*!
 * Grepダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgGrep101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(L"Grep");

	using target = CDlgGrep;
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_GREP_DIALOG, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);
}

/*!
 * Grep置換ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgGrepReplace001)
{
	// 置換文字列
	CSearchKeywordManager().AddToReplaceKeyArr( LR"(royalhost)" );

	// 表示されたモーダルダイアログを閉じる
	dialog::ModalDialogCloser closer(L"Grep置換", [] (HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndDlg, IDC_BUTTON_HELP);

		SendDlgCommand(hWndDlg, IDC_COMBO_TEXT2, CBN_DROPDOWN);

		// Grep置換はOKボタンを押下すると危険なのでコメントアウトする。
		//SendDlgCommand(hWndDlg, IDOK);

		SendDlgCommand(hWndDlg, IDCANCEL);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	using target = CDlgGrepReplace;
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_GREP_REPLACE_DLG, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);

	// 設定を元に戻す
	GetDllShareData().m_sSearchKeywords.m_aReplaceKeys.clear();
}

/*!
 * Grep置換ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgGrepReplace101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(L"Grep置換");

	using target = CDlgGrepReplace;
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_GREP_REPLACE_DLG, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);
}

#endif // if defined(_MSC_VER) &&  defined(_DEBUG)

/*!
 * 1行入力ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgInputBox001)
{
	// モックを解除してダイアログをテストする
	CDlgInput1::resetInstance();

	auto& cDlgInput1 = *CDlgInput1::getInstance();

	// 表示されたモーダルダイアログを閉じる
	dialog::ModalDialogCloser closer(L"汎用入力ダイアログ", [&cDlgInput1] (HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		// 文字数超過していたら切り詰める
		::SetDlgItemTextW(hWndDlg, IDC_EDIT_INPUT1, L"test");
		SendDlgCommand(hWndDlg, IDOK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り

	std::wstring buffer{ L"TES" };

	EXPECT_THAT(cDlgInput1.DoModal(unusedArg1, pcEditWnd->GetHwnd(), L"title", L"message", std::size(buffer) + 1, std::data(buffer)), IsTrue());

	EXPECT_THAT(buffer, StrEq(L"tes"));
}

/*!
 * 1行入力ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgInputBox002)
{
	// モックを解除してダイアログをテストする
	CDlgInput1::resetInstance();

	auto& cDlgInput1 = *CDlgInput1::getInstance();

	// 表示されたモーダルダイアログを閉じる
	dialog::ModalDialogCloser closer(L"汎用入力ダイアログ", [&cDlgInput1] (HWND hWndDlg) {
		// 文字数超過の場合は取り込まず、ダイアログは閉じない
		::SetDlgItemTextW(hWndDlg, IDC_EDIT_INPUT1, L"test");
		SendDlgCommand(hWndDlg, IDOK);

		// 適切な入力なら取り込む
		::SetDlgItemTextW(hWndDlg, IDC_EDIT_INPUT1, L"tes");
		SendDlgCommand(hWndDlg, IDOK);
	});

	std::wstring buffer{ L"TES" };

	EXPECT_THAT(cDlgInput1.DoModal(pcEditWnd->GetHwnd(), L"title", L"message", buffer, [] (HWND, std::wstring_view text, size_t cchBuffer) {
		if (text.empty()) return 0;
		return std::size(text) <= cchBuffer ? 1 : -1;
	}), IsTrue());

	EXPECT_THAT(buffer, StrEq(L"tes"));
}

#if defined(_MSC_VER) &&  defined(_DEBUG)

/*!
 * 1行入力ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgInputBox003)
{
	// マクロ関数を呼ぶためにWSHマクロマネージャーを使う
	mgr = std::unique_ptr<CMacroManagerBase>(CMacroFactory::getInstance()->Create(L"js"));

	auto& cDlgInput1 = static_cast<MockCDlgInput1&>(*CDlgInput1::getInstance());
	EXPECT_CALL(cDlgInput1, DoModal(_, _, _, _, _))
		.Times(1)
		.WillOnce(Return(TRUE));

	EXPECT_THAT(ExecMacroCommand(L"InputBox('test1', 'test2', 0)"), IsTrue());
}

/*!
 * 1行入力ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgInputBox101)
{
	// モックを解除してダイアログをテストする
	CDlgInput1::resetInstance();

	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(L"汎用入力ダイアログ");

	// マクロ関数を呼ぶためにWSHマクロマネージャーを使う
	mgr = std::unique_ptr<CMacroManagerBase>(CMacroFactory::getInstance()->Create(L"js"));

	EXPECT_THAT(ExecMacroCommand(L"InputBox('test1', 'test2', 0)"), IsTrue());
}

/*!
 * 指定行へジャンプダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgJump001)
{
	// 表示されたモーダルダイアログを閉じる
	dialog::ModalDialogCloser closer(L"指定行へジャンプ", [] (HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndDlg, IDC_BUTTON_HELP);

		apiwrap::CheckDlgButton(hWndDlg, IDC_CHECK_PLSQL, true);
		SendDlgCommand(hWndDlg, IDC_CHECK_PLSQL);

		SendDlgCommand(hWndDlg, IDC_COMBO_PLSQLBLOCKS, CBN_SELCHANGE);
		SendDlgCommand(hWndDlg, IDC_CHECK_PLSQL, CBN_SELCHANGE);

		apiwrap::CheckDlgButton(hWndDlg, IDC_CHECK_PLSQL, false);
		SendDlgCommand(hWndDlg, IDC_CHECK_PLSQL);

		SendDlgCommand(hWndDlg, IDC_BUTTON_JUMP);
		SendDlgCommand(hWndDlg, IDCANCEL);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(LS(STR_DLGJUMP1)), _, _, _))
		.Times(1)
		.WillRepeatedly(Return(IDOK));

	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	using target = CDlgJump;
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_JUMP_DIALOG, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);
}

/*!
 * 指定行へジャンプダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgJump101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(L"指定行へジャンプ");

	using target = CDlgJump;
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_JUMP_DIALOG, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);
}

#endif // if defined(_MSC_VER) &&  defined(_DEBUG)

/*!
 * 強調キーワード選択ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgKeywordSelect001)
{
	// 表示されたモーダルダイアログをOKボタンで閉じる
	dialog::ModalDialogCloser closer(L"強調キーワードの設定", [this] (HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		SendDlgCommand(hWndDlg, IDOK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り

	CDlgKeywordSelect cDlgKeywordSelect;
	const auto hWnd = pcEditWnd->GetHwnd();
	std::array<int, 10> nSet{};
	cDlgKeywordSelect.DoModal(unusedArg1, hWnd, nSet.data());
}

/*!
 * 強調キーワード選択ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgKeywordSelect101)
{
	// 表示されたタイプ別設定を閉じる
	dialog::PropertySheetCloser closer([] (HWND hWndDlg, HWND hWndPage) {
		SendDlgCommand(hWndPage, IDC_BUTTON_KEYWORD_SELECT);

		// OKボタンを押下して閉じる
		SendPsmPressButton(hWndDlg, PSBTN_OK);
	});

	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer2(L"強調キーワードの設定");

	using target = CDlgKeywordSelect;
	ShowPropType(ID_PROPTYPE_PAGENUM_COLOR);
}

#if defined(_MSC_VER) &&  defined(_DEBUG)

/*!
 * ファイルを開くダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgOpenFile101)
{
	// Vistaスタイルのファイルダイアログを有効にする
	GetDllShareData().m_Common.m_sEdit.m_bVistaStyleFileDialog = true;

	MockCDlgOpenFile::gm_Files.clear();

	auto& cDlgOpenFile = static_cast<MockCDlgOpenFile&>(*CDlgOpenFile::getInstance());
	EXPECT_CALL(cDlgOpenFile, DoModalOpenDlg(_, _, _))
		.Times(1)
		.WillOnce(testing::DoDefault());

	EXPECT_THAT(ExecMacroCommand(L"FileOpen('', 99, 0, '無題1')"), IsTrue());
}

#endif // if defined(_MSC_VER) &&  defined(_DEBUG)

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

	CDlgPluginOption cDlgPluginOption;
	const auto hWnd = pcEditWnd->GetHwnd();
	const auto propPlugin = std::make_unique<CPropPlugin>();

	// 表示されたモーダルダイアログを閉じる
	dialog::ModalDialogCloser closer(L"プラグインの設定", [] (HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// "%ls プラグインの設定"
		const auto title = strprintf(LS(STR_DLGPLUGINOPT_TITLE), L"Test WSH Plugin");
		EXPECT_THAT(apiwrap::GetWindowTextW(hWndDlg), StrEq(title));

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndDlg, IDC_BUTTON_HELP);

		SendDlgCommand(hWndDlg, IDOK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	cDlgPluginOption.DoModal(unusedArg1, hWnd, propPlugin.get(), pluginId);

	// プラグイン読み込み解除
	CPluginManager::getInstance()->UnloadAllPlugin();

	if (fexist(pluginPath)) {
		std::error_code ec;
		std::filesystem::remove_all(pluginPath, ec);
	}
}

/*!
 * プラグイン設定ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgPluginOption101)
{
	CDlgPluginOption cDlgPluginOption;
	const auto hWnd = pcEditWnd->GetHwnd();
	const auto propPlugin = std::make_unique<CPropPlugin>();

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(LS(STR_DLGPLUGINOPT_LOAD)), _, _, _))
		.Times(1)
		.WillOnce(Return(IDOK));

	// 存在しないプラグイン番号を指定すると、ダイアログは表示されない
	cDlgPluginOption.DoModal(unusedArg1, hWnd, propPlugin.get(), 0);
}

/*!
 * 印刷設定ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgPrintSetting001)
{
	// 表示されたモーダルダイアログをOKボタンで閉じる
	dialog::ModalDialogCloser closer(L"印刷ページ設定", [this] (HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndDlg, IDC_BUTTON_HELP);

		apiwrap::CheckDlgButton(hWndDlg, IDC_CHECK_LINENUMBER, true);
		SendDlgCommand(hWndDlg, IDC_CHECK_LINENUMBER);

		apiwrap::CheckDlgButton(hWndDlg, IDC_RADIO_LANDSCAPE, true);
		SendDlgCommand(hWndDlg, IDC_RADIO_LANDSCAPE);

		apiwrap::CheckDlgButton(hWndDlg, IDC_RADIO_PORTRAIT, true);
		SendDlgCommand(hWndDlg, IDC_RADIO_PORTRAIT);

		SendDlgCommand(hWndDlg, IDC_BUTTON_FONT_FOOT);
		SendDlgCommand(hWndDlg, IDC_BUTTON_FONT_FOOT);

		SendDlgCommand(hWndDlg, IDC_BUTTON_FONT_HEAD);
		SendDlgCommand(hWndDlg, IDC_BUTTON_FONT_HEAD);

		apiwrap::CheckDlgButton(hWndDlg, IDC_CHECK_USE_FONT_FOOT, true);
		SendDlgCommand(hWndDlg, IDC_CHECK_USE_FONT_FOOT);

		apiwrap::CheckDlgButton(hWndDlg, IDC_CHECK_USE_FONT_HEAD, true);
		SendDlgCommand(hWndDlg, IDC_CHECK_USE_FONT_HEAD);

		SendDlgCommand(hWndDlg, IDC_BUTTON_EDITSETTINGNAME);
		SendDlgCommand(hWndDlg, IDC_BUTTON_EDITSETTINGNAME);

		SendDlgCommand(hWndDlg, IDOK);
	});

	auto& cDlgInput1 = static_cast<MockCDlgInput1&>(*CDlgInput1::getInstance());
	EXPECT_CALL(cDlgInput1, DoModal(_, _, _, _, _))
		.Times(2)
		.WillOnce(Return(FALSE))
		.WillOnce(Invoke([] (HWND, LPCWSTR, LPCWSTR, LPWSTR pBuffer, size_t cchBuffer) -> BOOL {
			::wcscpy_s(pBuffer, cchBuffer, L"test");
			return TRUE;
		}));

	Comdlg32::setInstance<MockComdlg32>();

	auto pComdlg32 = static_cast<MockComdlg32*>(Comdlg32::getInstance());
	EXPECT_CALL(*pComdlg32, ChooseFontW(_))
		.Times(4)
		.WillOnce(Return(FALSE))
		.WillOnce(Invoke([] (LPCHOOSEFONTW pCf) -> BOOL {
			pCf->iPointSize = 9;
			return TRUE;
		}))
		.WillOnce(Return(FALSE))
		.WillOnce(Invoke([] (LPCHOOSEFONTW pCf) -> BOOL {
			pCf->iPointSize = 9;
			return TRUE;
		}));
	EXPECT_CALL(*pComdlg32, CommDlgExtendedError())
		.WillRepeatedly(Return(0));

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	CDlgPrintSetting cDlgPrintSetting;
	const auto hWnd = pcEditWnd->GetHwnd();
	int nCurrentPrintSetting = -1;
	int nLineNumberColumns = 10;
	cDlgPrintSetting.DoModal(unusedArg1, hWnd, &nCurrentPrintSetting, GetDllShareData().m_PrintSettingArr, nLineNumberColumns);
}

/*!
 * 印刷設定ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgPrintSetting101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(L"印刷ページ設定");

	using target = CDlgPrintSetting;
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_PRINT_PAGESETUP, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);
}

#if defined(_MSC_VER) &&  defined(_DEBUG)

/*!
 * プロファイルマネージャーダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgProfileMgr101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(L"プロファイルマネージャ", [this] (HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndDlg, IDC_BUTTON_HELP);

		SendDlgCommand(hWndDlg, IDC_BUTTON_PROF_CREATE);
		SendDlgCommand(hWndDlg, IDC_BUTTON_PROF_CREATE);

		SendDlgCommand(hWndDlg, IDC_BUTTON_PROF_DEFSET);
		SendDlgCommand(hWndDlg, IDC_BUTTON_PROF_DEFCLEAR);

		SendDlgCommand(hWndDlg, IDC_BUTTON_PROF_RENAME);
		SendDlgCommand(hWndDlg, IDC_BUTTON_PROF_RENAME);

		SendDlgCommand(hWndDlg, IDC_BUTTON_PROF_DELETE);

		SendDlgCommand(hWndDlg, IDCANCEL);
	});

	auto& cDlgInput1 = static_cast<MockCDlgInput1&>(*CDlgInput1::getInstance());
	EXPECT_CALL(cDlgInput1, DoModal(_, _, _, _, _))
		.Times(4)
		.WillOnce(Return(FALSE))
		.WillOnce(Invoke([] (HWND, LPCWSTR, LPCWSTR, LPWSTR pBuffer, size_t cchBuffer) -> BOOL {
			::wcscpy_s(pBuffer, cchBuffer, L"test1");
			return TRUE;
		}))
		.WillOnce(Return(FALSE))
		.WillOnce(Invoke([] (HWND, LPCWSTR, LPCWSTR, LPWSTR pBuffer, size_t cchBuffer) -> BOOL {
			::wcscpy_s(pBuffer, cchBuffer, L"test2");
			return TRUE;
		}));

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	using target = CDlgProfileMgr;
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_PROFILEMGR, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);
}

/*!
 * プロパティ情報ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgProperty001)
{
	// 表示されたモーダルダイアログをOKボタンで閉じる
	dialog::ModalDialogCloser closer(L"ファイルのプロパティ", [this] (HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndDlg, IDC_BUTTON_HELP);

		SendDlgCommand(hWndDlg, IDOK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	using target = CDlgProperty;
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_PROPERTY_FILE, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);
}

/*!
 * プロパティ情報ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgProperty101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(L"ファイルのプロパティ");

	using target = CDlgProperty;
	EXPECT_THAT(ExecMacroCommand(L"PropertyFile()"), IsTrue());
}

/*!
 * 置換ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgReplace001)
{
	// 検索条件
	CSearchKeywordManager().AddToSearchKeyArr(LR"(localhost)");

	// 置換文字列
	CSearchKeywordManager().AddToReplaceKeyArr( LR"(royalhost)" );

	// 表示されたモーダルダイアログを閉じる
	dialog::ModalDialogCloser closer(L"置換", [](HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndDlg, IDC_BUTTON_HELP);

		// ボタンID以外でOnCommandを空振りさせる
		SendDlgCommand(hWndDlg, IDC_STATIC_CURPATH);

		apiwrap::CheckDlgButton(hWndDlg, IDC_CHK_REGULAREXP, false);
		SendDlgCommand(hWndDlg, IDC_CHK_REGULAREXP);
		apiwrap::CheckDlgButton(hWndDlg, IDC_CHK_REGULAREXP, true);
		SendDlgCommand(hWndDlg, IDC_CHK_REGULAREXP);

		apiwrap::CheckDlgButton(hWndDlg, IDC_CHK_PASTE, true);
		SendDlgCommand(hWndDlg, IDC_CHK_PASTE);
		apiwrap::CheckDlgButton(hWndDlg, IDC_CHK_PASTE, false);
		SendDlgCommand(hWndDlg, IDC_CHK_PASTE);

		// 検索系ボタンの押下(空振りさせる)
		SendDlgCommand(hWndDlg, IDC_BUTTON_SEARCHNEXT);
		SendDlgCommand(hWndDlg, IDC_BUTTON_SEARCHPREV);
		SendDlgCommand(hWndDlg, IDC_BUTTON_SETMARK);
		SendDlgCommand(hWndDlg, IDC_BUTTON_REPALCE);
		SendDlgCommand(hWndDlg, IDC_BUTTON_REPALCEALL);

		SendDlgCommand(hWndDlg, IDC_COMBO_TEXT, CBN_DROPDOWN);
		SendDlgCommand(hWndDlg, IDC_COMBO_TEXT2, CBN_DROPDOWN);

		// 検索条件をセット
		apiwrap::SetDlgItemTextW(hWndDlg, IDC_COMBO_TEXT, L"test");
		apiwrap::SetDlgItemTextW(hWndDlg, IDC_COMBO_TEXT2, L"text");

		// 前方検索
		SendDlgCommand(hWndDlg, IDC_BUTTON_SEARCHNEXT);

		// 後方検索
		SendDlgCommand(hWndDlg, IDC_BUTTON_SEARCHPREV);

		// 該当行をマーク
		SendDlgCommand(hWndDlg, IDC_BUTTON_SETMARK);

		// 置換実行
		SendDlgCommand(hWndDlg, IDC_BUTTON_REPALCE);

		// 全置換実行
		SendDlgCommand(hWndDlg, IDC_BUTTON_REPALCEALL);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, _, _, _, _))
		.Times(8)
		.WillOnce(Return(IDOK))		// 文字列を指定してください。
		.WillOnce(Return(IDOK))		// 文字列を指定してください。
		.WillOnce(Return(IDOK))		// 文字列を指定してください。
		.WillOnce(Return(IDOK))		// 置換条件を指定してください。
		.WillOnce(Return(IDOK))		// 前方(↓) に文字列 'test' が１つも見つかりません。
		.WillOnce(Return(IDOK))		// 後方(↑) に文字列 'test' が１つも見つかりません。
		.WillOnce(Return(IDOK))		// 前方(↓) に文字列 'test' が１つも見つかりません。
		.WillOnce(Return(IDOK));	// 0箇所を置換しました。

	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	using target = CDlgReplace;
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_REPLACE_DIALOG, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);

	// キューに溜まるメッセージを処理する
	RunMessageLoop();

	// 設定を元に戻す
	GetDllShareData().m_sSearchKeywords.m_aSearchKeys.clear();
	GetDllShareData().m_sSearchKeywords.m_aReplaceKeys.clear();
}

/*!
 * 置換ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgReplace101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(L"置換");

	using target = CDlgReplace;
	EXPECT_THAT(ExecMacroCommand(L"ReplaceDialog()"), IsTrue());

	// キューに溜まるメッセージを処理する
	RunMessageLoop();
}

#endif // if defined(_MSC_VER) &&  defined(_DEBUG)

/*!
 * 文字色／背景色統一ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgSameColor001)
{
	// 表示されたモーダルダイアログをOKボタンで閉じる
	dialog::ModalDialogCloser closer(L"文字色統一", [this] (HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndDlg, IDC_BUTTON_HELP);

		SendDlgCommand(hWndDlg, IDC_BUTTON_SELALL);
		SendDlgCommand( hWndDlg, IDC_BUTTON_SELNOTING );

		SendDlgCommand(hWndDlg, IDOK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	CDlgSameColor cDlgSameColor;
	const auto hWnd = pcEditWnd->GetHwnd();
	const WORD wID = IDC_BUTTON_SAMETEXTCOLOR;
	auto m_nCurrentColorType = 1;
	auto& m_Types = pcEditDoc->m_cDocType.GetDocumentAttributeWrite();
	COLORREF cr = m_Types.m_ColorInfoArr[m_nCurrentColorType].m_sColorAttr.m_cTEXT;
	cDlgSameColor.DoModal(unusedArg1, hWnd, wID, &m_Types, cr);
}

/*!
 * 文字色／背景色統一ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgSameColor101)
{
	CDlgSameColor cDlgSameColor;
	const auto hWnd = pcEditWnd->GetHwnd();
	const WORD wID = 1;
	auto m_nCurrentColorType = 1;
	auto& m_Types = pcEditDoc->m_cDocType.GetDocumentAttributeWrite();
	COLORREF cr = m_Types.m_ColorInfoArr[m_nCurrentColorType].m_sColorAttr.m_cTEXT;
	cDlgSameColor.DoModal(unusedArg1, hWnd, wID, &m_Types, cr);
}

#if defined(_MSC_VER) &&  defined(_DEBUG)

/*!
 * 文字コードセット設定ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgSetCharSet001)
{
	// 表示されたモーダルダイアログをOKボタンで閉じる
	dialog::ModalDialogCloser closer(L"文字コードの指定", [this] (HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		// UTF-8を選択している
		SendDlgCommand(hWndDlg, IDC_COMBO_CHARSET, CBN_SELCHANGE);
		apiwrap::CheckDlgButton( hWndDlg, IDC_CHECK_BOM, true);

		// SJISを選択する
		ComboBox_SetCurSel(::GetDlgItem(hWndDlg, IDC_COMBO_CHARSET), CODE_SJIS);
		SendDlgCommand(hWndDlg, IDC_COMBO_CHARSET, CBN_SELCHANGE);

		// UTF8に戻す
		ComboBox_SetCurSel(::GetDlgItem(hWndDlg, IDC_COMBO_CHARSET), CODE_UTF8);
		SendDlgCommand(hWndDlg, IDC_COMBO_CHARSET, CBN_SELCHANGE);

		apiwrap::CheckDlgButton(hWndDlg, IDC_CHECK_CP, true);
		SendDlgCommand(hWndDlg, IDC_CHECK_CP);

		SendDlgCommand(hWndDlg, IDOK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り

	using target = CDlgSetCharSet;
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_CHG_CHARSET, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);
}

/*!
 * 文字コードセット設定ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgSetCharSet101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(L"文字コードの指定");

	EXPECT_THAT(ExecMacroCommand(L"ChgCharSet(99, 0)"), IsTrue());
}

#endif // if defined(_MSC_VER) &&  defined(_DEBUG)

/*!
 * タグジャンプダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgTagJumpList001)
{
	// 表示されたモーダルダイアログをOKボタンで閉じる
	dialog::ModalDialogCloser closer(L"ダイレクトタグジャンプ一覧", [this] (HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndDlg, IDC_BUTTON_HELP);

		CMyRect rc{};
		::GetClientRect(hWndDlg, &rc);
		FORWARD_WM_SIZE(hWndDlg, SIZE_RESTORED, rc.right, rc.bottom, ::SendMessageW);

		SendDlgCommand(hWndDlg, IDOK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	bool bDirectTagJump = false;
	CDlgTagJumpList cDlgTagJumpList(bDirectTagJump);
	const auto hWnd = pcEditWnd->GetHwnd();
	cDlgTagJumpList.DoModal(unusedArg1, hWnd, 0L);
}

/*!
 * タグジャンプダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgTagJumpList101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(L"ダイレクトタグジャンプ一覧");

	bool bDirectTagJump = false;
	CDlgTagJumpList cDlgTagJumpList(bDirectTagJump);
	const auto hWnd = pcEditWnd->GetHwnd();
	cDlgTagJumpList.DoModal(unusedArg1, hWnd, 0L);
}

#if defined(_MSC_VER) &&  defined(_DEBUG)

/*!
 * タグファイル作成ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgTagsMake001)
{
	// 表示されたモーダルダイアログをOKボタンで閉じる
	dialog::ModalDialogCloser closer1(L"タグファイルの作成", [] (HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndDlg, IDC_BUTTON_HELP);

		apiwrap::SetDlgItemTextW(hWndDlg, IDC_EDIT_TAG_MAKE_FOLDER, GetIniFileName().parent_path().c_str());

		SendDlgCommand(hWndDlg, IDC_BUTTON_TAG_MAKE_REF);

		SendDlgCommand(hWndDlg, IDC_BUTTON_FOLDER_UP);

		apiwrap::SetDlgItemTextW(hWndDlg, IDC_EDIT_TAG_MAKE_FOLDER, GetIniFileName().parent_path().c_str());

		SendDlgCommand(hWndDlg, IDOK);
	});

	// 表示されたフォルダー選択ダイアログをOKボタンで閉じる
	dialog::ModalDialogCloser closer2(L"開く" /* 指定できない */, [](HWND hWndDlg) {
		ASSERT_THAT(apiwrap::GetWindowTextW(hWndDlg), StrEq(L"タグ作成フォルダーの選択"));

		SendDlgCommand(hWndDlg, IDOK); // そのままOK押す
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, _, _, _, _))
		.Times(1)
		.WillOnce(Return(IDOK));

	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	using target = CDlgTagsMake;
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_TAGS_MAKE, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);
}

/*!
 * タグファイル作成ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgTagsMake101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(L"タグファイルの作成");

	using target = CDlgTagsMake;
	EXPECT_THAT(ExecMacroCommand(L"TagMake()"), IsTrue());
}

/*!
 * タイプ別設定インポート確認ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgTypeAscertain001)
{
	// 表示されたモーダルダイアログをOKボタンで閉じる
	dialog::ModalDialogCloser closer1(L"タイプ別設定一覧", [this] (HWND hWndDlg) {
		SendDlgCommand(hWndDlg, IDC_BUTTON_EXPORT);
		// 保存先ファイル名の入力はモックで実現する

		SendDlgCommand(hWndDlg, IDC_BUTTON_IMPORT);
		// 開くファイル名の入力はモックで実現する

		SendDlgCommand(hWndDlg, IDCANCEL);
	});

	// 表示されたモーダルダイアログをOKボタンで閉じる
	dialog::ModalDialogCloser closer2(L"インポート確認", [this] (HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndDlg, IDC_BUTTON_HELP);

		// 空振りさせる
		SendDlgCommand(hWndDlg, IDC_COMBO_COLORS);

		SendDlgCommand(hWndDlg, IDOK);
	});

	const auto& exportPath = typeConfigExportPath;
	MockCDlgOpenFile::gm_Files.emplace_back(exportPath.native());

	auto& cDlgOpenFile = static_cast<MockCDlgOpenFile&>(*CDlgOpenFile::getInstance());
	EXPECT_CALL(cDlgOpenFile, DoModal_GetSaveFileName(_))
		.Times(1)
		.WillOnce(testing::DoDefault());
	EXPECT_CALL(cDlgOpenFile, DoModal_GetOpenFileName(_, _))
		.Times(1)
		.WillOnce(testing::DoDefault());

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, _, _, _, _))
		.WillRepeatedly(Return(IDOK));

	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	using target = CDlgTypeAscertain;
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_TYPE_LIST, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);
}

#endif // if defined(_MSC_VER) &&  defined(_DEBUG)

/*!
 * ファイルタイプ一覧ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgTypeList001)
{
	// 表示されたモーダルダイアログをOKボタンで閉じる
	dialog::ModalDialogCloser closer1(L"タイプ別設定一覧", [this] (HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndDlg, IDC_BUTTON_HELP);

		SendDlgCommand(hWndDlg, IDC_BUTTON_EXPORT);
		// 保存先ファイル名の入力はモックで実現する

		SendDlgCommand(hWndDlg, IDC_BUTTON_IMPORT);
		// 開くファイル名の入力はモックで実現する

		SendDlgCommand(hWndDlg, IDC_BUTTON_UP_TYPE);
		SendDlgCommand(hWndDlg, IDC_BUTTON_DOWN_TYPE);
		SendDlgCommand(hWndDlg, IDC_BUTTON_DEL_TYPE);
		SendDlgCommand(hWndDlg, IDC_BUTTON_ADD_TYPE);
		SendDlgCommand(hWndDlg, IDC_BUTTON_COPY_TYPE);

		SendDlgCommand(hWndDlg, IDOK);
	});

	// 表示されたモーダルダイアログをOKボタンで閉じる
	dialog::ModalDialogCloser closer2(L"インポート確認", IDOK);

	// 表示されたタイプ別設定をOKボタンで閉じる
	dialog::PropertySheetCloser closer3(PSBTN_OK);

	const auto& exportPath = typeConfigExportPath;
	MockCDlgOpenFile::gm_Files.emplace_back(exportPath.native());

	auto& cDlgOpenFile = static_cast<MockCDlgOpenFile&>(*CDlgOpenFile::getInstance());
	EXPECT_CALL(cDlgOpenFile, DoModal_GetSaveFileName(_))
		.Times(1)
		.WillOnce(testing::DoDefault());
	EXPECT_CALL(cDlgOpenFile, DoModal_GetOpenFileName(_, _))
		.Times(1)
		.WillOnce(testing::DoDefault());

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, _, _, _, _))
		.Times(3)
		.WillOnce(Return(IDOK))
		.WillOnce(Return(IDOK))
		.WillOnce(Return(IDYES));

	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	using target = CDlgTypeList;
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_TYPE_LIST, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);
}

#if defined(_MSC_VER) &&  defined(_DEBUG)

/*!
 * ファイルタイプ一覧ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgTypeList101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(L"タイプ別設定一覧");

	using target = CDlgTypeList;
	EXPECT_THAT(ExecMacroCommand(L"TypeList()"), IsTrue());
}

#endif // if defined(_MSC_VER) &&  defined(_DEBUG)

/*!
 * ウインドウサイズダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgWinSize001)
{
	// 表示されたモーダルダイアログをOKボタンで閉じる
	dialog::ModalDialogCloser closer(L"ウィンドウの位置と大きさ", [this] (HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndDlg, IDC_BUTTON_HELP);

		apiwrap::CheckDlgButton(hWndDlg, IDC_RADIO_WINPOS_SET, true);
		SendDlgCommand(hWndDlg, IDC_RADIO_WINPOS_SET);
		apiwrap::CheckDlgButton(hWndDlg, IDC_RADIO_WINPOS_SAVE, true);
		SendDlgCommand(hWndDlg, IDC_RADIO_WINPOS_SAVE);
		apiwrap::CheckDlgButton(hWndDlg, IDC_RADIO_WINPOS_DEF, true);
		SendDlgCommand(hWndDlg, IDC_RADIO_WINPOS_DEF);

		apiwrap::CheckDlgButton(hWndDlg, IDC_RADIO_WINSIZE_SET, true);
		SendDlgCommand(hWndDlg, IDC_RADIO_WINSIZE_SET);
		apiwrap::CheckDlgButton(hWndDlg, IDC_RADIO_WINSIZE_SAVE, true);
		SendDlgCommand(hWndDlg, IDC_RADIO_WINSIZE_SAVE);
		apiwrap::CheckDlgButton(hWndDlg, IDC_RADIO_WINSIZE_DEF, true);
		SendDlgCommand(hWndDlg, IDC_RADIO_WINSIZE_DEF);

		SendDlgCommand(hWndDlg, IDOK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	CDlgWinSize cDlgWinSize;
	const auto hWnd = pcEditWnd->GetHwnd();
	EWinSizeMode eSaveWinSize = WINSIZEMODE_DEF;
	EWinSizeMode eSaveWinPos = WINSIZEMODE_DEF;
	int nWinSizeType = 0;
	RECT rc = {};
	cDlgWinSize.DoModal(unusedArg1, hWnd, eSaveWinSize, eSaveWinPos, nWinSizeType, rc);
}

#if defined(_MSC_VER) &&  defined(_DEBUG)

/*!
 * ウインドウサイズダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgWinSize101)
{
	// 表示された共通設定をOKボタンで閉じる
	dialog::PropertySheetCloser closer1([] (HWND hWndDlg, HWND hWndPage) {
		SendDlgCommand(hWndPage, IDC_BUTTON_WINSIZE);
		SendPsmPressButton(hWndDlg, PSBTN_CANCEL);
	});

	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer2(L"ウィンドウの位置と大きさ");

	using target = CDlgWinSize;
	ShowPropCommon(ID_PROPCOM_PAGENUM_WIN);
}

/*!
 * 補完ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowHokanMgr001)
{
	// 表示されたモーダルダイアログを閉じる
	dialog::ModalDialogCloser closer(std::nullopt /* タイトルなし */, [] (HWND hWndDlg) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndDlg, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndDlg, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndDlg, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		SendDlgCommand(hWndDlg, 0L);

		SendDlgCommand(hWndDlg, IDC_LIST_WORDS, LBN_SELCHANGE);

		CMyRect rc{};
		::GetClientRect(hWndDlg, &rc);
		FORWARD_WM_SIZE(hWndDlg, SIZE_RESTORED, rc.right, rc.bottom, ::SendMessageW);

		SendDlgCommand(hWndDlg, IDOK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り

	using target = CHokanMgr;
	const auto& cHokanMgr = pcEditWnd->m_cHokanMgr;
	const auto hWnd = pcEditWnd->GetHwnd();

	// データなしだとスカる
	EXPECT_THAT(ExecMacroCommand(L"Complete()"), IsTrue());

	// とりあえずデータを入れる
	auto pShareData = GetDllShareDataPtr();
	auto& sWorkBuffer = pShareData->m_sWorkBuffer;
	auto buffer = std::span(sWorkBuffer.GetWorkBuffer<WCHAR>(), sWorkBuffer.GetWorkBufferCount<WCHAR>());

	const auto& text = L"test\n\ntes";
	::wcsncpy_s(std::data(buffer), std::size(buffer), std::data(text), std::size(text));

	pcEditWnd->DispatchEvent(hWnd, MYWM_ADDSTRINGLEN_W, std::size(text), 0L);

	// データありで補完実行
	EXPECT_THAT(ExecMacroCommand(L"Complete()"), IsTrue());

	// 非表示ウィンドウはModalDialogCloserに拾われないので強制的に表示させる
	::ShowWindow(cHokanMgr.GetHwnd(), SW_SHOW);

	// キューに溜まるメッセージを処理する
	RunMessageLoop();

	// 全選択してゴミを削除する
	FORWARD_WM_COMMAND(hWnd, F_SELECTALL, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);
	FORWARD_WM_COMMAND(hWnd, F_DELETE, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon001)
{
	// 表示された共通設定をキャンセルボタンで閉じる
	dialog::PropertySheetCloser closer(PSBTN_CANCEL);

	// シート番号の指定がない場合
	EXPECT_THAT(ExecMacroCommand(L"OptionCommon()"), IsTrue());
}

#endif // if defined(_MSC_VER) &&  defined(_DEBUG)

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon002)
{
	// 表示された共通設定をOKボタンで閉じる
	dialog::PropertySheetCloser closer([] (HWND hWndDlg, HWND hWndPage) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndPage, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndPage, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndPage, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		FORWARD_WM_COMMAND(hWndPage, 0L, nullptr, BN_CLICKED,::SendMessageW);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndPage, IDC_BUTTON_HELP);

		// OKボタンを押下して閉じる
		SendPsmPressButton(hWndDlg, PSBTN_OK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	ShowPropCommon(ID_PROPCOM_PAGENUM_GENERAL);
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon003)
{
	// 表示された共通設定をOKボタンで閉じる
	dialog::PropertySheetCloser closer([] (HWND hWndDlg, HWND hWndPage) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndPage, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndPage, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndPage, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		FORWARD_WM_COMMAND(hWndPage, 0L, nullptr, BN_CLICKED,::SendMessageW);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndPage, IDC_BUTTON_HELP);

		// OKボタンを押下して閉じる
		SendPsmPressButton(hWndDlg, PSBTN_OK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	ShowPropCommon(ID_PROPCOM_PAGENUM_WIN);
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon004)
{
	// 表示された共通設定を閉じる
	dialog::PropertySheetCloser closer([] (HWND hWndDlg, HWND hWndPage) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndPage, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndPage, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndPage, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		FORWARD_WM_COMMAND(hWndPage, 0L, nullptr, BN_CLICKED,::SendMessageW);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndPage, IDC_BUTTON_HELP);

		SendDlgCommand(hWndPage, IDC_BUTTON_EXPORT);	// L"エクスポート(X)..."
		// 保存先ファイル名の入力はモックで実現する

		SendDlgCommand(hWndPage, IDC_BUTTON_IMPORT);	// L"インポート(I)..."
		// 開くファイル名の入力はモックで実現する

		// OKボタンを押下して閉じる
		SendPsmPressButton(hWndDlg, PSBTN_OK);
	});

	const auto& exportPath = mainmenuExportPath;
	MockCDlgOpenFile::gm_Files.emplace_back(exportPath.native());

	auto& cDlgOpenFile = static_cast<MockCDlgOpenFile&>(*CDlgOpenFile::getInstance());
	EXPECT_CALL(cDlgOpenFile, DoModal_GetSaveFileName(_))
		.Times(1)
		.WillOnce(testing::DoDefault());
	EXPECT_CALL(cDlgOpenFile, DoModal_GetOpenFileName(_, _))
		.Times(1)
		.WillOnce(testing::DoDefault());

	auto pUser32 = (MockUser32*)User32::getInstance();
	const auto exportMsg = std::wstring{ LS(STR_IMPEXP_OK_EXPORT) } + exportPath.native();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(exportMsg), _, _, _))
		.Times(1)
		.WillOnce(Return(IDOK));

	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	ShowPropCommon(ID_PROPCOM_PAGENUM_MAINMENU);
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon005)
{
	// 表示された共通設定をOKボタンで閉じる
	dialog::PropertySheetCloser closer([] (HWND hWndDlg, HWND hWndPage) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndPage, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndPage, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndPage, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		FORWARD_WM_COMMAND(hWndPage, 0L, nullptr, BN_CLICKED,::SendMessageW);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndPage, IDC_BUTTON_HELP);

		// OKボタンを押下して閉じる
		SendPsmPressButton(hWndDlg, PSBTN_OK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	ShowPropCommon(ID_PROPCOM_PAGENUM_TOOLBAR);
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon006)
{
	// 表示された共通設定をOKボタンで閉じる
	dialog::PropertySheetCloser closer([] (HWND hWndDlg, HWND hWndPage) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndPage, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndPage, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndPage, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		FORWARD_WM_COMMAND(hWndPage, 0L, nullptr, BN_CLICKED,::SendMessageW);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndPage, IDC_BUTTON_HELP);

		// OKボタンを押下して閉じる
		SendPsmPressButton(hWndDlg, PSBTN_OK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	ShowPropCommon(ID_PROPCOM_PAGENUM_TAB);
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon007)
{
	// 表示された共通設定をOKボタンで閉じる
	dialog::PropertySheetCloser closer([] (HWND hWndDlg, HWND hWndPage) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndPage, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndPage, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndPage, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		FORWARD_WM_COMMAND(hWndPage, 0L, nullptr, BN_CLICKED,::SendMessageW);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndPage, IDC_BUTTON_HELP);

		// OKボタンを押下して閉じる
		SendPsmPressButton(hWndDlg, PSBTN_OK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	ShowPropCommon(ID_PROPCOM_PAGENUM_STATUSBAR);
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon008)
{
	// 表示された共通設定をOKボタンで閉じる
	dialog::PropertySheetCloser closer([] (HWND hWndDlg, HWND hWndPage) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndPage, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndPage, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndPage, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		FORWARD_WM_COMMAND(hWndPage, 0L, nullptr, BN_CLICKED,::SendMessageW);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndPage, IDC_BUTTON_HELP);

		// OKボタンを押下して閉じる
		SendPsmPressButton(hWndDlg, PSBTN_OK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	ShowPropCommon(ID_PROPCOM_PAGENUM_EDIT);
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon009)
{
	// 表示された共通設定をOKボタンで閉じる
	dialog::PropertySheetCloser closer([] (HWND hWndDlg, HWND hWndPage) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndPage, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndPage, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndPage, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		FORWARD_WM_COMMAND(hWndPage, 0L, nullptr, BN_CLICKED,::SendMessageW);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndPage, IDC_BUTTON_HELP);

		// OKボタンを押下して閉じる
		SendPsmPressButton(hWndDlg, PSBTN_OK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	ShowPropCommon(ID_PROPCOM_PAGENUM_FILE);
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon010)
{
	// 表示された共通設定をOKボタンで閉じる
	dialog::PropertySheetCloser closer([] (HWND hWndDlg, HWND hWndPage) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndPage, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndPage, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndPage, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		FORWARD_WM_COMMAND(hWndPage, 0L, nullptr, BN_CLICKED,::SendMessageW);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndPage, IDC_BUTTON_HELP);

		// OKボタンを押下して閉じる
		SendPsmPressButton(hWndDlg, PSBTN_OK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	ShowPropCommon(ID_PROPCOM_PAGENUM_FILENAME);
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon011)
{
	// 表示された共通設定をOKボタンで閉じる
	dialog::PropertySheetCloser closer([] (HWND hWndDlg, HWND hWndPage) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndPage, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndPage, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndPage, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		FORWARD_WM_COMMAND(hWndPage, 0L, nullptr, BN_CLICKED,::SendMessageW);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndPage, IDC_BUTTON_HELP);

		// OKボタンを押下して閉じる
		SendPsmPressButton(hWndDlg, PSBTN_OK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	ShowPropCommon(ID_PROPCOM_PAGENUM_BACKUP);
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon012)
{
	// 表示された共通設定をOKボタンで閉じる
	dialog::PropertySheetCloser closer([] (HWND hWndDlg, HWND hWndPage) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndPage, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndPage, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndPage, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		FORWARD_WM_COMMAND(hWndPage, 0L, nullptr, BN_CLICKED,::SendMessageW);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndPage, IDC_BUTTON_HELP);

		// OKボタンを押下して閉じる
		SendPsmPressButton(hWndDlg, PSBTN_OK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	ShowPropCommon(ID_PROPCOM_PAGENUM_FORMAT);
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon013)
{
	// 表示された共通設定をOKボタンで閉じる
	dialog::PropertySheetCloser closer([] (HWND hWndDlg, HWND hWndPage) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndPage, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndPage, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndPage, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		FORWARD_WM_COMMAND(hWndPage, 0L, nullptr, BN_CLICKED,::SendMessageW);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndPage, IDC_BUTTON_HELP);

		// OKボタンを押下して閉じる
		SendPsmPressButton(hWndDlg, PSBTN_OK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	ShowPropCommon(ID_PROPCOM_PAGENUM_GREP);
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon014)
{
	// 表示された共通設定を閉じる
	dialog::PropertySheetCloser closer([] (HWND hWndDlg, HWND hWndPage) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndPage, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndPage, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndPage, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		FORWARD_WM_COMMAND(hWndPage, 0L, nullptr, BN_CLICKED,::SendMessageW);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndPage, IDC_BUTTON_HELP);

		SendDlgCommand(hWndPage, IDC_BUTTON_EXPORT);	// L"エクスポート(X)..."
		// 保存先ファイル名の入力はモックで実現する

		SendDlgCommand(hWndPage, IDC_BUTTON_IMPORT);	// L"インポート(I)..."
		// 開くファイル名の入力はモックで実現する

		// OKボタンを押下して閉じる
		SendPsmPressButton(hWndDlg, PSBTN_OK);
	});

	const auto& exportPath = keybindExportPath;
	MockCDlgOpenFile::gm_Files.emplace_back(exportPath.native());

	auto& cDlgOpenFile = static_cast<MockCDlgOpenFile&>(*CDlgOpenFile::getInstance());
	EXPECT_CALL(cDlgOpenFile, DoModal_GetSaveFileName(_))
		.Times(1)
		.WillOnce(testing::DoDefault());
	EXPECT_CALL(cDlgOpenFile, DoModal_GetOpenFileName(_, _))
		.Times(1)
		.WillOnce(testing::DoDefault());

	auto pUser32 = (MockUser32*)User32::getInstance();
	const auto exportMsg = std::wstring{ LS(STR_IMPEXP_OK_EXPORT) } + exportPath.native();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(exportMsg), _, _, _))
		.Times(1)
		.WillOnce(Return(IDOK));

	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	ShowPropCommon(ID_PROPCOM_PAGENUM_KEYBOARD);
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon015)
{
	// 表示された共通設定を閉じる
	dialog::PropertySheetCloser closer([] (HWND hWndDlg, HWND hWndPage) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndPage, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndPage, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndPage, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		FORWARD_WM_COMMAND(hWndPage, 0L, nullptr, BN_CLICKED,::SendMessageW);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndPage, IDC_BUTTON_HELP);

		SendDlgCommand(hWndPage, IDC_BUTTON_EXPORT);	// L"エクスポート(X)..."
		// 保存先ファイル名の入力はモックで実現する

		SendDlgCommand(hWndPage, IDC_BUTTON_IMPORT);	// L"インポート(I)..."
		// 開くファイル名の入力はモックで実現する

		// OKボタンを押下して閉じる
		SendPsmPressButton(hWndDlg, PSBTN_OK);
	});

	const auto& exportPath = custmenuExportPath;
	MockCDlgOpenFile::gm_Files.emplace_back(exportPath.native());

	auto& cDlgOpenFile = static_cast<MockCDlgOpenFile&>(*CDlgOpenFile::getInstance());
	EXPECT_CALL(cDlgOpenFile, DoModal_GetSaveFileName(_))
		.Times(1)
		.WillOnce(testing::DoDefault());
	EXPECT_CALL(cDlgOpenFile, DoModal_GetOpenFileName(_, _))
		.Times(1)
		.WillOnce(testing::DoDefault());

	auto pUser32 = (MockUser32*)User32::getInstance();
	const auto exportMsg = std::wstring{ LS(STR_IMPEXP_OK_EXPORT) } + exportPath.native();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(exportMsg), _, _, _))
		.Times(1)
		.WillOnce(Return(IDOK));

	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	ShowPropCommon(ID_PROPCOM_PAGENUM_CUSTMENU);
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon016)
{
	// 表示された共通設定を閉じる
	dialog::PropertySheetCloser closer([] (HWND hWndDlg, HWND hWndPage) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndPage, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndPage, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndPage, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		FORWARD_WM_COMMAND(hWndPage, 0L, nullptr, BN_CLICKED,::SendMessageW);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndPage, IDC_BUTTON_HELP);

		SendDlgCommand(hWndPage, IDC_BUTTON_EXPORT);	// L"エクスポート(X)..."
		// 保存先ファイル名の入力はモックで実現する

		SendDlgCommand(hWndPage, IDC_BUTTON_IMPORT);	// L"インポート(I)..."
		// 開くファイル名の入力はモックで実現する

		// OKボタンを押下して閉じる
		SendPsmPressButton(hWndDlg, PSBTN_OK);
	});

	const auto& exportPath = keywordExportPath;
	MockCDlgOpenFile::gm_Files.emplace_back(exportPath.native());

	auto& cDlgOpenFile = static_cast<MockCDlgOpenFile&>(*CDlgOpenFile::getInstance());
	EXPECT_CALL(cDlgOpenFile, DoModal_GetSaveFileName(_))
		.Times(1)
		.WillOnce(testing::DoDefault());
	EXPECT_CALL(cDlgOpenFile, DoModal_GetOpenFileName(_, _))
		.Times(1)
		.WillOnce(testing::DoDefault());

	auto pUser32 = (MockUser32*)User32::getInstance();
	const auto exportMsg = std::wstring{ LS(STR_IMPEXP_OK_EXPORT) } + exportPath.native();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(exportMsg), _, _, _))
		.Times(1)
		.WillOnce(Return(IDOK));

	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	ShowPropCommon(ID_PROPCOM_PAGENUM_KEYWORD);
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon017)
{
	// 表示された共通設定をOKボタンで閉じる
	dialog::PropertySheetCloser closer([] (HWND hWndDlg, HWND hWndPage) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndPage, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndPage, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndPage, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		FORWARD_WM_COMMAND(hWndPage, 0L, nullptr, BN_CLICKED,::SendMessageW);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndPage, IDC_BUTTON_HELP);

		// OKボタンを押下して閉じる
		SendPsmPressButton(hWndDlg, PSBTN_OK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	ShowPropCommon(ID_PROPCOM_PAGENUM_HELPER);
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon018)
{
	// 表示された共通設定をOKボタンで閉じる
	dialog::PropertySheetCloser closer([] (HWND hWndDlg, HWND hWndPage) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndPage, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndPage, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndPage, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		FORWARD_WM_COMMAND(hWndPage, 0L, nullptr, BN_CLICKED,::SendMessageW);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndPage, IDC_BUTTON_HELP);

		// OKボタンを押下して閉じる
		SendPsmPressButton(hWndDlg, PSBTN_OK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	ShowPropCommon(ID_PROPCOM_PAGENUM_MACRO);
}

/*!
 * 共通設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropCommon019)
{
	// 表示された共通設定をOKボタンで閉じる
	dialog::PropertySheetCloser closer([] (HWND hWndDlg, HWND hWndPage) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndPage, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndPage, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndPage, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		FORWARD_WM_COMMAND(hWndPage, 0L, nullptr, BN_CLICKED,::SendMessageW);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndPage, IDC_BUTTON_HELP);

		// OKボタンを押下して閉じる
		SendPsmPressButton(hWndDlg, PSBTN_OK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	ShowPropCommon(ID_PROPCOM_PAGENUM_PLUGIN);
}

/*!
 * 共通設定プロパティーシートの表示テスト
 *
 * キーワードセットが0件のとき、セット依存のコントロールが無効化されること
 */
TEST_F(EditWndTest, ShowPropCommon020)
{
	// 表示された共通設定を閉じる
	dialog::PropertySheetCloser closer([] (HWND hWndDlg, HWND hWndPage) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndPage, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndPage, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndPage, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		FORWARD_WM_COMMAND(hWndPage, 0L, nullptr, BN_CLICKED,::SendMessageW);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndPage, IDC_BUTTON_HELP);

		// キーワードセットが選択されていないので、セット依存のコントロールは無効
		const auto hWndKeySetRename = ::GetDlgItem(hWndPage, IDC_BUTTON_KEYSETRENAME);	// L"変更(H)..."
		EXPECT_THAT(hWndKeySetRename, Ne(nullptr));
		EXPECT_THAT(::IsWindowEnabled(hWndKeySetRename), IsFalse());

		const auto hWndKeyClean = ::GetDlgItem(hWndPage, IDC_BUTTON_KEYCLEAN);	// L"整理(O)"
		EXPECT_THAT(hWndKeyClean, Ne(nullptr));
		EXPECT_THAT(::IsWindowEnabled(hWndKeyClean), IsFalse());

		// セット追加はセット数0でも押せる必要がある（押せないと復帰できない）
		const auto hWndAddSet = ::GetDlgItem(hWndPage, IDC_BUTTON_ADDSET);	// L"セット追加(M)..."
		EXPECT_THAT(hWndAddSet, Ne(nullptr));
		EXPECT_THAT(::IsWindowEnabled(hWndAddSet), IsTrue());

		// キャンセルボタンを押下して閉じる
		SendPsmPressButton(hWndDlg, PSBTN_CANCEL);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	auto& cKeyWordSetMgr = GetDllShareData().m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr;

	// キーワードセットが0件の状態を作る
	const auto nKeyWordSetNumOrg = cKeyWordSetMgr.m_nKeyWordSetNum;
	const auto nCurrentKeyWordSetIdxOrg = cKeyWordSetMgr.m_nCurrentKeyWordSetIdx;
	cKeyWordSetMgr.m_nKeyWordSetNum = 0;
	cKeyWordSetMgr.m_nCurrentKeyWordSetIdx = -1;

	ShowPropCommon(ID_PROPCOM_PAGENUM_KEYWORD);

	// 共有データを元に戻す
	cKeyWordSetMgr.m_nCurrentKeyWordSetIdx = nCurrentKeyWordSetIdxOrg;
	cKeyWordSetMgr.m_nKeyWordSetNum = nKeyWordSetNumOrg;
}

#if defined(_MSC_VER) &&  defined(_DEBUG)

/*!
 * タイプ別設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropType001)
{
	// 表示されたタイプ別設定をキャンセルボタンで閉じる
	dialog::PropertySheetCloser closer(PSBTN_CANCEL);

	EXPECT_THAT(ExecMacroCommand(L"OptionType()"), IsTrue());
}

#endif // if defined(_MSC_VER) &&  defined(_DEBUG)

/*!
 * タイプ別設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropType002)
{
	// 表示されたタイプ別設定をOKボタンで閉じる
	dialog::PropertySheetCloser closer([] (HWND hWndDlg, HWND hWndPage) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndPage, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndPage, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndPage, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		FORWARD_WM_COMMAND(hWndPage, 0L, nullptr, BN_CLICKED,::SendMessageW);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndPage, IDC_BUTTON_HELP);

		// OKボタンを押下して閉じる
		SendPsmPressButton(hWndDlg, PSBTN_OK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	ShowPropType(ID_PROPTYPE_PAGENUM_SCREEN);
}

/*!
 * タイプ別設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropType003)
{
	// 表示されたタイプ別設定を閉じる
	dialog::PropertySheetCloser closer([] (HWND hWndDlg, HWND hWndPage) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndPage, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndPage, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndPage, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		FORWARD_WM_COMMAND(hWndPage, 0L, nullptr, BN_CLICKED,::SendMessageW);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndPage, IDC_BUTTON_HELP);

		SendDlgCommand(hWndPage, IDC_BUTTON_TEXTCOLOR);
		SendDlgCommand(hWndPage, IDC_BUTTON_BACKCOLOR);

		SendDlgCommand(hWndPage, IDC_BUTTON_SAMETEXTCOLOR);	// L"文字色統一(<)..."

		SendDlgCommand(hWndPage, IDC_BUTTON_SAMEBKCOLOR);	// L"背景色統一(>)..."

		SendDlgCommand(hWndPage, IDC_BUTTON_EXPORT);	// L"エクスポート(X)..."
		// 保存先ファイル名の入力はモックで実現する

		SendDlgCommand(hWndPage, IDC_BUTTON_IMPORT);	// L"インポート(I)..."
		// 開くファイル名の入力はモックで実現する

		// OKボタンを押下して閉じる
		SendPsmPressButton(hWndDlg, PSBTN_OK);
	});

	// 表示されたモーダルダイアログをOKボタンで閉じる
	dialog::ModalDialogCloser closer2(L"文字色統一", IDOK);

	// 表示されたモーダルダイアログをOKボタンで閉じる
	dialog::ModalDialogCloser closer3(L"文字色統一", [] (HWND hWndDlg) {
		ASSERT_THAT(apiwrap::GetWindowTextW(hWndDlg), StrEq(L"背景色統一"));
		SendDlgCommand(hWndDlg, IDOK);
	});

	const auto& exportPath = colorizeExportPath;
	MockCDlgOpenFile::gm_Files.emplace_back(exportPath.native());

	auto& cDlgOpenFile = static_cast<MockCDlgOpenFile&>(*CDlgOpenFile::getInstance());
	EXPECT_CALL(cDlgOpenFile, DoModal_GetSaveFileName(_))
		.Times(1)
		.WillOnce(testing::DoDefault());
	EXPECT_CALL(cDlgOpenFile, DoModal_GetOpenFileName(_, _))
		.Times(1)
		.WillOnce(testing::DoDefault());

	Comdlg32::setInstance<MockComdlg32>();

	auto pComdlg32 = static_cast<MockComdlg32*>(Comdlg32::getInstance());
	EXPECT_CALL(*pComdlg32, ChooseColorW(_))
		.Times(2)
		.WillOnce(Invoke([] (LPCHOOSECOLORW pCc) -> BOOL {
			pCc->rgbResult = RGB(0xff, 0xff, 0);
			return TRUE;
		}))
		.WillOnce(Invoke([] (LPCHOOSECOLORW pCc) -> BOOL {
			pCc->rgbResult = RGB(0xff, 0xff, 0);
			return TRUE;
		}));

	auto pUser32 = (MockUser32*)User32::getInstance();
	const auto exportMsg = std::wstring{ LS(STR_IMPEXP_OK_EXPORT) } + exportPath.native();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(exportMsg), _, _, _))
		.Times(1)
		.WillOnce(Return(IDOK));

	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	ShowPropType(ID_PROPTYPE_PAGENUM_COLOR);
}

/*!
 * タイプ別設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropType004)
{
	// 表示されたタイプ別設定を閉じる
	dialog::PropertySheetCloser closer([] (HWND hWndDlg, HWND hWndPage) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndPage, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndPage, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndPage, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		FORWARD_WM_COMMAND(hWndPage, 0L, nullptr, BN_CLICKED,::SendMessageW);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndPage, IDC_BUTTON_HELP);

		// トラックバーのハンドルを取得する
		const auto hWndTrackbar = ::GetDlgItem(hWndPage, IDC_TRACKBAR_BACKIMG_TRANSPARENCY);
		EXPECT_THAT(hWndTrackbar, Ne(nullptr));
		::SetFocus(hWndTrackbar);

		// トラックバーのウインドウ矩形を取得する
		RECT rcTrackbar{};
		EXPECT_THAT(::GetWindowRect(hWndTrackbar, &rcTrackbar), IsTrue());

		const LONG x = (rcTrackbar.left + rcTrackbar.right) / 2;
		const LONG y = (rcTrackbar.top + rcTrackbar.bottom) / 2;

		std::vector<INPUT> inputs{};
		inputs.emplace_back(MakeMouseInputMove(x, y));
		inputs.emplace_back(MakeMouseInputWheel(-WHEEL_DELTA));
		EXPECT_THAT(SendInput(inputs), Eq(std::size(inputs)));

		FORWARD_WM_HSCROLL(hWndPage, hWndTrackbar, TB_LINEDOWN, -WHEEL_DELTA, ::SendMessageW);

		// OKボタンを押下して閉じる
		SendPsmPressButton(hWndDlg, PSBTN_OK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	ShowPropType(ID_PROPTYPE_PAGENUM_WINDOW);
}

/*!
 * タイプ別設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropType005)
{
	// 表示されたタイプ別設定をOKボタンで閉じる
	dialog::PropertySheetCloser closer([] (HWND hWndDlg, HWND hWndPage) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndPage, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndPage, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndPage, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		FORWARD_WM_COMMAND(hWndPage, 0L, nullptr, BN_CLICKED,::SendMessageW);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndPage, IDC_BUTTON_HELP);

		// OKボタンを押下して閉じる
		SendPsmPressButton(hWndDlg, PSBTN_OK);
	});

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	ShowPropType(ID_PROPTYPE_PAGENUM_SUPPORT);
}

/*!
 * タイプ別設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropType006)
{
	// 表示されたタイプ別設定を閉じる
	dialog::PropertySheetCloser closer([] (HWND hWndDlg, HWND hWndPage) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndPage, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndPage, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndPage, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		FORWARD_WM_COMMAND(hWndPage, 0L, nullptr, BN_CLICKED,::SendMessageW);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndPage, IDC_BUTTON_HELP);

		SendDlgCommand(hWndPage, IDC_BUTTON_REGEX_EXPORT);	// L"エクスポート(X)..."
		// 保存先ファイル名の入力はモックで実現する

		SendDlgCommand(hWndPage, IDC_BUTTON_REGEX_IMPORT);	// L"インポート(I)..."
		// 開くファイル名の入力はモックで実現する

		// OKボタンを押下して閉じる
		SendPsmPressButton(hWndDlg, PSBTN_OK);
	});

	const auto& exportPath = regexKeywordExportPath;
	MockCDlgOpenFile::gm_Files.emplace_back(exportPath.native());

	auto& cDlgOpenFile = static_cast<MockCDlgOpenFile&>(*CDlgOpenFile::getInstance());
	EXPECT_CALL(cDlgOpenFile, DoModal_GetSaveFileName(_))
		.Times(1)
		.WillOnce(testing::DoDefault());
	EXPECT_CALL(cDlgOpenFile, DoModal_GetOpenFileName(_, _))
		.Times(1)
		.WillOnce(testing::DoDefault());

	auto pUser32 = (MockUser32*)User32::getInstance();
	const auto exportMsg = std::wstring{ LS(STR_IMPEXP_OK_EXPORT) } + exportPath.native();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(exportMsg), _, _, _))
		.Times(1)
		.WillOnce(Return(IDOK));

	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	ShowPropType(ID_PROPTYPE_PAGENUM_REGEX);
}

/*!
 * タイプ別設定プロパティーシートの表示テスト
 */
TEST_F(EditWndTest, ShowPropType007)
{
	// 表示されたタイプ別設定を閉じる
	dialog::PropertySheetCloser closer([] (HWND hWndDlg, HWND hWndPage) {
		// 処理対象でないメッセージを送信して空振りさせる
		::SendMessageW(hWndPage, WM_NULL, 0L, 0L);

		// WM_HELPを送信してヘルプ表示処理を空振りさせる
		HELPINFO hi{};
		::SendMessageW(hWndPage, WM_HELP, 0L, LPARAM(&hi));

		// コンテキストメニュー表示を空振りさせる
		FORWARD_WM_CONTEXTMENU(hWndPage, nullptr, 0L, 0L, ::SendMessageW);

		// 処理対象でないボタンIDを送信して空振りさせる
		FORWARD_WM_COMMAND(hWndPage, 0L, nullptr, BN_CLICKED,::SendMessageW);

		// ヘルプボタンを押下する
		SendDlgCommand(hWndPage, IDC_BUTTON_HELP);

		SendDlgCommand(hWndPage, IDC_BUTTON_KEYHELP_EXPORT);	// L"エクスポート(X)..."
		// 保存先ファイル名の入力はモックで実現する

		SendDlgCommand(hWndPage, IDC_BUTTON_KEYHELP_IMPORT);	// L"インポート(I)..."
		// 開くファイル名の入力はモックで実現する

		// OKボタンを押下して閉じる
		SendPsmPressButton(hWndDlg, PSBTN_OK);
	});

	const auto& exportPath = keywordHelpExportPath;
	MockCDlgOpenFile::gm_Files.emplace_back(exportPath.native());

	auto& cDlgOpenFile = static_cast<MockCDlgOpenFile&>(*CDlgOpenFile::getInstance());
	EXPECT_CALL(cDlgOpenFile, DoModal_GetSaveFileName(_))
		.Times(1)
		.WillOnce(testing::DoDefault());
	EXPECT_CALL(cDlgOpenFile, DoModal_GetOpenFileName(_, _))
		.Times(1)
		.WillOnce(testing::DoDefault());

	auto pUser32 = (MockUser32*)User32::getInstance();
	const auto exportMsg = std::wstring{ LS(STR_IMPEXP_OK_EXPORT) } + exportPath.native();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(exportMsg), _, _, _))
		.Times(1)
		.WillOnce(Return(IDOK));

	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_WM_HELP, _)).WillRepeatedly(testing::DoDefault());			// WM_HELP 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXTMENU, _)).WillRepeatedly(testing::DoDefault());		// WM_CONTEXTMENU 空振り
	EXPECT_CALL(*pUser32, WinHelpW(_, IsNull(), HELP_CONTEXT, _)).WillRepeatedly(testing::DoDefault());	// IDC_BUTTON_HELP

	ShowPropType(ID_PROPTYPE_PAGENUM_KEYHELP);
}

TEST_F(EditWndTest, DISABLED_Timeout101)
{
	StartUiaThread([] (const IUIAutomation*, std::stop_token st) {
		std::mutex mutex;
		std::condition_variable_any condition;
		std::unique_lock lock(mutex);
		condition.wait_for(lock, st, std::chrono::seconds(31), [] { return false; });
	});
}

TEST(CDiffManager, test001)
{
	auto pcDiffManager = CDiffManager::getInstance();

	ASSERT_THAT(pcDiffManager, NotNull());

	ASSERT_THAT(pcDiffManager->IsDiffUse(), IsFalse());

	pcDiffManager->SetDiffUse(true);
	ASSERT_THAT(pcDiffManager->IsDiffUse(), IsTrue());
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
