//	$Id$
/*!	@file
	@brief 外部コマンド実行ダイアログ

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "CDlgExec.h"
#include "funccode.h"	//Stonee, 2001/03/12  コメントアウトされてたのを有効にした
#include "sakura_rc.h"
#include "etc_uty.h"	//Stonee, 2001/03/12
#include <windows.h>		//Mar. 28, 2001 JEPRO (一応入れたが不要？)
#include <stdio.h>			//Mar. 28, 2001 JEPRO (一応入れたが不要？)
#include <commctrl.h>		//Mar. 28, 2001 JEPRO
#include "CDlgOpenFile.h"	//Mar. 28, 2001 JEPRO
#include "debug.h"// 2002/2/10 aroka ヘッダ整理

//外部コマンド CDlgExec.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
const DWORD p_helpids[] = {	//12100
	IDC_BUTTON_REFERENCE,			HIDC_EXEC_BUTTON_REFERENCE,		//参照
	IDOK,							HIDOK_EXEC,						//実行
	IDCANCEL,						HIDCANCEL_EXEC,					//キャンセル
	IDC_BUTTON_HELP,				HIDC_EXEC_BUTTON_HELP,			//ヘルプ
	IDC_CHECK_GETSTDOUT,			HIDC_EXEC_CHECK_GETSTDOUT,		//標準出力を得る
	IDC_COMBO_m_szCommand,			HIDC_EXEC_COMBO_m_szCommand,	//コマンド
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

CDlgExec::CDlgExec()
{
	m_szCommand[0] = '\0';	/* コマンドライン */
//	m_bGetStdout = /*FALSE*/TRUE;	// 標準出力を得る	//Mar. 21, 2001 JEPRO [得ない]をデフォルトに変更	//Jul. 03, 2001 JEPRO [得る]がデフォルトとなるように戻した

	return;
}




/* モーダルダイアログの表示 */
int CDlgExec::DoModal( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam )
{
	m_szCommand[0] = '\0';	/* コマンドライン */
	return CDialog::DoModal( hInstance, hwndParent, IDD_EXEC, lParam );
}




/* ダイアログデータの設定 */
void CDlgExec::SetData( void )
{
//	MYTRACE( "CDlgExec::SetData()" );
	int		i;
	HWND	hwndCombo;

	/*****************************
	*           初期             *
	*****************************/
	/* ユーザーがコンボ ボックスのエディット コントロールに入力できるテキストの長さを制限する */
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_m_szCommand ), CB_LIMITTEXT, (WPARAM)sizeof( m_szCommand ) - 1, 0 );
	/* コンボボックスのユーザー インターフェイスを拡張インターフェースにする */
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_m_szCommand ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );


	// 標準出力を得る
//	From Here Sept. 12, 2000 jeprotest
//@@@ 2002.01.08 YAZAKI 設定を保存するためにShareDataに移動
	::CheckDlgButton( m_hWnd, IDC_CHECK_GETSTDOUT, m_pShareData->m_bGetStdout/*m_bGetStdout*/ ? BST_CHECKED : BST_UNCHECKED );
//	::CheckDlgButton( m_hWnd, IDC_CHECK_GETSTDOUT, TRUE );
//	To Here Sept. 12, 2000 	うまくいかないので元に戻してある


	/*****************************
	*         データ設定         *
	*****************************/
//	HWND hwndCombo;
	strcpy( m_szCommand, m_pShareData->m_szCmdArr[0] );
	hwndCombo = ::GetDlgItem( m_hWnd, IDC_COMBO_m_szCommand );
	::SendMessage( hwndCombo, CB_RESETCONTENT, 0, 0 );
	::SetDlgItemText( m_hWnd, IDC_COMBO_TEXT, m_szCommand );
	for( i = 0; i < m_pShareData->m_nCmdArrNum; ++i ){
		::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM)m_pShareData->m_szCmdArr[i] );
	}
	::SendMessage( hwndCombo, CB_SETCURSEL, 0, 0 );
	return;
}




/* ダイアログデータの取得 */
int CDlgExec::GetData( void )
{
	::GetDlgItemText( m_hWnd, IDC_COMBO_m_szCommand, m_szCommand, sizeof( m_szCommand ));

	// 標準出力を得る
//	From Here Sept. 12, 2000 jeprotest
	if( BST_CHECKED == ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_GETSTDOUT ) ){
//	if( ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_GETSTDOUT ) ){
//	To Here Sept. 12, 2000 うまくいかないので元に戻してある
//@@@ 2002.01.08 YAZAKI 設定を保存するためにShareDataに移動
		 m_pShareData->m_bGetStdout = TRUE;
	}else{
		 m_pShareData->m_bGetStdout = FALSE;
	}
	return 1;
}



BOOL CDlgExec::OnBnClicked( int wID )
{
//	int	nRet;
//	CEditView*	pcEditView = (CEditView*)m_lParam;
	switch( wID ){
	//	From Here Sept. 12, 2000 jeprotest
	case IDC_CHECK_GETSTDOUT:
//@@@ 2002.01.08 YAZAKI GetData()で取得するため
#if 0
//		if( BST_CHECKED == ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_GETSTDOUT ) ){
		if( ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_GETSTDOUT ) ){
			 m_pShareData->m_bGetStdout = TRUE;
		}else{
			 m_pShareData->m_bGetStdout = FALSE;
		}
#endif
		break;
	//	To Here Sept. 12, 2000 うまくいかないので元に戻してある
	case IDC_BUTTON_HELP:
		/* 「検索」のヘルプ */
		//Stonee, 2001/03/12 第四引数を、機能番号からヘルプトピック番号を調べるようにした
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_EXECCOMMAND_DIALOG) );
		break;

	//From Here Mar. 28, 2001 JEPRO
	case IDC_BUTTON_REFERENCE:	/* ファイル名の「参照...」ボタン */
		{
			CDlgOpenFile	cDlgOpenFile;
			char*			pszMRU = NULL;;
			char*			pszOPENFOLDER = NULL;;
			char			szPath[_MAX_PATH + 1];
			strcpy( szPath, m_szCommand );
			/* ファイルオープンダイアログの初期化 */
			cDlgOpenFile.Create(
				m_hInstance,
				m_hWnd,
				"*.com;*.exe;*.bat",
				m_szCommand,
				(const char **)&pszMRU,
				(const char **)&pszOPENFOLDER
			);
			if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
				strcpy( m_szCommand, szPath );
				::SetDlgItemText( m_hWnd, IDC_COMBO_m_szCommand, m_szCommand );
			}
		}
		return TRUE;
	//To Here Mar. 28, 2001

	case IDOK:			/* 下検索 */
		/* ダイアログデータの取得 */
		GetData();
		CloseDialog( 1 );
		return TRUE;
	case IDCANCEL:
		CloseDialog( 0 );
		return TRUE;
	}
	return FALSE;
}

//@@@ 2002.01.18 add start
LPVOID CDlgExec::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

/*[EOF]*/
