#pragma once

#include "doc/CDocListener.h"

class CEditWnd;

class CMainStatusBar : public CDocListenerEx{
public:
	//作成・破棄
	CMainStatusBar(CEditWnd* pOwner);
	void CreateStatusBar();		// ステータスバー作成
	void DestroyStatusBar();		/* ステータスバー破棄 */
	void SendStatusMessage2( const TCHAR* msg );	//	Jul. 9, 2005 genta メニューバー右端には出したくない長めのメッセージを出す
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
	void SetStatusText(int nIndex, int nOption, const TCHAR* pszText);
private:
	CEditWnd*	m_pOwner;
	HWND		m_hwndStatusBar;
	HWND		m_hwndProgressBar;
};

