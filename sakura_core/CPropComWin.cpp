//	$Id$
//	Copyright (C) 1998-2000, Norio Nakatani

#include "CPropCommon.h"





/* メッセージ処理 */
BOOL CPropCommon::DispatchEvent_PROP_WIN(
    HWND	hwndDlg,	// handle to dialog box
    UINT	uMsg,	// message
    WPARAM	wParam,	// first message parameter
    LPARAM	lParam 	// second message parameter
)
{
//	WORD		wNotifyCode;
//	WORD		wID;
//	HWND		hwndCtl;

// From Here Sept. 9, 2000 JEPRO
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
// To Here Sept. 9, 2000

	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
//	int			nVal;
	int			nVal;	//Sept.21, 2000 JEPRO スピン要素を加えたので復活させた
//    LPDRAWITEMSTRUCT pDis;

	switch( uMsg ){

	case WM_INITDIALOG:
		/* ダイアログデータの設定 p1 */
		SetData_PROP_WIN( hwndDlg );
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

		/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */
		/* ルーラー高さ */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_nRulerHeight ), EM_LIMITTEXT, (WPARAM)2, 0 );
		/* ルーラーとテキストの隙間 */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_nRulerBottomSpace ), EM_LIMITTEXT, (WPARAM)2, 0 );

		return TRUE;
//****	From Here Sept. 21, 2000 JEPRO ダイアログ要素にスピンを入れるので以下のWM_NOTIFYをコメントアウトにし下に修正を置いた
//	case WM_NOTIFY:
//		idCtrl = (int)wParam;
//		pNMHDR = (NMHDR*)lParam;
//		pMNUD  = (NM_UPDOWN*)lParam;
////		switch( idCtrl ){
////		default:
//			switch( pNMHDR->code ){
//			case PSN_HELP:
//				OnHelp( hwndDlg, IDD_PROP_WIN );
//				return TRUE;
//			case PSN_KILLACTIVE:
////				MYTRACE( "p1 PSN_KILLACTIVE\n" );
//				/* ダイアログデータの取得 p1 */
//				GetData_PROP_WIN( hwndDlg );
//				return TRUE;
//			}
////			break;	//	Sept. 9, 2000 JEPRO この行は下のbreakとダブっていて冗長なので削除してよいはず
////		}
//		break;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch( idCtrl ){
		default:
			switch( pNMHDR->code ){
			case PSN_HELP:
				OnHelp( hwndDlg, IDD_PROP_WIN );
				return TRUE;
			case PSN_KILLACTIVE:
//				MYTRACE( "p1 PSN_KILLACTIVE\n" );
				/* ダイアログデータの取得 p1 */
				GetData_PROP_WIN( hwndDlg );
				return TRUE;
			}
			break;
		case IDC_SPIN_nRulerHeight:
			/* ルーラ－の高さ */
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_nRulerHeight, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 13 ){
				nVal = 13;
			}
			if( nVal > 32 ){
				nVal = 32;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_nRulerHeight, nVal, FALSE );
			return TRUE;
		case IDC_SPIN_nRulerBottomSpace:
			/* ルーラーとテキストの隙間 */
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_nRulerBottomSpace, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 0 ){
				nVal = 0;
			}
			if( nVal > 32 ){
				nVal = 32;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_nRulerBottomSpace, nVal, FALSE );
			return TRUE;
		}
//****	To Here Sept. 21, 2000 JEPRO ダイアログ要素にスピンを入れるので以下のWM_NOTIFYをコメントアウトにし下に修正を置いた
//	From Here Sept. 9, 2000 JEPRO
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* 通知コード */
		wID         = LOWORD(wParam);	/* 項目ID､ コントロールID､ またはアクセラレータID */
		hwndCtl     = (HWND) lParam;	/* コントロールのハンドル */
		switch( wNotifyCode ){
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			switch( wID ){
			//	ファンクションキーを表示する時だけその位置指定をEnableに設定
			case IDC_CHECK_DispFUNCKEYWND:
				EnableWinPropInput( hwndDlg );
				break;
			}
			break;
		}
		break;
//	To Here Sept. 9, 2000

	}
	return FALSE;
}

/* ダイアログデータの設定 */
void CPropCommon::SetData_PROP_WIN( HWND hwndDlg )
{
//	BOOL	bRet;

	/* 次回ウィンドウを開いたときツールバーを表示する */
	::CheckDlgButton( hwndDlg, IDC_CHECK_DispTOOLBAR, m_Common.m_bDispTOOLBAR );

	/* 次回ウィンドウを開いたときファンクションキーを表示する */
	::CheckDlgButton( hwndDlg, IDC_CHECK_DispFUNCKEYWND, m_Common.m_bDispFUNCKEYWND );

	/* ファンクションキー表示位置／0:上 1:下 */
	if( 0 == m_Common.m_nFUNCKEYWND_Place ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1, TRUE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2, FALSE );
	}else{
		::CheckDlgButton( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2, TRUE );
	}

	/* 次回ウィンドウを開いたときステータスバーを表示する */
	::CheckDlgButton( hwndDlg, IDC_CHECK_DispSTATUSBAR, m_Common.m_bDispSTATUSBAR );

	/* ウィンドウサイズ継承 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_WINSIZE, m_Common.m_bSaveWindowSize );

	/* ルーラー高さ */
	::SetDlgItemInt( hwndDlg, IDC_EDIT_nRulerHeight, m_Common.m_nRulerHeight, FALSE );
	/* ルーラーとテキストの隙間 */
	::SetDlgItemInt( hwndDlg, IDC_EDIT_nRulerBottomSpace, m_Common.m_nRulerBottomSpace, FALSE );

	/* ルーラーのタイプ */
	if( 0 == m_Common.m_nRulerType ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_nRulerType_0, TRUE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_nRulerType_1, FALSE );
	}else{
		::CheckDlgButton( hwndDlg, IDC_RADIO_nRulerType_0, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_nRulerType_1, TRUE );
	}

	/* 水平スクロールバー */
	::CheckDlgButton( hwndDlg, IDC_CHECK_bScrollBarHorz, m_Common.m_bScrollBarHorz );

	/* アイコン付きメニュー */
	::CheckDlgButton( hwndDlg, IDC_CHECK_bMenuIcon, m_Common.m_bMenuIcon );


	//	Fronm Here Sept. 9, 2000 JEPRO
	//	ファンクションキーを表示する時だけその位置指定をEnableに設定
	EnableWinPropInput( hwndDlg );
	//	To Here Sept. 9, 2000

	return;
}





/* ダイアログデータの取得 */
int CPropCommon::GetData_PROP_WIN( HWND hwndDlg )
{
	m_nPageNum = ID_PAGENUM_WIN;

	/* 次回ウィンドウを開いたときツールバーを表示する */
	m_Common.m_bDispTOOLBAR = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DispTOOLBAR );
	
	/* 次回ウィンドウを開いたときファンクションキーを表示する */
	m_Common.m_bDispFUNCKEYWND = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DispFUNCKEYWND );

	/* ファンクションキー表示位置／0:上 1:下 */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1 ) ){
		m_Common.m_nFUNCKEYWND_Place = 0;
	}
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2) ){
		m_Common.m_nFUNCKEYWND_Place = 1;
	}

	/* 次回ウィンドウを開いたときステータスバーを表示する */
	m_Common.m_bDispSTATUSBAR = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DispSTATUSBAR );

	/* ウィンドウサイズ継承 */
	m_Common.m_bSaveWindowSize = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_WINSIZE );

	/* ルーラーのタイプ */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_nRulerType_0 ) ){
		m_Common.m_nRulerType = 0;
	}
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_nRulerType_1 ) ){
		m_Common.m_nRulerType = 1;
	}
	
	/* ルーラー高さ */
	m_Common.m_nRulerHeight = ::GetDlgItemInt( hwndDlg, IDC_EDIT_nRulerHeight, NULL, FALSE );
	if( m_Common.m_nRulerHeight < 13 ){
		m_Common.m_nRulerHeight = 13;
	}
	if( m_Common.m_nRulerHeight > 32 ){
		m_Common.m_nRulerHeight = 32;
	}
	/* ルーラーとテキストの隙間 */
	m_Common.m_nRulerBottomSpace = ::GetDlgItemInt( hwndDlg, IDC_EDIT_nRulerBottomSpace, NULL, FALSE );
	if( m_Common.m_nRulerBottomSpace < 0 ){
		m_Common.m_nRulerBottomSpace = 0;
	}
	if( m_Common.m_nRulerBottomSpace > 32 ){
		m_Common.m_nRulerBottomSpace = 32;
	}

	/* 水平スクロールバー */
	m_Common.m_bScrollBarHorz = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bScrollBarHorz );

	/* アイコン付きメニュー */
	m_Common.m_bMenuIcon = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bMenuIcon );





	return TRUE;
}





//	From Here Sept. 9, 2000 JEPRO
//	チェック状態に応じてダイアログボックス要素のEnable/Disableを
//	適切に設定する
void CPropCommon::EnableWinPropInput( HWND hwndDlg )
{
	//	ファクションキーを表示するかどうか
		if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DispFUNCKEYWND ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_GROUP_FUNCKEYWND_POSITION ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1 ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2 ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_GROUP_FUNCKEYWND_POSITION ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1 ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2 ), FALSE );
	}
}
//	To Here Sept. 9, 2000



/*[EOF]*/
