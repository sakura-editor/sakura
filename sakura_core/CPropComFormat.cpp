//	$Id$
/*!	@file
	共通設定ダイアログボックス、「書式」ページ

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "CPropCommon.h"
#include "etc_uty.h"


//@@@ 2001.02.04 Start by MIK: Popup Help
const DWORD p_helpids[] = {	//10400
	IDC_EDIT_DFORM,						10440,	//日付書式
	IDC_EDIT_TFORM,						10441,	//時刻書式
	IDC_EDIT_DFORM_EX,					10442,	//日付書式（表示例）
	IDC_EDIT_TFORM_EX,					10443,	//時刻書式（表示例）
	IDC_EDIT_MIDASHIKIGOU,				10444,	//見出し記号
	IDC_EDIT_INYOUKIGOU,				10445,	//引用符
	IDC_RADIO_DFORM_0,					10460,	//日付書式（標準）
	IDC_RADIO_DFORM_1,					10461,	//日付書式（カスタム）
	IDC_RADIO_TFORM_0,					10462,	//時刻書式（標準）
	IDC_RADIO_TFORM_1,					10463,	//時刻書式（カスタム）
//	IDC_STATIC,							-1,
	0, 0
};
//@@@ 2001.02.04 End

//	From Here Jun. 2, 2001 genta
/*!
	@param hwndDlg ダイアログボックスのWindow Handle
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
BOOL CALLBACK CPropCommon::DlgProc_PROP_FORMAT(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( DispatchEvent_p9, hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

void CPropCommon::ChangeDateExample( HWND hwndDlg )
{
	/* ダイアログデータの取得 p9 */
	GetData_p9( hwndDlg );

	/* 日付をフォーマット */
	char szText[1024];
	::MyGetDateFormat( szText, sizeof( szText ) - 1, m_Common.m_nDateFormatType, m_Common.m_szDateFormat );
	::SetDlgItemText( hwndDlg, IDC_EDIT_DFORM_EX, szText );
	return;
}
void CPropCommon::ChangeTimeExample( HWND hwndDlg )
{
	/* ダイアログデータの取得 p9 */
	GetData_p9( hwndDlg );

	/* 時刻をフォーマット */
	char szText[1024];
	::MyGetTimeFormat( szText, sizeof( szText ) - 1, m_Common.m_nTimeFormatType, m_Common.m_szTimeFormat );
	::SetDlgItemText( hwndDlg, IDC_EDIT_TFORM_EX, szText );
	return;
}


/* p9 メッセージ処理 */
BOOL CPropCommon::DispatchEvent_p9(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,	// message
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
//	int			nVal;

	switch( uMsg ){
	case WM_INITDIALOG:
		/* ダイアログデータの設定 p9 */
		SetData_p9( hwndDlg );
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

		ChangeDateExample( hwndDlg );
		ChangeTimeExample( hwndDlg );


		/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */
//		/* 外部ヘルプ１ */
//		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_EXTHELP1 ), EM_LIMITTEXT, (WPARAM)(_MAX_PATH - 1 ), 0 );
//		/* 外部HTMLヘルプ */
//		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_EXTHTMLHELP ), EM_LIMITTEXT, (WPARAM)(_MAX_PATH - 1 ), 0 );


		/* 見出し記号 */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_MIDASHIKIGOU ), EM_LIMITTEXT, (WPARAM)(sizeof(m_Common.m_szMidashiKigou) - 1 ), 0 );

		/* 引用符 */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_INYOUKIGOU ), EM_LIMITTEXT, (WPARAM)(sizeof(m_Common.m_szInyouKigou) - 1 ), 0 );

		/* 日付書式 */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_DFORM ), EM_LIMITTEXT, (WPARAM)(sizeof(m_Common.m_szDateFormat) - 1 ), 0 );

		/* 時刻書式 */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_TFORM ), EM_LIMITTEXT, (WPARAM)(sizeof(m_Common.m_szTimeFormat) - 1 ), 0 );



		return TRUE;
	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	/* 通知コード */
		wID			= LOWORD(wParam);	/* 項目ID､ コントロールID､ またはアクセラレータID */
		hwndCtl		= (HWND) lParam;	/* コントロールのハンドル */
		switch( wNotifyCode ){
		case EN_CHANGE:
			if( IDC_EDIT_DFORM == wID ){
				ChangeDateExample( hwndDlg );
				return 0;
			}
			if( IDC_EDIT_TFORM == wID  ){
				ChangeTimeExample( hwndDlg );
				return 0;
			}
			break;

		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			switch( wID ){
			case IDC_RADIO_DFORM_0:
			case IDC_RADIO_DFORM_1:
				ChangeDateExample( hwndDlg );
			//	From Here Sept. 10, 2000 JEPRO
			//	日付書式 0=標準 1=カスタム
			//	日付書式をカスタムにするときだけ書式指定文字入力をEnableに設定
				EnableFormatPropInput( hwndDlg );
			//	To Here Sept. 10, 2000
				return 0;
			case IDC_RADIO_TFORM_0:
			case IDC_RADIO_TFORM_1:
				ChangeTimeExample( hwndDlg );
			//	From Here Sept. 10, 2000 JEPRO
			//	時刻書式 0=標準 1=カスタム
			//	時刻書式をカスタムにするときだけ書式指定文字入力をEnableに設定
				EnableFormatPropInput( hwndDlg );
			//	To Here Sept. 10, 2000
				return 0;





//			case IDC_BUTTON_OPENHELP1:	/* 外部ヘルプ１の「参照...」ボタン */
//				{
//					CDlgOpenFile	cDlgOpenFile;
//					char*			pszMRU = NULL;;
//					char*			pszOPENFOLDER = NULL;;
//					char			szPath[_MAX_PATH + 1];
//					strcpy( szPath, m_Common.m_szExtHelp1 );
//					/* ファイルオープンダイアログの初期化 */
//					cDlgOpenFile.Create(
//						m_hInstance,
//						hwndDlg,
//						"*.hlp",
//						m_Common.m_szExtHelp1,
//						(const char **)&pszMRU,
//						(const char **)&pszOPENFOLDER
//					);
//					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
//						strcpy( m_Common.m_szExtHelp1, szPath );
//						::SetDlgItemText( hwndDlg, IDC_EDIT_EXTHELP1, m_Common.m_szExtHelp1 );
//					}
//				}
//				return TRUE;
//			case IDC_BUTTON_OPENEXTHTMLHELP:	/* 外部HTMLヘルプの「参照...」ボタン */
//				{
//					CDlgOpenFile	cDlgOpenFile;
//					char*			pszMRU = NULL;;
//					char*			pszOPENFOLDER = NULL;;
//					char			szPath[_MAX_PATH + 1];
//					strcpy( szPath, m_Common.m_szExtHtmlHelp );
//					/* ファイルオープンダイアログの初期化 */
//					cDlgOpenFile.Create(
//						m_hInstance,
//						hwndDlg,
//						"*.chm;*.col",
//						m_Common.m_szExtHtmlHelp,
//						(const char **)&pszMRU,
//						(const char **)&pszOPENFOLDER
//					);
//					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
//						strcpy( m_Common.m_szExtHtmlHelp, szPath );
//						::SetDlgItemText( hwndDlg, IDC_EDIT_EXTHTMLHELP, m_Common.m_szExtHtmlHelp );
//					}
//				}
//				return TRUE;
//
			}
		}
		break;
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
//		switch( idCtrl ){
//		case ???????:
//			return 0L;
//		default:
			switch( pNMHDR->code ){
			case PSN_HELP:
				OnHelp( hwndDlg, IDD_PROP_FORMAT );
				return TRUE;
			case PSN_KILLACTIVE:
//				MYTRACE( "p9 PSN_KILLACTIVE\n" );
				/* ダイアログデータの取得 p9 */
				GetData_p9( hwndDlg );
				return TRUE;
			}
			break;
//		}

//		MYTRACE( "pNMHDR->hwndFrom=%xh\n", pNMHDR->hwndFrom );
//		MYTRACE( "pNMHDR->idFrom  =%xh\n", pNMHDR->idFrom );
//		MYTRACE( "pNMHDR->code    =%xh\n", pNMHDR->code );
//		MYTRACE( "pMNUD->iPos    =%d\n", pMNUD->iPos );
//		MYTRACE( "pMNUD->iDelta  =%d\n", pMNUD->iDelta );
		break;

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			::WinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (DWORD)(LPVOID)p_helpids );
		}
		return TRUE;
		/*NOTREACHED*/
		break;
//@@@ 2001.02.04 End

	}
	return FALSE;
}





/* ダイアログデータの設定 p9 */
void CPropCommon::SetData_p9( HWND hwndDlg )
{

	/* 見出し記号 */
	::SetDlgItemText( hwndDlg, IDC_EDIT_MIDASHIKIGOU, m_Common.m_szMidashiKigou );

//	/* 外部ヘルプ１ */
//	::SetDlgItemText( hwndDlg, IDC_EDIT_EXTHELP1, m_Common.m_szExtHelp1 );
//
//	/* 外部HTMLヘルプ */
//	::SetDlgItemText( hwndDlg, IDC_EDIT_EXTHTMLHELP, m_Common.m_szExtHtmlHelp );

	/* 引用符 */
	::SetDlgItemText( hwndDlg, IDC_EDIT_INYOUKIGOU, m_Common.m_szInyouKigou );


	//日付書式のタイプ
	if( 0 == m_Common.m_nDateFormatType ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_DFORM_0, BST_CHECKED );
	}else{
		::CheckDlgButton( hwndDlg, IDC_RADIO_DFORM_1, BST_CHECKED );
	}
	//日付書式
	::SetDlgItemText( hwndDlg, IDC_EDIT_DFORM, m_Common.m_szDateFormat );

	//時刻書式のタイプ
	if( 0 == m_Common.m_nTimeFormatType ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_TFORM_0, BST_CHECKED );
	}else{
		::CheckDlgButton( hwndDlg, IDC_RADIO_TFORM_1, BST_CHECKED );
	}
	//時刻書式
	::SetDlgItemText( hwndDlg, IDC_EDIT_TFORM, m_Common.m_szTimeFormat );

	//	From Here Sept. 10, 2000 JEPRO
	//	日付/時刻書式 0=標準 1=カスタム
	//	日付/時刻書式をカスタムにするときだけ書式指定文字入力をEnableに設定
	EnableFormatPropInput( hwndDlg );
	//	To Here Sept. 10, 2000

	return;
}




/* ダイアログデータの取得 p9 */
int CPropCommon::GetData_p9( HWND hwndDlg )
{
	m_nPageNum = ID_PAGENUM_FORMAT;

	/* 見出し記号 */
	::GetDlgItemText( hwndDlg, IDC_EDIT_MIDASHIKIGOU, m_Common.m_szMidashiKigou, sizeof(m_Common.m_szMidashiKigou) );

//	/* 外部ヘルプ１ */
//	::GetDlgItemText( hwndDlg, IDC_EDIT_EXTHELP1, m_Common.m_szExtHelp1, MAX_PATH - 1 );
//
//	/* 外部HTMLヘルプ */
//	::GetDlgItemText( hwndDlg, IDC_EDIT_EXTHTMLHELP, m_Common.m_szExtHtmlHelp, MAX_PATH - 1 );

	/* 引用符 */
	::GetDlgItemText( hwndDlg, IDC_EDIT_INYOUKIGOU, m_Common.m_szInyouKigou, sizeof(m_Common.m_szInyouKigou) );


	//日付書式のタイプ
	if( BST_CHECKED == ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_DFORM_0 ) ){
		m_Common.m_nDateFormatType = 0;
	}else{
		m_Common.m_nDateFormatType = 1;
	}
	//日付書式
	::GetDlgItemText( hwndDlg, IDC_EDIT_DFORM, m_Common.m_szDateFormat, sizeof( m_Common.m_szDateFormat ) - 1 );

	//時刻書式のタイプ
	if( BST_CHECKED == ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_TFORM_0 ) ){
		m_Common.m_nTimeFormatType = 0;
	}else{
		m_Common.m_nTimeFormatType = 1;
	}

	//時刻書式
	::GetDlgItemText( hwndDlg, IDC_EDIT_TFORM, m_Common.m_szTimeFormat, sizeof( m_Common.m_szTimeFormat ) - 1 );












	return TRUE;
}





//	From Here Sept. 10, 2000 JEPRO
//	チェック状態に応じてダイアログボックス要素のEnable/Disableを
//	適切に設定する
void CPropCommon::EnableFormatPropInput( HWND hwndDlg )
{
	//	日付書式をカスタムにするかどうか
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_DFORM_1 ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_DFORM ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_DFORM ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_DFORM ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_DFORM ), FALSE );
	}

	//	時刻書式をカスタムにするかどうか
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_TFORM_1 ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_TFORM ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_TFORM ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_TFORM ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_TFORM ), FALSE );
	}
}
//	To Here Sept. 10, 2000


/*[EOF]*/
