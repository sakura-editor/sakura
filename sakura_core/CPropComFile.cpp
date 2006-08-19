/*! @file
	@brief 共通設定ダイアログボックス、「書式」ページ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, MIK, jepro
	Copyright (C) 2002, YAZAKI, MIK, aroka, hor
	Copyright (C) 2004, genta, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "stdafx.h"
#include "CPropCommon.h"
#include "debug.h" // 2002/2/10 aroka


//@@@ 2001.02.04 Start by MIK: Popup Help
#if 1	//@@@ 2002.01.03 add MIK
#include "sakura.hh"
static const DWORD p_helpids[] = {	//01310
	IDC_CHECK_EXCVLUSIVE_NO,				HIDC_CHECK_EXCVLUSIVE_NO,				//ファイルの排他制御（排他制御しない）
	IDC_CHECK_bCheckFileTimeStamp,			HIDC_CHECK_bCheckFileTimeStamp,			//更新の監視
	IDC_CHECK_EXCVLUSIVE_WRITE,				HIDC_CHECK_EXCVLUSIVE_WRITE,			//ファイルの排他制御（上書き禁止）
	IDC_CHECK_EXCVLUSIVE_READWRITE,			HIDC_CHECK_EXCVLUSIVE_READWRITE,		//ファイルお排他制御（読み書き禁止）
	IDC_CHECK_ENABLEUNMODIFIEDOVERWRITE,	HIDC_CHECK_ENABLEUNMODIFIEDOVERWRITE,	//無変更でも上書き
	IDC_CHECK_AUTOSAVE,						HIDC_CHECK_AUTOSAVE,					//自動的に保存
	IDC_CHECK_bDropFileAndClose,			HIDC_CHECK_bDropFileAndClose,			//閉じて開く
	IDC_CHECK_RestoreCurPosition,			HIDC_CHECK_RestoreCurPosition,			//カーソル位置の復元
	IDC_CHECK_AutoMIMEDecode,				HIDC_CHECK_AutoMIMEDecode,				//MIMEデコード
	IDC_EDIT_AUTOBACKUP_INTERVAL,			HIDC_EDIT_AUTOBACKUP_INTERVAL,			//自動保存間隔
	IDC_EDIT_nDropFileNumMax,				HIDC_EDIT_nDropFileNumMax,				//ファイルドロップ最大数
	IDC_SPIN_AUTOBACKUP_INTERVAL,			HIDC_EDIT_AUTOBACKUP_INTERVAL,
	IDC_SPIN_nDropFileNumMax,				HIDC_EDIT_nDropFileNumMax,
	IDC_CHECK_RestoreBookmarks,				HIDC_CHECK_RestoreBookmarks,			// 2002.01.16 hor ブックマークの復元
	IDC_CHECK_QueryIfCodeChange,			HIDC_CHECK_QueryIfCodeChange,			//前回と異なる文字コードのとき問い合わせを行う	// 2006.08.06 ryoji
	IDC_CHECK_AlertIfFileNotExist,			HIDC_CHECK_AlertIfFileNotExist,			//開こうとしたファイルが存在しないとき警告する	// 2006.08.06 ryoji
//	IDC_STATIC,								-1,
	0, 0
};
#else
static const DWORD p_helpids[] = {	//01310
	IDC_CHECK_EXCVLUSIVE_NO,				10310,	//ファイルの排他制御（排他制御しない）
	IDC_CHECK_bCheckFileTimeStamp,			10311,	//更新の監視
	IDC_CHECK_EXCVLUSIVE_WRITE,				10312,	//ファイルの排他制御（上書き禁止）
	IDC_CHECK_EXCVLUSIVE_READWRITE,			10313,	//ファイルお排他制御（読み書き禁止）
	IDC_CHECK_ENABLEUNMODIFIEDOVERWRITE,	10314,	//無変更でも上書き
	IDC_CHECK_AUTOSAVE,						10315,	//自動的に保存
	IDC_CHECK_bDropFileAndClose,			10316,	//閉じて開く
	IDC_CHECK_RestoreCurPosition,			10317,	//カーソル位置の復元
	IDC_CHECK_AutoMIMEDecode,				10318,	//MIMEデコード
	IDC_EDIT_AUTOBACKUP_INTERVAL,			10340,	//自動保存間隔
	IDC_EDIT_nDropFileNumMax,				10341,	//ファイルドロップ最大数
	IDC_CHECK_RestoreBookmarks				10342,	// 2002.01.16 hor ブックマークの復元
	IDC_SPIN_AUTOBACKUP_INTERVAL,			-1,
	IDC_SPIN_nDropFileNumMax,				-1,
//	IDC_STATIC,								-1,
	0, 0
};
#endif
//@@@ 2001.02.04 End

//	From Here Jun. 2, 2001 genta
/*!
	@param hwndDlg ダイアログボックスのWindow Handle
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CALLBACK CPropCommon::DlgProc_PROP_FILE(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( &CPropCommon::DispatchEvent_p2, hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

/*! ファイルページ メッセージ処理 */
INT_PTR CPropCommon::DispatchEvent_p2(
	HWND	hwndDlg,	//!< handle to dialog box
	UINT	uMsg,	//!< message
	WPARAM	wParam,	//!< first message parameter
	LPARAM	lParam 	//!< second message parameter
)
{
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
//	int			nVal;
	int			nVal;	//Sept.21, 2000 JEPRO スピン要素を加えたので復活させた
//	char		szFolder[_MAX_PATH];

	switch( uMsg ){
	case WM_INITDIALOG:
		/* ダイアログデータの設定 p2 */
		SetData_p2( hwndDlg );
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

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
//				OnHelp( hwndDlg, IDD_PROP_FILE );
//				return TRUE;
//			case PSN_KILLACTIVE:
////				MYTRACE( "p2 PSN_KILLACTIVE\n" );
//				/* ダイアログデータの取得 p2 */
//				GetData_p2( hwndDlg );
//				return TRUE;
//			}
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
				OnHelp( hwndDlg, IDD_PROP_FILE );
				return TRUE;
			case PSN_KILLACTIVE:
//				MYTRACE( "p2 PSN_KILLACTIVE\n" );
				/* ダイアログデータの取得 p2 */
				GetData_p2( hwndDlg );
				return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
			case PSN_SETACTIVE:
				m_nPageNum = ID_PAGENUM_FILE;
				return TRUE;
			}
		break;
		case IDC_SPIN_nDropFileNumMax:
			/* 一度にドロップ可能なファイル数 */
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_nDropFileNumMax, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 1 ){
				nVal = 1;
			}
			if( nVal > 99 ){
				nVal = 99;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_nDropFileNumMax, nVal, FALSE );
			return TRUE;
//@@@ 2001.03.21 Start by MIK
			/*NOTREACHED*/
//			break;
		case IDC_SPIN_AUTOBACKUP_INTERVAL:
			/* バックアップ間隔 */
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 1 ){
				nVal = 1;
			}
			if( nVal > 35791 ){
				nVal = 35791;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL, nVal, FALSE );
			return TRUE;
			/*NOTREACHED*/
//			break;
//@@@ 2001.03.21 End by MIK
		}
//****	To Here Sept. 21, 2000 JEPRO ダイアログ要素にスピンを入れるのでWM_NOTIFYをコメントアウトにしその下に修正を置いた
		break;

	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	/* 通知コード */
		wID			= LOWORD(wParam);	/* 項目ID､ コントロールID､ またはアクセラレータID */
		hwndCtl		= (HWND) lParam;	/* コントロールのハンドル */
		switch( wNotifyCode ){
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			switch( wID ){
			case IDC_CHECK_EXCVLUSIVE_NO:
			case IDC_CHECK_EXCVLUSIVE_WRITE:
			case IDC_CHECK_EXCVLUSIVE_READWRITE:
			case IDC_CHECK_bDropFileAndClose:/* ファイルをドロップしたときは閉じて開く */
			case IDC_CHECK_AUTOSAVE:
				EnableFilePropInput(hwndDlg);
				break;
			}
			break;
		}
		break;

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			::WinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );
		}
		return TRUE;
		/*NOTREACHED*/
//		break;
//@@@ 2001.02.04 End

//@@@ 2001.12.22 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		::WinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );
		return TRUE;
//@@@ 2001.12.22 End

	}
	return FALSE;
}





/*! ファイルページ: ダイアログデータの設定
	共有メモリからデータを読み出して各コントロールに値を設定する。

	@par バックアップ世代数が妥当な値かどうかのチェックも行う。不適切な値の時は
	最も近い適切な値を設定する。

	@param hwndDlg プロパティページのWindow Handle
*/
void CPropCommon::SetData_p2( HWND hwndDlg )
{
	/*--- p2 ---*/
	/* ファイルの排他制御モード */
	switch( m_Common.m_nFileShareMode ){
	case OF_SHARE_DENY_WRITE:	/* 書き込み禁止 */
		::CheckDlgButton( hwndDlg, IDC_CHECK_EXCVLUSIVE_WRITE, BST_CHECKED );
		break;
	case OF_SHARE_EXCLUSIVE:	/* 読み書き禁止 */
		::CheckDlgButton( hwndDlg, IDC_CHECK_EXCVLUSIVE_READWRITE, BST_CHECKED );
		break;
	case 0:	/* 排他なし */
	default:	/* 排他なし */
		::CheckDlgButton( hwndDlg, IDC_CHECK_EXCVLUSIVE_NO, BST_CHECKED );
		break;
	}
	/* 更新の監視 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_bCheckFileTimeStamp, m_Common.m_bCheckFileTimeStamp );

	/* 無変更でも上書きするか */
	::CheckDlgButton( hwndDlg, IDC_CHECK_ENABLEUNMODIFIEDOVERWRITE, m_Common.m_bEnableUnmodifiedOverwrite );

	/* ファイルをドロップしたときは閉じて開く */
	::CheckDlgButton( hwndDlg, IDC_CHECK_bDropFileAndClose, m_Common.m_bDropFileAndClose );
	/* 一度にドロップ可能なファイル数 */
	::SetDlgItemInt( hwndDlg, IDC_EDIT_nDropFileNumMax, m_Common.m_nDropFileNumMax, FALSE );

	//	From Here Aug. 21, 2000 genta
	//	自動保存の有効・無効
	::CheckDlgButton( hwndDlg, IDC_CHECK_AUTOSAVE, m_Common.IsAutoBackupEnabled() );

	char buf[6];
	int nN;

	nN = m_Common.GetAutoBackupInterval();
	nN = nN < 1  ?  1 : nN;
	nN = nN > 35791 ? 35791 : nN;

	wsprintf( buf, "%d", nN);
	::SetDlgItemText( hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL, buf );
	//	To Here Aug. 21, 2000 genta

	//	Oct. 27, 2000 genta	カーソル位置復元フラグ
	::CheckDlgButton( hwndDlg, IDC_CHECK_RestoreCurPosition, m_Common.GetRestoreCurPosition() );
	// 2002.01.16 hor ブックマーク復元フラグ
	::CheckDlgButton( hwndDlg, IDC_CHECK_RestoreBookmarks, m_Common.GetRestoreBookmarks() );
	//	Nov. 12, 2000 genta	MIME Decodeフラグ
	::CheckDlgButton( hwndDlg, IDC_CHECK_AutoMIMEDecode, m_Common.GetAutoMIMEdecode() );
	//	Oct. 03, 2004 genta 前回と異なる文字コードのときに問い合わせを行うかどうかのフラグ
	::CheckDlgButton( hwndDlg, IDC_CHECK_QueryIfCodeChange, m_Common.GetQueryIfCodeChange() );
	//	Oct. 09, 2004 genta 開こうとしたファイルが存在しないとき警告するかどうかのフラグ
	::CheckDlgButton( hwndDlg, IDC_CHECK_AlertIfFileNotExist, m_Common.GetAlertIfFileNotExist() );

	EnableFilePropInput(hwndDlg);
	return;
}

/*! ファイルページ ダイアログデータの取得
	ダイアログボックスに設定されたデータを共有メモリに反映させる

	@par バックアップ世代数が妥当な値かどうかのチェックも行う。不適切な値の時は
	最も近い適切な値を設定する。

	@param hwndDlg プロパティページのWindow Handle
	@return 常にTRUE
*/
int CPropCommon::GetData_p2( HWND hwndDlg )
{
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
//	m_nPageNum = ID_PAGENUM_FILE;

	/* ファイルの排他制御モード */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_EXCVLUSIVE_NO ) ){	/* 排他なし */
		m_Common.m_nFileShareMode = 0;
	}else
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_EXCVLUSIVE_WRITE ) ){	/* 書き込み禁止 */
		m_Common.m_nFileShareMode = OF_SHARE_DENY_WRITE	;
	}else
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_EXCVLUSIVE_READWRITE ) ){	/* 読み書き禁止 */
		m_Common.m_nFileShareMode = OF_SHARE_EXCLUSIVE;
	}else{
		/* 排他なし */
		m_Common.m_nFileShareMode = 0;
	}
	/* 更新の監視 */
	m_Common.m_bCheckFileTimeStamp = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bCheckFileTimeStamp );

	/* 無変更でも上書きするか */
	m_Common.m_bEnableUnmodifiedOverwrite = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ENABLEUNMODIFIEDOVERWRITE );

	/* ファイルをドロップしたときは閉じて開く */
	m_Common.m_bDropFileAndClose = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bDropFileAndClose );
	/* 一度にドロップ可能なファイル数 */
	m_Common.m_nDropFileNumMax = ::GetDlgItemInt( hwndDlg, IDC_EDIT_nDropFileNumMax, NULL, FALSE );
	if( 1 > m_Common.m_nDropFileNumMax ){
		m_Common.m_nDropFileNumMax = 1;
	}
	if( 99 < m_Common.m_nDropFileNumMax ){	//Sept. 21, 2000, JEPRO 16より大きいときに99と制限されていたのを修正(16→99と変更)
		m_Common.m_nDropFileNumMax = 99;
	}

	//	From Here Aug. 16, 2000 genta
	//	自動保存を行うかどうか
	m_Common.EnableAutoBackup( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_AUTOSAVE ) == TRUE );

	//	自動保存間隔の取得
	char szNumBuf[/*6*/ 7];	//@@@ 2001.03.21 by MIK
	int	 nN;
	char *pDigit;

	::GetDlgItemText( hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL, szNumBuf, /*5*/ 6 );	//@@@ 2001.03.21 by MIK

	for( nN = 0, pDigit = szNumBuf; *pDigit != '\0'; pDigit++ ){
		if( '0' <= *pDigit && *pDigit <= '9' ){
			nN = nN * 10 + *pDigit - '0';
		}
		else
			break;
	}
	nN = nN < 1  ?  1 : nN;
	nN = nN > 35791 ? 35791 : nN;
	m_Common.SetAutoBackupInterval( nN );

	//	To Here Aug. 16, 2000 genta

	//	Oct. 27, 2000 genta	カーソル位置復元フラグ
	m_Common.SetRestoreCurPosition( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_RestoreCurPosition ) == TRUE );
	// 2002.01.16 hor ブックマーク復元フラグ
	m_Common.SetRestoreBookmarks( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_RestoreBookmarks ) == TRUE );
	//	Nov. 12, 2000 genta	MIME Decodeフラグ
	m_Common.SetAutoMIMEdecode( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_AutoMIMEDecode ) == TRUE );
	//	Oct. 03, 2004 genta 前回と異なる文字コードのときに問い合わせを行うかどうかのフラグ
	m_Common.SetQueryIfCodeChange( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_QueryIfCodeChange ) == TRUE );
	//	Oct. 03, 2004 genta 前回と異なる文字コードのときに問い合わせを行うかどうかのフラグ
	m_Common.SetAlertIfFileNotExist( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_AlertIfFileNotExist ) == TRUE );

	return TRUE;
}

//	From Here Aug. 21, 2000 genta
/*!	チェック状態に応じてダイアログボックス要素のEnable/Disableを
	適切に設定する

	@param hwndDlg プロパティシートのWindow Handle
*/
void CPropCommon::EnableFilePropInput(HWND hwndDlg)
{

	//	Drop時の動作
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bDropFileAndClose ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_AUTOSAVE3 ), FALSE );	// added Sept. 6, JEPRO 自動保存にしたときだけEnableになるように変更
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_AUTOSAVE4 ), FALSE );	// added Sept. 6, JEPRO	同上
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_nDropFileNumMax ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_SPIN_nDropFileNumMax ), FALSE );// added Oct. 6, JEPRO ファイルオープンを「閉じて開く」にしたときはDisableになるように変更
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_AUTOSAVE3 ), TRUE );	// added Sept. 6, JEPRO	同上
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_AUTOSAVE4 ), TRUE );	// added Sept. 6, JEPRO	同上
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_nDropFileNumMax ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_SPIN_nDropFileNumMax ), TRUE );// added Oct. 6, JEPRO ファイルオープンを「複数ファイルドロップ」にしたときだけEnableになるように変更
	}

	//	排他するかどうか
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_EXCVLUSIVE_NO ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_bCheckFileTimeStamp ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_bCheckFileTimeStamp ), FALSE );
	}

	//	自動保存
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_AUTOSAVE ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_AUTOSAVE ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_AUTOSAVE2 ), TRUE );	//Sept. 6, 2000 JEPRO 自動保存にしたときだけEnableになるように変更
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_SPIN_AUTOBACKUP_INTERVAL ), TRUE );	//@@@ 2001.03.21 by MIK
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_AUTOSAVE ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_AUTOSAVE2 ), FALSE );	//Sept. 6, 2000 JEPRO 同上
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_SPIN_AUTOBACKUP_INTERVAL ), FALSE );	//@@@ 2001.03.21 by MIK
	}
}
//	To Here Aug. 21, 2000 genta


/*[EOF]*/
