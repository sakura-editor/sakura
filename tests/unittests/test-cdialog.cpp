/*! @file */
/*
	Copyright (C) 2023, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
 */
#include "dlg/CDialog.h"

#include "MockShareDataAccessor.hpp"

#include "MockUser32Dll.hpp"

#include "TAutoCloseDialog.hpp"

#include <functional>

extern HINSTANCE GetLanguageResourceLibrary();

/*
 * ダイアログクラステンプレートをテストするためのクラス
 
 * 自動テストで実行できるように作成したもの。
 * 初期表示後、勝手に閉じる仕様。
 */
class CDialog1 : public TAutoCloseDialog<CDialog, IDC_EDIT_INPUT1>
{
private:
	DLGPROC _pfnDlgProc = nullptr;

public:
	explicit CDialog1(std::shared_ptr<User32Dll> User32Dll_ = std::make_shared<User32Dll>());
	~CDialog1() override = default;

	INT_PTR DoModalCustom(HWND hWndParent);
	HWND DoModeless1(HWND hWndParent, int nShowCmd);

	INT_PTR CallDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) const;

	using CDialog::DispatchDlgEvent;

protected:
	BOOL    OnDlgInitDialog(HWND hDlg, HWND hWndFocus, LPARAM lParam) override;
};

/*!
 * コンストラクター
 */
CDialog1::CDialog1(std::shared_ptr<User32Dll> User32Dll_)
	: TAutoCloseDialog(IDD_INPUT1, std::move(User32Dll_))
{
}

/*!
 * モーダルダイアログを表示する
 *
 * CDialog::DoModalは無駄なパラメーターを要求するので、
 * テストコードを簡潔に記述できるように作成したもの。
 */
INT_PTR CDialog1::DoModalCustom(HWND hWndParent)
{
	const auto hInstance = (HINSTANCE)nullptr;
	const auto lParam    = (LPARAM)NULL;
	return __super::DoModal(hInstance, hWndParent, IDD_INPUT1, lParam);
}

/*!
 * モードレスダイアログを表示する
 *
 * CDialog::DoModelessは無駄なパラメーターを要求するので、
 * テストコードを簡潔に記述できるように作成したもの。
 */
HWND CDialog1::DoModeless1(HWND hWndParent, int nCmdShow)
{
	const auto hInstance = (HINSTANCE)nullptr;
	const auto lParam    = (LPARAM)NULL;
	return __super::DoModeless(hInstance, hWndParent, IDD_INPUT1, lParam, nCmdShow);
}

/*!
 * ダイアログプロシージャを呼び出します。
 *
 * ポインタ変数は起動時にするので、一度開いてから使います。
 */
INT_PTR CDialog1::CallDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) const
{
	return _pfnDlgProc ? _pfnDlgProc(hDlg, uMsg, wParam, lParam) : FALSE;
}

/*!
 * WM_INITDIALOG処理
 *
 * ダイアログ構築後、最初に受け取るメッセージを処理する。
 *
 * @param [in] hDlg 宛先ウインドウのハンドル
 * @param [in] hWndFocus フォーカスを受け取る子ウインドウのハンドル
 * @param [in] lParam ダイアログパラメーター
 * @retval TRUE  フォーカスが設定されます。
 * @retval FALSE フォーカスは設定されません。
 */
BOOL CDialog1::OnDlgInitDialog(HWND hDlg, HWND hWndFocus, LPARAM lParam)
{
	// 派生元クラスに処理を委譲する
	const auto ret = __super::OnDlgInitDialog(hDlg, hWndFocus, lParam);

	// ダイアログプロシージャをメンバー変数に格納する
	_pfnDlgProc = std::bit_cast<DLGPROC>(GetWindowLongPtrW(hDlg, DWLP_DLGPROC));

	// 派生元クラスが返した戻り値をそのまま返す
	return ret;
}

class mock_dialog_1 : public CDialog1
{
public:
	explicit mock_dialog_1(std::shared_ptr<User32Dll> User32Dll_ = std::make_shared<User32Dll>())
		: CDialog1(std::move(User32Dll_))
	{
	}

	MOCK_METHOD3_T(OnInitDialog, BOOL(HWND, WPARAM, LPARAM));
	MOCK_METHOD0_T(OnDestroy, BOOL());
	MOCK_METHOD2_T(OnMove, BOOL(WPARAM, LPARAM));
	MOCK_METHOD2_T(OnCommand, BOOL(WPARAM, LPARAM));
	MOCK_METHOD1_T(OnNotify, BOOL(LPNMHDR));
	MOCK_METHOD1_T(OnTimer, BOOL(WPARAM));
	MOCK_METHOD2_T(OnKeyDown, BOOL(WPARAM, LPARAM));
	MOCK_METHOD2_T(OnKillFocus, BOOL(WPARAM, LPARAM));
	MOCK_METHOD2_T(OnActivate, BOOL(WPARAM, LPARAM));
	MOCK_METHOD2_T(OnPopupHelp, BOOL(WPARAM, LPARAM));
	MOCK_METHOD2_T(OnContextMenu, BOOL(WPARAM, LPARAM));
	MOCK_METHOD2_T(OnDrawItem, BOOL(WPARAM, LPARAM));
};

/*
 * ダイアログクラステンプレートをテストするためのクラス

 * 自動テストで実行できるように作成したもの。
 */
class CDialog2 : public CSizeRestorableDialog
{
private:
	static constexpr auto DIALOG_ID = IDD_INPUT1;

public:
	explicit CDialog2(std::shared_ptr<ShareDataAccessor> ShareDataAccessor_, std::shared_ptr<User32Dll> User32Dll_ = std::make_shared<User32Dll>());
	~CDialog2() override = default;

	HWND DoModeless1(HWND hWndParent, int nShowCmd);

	template<typename TFunc>
	HWND DoModeless2(HWND hWndParent, const TFunc& func, int nShowCmd);

	using CDialog::DispatchDlgEvent;
};

/*!
 * コンストラクター
 */
CDialog2::CDialog2(std::shared_ptr<ShareDataAccessor> ShareDataAccessor_, std::shared_ptr<User32Dll> User32Dll_)
	: CSizeRestorableDialog(DIALOG_ID, std::move(ShareDataAccessor_), std::move(User32Dll_))
{
}

/*!
 * モードレスダイアログを表示する
 *
 * CDialog::DoModelessは無駄なパラメーターを要求するので、
 * テストコードを簡潔に記述できるように作成したもの。
 */
HWND CDialog2::DoModeless1(HWND hWndParent, int nCmdShow)
{
	const auto hInstance = (HINSTANCE)nullptr;
	const auto lParam    = (LPARAM)NULL;
	return __super::DoModeless(hInstance, hWndParent, IDD_INPUT1, lParam, nCmdShow);
}

/*!
 * モードレスダイアログを表示する
 *
 * テストコードを簡潔に記述できるように作成したもの。
 * 本体に統合したほうがよいコードだが、既存呼び出し元がテスト不可なので一旦作るだけ。
 */
template<typename TFunc>
HWND CDialog2::DoModeless2(HWND hWndParent, const TFunc& func, int nCmdShow)
{
	const auto hLangRsrcInstance = GetLanguageResourceLibrary();

	const auto hResInfo = FindResourceW(hLangRsrcInstance, MAKEINTRESOURCE(IDD_INPUT1), RT_DIALOG);
	if (!hResInfo) return nullptr;

	const auto hResData = LoadResource(hLangRsrcInstance, hResInfo);
	if (!hResData) return nullptr;

	const auto pDlgTemplate = std::bit_cast<LPDLGTEMPLATE>(LockResource(hResData));
	if (!pDlgTemplate) return nullptr;

	const auto dwDlgTemplateSize = SizeofResource(hLangRsrcInstance, hResInfo);

	auto buffer = std::vector<uint8_t>(dwDlgTemplateSize);
	auto lpDlgTemplate = std::bit_cast<LPDLGTEMPLATE>(buffer.data());

	memcpy_s(lpDlgTemplate, dwDlgTemplateSize, pDlgTemplate, dwDlgTemplateSize);
	func(*lpDlgTemplate);

	const auto hInstance = (HINSTANCE)nullptr;
	const auto lParam    = (LPARAM)NULL;
	return __super::DoModeless(hInstance, hWndParent, lpDlgTemplate, lParam, nCmdShow);
}

class mock_dialog_2 : public CDialog2
{
public:
	explicit mock_dialog_2(std::shared_ptr<ShareDataAccessor> ShareDataAccessor_, std::shared_ptr<User32Dll> User32Dll_ = std::make_shared<User32Dll>())
		: CDialog2(std::move(ShareDataAccessor_), std::move(User32Dll_))
	{
	}

	MOCK_METHOD2_T(OnSize, BOOL(WPARAM, LPARAM));
};

using ::testing::_;
using ::testing::Return;

/*!
 * モーダルダイアログ表示、正常系テスト
 */
TEST(CDialog, SimpleDoModal)
{
	CDialog1 dlg;
	EXPECT_EQ(IDOK, dlg.DoModalCustom(nullptr));
	EXPECT_FALSE(dlg.CallDialogProc(nullptr, WM_NULL, 0, 0));
}

/*!
 * モーダルダイアログ表示、正常系テスト
 *
 * Windows APIの呼び出しパラメーターを確認する
 */
TEST(CDialog, MockedDoModal)
{
	// メッセージリソースDLLのインスタンスハンドル
	const auto hLangRsrcInstance = GetLanguageResourceLibrary();

	// 親ウインドウのハンドル(ダミー)
	const auto hWndParent = (HWND)0x1234;

	auto pUser32Dll = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pUser32Dll, DialogBoxParamW(hLangRsrcInstance, MAKEINTRESOURCEW(IDD_INPUT1), hWndParent, _, _)).WillOnce(Return(IDCANCEL));

	mock_dialog_1 mock(std::move(pUser32Dll));
	EXPECT_EQ(IDCANCEL, mock.DoModalCustom(hWndParent));
}

/*!
 * モードレスダイアログ表示、正常系テスト
 */
TEST(CSizeRestorableDialog, SimpleDoModeless1)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	CDialog2 dlg(std::move(pShareDataAccessor));
	EXPECT_NE(nullptr, dlg.DoModeless1(nullptr, SW_SHOW));
}

/*!
 * モードレスダイアログ表示、正常系テスト
 *
 * Windows APIの呼び出しパラメーターを確認する
 */
TEST(CSizeRestorableDialog, MockedDoModeless1)
{
	// メッセージリソースDLLのインスタンスハンドル
	const auto hLangRsrcInstance = GetLanguageResourceLibrary();

	// 親ウインドウのハンドル(ダミー)
	const auto hWndParent = (HWND)0x1234;

	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto pUser32Dll = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pUser32Dll, CreateDialogParamW(hLangRsrcInstance, MAKEINTRESOURCEW(IDD_INPUT1), hWndParent, _, _)).WillOnce(Return(hDlg));
	EXPECT_CALL(*pUser32Dll, ShowWindow(hDlg, SW_SHOW)).WillOnce(Return(true));

	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	mock_dialog_2 mock(std::move(pShareDataAccessor), std::move(pUser32Dll));
	EXPECT_EQ(hDlg, mock.DoModeless1(hWndParent, SW_SHOW));
}

/*!
 * モードレスダイアログ表示、正常系テスト
 */
TEST(CSizeRestorableDialog, SimpleDoModeless2)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	CDialog2 dlg(std::move(pShareDataAccessor));
	EXPECT_NE(nullptr, dlg.DoModeless2(nullptr, [](DLGTEMPLATE& dlgTemplate) { dlgTemplate.style = WS_OVERLAPPEDWINDOW | DS_SETFONT; }, SW_SHOWDEFAULT));
}

/*!
 * モードレスダイアログ表示、正常系テスト
 *
 * Windows APIの呼び出しパラメーターを確認する
 */
TEST(CSizeRestorableDialog, MockedDoModeless2)
{
	// 親ウインドウのハンドル(ダミー)
	const auto hWndParent = (HWND)0x1234;

	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto pUser32Dll = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pUser32Dll, CreateDialogIndirectParamW(_, _, hWndParent, _, _)).WillOnce(Return(hDlg));
	EXPECT_CALL(*pUser32Dll, ShowWindow(hDlg, SW_SHOWDEFAULT)).WillOnce(Return(TRUE));

	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	mock_dialog_2 dlg(std::move(pShareDataAccessor), std::move(pUser32Dll));
	EXPECT_NE(nullptr, dlg.DoModeless2(hWndParent, [](DLGTEMPLATE& dlgTemplate) { dlgTemplate.style = WS_OVERLAPPEDWINDOW | DS_SETFONT; }, SW_SHOWDEFAULT));
}

TEST(CDialog, MockedDispachDlgEvent_OnInitDialog)
{
	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	const auto hWndFocus = (HWND)0x1234;

	mock_dialog_1 mock;

	auto wParam = (WPARAM)hWndFocus;
	auto lParam = std::bit_cast<LPARAM>(&mock);

	EXPECT_CALL(mock, OnInitDialog(hDlg, wParam, lParam)).WillOnce(Return(true));

	EXPECT_TRUE(mock.DispatchDlgEvent(hDlg, WM_INITDIALOG, wParam, lParam));
}

TEST(CDialog, MockedDispachDlgEvent_OnMove)
{
	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto wParam = (WPARAM)0x1111;
	auto lParam = (LPARAM)0x2222;

	mock_dialog_1 mock;
	EXPECT_CALL(mock, OnMove(_, lParam)).WillOnce(Return(true));

	EXPECT_TRUE(mock.DispatchDlgEvent(hDlg, WM_MOVE, wParam, lParam));
}

TEST(CDialog, MockedDispachDlgEvent_OnCommand)
{
	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto wParam = (WPARAM)0x1111;
	auto lParam = (LPARAM)0x2222;

	mock_dialog_1 mock;
	EXPECT_CALL(mock, OnCommand(wParam, lParam)).WillOnce(Return(false));

	EXPECT_FALSE(mock.DispatchDlgEvent(hDlg, WM_COMMAND, wParam, lParam));
}

TEST(CDialog, MockedDispachDlgEvent_OnNotify)
{
	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto wParam = (WPARAM)0x1111;
	auto lParam = (LPARAM)0x2222;

	mock_dialog_1 mock;
	EXPECT_CALL(mock, OnNotify((NMHDR*)lParam)).WillOnce(Return(false));

	EXPECT_FALSE(mock.DispatchDlgEvent(hDlg, WM_NOTIFY, 0, lParam));
}

TEST(CDialog, MockedDispachDlgEvent_OnTimer)
{
	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto wParam = (WPARAM)0x1111;
	auto lParam = (LPARAM)0x2222;

	mock_dialog_1 mock;
	EXPECT_CALL(mock, OnTimer(wParam)).WillOnce(Return(true));

	EXPECT_TRUE(mock.DispatchDlgEvent(hDlg, WM_TIMER, wParam, lParam));
}

TEST(CDialog, MockedDispachDlgEvent_OnKeyDown)
{
	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto wParam = (WPARAM)0x1111;
	auto lParam = (LPARAM)0x2222;

	mock_dialog_1 mock;
	EXPECT_CALL(mock, OnKeyDown(wParam, lParam)).WillOnce(Return(true));

	EXPECT_TRUE(mock.DispatchDlgEvent(hDlg, WM_KEYDOWN, wParam, lParam));
}

TEST(CDialog, MockedDispachDlgEvent_OnActivate)
{
	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto wParam = (WPARAM)0x1111;
	auto lParam = (LPARAM)0x2222;

	mock_dialog_1 mock;
	EXPECT_CALL(mock, OnActivate(wParam, lParam)).WillOnce(Return(false));

	EXPECT_FALSE(mock.DispatchDlgEvent(hDlg, WM_ACTIVATE, wParam, lParam));
}

TEST(CDialog, MockedDispachDlgEvent_OnKillFocus)
{
	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto wParam = (WPARAM)0x1111;
	auto lParam = (LPARAM)0x2222;

	mock_dialog_1 mock;
	EXPECT_CALL(mock, OnKillFocus(wParam, _)).WillOnce(Return(false));

	EXPECT_FALSE(mock.DispatchDlgEvent(hDlg, WM_KILLFOCUS, wParam, lParam));
}

TEST(CDialog, MockedDispachDlgEvent_OnPopupHelp)
{
	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto wParam = (WPARAM)0x1111;
	auto lParam = (LPARAM)0x2222;

	mock_dialog_1 mock;
	EXPECT_CALL(mock, OnPopupHelp(_, lParam)).WillOnce(Return(true));

	EXPECT_TRUE(mock.DispatchDlgEvent(hDlg, WM_HELP, wParam, lParam));
}

TEST(CDialog, MockedDispachDlgEvent_OnContextMenu)
{
	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto wParam = (WPARAM)0x1111;
	auto lParam = (LPARAM)0x2222;

	mock_dialog_1 mock;
	EXPECT_CALL(mock, OnContextMenu(wParam, lParam)).WillOnce(Return(true));

	EXPECT_TRUE(mock.DispatchDlgEvent(hDlg, WM_CONTEXTMENU, wParam, lParam));
}

TEST(CDialog, MockedDispachDlgEvent_OnSize)
{
	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto wParam = (WPARAM)0x1111;
	auto lParam = (LPARAM)0x2222;

	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	mock_dialog_2 mock(std::move(pShareDataAccessor));
	EXPECT_CALL(mock, OnSize(wParam, lParam)).WillOnce(Return(false));

	EXPECT_FALSE(mock.DispatchDlgEvent(hDlg, WM_SIZE, wParam, lParam));
}

TEST(CSizeRestorableDialog, MockedDispachDlgEvent_OnDrawItem)
{
	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto wParam = (WPARAM)0x1111;
	auto lParam = (LPARAM)0x2222;

	mock_dialog_1 mock;
	EXPECT_CALL(mock, OnDrawItem(wParam, lParam)).WillOnce(Return(true));

	EXPECT_TRUE(mock.DispatchDlgEvent(hDlg, WM_DRAWITEM, wParam, lParam));
}

TEST(CDialog, GetHelpIdTable)
{
	mock_dialog_1 dlg;
	EXPECT_TRUE(dlg.GetHelpIdTable());
}
