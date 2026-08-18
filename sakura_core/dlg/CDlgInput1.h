/*!	@file
	@brief 1行入力ダイアログボックス

	@author Norio Nakatani
	@date	1998/05/31 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#ifndef SAKURA_CDLGINPUT1_43CB765B_D257_4DBC_85E9_D2587B7E9D8E_H_
#define SAKURA_CDLGINPUT1_43CB765B_D257_4DBC_85E9_D2587B7E9D8E_H_
#pragma once

#include "util/design_template.h"

#include <span>
#include <string>
#include <string_view>

class CDlgInput1;

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
 * @brief １行入力ダイアログボックス
 */
class CDlgInput1 : public TSakuraSingleton<CDlgInput1>
{
private:
	using SBuffer = std::span<WCHAR>;
	using SFuncType = std::function<int(HWND, std::wstring_view, size_t)>;

	using Me = CDlgInput1;

public:
	static INT_PTR CALLBACK DlgProc(
		HWND hWndDlg,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam
	);

	static int NoValidation(HWND hWndDlg, std::wstring_view text, size_t cchBuffer);

	static LRESULT CALLBACK SubclassProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		UINT_PTR uIdSubclass,
		DWORD_PTR dwRefData
	);

	virtual ~CDlgInput1() noexcept = default;

	BOOL DoModal(
		_Reserved_ HINSTANCE unusedArg1 [[maybe_unused]],
		_In_opt_ HWND hWndOwner,
		_In_z_ const WCHAR* pszTitle,
		_In_z_ const WCHAR* pszMessage,
		size_t cchMaxTextLength,
		_Inout_updates_z_(cchMaxTextLength + 1) WCHAR* pszText
	);

	BOOL DoModal(
		_In_opt_ HWND hWndOwner,
		std::wstring_view title,
		std::wstring_view message,
		std::span<WCHAR> buffer,
		const std::optional<SFuncType>& optFunc = std::nullopt
	);

	BOOL DoModal(
		_In_opt_ HWND hWndOwner,
		std::wstring_view title,
		std::wstring_view message,
		std::wstring& buffer,
		const std::optional<SFuncType>& optFunc = std::nullopt
	);

	virtual BOOL DoModal(
		_In_opt_ HWND hWndOwner,
		_In_z_ LPCWSTR pszTitle,
		_In_z_ LPCWSTR pszMessage,
		_Inout_updates_z_(cchBuffer) LPWSTR pBuffer,
		size_t cchBuffer
	);

	INT_PTR	DispatchDlgEvent(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT	DispatchEvent(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	int		GetDlgData(HWND hWndDlg);
	void	SetDlgData(HWND hWndDlg) const;

	bool	OnInitDialog(HWND hWndDlg, HWND hWndFocus, LPARAM lParam);
	void	OnCommand(HWND hWnd, int id, HWND hWndCtl, UINT notifyCode);

	HWND			m_hWnd;		/* このダイアログのハンドル */

	std::wstring	m_Title;	/* ダイアログタイトル */
	std::wstring	m_Message;	/* メッセージ */
	SBuffer			m_Text;		/* テキスト */

	SFuncType		m_Func = &NoValidation;
};

#endif /* SAKURA_CDLGINPUT1_43CB765B_D257_4DBC_85E9_D2587B7E9D8E_H_ */
