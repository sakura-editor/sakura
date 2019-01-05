/*!	@file
	@brief 文字コードセット設定ダイアログボックス

	@author Uchi
	@date 2010/6/14  新規作成
*/
/*
	Copyright (C) 2010, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CDLGSETCHARSET_H_
#define _CDLGSETCHARSET_H_

#include "dlg/CDialog.h"

//! 文字コードセット設定ダイアログボックス
class CDlgSetCharSet : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgSetCharSet();
	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE hInstance, HWND hwndParent, ECodeType* pnCharSet, bool* pbBom );	/* モーダルダイアログの表示 */

	ECodeType*	m_pnCharSet;			// 文字コードセット
	bool*		m_pbBom;				// BOM
	bool		m_bCP;

	HWND		m_hwndCharSet;
	HWND		m_hwndCheckBOM;

protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL	OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
	BOOL	OnBnClicked(int wID);
	BOOL	OnCbnSelChange(HWND hwndCtl, int wID);
	LPVOID	GetHelpIdTable( void );

	void	SetData( void );	/* ダイアログデータの設定 */
	int 	GetData( void );	/* ダイアログデータの取得 */

	void	SetBOM( void );		// BOM の設定
};

///////////////////////////////////////////////////////////////////////
#endif /* _CDLGSETCHARSET_H_ */
