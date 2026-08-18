/*!	@file
	@brief 1行入力ダイアログボックス

	@author Norio Nakatani
	@date	1998/05/31 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, MIK
	Copyright (C) 2003, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "dlg/CDlgInput1.h"

#include "util/shell.h"
#include "util/window.h"

#include "CSelectLang.h"

#include "sakura_rc.h"
#include "sakura.hh"

// 入力 CDlgInput1.cpp	//@@@ 2002.01.07 add start MIK
static const DWORD p_helpids[] = {	//13000
	IDOK,					HIDOK_DLG1,
	IDCANCEL,				HIDCANCEL_DLG1,
	IDC_EDIT_INPUT1,		HIDC_DLG1_EDIT1,	//入力フィールド	IDC_EDIT1->IDC_EDIT_INPUT1	2008/7/3 Uchi
	IDC_STATIC_MSG,			HIDC_DLG1_EDIT1,	//メッセージ
//	IDC_STATIC,				-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

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
/* static */ INT_PTR CALLBACK CDlgInput1::DlgProc(
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
 * @brief 入力データを検証する関数
 *
 * @param hWndDlg [in] 宛先ウィンドウのハンドル
 * @param text [in] 入力データ
 * @param cchBuffer [in] バッファーサイズ
 *
 * @returns データを取り込んでよいか
 * @retval > 0 取り込んでよい
 * @retval = 0 データがない
 * @retval < 0 取り込んではいけない
 */
/* static */ int CDlgInput1::NoValidation(
	HWND /* hWndDlg */,
	std::wstring_view /* text */,
	size_t /* cchBuffer */
)
{
	return 1;	// データを取り込んでよい
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
/* static */ LRESULT CALLBACK CDlgInput1::SubclassProc(
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
 * @brief モーダルダイアログの表示
 *
 * @note 移行元実装。
 *
 * @param [in, opt] モジュールハンドル。
 * @param hWndOwner [in, opt] オーナーウィンドウのハンドル
 * @param pszTitle [in] タイトル文字列
 * @param pszMessage [in] メッセージ文字列
 * @param cchMaxTextLength [in] 入力可能文字数
 * @param pszText [in, out] 入力値を受け取るバッファ
 *
 * @note 既存コード互換用に残しておく
 */
BOOL CDlgInput1::DoModal(
	_Reserved_ HINSTANCE [[maybe_unused]],
	_In_opt_ HWND hWndOwner,
	_In_z_ const WCHAR*	pszTitle,
	_In_z_ const WCHAR*	pszMessage,
	size_t cchMaxTextLength,
	_Inout_updates_z_(cchMaxTextLength + 1) WCHAR* pszText
)
{
	assert(pszTitle);
	assert(pszMessage);
	assert(0 < cchMaxTextLength);

	// 移行先実装を呼び出す
	return DoModal(
		hWndOwner,
		pszTitle,
		pszMessage,
		std::span(pszText, cchMaxTextLength + 1)
	);
}

/*!
 * @brief モーダルダイアログの表示
 *
 * @note 移行先実装。
 *
 * @param hWndOwner [in, opt] オーナーウィンドウのハンドル
 * @param title [in] タイトル文字列
 * @param message [in] メッセージ文字列
 * @param buffer [out] 入力値を受け取るバッファ
 */
BOOL CDlgInput1::DoModal(
	_In_opt_ HWND hWndOwner,
	std::wstring_view title,
	std::wstring_view message,
	std::span<WCHAR> buffer,
	const std::optional<SFuncType>& optFunc
)
{
	// 検証関数を取り出す。指定がなければ「検証なし」とする
	m_Func = optFunc.value_or(&NoValidation);

	// 内部実装を呼び出す。呼出先はテスト時に差し替え可能。
	return DoModal(
		hWndOwner,
		std::data(title),
		std::data(message),
		std::data(buffer),
		std::size(buffer)
	);
}

/*!
 * @brief モーダルダイアログの表示
 *
 * @note std::wstringをバッファにする版。
 *
 * @param hWndOwner [in, opt] オーナーウィンドウのハンドル
 * @param title [in] タイトル文字列
 * @param message [in] メッセージ文字列
 * @param buffer [out] 入力値を受け取るバッファ。呼び出し側で適切なサイズにresizeすること。
 */
BOOL CDlgInput1::DoModal(
	_In_opt_ HWND hWndOwner,
	std::wstring_view title,
	std::wstring_view message,
	std::wstring& buffer,
	const std::optional<SFuncType>& optFunc
)
{
	// 移行先実装を呼び出す
	return DoModal(
		hWndOwner,
		title,
		message,
		std::span(std::data(buffer), std::size(buffer) + 1),
		optFunc
	);
}

/*!
 * @brief モーダルダイアログの表示
 *
 * @param hWndOwner [in, opt] オーナーウィンドウのハンドル
 * @param pszTitle [in] タイトル文字列
 * @param pszMessage [in] メッセージ文字列
 * @param pBuffer [in, out] 入力値を受け取るバッファ
 * @param cchBuffer [in] 入力値を受け取るバッファのサイズ。NUL終端を含むサイズを指定する。
 *
 * @note 直接呼ばないでください。
 * @note この関数の動作はモックで上書き可能。
 */
BOOL CDlgInput1::DoModal(
	_In_opt_ HWND hWndOwner,
	_In_z_ LPCWSTR pszTitle,
	_In_z_ LPCWSTR pszMessage,
	_Inout_updates_z_(cchBuffer) LPWSTR pBuffer,
	size_t cchBuffer
)
{
	assert(pszTitle);
	assert(pszMessage);
	assert(0 < cchBuffer);

#if 0 // 以下にヒットする場合、バッファが初期化されていない。結構引っかかるので無効化しておく。
	assert_warning(0 == pBuffer[cchBuffer - 1]);
#endif

	pBuffer[cchBuffer - 1] = L'\0';

	m_Title = pszTitle;
	m_Message = pszMessage;
	m_Text = std::span(pBuffer, cchBuffer);

	return (BOOL)::DialogBoxParamW(
		CSelectLang::getLangRsrcInstance(),
		MAKEINTRESOURCE(IDD_INPUT1),
		hWndOwner,
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
INT_PTR CDlgInput1::DispatchDlgEvent(
	HWND hWndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
)
{
	if (WM_INITDIALOG == uMsg) {
		// WM_INITDIALOGを処理させる
		const auto bRet = HANDLE_WM_INITDIALOG(hWndDlg, wParam, lParam, OnInitDialog);

		// 初期データを反映する
		SetDlgData(hWndDlg);

		return bRet;	// WM_INITDIALOGの戻り値は特殊。DWLP_MSGRESULTに格納しなくてもよい。
	}

	return FALSE;
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
LRESULT CDlgInput1::DispatchEvent(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
)
{
	assert(m_hWnd == hWnd);

	m_hWnd = hWnd;

	switch (uMsg) {
// clang-format off
	HANDLE_MSG(hWnd, WM_COMMAND,						OnCommand);
// clang-format on

	case WM_HELP:
		if (const auto p = std::bit_cast<LPHELPINFO>(lParam)) {
			MyWinHelp(HWND(p->hItemHandle), HELP_WM_HELP, ULONG_PTR(p_helpids));
		}
		return 0L;

	case WM_CONTEXTMENU:
		MyWinHelp(hWnd, HELP_CONTEXTMENU, ULONG_PTR(p_helpids));
		return 0L;

	default:
		break;
	}

	//あとはデフォルトに任せる
	return ::DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

/*!
 * @brief ダイアログからクラスデータを更新する
 *
 * @param hWndDlg [in] 宛先ウィンドウのハンドル
 *
 * @return データを取得できたかどうか。
 * @retval > 0 正しく取り込めた
 * @retval = 0 取り込めなかった
 * @retval < 0 入力エラーを検出した
 */
int CDlgInput1::GetDlgData(HWND hWndDlg)
{
	assert(m_hWnd == hWndDlg);

	m_hWnd = hWndDlg;

	const auto result = apiwrap::GetDlgItemTextW(hWndDlg, IDC_EDIT_INPUT1);
	if (!result) return -1;

	const auto ret = m_Func(hWndDlg, result.text, std::size(m_Text));
	if (0 <= ret) {
		::wcsncpy_s(std::data(m_Text), std::size(m_Text), std::data(result.text), _TRUNCATE);
	}

	return ret;
}

/*!
 * @brief クラスデータをダイアログに反映する
 *
 * @param hWndDlg [in] 宛先ウィンドウのハンドル
 */
void CDlgInput1::SetDlgData(HWND hWndDlg) const
{
	apiwrap::SetDlgItemTextW(hWndDlg, IDC_EDIT_INPUT1, std::data(m_Text));
}

/*!
 * @brief WM_INITDIALOGハンドラ
 *
 * @param hWndDlg [in] 宛先ウィンドウのハンドル
 * @param hWndFocus [in, opt] フォーカスを受け取るコントロールのハンドル
 * @param lParam [in] ダイアログ作成時に渡されたパラメーター
 *
 * @return システムが設定した初期フォーカスを承認するかどうか。
 * @retval true  初期フォーカスを変更しない。
 * @retval false 初期フォーカスを変更した。
 */
bool CDlgInput1::OnInitDialog(HWND hWndDlg, HWND, LPARAM)
{
	assert(m_hWnd == hWndDlg);

	m_hWnd = hWndDlg;

	UpdateDialogFont(hWndDlg);

	apiwrap::SetWindowTextW(hWndDlg, m_Title);
	apiwrap::SetDlgItemTextW(hWndDlg, IDC_STATIC_MSG, m_Message);

	apiwrap::LimitEditText(hWndDlg, IDC_EDIT_INPUT1, m_Text);

	return true;
}

/*!
 * @brief WM_COMMANDハンドラ
 *
 * @param hWnd [in] 宛先ウィンドウのハンドル
 * @param id [in] 送信元コントロールのID
 * @param hWndCtl [in, opt] 送信元コントロールのハンドル
 * @param notifyCode [in, opt] 通知コード
 *
 * @note このメッセージに戻り値はありません。
 */
void CDlgInput1::OnCommand(HWND hWnd, int id, HWND, UINT)
{
	assert(m_hWnd == hWnd);

	m_hWnd = hWnd;

	// OKボタンとキャンセルボタンは通知コードに依らず処理させる
	switch(id) {
	case IDOK:
		if (const auto ret = GetDlgData(hWnd);
			0 <= ret)
		{
			::EndDialog(hWnd, static_cast<bool>(ret));
		}
		return;

	case IDCANCEL:
		::EndDialog(hWnd, FALSE);
		return;

	default:
		break;
	}
}
