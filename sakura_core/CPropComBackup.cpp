//	$Id$
//	Copyright (C) 1998-2000, Norio Nakatani
#include "CPropCommon.h"

#include "etc_uty.h"

//@@@ 2001.02.04 Start by MIK: Popup Help
const DWORD p_helpids[] = {	//10000
	IDC_BUTTON_BACKUP_FOLDER_REF,	10000,	//バックアップフォルダ参照
	IDC_CHECK_BACKUP,				10010,	//バックアップの作成
	IDC_CHECK_BACKUP_YEAR,			10011,	//バックアップファイル名（西暦年）
	IDC_CHECK_BACKUP_MONTH,			10012,	//バックアップファイル名（月）
	IDC_CHECK_BACKUP_DAY,			10013,	//バックアップファイル名（日）
	IDC_CHECK_BACKUP_HOUR,			10014,	//バックアップファイル名（時）
	IDC_CHECK_BACKUP_MIN,			10015,	//バックアップファイル名（分）
	IDC_CHECK_BACKUP_SEC,			10016,	//バックアップファイル名（秒）
	IDC_CHECK_BACKUPDIALOG,			10017,	//作成前に確認
	IDC_CHECK_BACKUPFOLDER,			10018,	//指定フォルダに作成
	IDC_EDIT_BACKUPFOLDER,			10040,	//保存フォルダ名
	IDC_EDIT_BACKUP_3,				10041,	//世代数
	IDC_RADIO_BACKUP_TYPE1,			10060,	//バックアップの種類（拡張子）
	IDC_RADIO_BACKUP_TYPE2,			10061,	//バックアップの種類（連番）
	IDC_RADIO_BACKUP_TYPE3,			10062,	//バックアップの種類（日付・時刻）
	IDC_SPIN_BACKUP_GENS,			-1,
//	IDC_STATIC,						-1,
	0, 0
};
//@@@ 2001.02.04 End



/* メッセージ処理 */
BOOL CPropCommon::DispatchEvent_PROP_BACKUP( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
//	int			nVal;
	int			nVal;	//Sept.21, 2000 JEPRO スピン要素を加えたので復活させた
	char		szFolder[_MAX_PATH];
//	int			nDummy;
//	int			nCharChars;

	switch( uMsg ){

	case WM_INITDIALOG:
		/* ダイアログデータの設定 p1 */
		SetData_PROP_BACKUP( hwndDlg );
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

		/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */
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
//				OnHelp( hwndDlg, IDD_PROP_BACKUP );
//				return TRUE;
//			case PSN_KILLACTIVE:
//				/* ダイアログデータの取得 p1 */
//				GetData_PROP_BACKUP( hwndDlg );
//				return TRUE;
//			}
//			break;
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
				OnHelp( hwndDlg, IDD_PROP_BACKUP );
				return TRUE;
			case PSN_KILLACTIVE:
				/* ダイアログデータの取得 p1 */
				GetData_PROP_BACKUP( hwndDlg );
				return TRUE;
			}
			break;

		case IDC_SPIN_BACKUP_GENS:
			/* バックアップファイルの世代数 */
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_BACKUP_3, NULL, FALSE );
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
			::SetDlgItemInt( hwndDlg, IDC_EDIT_BACKUP_3, nVal, FALSE );
			return TRUE;
		}
//****	To Here Sept. 21, 2000 JEPRO ダイアログ要素にスピンを入れるので以下のWM_NOTIFYをコメントアウトにし下に修正を置いた
		

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* 通知コード */
		wID         = LOWORD(wParam);	/* 項目ID､ コントロールID､ またはアクセラレータID */
		hwndCtl     = (HWND) lParam;	/* コントロールのハンドル */
		switch( wNotifyCode ){
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			switch( wID ){
			case IDC_RADIO_BACKUP_TYPE1:
			case IDC_RADIO_BACKUP_TYPE2:
				//	Aug. 16, 2000 genta
				//	バックアップ方式追加
			case IDC_RADIO_BACKUP_TYPE3:
			case IDC_CHECK_BACKUP:
			case IDC_CHECK_BACKUPFOLDER:
				//	Aug. 21, 2000 genta
			case IDC_CHECK_AUTOSAVE:
				EnableBackupInput(hwndDlg);
				return TRUE;
			case IDC_BUTTON_BACKUP_FOLDER_REF:	/* フォルダ参照 */
//				strcpy( szFolder, m_Common.m_szBackUpFolder );
				/* バックアップを作成するフォルダ */
				::GetDlgItemText( hwndDlg, IDC_EDIT_BACKUPFOLDER, szFolder, MAX_PATH - 1 );

				if( SelectDir( hwndDlg, "バックアップを作成するフォルダを選んでください", (const char *)szFolder, (char *)szFolder ) ){
					strcpy( m_Common.m_szBackUpFolder, szFolder );
					::SetDlgItemText( hwndDlg, IDC_EDIT_BACKUPFOLDER, m_Common.m_szBackUpFolder );
				}
				return TRUE;
			}
		}
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


/* ダイアログデータの設定 */
void CPropCommon::SetData_PROP_BACKUP( HWND hwndDlg )
{
//	BOOL	bRet;

//	BOOL				m_bGrepExitConfirm;	/* Grepモードで保存確認するか */


	/* バックアップの作成 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUP, m_Common.m_bBackUp );
	/* バックアップの作成前に確認 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUPDIALOG, m_Common.m_bBackUpDialog );
	/* 指定フォルダにバックアップを作成する */
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUPFOLDER, m_Common.m_bBackUpFolder );

	/* バックアップファイル名のタイプ 1=(.bak) 2=*_日付.* */
	switch( m_Common.GetBackupType()){
	case 2:
		::CheckDlgButton( hwndDlg, IDC_RADIO_BACKUP_TYPE2, 1 );
		break;
	case 3:
		::CheckDlgButton( hwndDlg, IDC_RADIO_BACKUP_TYPE3, 1 );
		break;
	case 1:
	default:
		::CheckDlgButton( hwndDlg, IDC_RADIO_BACKUP_TYPE1, 1 );
		break;
	}
	/* バックアップファイル名：日付の年 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUP_YEAR, m_Common.GetBackupOpt(BKUP_YEAR)  );
	/* バックアップファイル名：日付の月 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUP_MONTH, m_Common.GetBackupOpt(BKUP_MONTH) );
	/* バックアップファイル名：日付の日 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUP_DAY, m_Common.GetBackupOpt(BKUP_DAY));
	/* バックアップファイル名：日付の時 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUP_HOUR, m_Common.GetBackupOpt(BKUP_HOUR) );
	/* バックアップファイル名：日付の分 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUP_MIN, m_Common.GetBackupOpt(BKUP_MIN) );
	/* バックアップファイル名：日付の秒 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUP_SEC, m_Common.GetBackupOpt(BKUP_SEC) );

	/* バックアップを作成するフォルダ */
	::SetDlgItemText( hwndDlg, IDC_EDIT_BACKUPFOLDER, m_Common.m_szBackUpFolder );

	//	From Here Aug. 16, 2000 genta
	int nN = m_Common.GetBackupCount();
	nN = nN < 1  ?  1 : nN;
	nN = nN > 99 ? 99 : nN;

	char buf[6];
	wsprintf( buf, "%d", nN);
	::SetDlgItemText( hwndDlg, IDC_EDIT_BACKUP_3, buf );
	//	To Here Aug. 16, 2000 genta

	EnableBackupInput(hwndDlg);
	return;
}








/* ダイアログデータの取得 */
int CPropCommon::GetData_PROP_BACKUP( HWND hwndDlg )
{
	m_nPageNum = ID_PAGENUM_BACKUP;

	/* バックアップの作成 */
	m_Common.m_bBackUp = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP );
	/* バックアップの作成前に確認 */
	m_Common.m_bBackUpDialog = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUPDIALOG );
	/* 指定フォルダにバックアップを作成する */
	m_Common.m_bBackUpFolder = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUPFOLDER );


	/* バックアップファイル名のタイプ 1=(.bak) 2=*_日付.* */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_TYPE1 ) ){
		m_Common.SetBackupType(1);
	}
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_TYPE2 ) ){
		m_Common.SetBackupType(2);
	}

	//	Aug. 16, 2000 genta
	//	3 = *.b??
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_TYPE3 ) ){
		m_Common.SetBackupType(3);
	}

	/* バックアップファイル名：日付の年 */
	m_Common.SetBackupOpt(BKUP_YEAR, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_YEAR ) == BST_CHECKED);
	/* バックアップファイル名：日付の月 */
	m_Common.SetBackupOpt(BKUP_MONTH, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_MONTH ) == BST_CHECKED);
	/* バックアップファイル名：日付の日 */
	m_Common.SetBackupOpt(BKUP_DAY, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_DAY ) == BST_CHECKED);
	/* バックアップファイル名：日付の時 */
	m_Common.SetBackupOpt(BKUP_HOUR, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_HOUR ) == BST_CHECKED);
	/* バックアップファイル名：日付の分 */
	m_Common.SetBackupOpt(BKUP_MIN, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_MIN ) == BST_CHECKED);
	/* バックアップファイル名：日付の秒 */
	m_Common.SetBackupOpt(BKUP_SEC, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_SEC ) == BST_CHECKED);

	/* バックアップを作成するフォルダ */
	::GetDlgItemText( hwndDlg, IDC_EDIT_BACKUPFOLDER, m_Common.m_szBackUpFolder, MAX_PATH - 1 );

//	int nDummy;
//	int nCharChars;
//	nDummy = strlen( m_Common.m_szBackUpFolder );
//	if( 0 < nDummy ){
//		/* フォルダの最後が「半角かつ'\\'」でない場合は、付加する */
//		nCharChars = &m_Common.m_szBackUpFolder[nDummy] - CMemory::MemCharPrev( m_Common.m_szBackUpFolder, nDummy, &m_Common.m_szBackUpFolder[nDummy] );
//		if( 1 == nCharChars && m_Common.m_szBackUpFolder[nDummy - 1] == '\\' ){
//		}else{
//			strcat( m_Common.m_szBackUpFolder, "\\" );
//		}
//	}

	//	From Here Aug. 16, 2000 genta
	//	世代数の取得
	char szNumBuf[6];
	int	 nN;
	char *pDigit;
	::GetDlgItemText( hwndDlg, IDC_EDIT_BACKUP_3, szNumBuf, 4 );

	for( nN = 0, pDigit = szNumBuf; *pDigit != '\0'; pDigit++ ){
		if( '0' <= *pDigit && *pDigit <= '9' ){
			nN = nN * 10 + *pDigit - '0';
		}
		else
			break;
	}
	nN = nN < 1  ?  1 : nN;
	nN = nN > 99 ? 99 : nN;
	m_Common.SetBackupCount( nN );
	//	To Here Aug. 16, 2000 genta

	return TRUE;
}

//	From Here Aug. 16, 2000 genta
//	チェック状態に応じてダイアログボックス要素のEnable/Disableを
//	適切に設定する
void CPropCommon::EnableBackupInput(HWND hwndDlg)
{
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_BACKUP_TYPE1 ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_BACKUP_TYPE2 ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_BACKUP_TYPE3 ), TRUE );
		if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_TYPE2 ) ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_YEAR ), TRUE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_MONTH ), TRUE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_DAY ), TRUE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_HOUR ), TRUE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_MIN ), TRUE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_SEC ), TRUE );
		}else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_YEAR ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_MONTH ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_DAY ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_HOUR ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_MIN ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_SEC ), FALSE );
		}
		if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_TYPE3 ) ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_BACKUP_3 ), TRUE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_BACKUP_3 ), TRUE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_SPIN_BACKUP_GENS ), TRUE );	// added Oct. 6, JEPRO Enableになるように変更
		}
		else {
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_BACKUP_3 ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_BACKUP_3 ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_SPIN_BACKUP_GENS ), FALSE );	// added Oct. 6, JEPRO Disableになるように変更
		}
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUPDIALOG ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUPFOLDER ), TRUE );

		if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUPFOLDER ) ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_BACKUP_4 ), TRUE );	// added Sept. 6, JEPRO フォルダ指定したときだけEnableになるように変更
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_BACKUPFOLDER ), TRUE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKUP_FOLDER_REF ), TRUE );
		}else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_BACKUP_4 ), FALSE );	// added Sept. 6, JEPRO フォルダ指定したときだけEnableになるように変更
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_BACKUPFOLDER ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKUP_FOLDER_REF ), FALSE );
		}

	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_BACKUP_TYPE1 ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_BACKUP_TYPE2 ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_BACKUP_TYPE3 ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_YEAR ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_MONTH ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_DAY ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_HOUR ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_MIN ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_SEC ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_BACKUP_3 ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_BACKUP_3 ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUPDIALOG ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUPFOLDER ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_BACKUP_4 ), FALSE );	// added Sept. 6, JEPRO フォルダ指定したときだけEnableになるように変更
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_BACKUPFOLDER ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKUP_FOLDER_REF ), FALSE );
	}
}
//	To Here Aug. 16, 2000 genta
