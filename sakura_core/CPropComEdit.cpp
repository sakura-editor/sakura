//	$Id$
//	Copyright (C) 1998-2000, Norio Nakatani

#include "CPropCommon.h"





/* メッセージ処理 */
BOOL CPropCommon::DispatchEvent_PROP_EDIT(
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
//    LPDRAWITEMSTRUCT pDis;

	switch( uMsg ){

	case WM_INITDIALOG:
		/* ダイアログデータの設定 p1 */
		SetData_PROP_EDIT( hwndDlg );
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

		/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */

		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* 通知コード */
		wID         = LOWORD(wParam);	/* 項目ID､ コントロールID､ またはアクセラレータID */
		hwndCtl     = (HWND) lParam;	/* コントロールのハンドル */
		switch( wNotifyCode ){
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			switch( wID ){
			case IDC_CHECK_DRAGDROP:	/* タスクトレイを使う */
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DRAGDROP ) ){
//	From Here Sept. 9, 2000 JEPRO
//	前のチェック状態が残るように次の行をコメントアウトに変更
//					::CheckDlgButton( hwndDlg, IDC_CHECK_DROPSOURCE, TRUE );	/* DropSource */
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DROPSOURCE ), TRUE );
//	To Here Sept. 9, 2000
				}else{
//	From Here Sept. 9, 2000 JEPRO
//	前のチェック状態が残るように次の行をコメントアウトに変更
//					::CheckDlgButton( hwndDlg, IDC_CHECK_DROPSOURCE, FALSE );	/* DropSource */
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DROPSOURCE ), FALSE );
//	To Here Sept. 9, 2000
				}
				return TRUE;
			}
			break;
		}
		break;
				
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
//		switch( idCtrl ){
//		default:
			switch( pNMHDR->code ){
			case PSN_HELP:
				OnHelp( hwndDlg, IDD_PROP_EDIT );
				return TRUE;
			case PSN_KILLACTIVE:
#ifdef _DEBUG
				MYTRACE( "p1 PSN_KILLACTIVE\n" );
#endif
				/* ダイアログデータの取得 p1 */
				GetData_PROP_EDIT( hwndDlg );
				return TRUE;
			}
			break;
//		}
		break;
	}
	return FALSE;
}


/* ダイアログデータの設定 */
void CPropCommon::SetData_PROP_EDIT( HWND hwndDlg )
{
//	BOOL	bRet;

	/* Drag && Drop編集 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_DRAGDROP, m_Common.m_bUseOLE_DragDrop );
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DRAGDROP ) ){
//	From Here Sept. 9, 2000 JEPRO
//	前のチェック状態が残るように次の行をコメントアウトに変更
//		::CheckDlgButton( hwndDlg, IDC_CHECK_DROPSOURCE, TRUE );	/* DropSource */
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DROPSOURCE ), TRUE );
//	To Here Sept. 9, 2000
	}else{
//	From Here Sept. 9, 2000 JEPRO
//	前のチェック状態が残るように次の行をコメントアウトに変更
//		::CheckDlgButton( hwndDlg, IDC_CHECK_DROPSOURCE, FALSE );	/* DropSource */
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DROPSOURCE ), FALSE );
//	To Here Sept. 9, 2000
	}

	/* DropSource */
	::CheckDlgButton( hwndDlg, IDC_CHECK_DROPSOURCE, m_Common.m_bUseOLE_DropSource );

	/* 折り返し行に改行を付けてコピー */
	::CheckDlgButton( hwndDlg, IDC_CHECK_ADDCRLFWHENCOPY, m_Common.m_bAddCRLFWhenCopy );
	
	/* コピーしたら選択解除 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_COPYnDISABLESELECTEDAREA, m_Common.m_bCopyAndDisablSelection );

	/* 改行は上書きしない */
	::CheckDlgButton( hwndDlg, IDC_CHECK_bNotOverWriteCRLF, m_Common.m_bNotOverWriteCRLF );
	return;
}








/* ダイアログデータの取得 */
int CPropCommon::GetData_PROP_EDIT( HWND hwndDlg )
{
	m_nPageNum = ID_PAGENUM_EDIT;

	/* Drag && Drop編集 */
	m_Common.m_bUseOLE_DragDrop = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DRAGDROP );
	/* DropSource */
	m_Common.m_bUseOLE_DropSource = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DROPSOURCE );

	/* 折り返し行に改行を付けてコピー */
	m_Common.m_bAddCRLFWhenCopy = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ADDCRLFWHENCOPY );

	/* コピーしたら選択解除 */
	m_Common.m_bCopyAndDisablSelection = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_COPYnDISABLESELECTEDAREA );
	
	/* 改行は上書きしない */
	m_Common.m_bNotOverWriteCRLF = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bNotOverWriteCRLF );
	return TRUE;
}
