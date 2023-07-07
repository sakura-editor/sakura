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
#include <gmock/gmock.h>

#include "dlg/CDialog.h"

#include "CSelectLang.h"

#include "sakura_rc.h"

#include <functional>

/*
 * ダイアログクラステンプレートをテストするためのクラス
 
 * 自動テストで実行できるように作成したもの。
 * 初期表示後、勝手に閉じる仕様。
 */
class CDialog1 : public CDialog
{
private:
	static constexpr auto DIALOG_ID          = IDD_INPUT1;
	static constexpr auto TIMERID_FIRST_IDLE = 1;

	DLGPROC _pfnDlgProc = nullptr;

public:
	explicit CDialog1(bool bSizable = false);
	~CDialog1() override = default;

	INT_PTR DoModalCustom(HWND hWndParent);
	HWND DoModeless1(HWND hWndParent, int nShowCmd);

	template<typename TFunc>
	HWND DoModeless2(HWND hWndParent, const TFunc& func, int nShowCmd);

	INT_PTR CallDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) const;

protected:
	BOOL OnInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam) override;
	BOOL OnTimer(WPARAM wParam) override;
};

/*!
 * コンストラクター
 */
CDialog1::CDialog1(bool bSizable)
	: CDialog(bSizable, false) //DllShareDataのチェックをしない
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
	HINSTANCE hInstance = nullptr;
	LPARAM    lParam    = NULL;
	return CDialog::DoModal(hInstance, hWndParent, DIALOG_ID, lParam);
}

/*!
 * モードレスダイアログを表示する
 *
 * CDialog::DoModelessは無駄なパラメーターを要求するので、
 * テストコードを簡潔に記述できるように作成したもの。
 */
HWND CDialog1::DoModeless1(HWND hWndParent, int nCmdShow)
{
	HINSTANCE hInstance = nullptr;
	LPARAM    lParam    = NULL;
	return CDialog::DoModeless(hInstance, hWndParent, DIALOG_ID, lParam, nCmdShow);
}

/*!
 * モードレスダイアログを表示する
 *
 * テストコードを簡潔に記述できるように作成したもの。
 * 本体に統合したほうがよいコードだが、既存呼び出し元がテスト不可なので一旦作るだけ。
 */
template<typename TFunc>
HWND CDialog1::DoModeless2(HWND hWndParent, const TFunc& func, int nCmdShow)
{
	HINSTANCE hLangRsrcInstance = CSelectLang::getLangRsrcInstance();

	const auto hResInfo = FindResourceW(hLangRsrcInstance, MAKEINTRESOURCE(DIALOG_ID), RT_DIALOG);
	if (!hResInfo) return nullptr;

	const auto hResData = LoadResource(hLangRsrcInstance, hResInfo);
	if (!hResData) return nullptr;

	const auto pDlgTemplate = std::bit_cast<LPDLGTEMPLATE>(LockResource(hResData));
	if (!pDlgTemplate) return nullptr;

	const auto dwDlgTemplateSize = SizeofResource(hLangRsrcInstance, hResInfo);

	auto lpTemplate = std::bit_cast<LPDLGTEMPLATE>(GlobalAlloc(GMEM_FIXED, dwDlgTemplateSize));
	if (!lpTemplate) return nullptr;

	CopyMemory(lpTemplate, pDlgTemplate, dwDlgTemplateSize);
	func(*lpTemplate);

	HINSTANCE hInstance = nullptr;
	LPARAM    lParam    = NULL;
	const auto hWnd = CDialog::DoModeless(hInstance, hWndParent, lpTemplate, lParam, nCmdShow);

	GlobalFree(lpTemplate);

	return hWnd;
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
 * @param [in] wParam フォーカスを受け取る子ウインドウのハンドル
 * @param [in] lParam ダイアログパラメーター
 * @retval TRUE フォーカスが設定されます。
 * @retval FALSE フォーカスは設定されません。
 */
BOOL CDialog1::OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	// 派生元クラスに処理を委譲する
	const auto ret = __super::OnInitDialog(hDlg, wParam, lParam);

	// ダイアログプロシージャをメンバー変数に格納する
	_pfnDlgProc = std::bit_cast<DLGPROC>(GetWindowLongPtrW(hDlg, DWLP_DLGPROC));

	// サイズ変更可能な場合、サイズボックスを作っておく
	if (m_bSizable)
	{
		CreateSizeBox();
	}

	// タイマーを起動する
	::SetTimer(hDlg, TIMERID_FIRST_IDLE, 0, nullptr);

	// 派生元クラスが返した戻り値をそのまま返す
	return ret;
}

/*!
 * WM_TIMER処理
 *
 * タイマーイベントを処理する。
 *
 * @retval TRUE メッセージは処理された（≒デフォルト処理は呼び出されない。）
 * @retval FALSE メッセージは処理されなかった（≒デフォルト処理が呼び出される。）
 */
BOOL CDialog1::OnTimer(WPARAM wParam)
{
	if (const auto id = static_cast<UINT>(wParam);
		id == TIMERID_FIRST_IDLE)
	{
		// プログラム的に「Enterキー押下」を発生させる
		INPUT input = {};

		// WM_KEYDOWNを発生させる
		input.type = INPUT_KEYBOARD;
		input.ki.wVk = VK_RETURN;  // Enterキーの仮想キーコード
		input.ki.dwFlags = 0;      // キーを押す
		SendInput(1, &input, sizeof(INPUT));

		// WM_KEYUPを発生させる
		input.ki.dwFlags = KEYEVENTF_KEYUP;  // キーを離す
		SendInput(1, &input, sizeof(INPUT));

		return TRUE;
	}

	return FALSE;
}

class mock_dialog_1 : public CDialog1
{
public:
	MOCK_CONST_METHOD5_T(CreateDialogIndirectParamW, HWND(HINSTANCE, LPCDLGTEMPLATEW, HWND, DLGPROC, LPARAM));
	MOCK_CONST_METHOD5_T(CreateDialogParamW, HWND(HINSTANCE, LPCWSTR, HWND, DLGPROC, LPARAM));
	MOCK_CONST_METHOD5_T(DialogBoxParamW, INT_PTR(HINSTANCE, LPCWSTR, HWND, DLGPROC, LPARAM));
	MOCK_CONST_METHOD2_T(ShowWindow, bool(HWND, int));
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
}

/*!
 * モーダルダイアログ表示、正常系テスト
 *
 * Windows APIの呼び出しパラメーターを確認する
 */
TEST(CDialog, MockedDoModal)
{
	// メッセージリソースDLLのインスタンスハンドル
	auto hLangRsrcInstance = CSelectLang::getLangRsrcInstance();

	// 親ウインドウのハンドル(ダミー)
	const auto hWndParent = (HWND)0x1234;

	mock_dialog_1 mock;
	EXPECT_CALL(mock, DialogBoxParamW(hLangRsrcInstance, MAKEINTRESOURCEW(IDD_INPUT1), hWndParent, _, std::bit_cast<LPARAM>(&mock)))
		.WillOnce(Return(IDCANCEL));

	EXPECT_EQ(IDCANCEL, mock.DoModalCustom(hWndParent));
}

/*!
 * モードレスダイアログ表示、正常系テスト
 */
TEST(CDialog, SimpleDoModeless1)
{
	CDialog1 dlg;
	EXPECT_NE(nullptr, dlg.DoModeless1(nullptr, SW_SHOW));

	EXPECT_FALSE(dlg.CallDialogProc(nullptr, WM_NULL, 0, 0));
}

/*!
 * モードレスダイアログ表示、正常系テスト
 *
 * Windows APIの呼び出しパラメーターを確認する
 */
TEST(CDialog, MockedDoModeless1)
{
	// メッセージリソースDLLのインスタンスハンドル
	auto hLangRsrcInstance = CSelectLang::getLangRsrcInstance();

	// 親ウインドウのハンドル(ダミー)
	const auto hWndParent = (HWND)0x1234;

	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	mock_dialog_1 mock;
	EXPECT_CALL(mock, CreateDialogParamW(hLangRsrcInstance, MAKEINTRESOURCEW(IDD_INPUT1), hWndParent, _, std::bit_cast<LPARAM>(&mock)))
		.WillOnce(Return(hDlg));
	EXPECT_CALL(mock, ShowWindow(hDlg, SW_SHOW))
		.WillOnce(Return(true));

	EXPECT_EQ(hDlg, mock.DoModeless1(hWndParent, SW_SHOW));
}

/*!
 * モードレスダイアログ表示、正常系テスト
 */
TEST(CDialog, SimpleDoModeless2)
{
	CDialog1 dlg(true);
	EXPECT_NE(nullptr, dlg.DoModeless2(nullptr, [](DLGTEMPLATE& dlgTemplate) { dlgTemplate.style = WS_OVERLAPPEDWINDOW | DS_SETFONT; }, SW_SHOWDEFAULT));
}

/*!
 * モードレスダイアログ表示、正常系テスト
 *
 * Windows APIの呼び出しパラメーターを確認する
 */
TEST(CDialog, MockedDoModeless2)
{
	// メモリ上に展開したダイアログテンプレートのダミー
	auto lpTemplate = std::bit_cast<LPCDLGTEMPLATE>(static_cast<size_t>(0x87654321));

	// 親ウインドウのハンドル(ダミー)
	const auto hWndParent = (HWND)0x1234;

	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	mock_dialog_1 mock;
	EXPECT_CALL(mock, CreateDialogIndirectParamW(nullptr, lpTemplate, hWndParent, _, std::bit_cast<LPARAM>(&mock)))
		.WillOnce(Return(hDlg));
	EXPECT_CALL(mock, ShowWindow(hDlg, SW_SHOWDEFAULT))
		.WillOnce(Return(true));

	EXPECT_EQ(hDlg, mock.DoModeless(nullptr, hWndParent, lpTemplate, NULL, SW_SHOWDEFAULT));
}
