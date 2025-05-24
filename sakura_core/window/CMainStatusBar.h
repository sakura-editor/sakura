/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CMAINSTATUSBAR_E2FC11D7_4513_4F96_BDCC_E9B278ED0718_H_
#define SAKURA_CMAINSTATUSBAR_E2FC11D7_4513_4F96_BDCC_E9B278ED0718_H_
#pragma once

#include "doc/CDocListener.h"

class CEditWnd;

class CMainStatusBar : public CDocListenerEx{
public:
	//作成・破棄
	CMainStatusBar(CEditWnd* pOwner);
	void CreateStatusBar();		// ステータスバー作成
	void DestroyStatusBar();		/* ステータスバー破棄 */
	void SendStatusMessage2( const WCHAR* msg );	//	Jul. 9, 2005 genta メニューバー右端には出したくない長めのメッセージを出す
	/*!	SendStatusMessage2()が効き目があるかを予めチェック
		@date 2005.07.09 genta
		@note もしSendStatusMessage2()でステータスバー表示以外の処理を追加
		する場合にはここを変更しないと新しい場所への出力が行われない．
		
		@sa SendStatusMessage2
	*/
	bool SendStatusMessage2IsEffective() const
	{
		return NULL != m_hwndStatusBar;
	}

	//取得
	HWND GetStatusHwnd() const{ return m_hwndStatusBar; }
	HWND GetProgressHwnd() const{ return m_hwndProgressBar; }

	//設定
	bool SetStatusText(int nIndex, int nOption, const WCHAR* pszText, size_t textLen = SIZE_MAX);
	void ShowProgressBar(bool bShow) const;
private:
	CEditWnd*	m_pOwner;
	HWND		m_hwndStatusBar;
	HWND		m_hwndProgressBar;
};
#endif /* SAKURA_CMAINSTATUSBAR_E2FC11D7_4513_4F96_BDCC_E9B278ED0718_H_ */
