/*!	@file
	@brief 共通設定ダイアログボックス、「バックアップ」ページ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta, jepro
	Copyright (C) 2001, MIK, asa-o, genta, jepro
	Copyright (C) 2002, MIK, YAZAKI, genta, Moca
	Copyright (C) 2003, KEITA
	Copyright (C) 2004, genta
	Copyright (C) 2005, genta, aroka
	Copyright (C) 2006, ryoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2010, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "prop/CPropCommon.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

//@@@ 2001.02.04 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//10000
	IDC_BUTTON_BACKUP_FOLDER_REF,	HIDC_BUTTON_BACKUP_FOLDER_REF,	//バックアップフォルダ参照
	IDC_CHECK_BACKUP,				HIDC_CHECK_BACKUP,				//バックアップの作成
	IDC_CHECK_BACKUP_YEAR,			HIDC_CHECK_BACKUP_YEAR,			//バックアップファイル名（西暦年）
	IDC_CHECK_BACKUP_MONTH,			HIDC_CHECK_BACKUP_MONTH,		//バックアップファイル名（月）
	IDC_CHECK_BACKUP_DAY,			HIDC_CHECK_BACKUP_DAY,			//バックアップファイル名（日）
	IDC_CHECK_BACKUP_HOUR,			HIDC_CHECK_BACKUP_HOUR,			//バックアップファイル名（時）
	IDC_CHECK_BACKUP_MIN,			HIDC_CHECK_BACKUP_MIN,			//バックアップファイル名（分）
	IDC_CHECK_BACKUP_SEC,			HIDC_CHECK_BACKUP_SEC,			//バックアップファイル名（秒）
	IDC_CHECK_BACKUPDIALOG,			HIDC_CHECK_BACKUPDIALOG,		//作成前に確認
	IDC_CHECK_BACKUPFOLDER,			HIDC_CHECK_BACKUPFOLDER,		//指定フォルダに作成
	IDC_CHECK_BACKUP_FOLDER_RM,		HIDC_CHECK_BACKUP_FOLDER_RM,	//指定フォルダに作成(リムーバブルメディアのみ)
	IDC_CHECK_BACKUP_DUSTBOX,		HIDC_CHECK_BACKUP_DUSTBOX,		//バックアップファイルをごみ箱に放り込む	//@@@ 2001.12.11 add MIK
	IDC_EDIT_BACKUPFOLDER,			HIDC_EDIT_BACKUPFOLDER,			//保存フォルダ名
	IDC_EDIT_BACKUP_3,				HIDC_EDIT_BACKUP_3,				//世代数
	IDC_RADIO_BACKUP_TYPE1,			HIDC_RADIO_BACKUP_TYPE1,		//バックアップの種類（拡張子）
//	IDC_RADIO_BACKUP_TYPE2,			HIDC_RADIO_BACKUP_TYPE2NEWHID,		//バックアップの種類（日付・時刻） // 2002.11.09 Moca HIDが.._TYPE3と逆だった	// Jun.  5, 2004 genta 廃止
	IDC_RADIO_BACKUP_TYPE3,			HIDC_RADIO_BACKUP_TYPE3NEWHID,		//バックアップの種類（連番）// 2002.11.09 Moca HIDが.._TYPE2と逆だった
	IDC_RADIO_BACKUP_DATETYPE1,		HIDC_RADIO_BACKUP_DATETYPE1,	//付加する日時の種類（作成日時）	//Jul. 05, 2001 JEPRO 追加
	IDC_RADIO_BACKUP_DATETYPE2,		HIDC_RADIO_BACKUP_DATETYPE2,	//付加する日時の種類（更新日時）	//Jul. 05, 2001 JEPRO 追加
	IDC_SPIN_BACKUP_GENS,			HIDC_EDIT_BACKUP_3,				//保存する世代数のスピン
	IDC_CHECK_BACKUP_RETAINEXT,		HIDC_CHECK_BACKUP_RETAINEXT,	//元の拡張子を保存	// 2006.08.06 ryoji
	IDC_CHECK_BACKUP_ADVANCED,		HIDC_CHECK_BACKUP_ADVANCED,		//詳細設定	// 2006.08.06 ryoji
	IDC_EDIT_BACKUPFILE,			HIDC_EDIT_BACKUPFILE,			//詳細設定のエディットボックス	// 2006.08.06 ryoji
	IDC_RADIO_BACKUP_DATETYPE1A,	HIDC_RADIO_BACKUP_DATETYPE1A,	//付加する日時の種類（作成日時）※詳細設定ON用	// 2009.02.20 ryoji
	IDC_RADIO_BACKUP_DATETYPE2A,	HIDC_RADIO_BACKUP_DATETYPE2A,	//付加する日時の種類（更新日時）※詳細設定ON用	// 2009.02.20 ryoji
//	IDC_STATIC,						-1,
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
INT_PTR CALLBACK CPropBackup::DlgProc_page(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( reinterpret_cast<pDispatchPage>(&CPropBackup::DispatchEvent), hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta


/* メッセージ処理 */
INT_PTR CPropBackup::DispatchEvent( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	WORD		wNotifyCode;
	WORD		wID;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
//	int			nVal;
	int			nVal;	//Sept.21, 2000 JEPRO スピン要素を加えたので復活させた
//	int			nDummy;
//	int			nCharChars;

	switch( uMsg ){

	case WM_INITDIALOG:
		/* ダイアログデータの設定 Backup */
		SetData( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */
		//	Oct. 5, 2002 genta バックアップフォルダ名の入力サイズを指定
		//	Oct. 8, 2002 genta 最後に付加される\の領域を残すためバッファサイズ-1しか入力させない
		EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_BACKUPFOLDER ), _countof2(m_Common.m_sBackup.m_szBackUpFolder) - 1 - 1 );
		// 20051107 aroka
		EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_BACKUPFILE ), _countof2(m_Common.m_sBackup.m_szBackUpPathAdvanced) - 1 - 1 );
		return TRUE;

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
				/* ダイアログデータの取得 Backup */
				GetData( hwndDlg );
				return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
			case PSN_SETACTIVE:
				m_nPageNum = ID_PROPCOM_PAGENUM_BACKUP;
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
		break;

	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	/* 通知コード */
		wID			= LOWORD(wParam);	/* 項目ID､ コントロールID､ またはアクセラレータID */
		switch( wNotifyCode ){
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			switch( wID ){
			case IDC_RADIO_BACKUP_TYPE1:
				//	Aug. 16, 2000 genta
				//	バックアップ方式追加
			case IDC_RADIO_BACKUP_TYPE3:
			case IDC_CHECK_BACKUP:
			case IDC_CHECK_BACKUPFOLDER:
				//	Aug. 21, 2000 genta
			case IDC_CHECK_AUTOSAVE:
			//	Jun.  5, 2004 genta IDC_RADIO_BACKUP_TYPE2を廃止して，
			//	IDC_RADIO_BACKUP_DATETYPE1, IDC_RADIO_BACKUP_DATETYPE2を同列に持ってきた
			case IDC_RADIO_BACKUP_DATETYPE1:
			case IDC_RADIO_BACKUP_DATETYPE2:
			// 20051107 aroka
			case IDC_CHECK_BACKUP_ADVANCED:
				GetData( hwndDlg );
				UpdateBackupFile( hwndDlg );
				EnableBackupInput(hwndDlg);
				return TRUE;
			case IDC_BUTTON_BACKUP_FOLDER_REF:	/* フォルダ参照 */
				{
					/* バックアップを作成するフォルダ */
					TCHAR		szFolder[_MAX_PATH];
					::DlgItem_GetText( hwndDlg, IDC_EDIT_BACKUPFOLDER, szFolder, _countof( szFolder ));

					if( SelectDir( hwndDlg, LS(STR_PROPCOMBK_SEL_FOLDER), szFolder, szFolder ) ){
						_tcscpy( m_Common.m_sBackup.m_szBackUpFolder, szFolder );
						::DlgItem_SetText( hwndDlg, IDC_EDIT_BACKUPFOLDER, m_Common.m_sBackup.m_szBackUpFolder );
					}
					UpdateBackupFile( hwndDlg );
				}
				return TRUE;
			default: // 20051107 aroka Default節 追加
				GetData( hwndDlg );
				UpdateBackupFile( hwndDlg );
			}
			break;	/* BN_CLICKED */
		case EN_CHANGE: // 20051107 aroka フォルダが変更されたらリアルタイムにエディットボックス内を更新
			switch( wID ){
			case IDC_EDIT_BACKUPFOLDER:
				// 2009.02.21 ryoji 後ろに\が追加されるので，1文字余裕をみる必要がある．
				::DlgItem_GetText( hwndDlg, IDC_EDIT_BACKUPFOLDER, m_Common.m_sBackup.m_szBackUpFolder, _countof2(m_Common.m_sBackup.m_szBackUpFolder) - 1 );
				UpdateBackupFile( hwndDlg );
				break;
			}
			break;	/* EN_CHANGE */
		}
		break;	/* WM_COMMAND */

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;
		/*NOTREACHED*/
		//break;
//@@@ 2001.02.04 End

//@@@ 2001.12.22 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
//@@@ 2001.12.22 End

	}
	return FALSE;
}


/*! ダイアログデータの設定
	@date 2004.06.05 genta 元の拡張子を残す設定を追加．
		日時指定でチェックボックスが空欄で残ると設定されない問題を避けるため，
		IDC_RADIO_BACKUP_TYPE2
		を廃止してレイアウト変更
*/
void CPropBackup::SetData( HWND hwndDlg )
{
//	BOOL	bRet;

//	BOOL				m_bGrepExitConfirm;	/* Grepモードで保存確認するか */


	/* バックアップの作成 */
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_BACKUP, m_Common.m_sBackup.m_bBackUp );
	/* バックアップの作成前に確認 */
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_BACKUPDIALOG, m_Common.m_sBackup.m_bBackUpDialog );
//	/* 指定フォルダにバックアップを作成する */ //	20051107 aroka 「バックアップの作成」に連動させる
//	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUPFOLDER, .m_sBackup.m_bBackUpFolder );

	/* バックアップファイル名のタイプ 1=(.bak) 2=*_日付.* */
	//	Jun.  5, 2004 genta 元の拡張子を残す設定(5,6)を追加．
	switch( m_Common.m_sBackup.GetBackupType() ){
	case 2:
		::CheckDlgButton( hwndDlg, IDC_RADIO_BACKUP_DATETYPE1, 1 );	// 付加する日付のタイプ(現時刻)
		break;
	case 3:
	case 6:
		::CheckDlgButton( hwndDlg, IDC_RADIO_BACKUP_TYPE3, 1 );
		break;
	case 4:
		::CheckDlgButton( hwndDlg, IDC_RADIO_BACKUP_DATETYPE2, 1 );	// 付加する日付のタイプ(前回の保存時刻)
		break;
	case 5:
	case 1:
	default:
		::CheckDlgButton( hwndDlg, IDC_RADIO_BACKUP_TYPE1, 1 );
		break;
	}
	
	//	Jun.  5, 2004 genta 元の拡張子を残す設定(5,6)を追加．
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUP_RETAINEXT,
		( m_Common.m_sBackup.GetBackupType() == 5 || m_Common.m_sBackup.GetBackupType() == 6 ) ? 1 : 0
	 );

	/* バックアップファイル名：日付の年 */
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_BACKUP_YEAR, m_Common.m_sBackup.GetBackupOpt(BKUP_YEAR) );
	/* バックアップファイル名：日付の月 */
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_BACKUP_MONTH, m_Common.m_sBackup.GetBackupOpt(BKUP_MONTH) );
	/* バックアップファイル名：日付の日 */
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_BACKUP_DAY, m_Common.m_sBackup.GetBackupOpt(BKUP_DAY) );
	/* バックアップファイル名：日付の時 */
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_BACKUP_HOUR, m_Common.m_sBackup.GetBackupOpt(BKUP_HOUR) );
	/* バックアップファイル名：日付の分 */
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_BACKUP_MIN, m_Common.m_sBackup.GetBackupOpt(BKUP_MIN) );
	/* バックアップファイル名：日付の秒 */
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_BACKUP_SEC, m_Common.m_sBackup.GetBackupOpt(BKUP_SEC) );

	/* 指定フォルダにバックアップを作成する */ // 20051107 aroka 移動：連動対象にする。
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_BACKUPFOLDER, m_Common.m_sBackup.m_bBackUpFolder );
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_BACKUP_FOLDER_RM, m_Common.m_sBackup.m_bBackUpFolderRM );	// 2010/5/27 Uchi

	/* バックアップを作成するフォルダ */
	::DlgItem_SetText( hwndDlg, IDC_EDIT_BACKUPFOLDER, m_Common.m_sBackup.m_szBackUpFolder );

	/* バックアップファイルをごみ箱に放り込む */	//@@@ 2001.12.11 add MIK
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUP_DUSTBOX, m_Common.m_sBackup.m_bBackUpDustBox?BST_CHECKED:BST_UNCHECKED );	//@@@ 2001.12.11 add MIK

	/* バックアップ先フォルダを詳細設定する */ // 20051107 aroka
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUP_ADVANCED, m_Common.m_sBackup.m_bBackUpPathAdvanced?BST_CHECKED:BST_UNCHECKED );

	/* バックアップを作成するフォルダの詳細設定 */ // 20051107 aroka
	::DlgItem_SetText( hwndDlg, IDC_EDIT_BACKUPFILE, m_Common.m_sBackup.m_szBackUpPathAdvanced );

	/* バックアップを作成するフォルダの詳細設定 */ // 20051128 aroka
	switch( m_Common.m_sBackup.GetBackupTypeAdv() ){
	case 2:
		::CheckDlgButton( hwndDlg, IDC_RADIO_BACKUP_DATETYPE1A, 1 );	// 付加する日付のタイプ(現時刻)
		break;
	case 4:
		::CheckDlgButton( hwndDlg, IDC_RADIO_BACKUP_DATETYPE2A, 1 );	// 付加する日付のタイプ(前回の保存時刻)
		break;
	default:
		::CheckDlgButton( hwndDlg, IDC_RADIO_BACKUP_DATETYPE1A, 1 );
		break;
	}

	//	From Here Aug. 16, 2000 genta
	int nN = m_Common.m_sBackup.GetBackupCount();
	nN = nN < 1  ?  1 : nN;
	nN = nN > 99 ? 99 : nN;

	::SetDlgItemInt( hwndDlg, IDC_EDIT_BACKUP_3, nN, FALSE );	//	Oct. 29, 2001 genta
	//	To Here Aug. 16, 2000 genta

	UpdateBackupFile( hwndDlg );

	EnableBackupInput(hwndDlg);
	return;
}








/* ダイアログデータの取得 */
int CPropBackup::GetData( HWND hwndDlg )
{
	/* バックアップの作成 */
	m_Common.m_sBackup.m_bBackUp = ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_BACKUP );
	/* バックアップの作成前に確認 */
	m_Common.m_sBackup.m_bBackUpDialog = ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_BACKUPDIALOG );
//	/* 指定フォルダにバックアップを作成する */ // 20051107 aroka 「バックアップの作成」に連動させる
//	m_Common.m_sBackup.m_bBackUpFolder = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUPFOLDER );


	/* バックアップファイル名のタイプ 1=(.bak) 2=*_日付.* */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_TYPE1 ) ){
		//	Jun.  5, 2005 genta 拡張子を残すパターンを追加
		if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_RETAINEXT )){
			m_Common.m_sBackup.SetBackupType(5);
		}
		else {
			m_Common.m_sBackup.SetBackupType(1);
		}
	}
//	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_TYPE2 ) ){
		// 2001/06/05 Start by asa-o: 日付のタイプ
		if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_DATETYPE1 ) ){
			m_Common.m_sBackup.SetBackupType(2);	// 現時刻
		}
		if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_DATETYPE2 ) ){
			m_Common.m_sBackup.SetBackupType(4);	// 前回の保存時刻
		}
		// 2001/06/05 End
//	}

	//	Aug. 16, 2000 genta
	//	3 = *.b??
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_TYPE3 ) ){
		if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_RETAINEXT )){
			m_Common.m_sBackup.SetBackupType(6);
		}
		else {
			m_Common.m_sBackup.SetBackupType(3);
		}
	}

	/* バックアップファイル名：日付の年 */
	m_Common.m_sBackup.SetBackupOpt(BKUP_YEAR, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_YEAR ) == BST_CHECKED);
	/* バックアップファイル名：日付の月 */
	m_Common.m_sBackup.SetBackupOpt(BKUP_MONTH, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_MONTH ) == BST_CHECKED);
	/* バックアップファイル名：日付の日 */
	m_Common.m_sBackup.SetBackupOpt(BKUP_DAY, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_DAY ) == BST_CHECKED);
	/* バックアップファイル名：日付の時 */
	m_Common.m_sBackup.SetBackupOpt(BKUP_HOUR, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_HOUR ) == BST_CHECKED);
	/* バックアップファイル名：日付の分 */
	m_Common.m_sBackup.SetBackupOpt(BKUP_MIN, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_MIN ) == BST_CHECKED);
	/* バックアップファイル名：日付の秒 */
	m_Common.m_sBackup.SetBackupOpt(BKUP_SEC, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_SEC ) == BST_CHECKED);

	/* 指定フォルダにバックアップを作成する */ // 20051107 aroka 移動
	m_Common.m_sBackup.m_bBackUpFolder = ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_BACKUPFOLDER );
	m_Common.m_sBackup.m_bBackUpFolderRM = ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_BACKUP_FOLDER_RM );	// 2010/5/27 Uchi

	/* バックアップを作成するフォルダ */
	//	Oct. 5, 2002 genta サイズをsizeof()で指定
	//	Oct. 8, 2002 genta 後ろに\が追加されるので，1文字余裕を見る必要がある．
	::DlgItem_GetText( hwndDlg, IDC_EDIT_BACKUPFOLDER, m_Common.m_sBackup.m_szBackUpFolder, _countof2(m_Common.m_sBackup.m_szBackUpFolder) - 1);

	/* バックアップファイルをごみ箱に放り込む */	//@@@ 2001.12.11 add MIK
	m_Common.m_sBackup.m_bBackUpDustBox = (BST_CHECKED==::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_DUSTBOX ));	//@@@ 2001.12.11 add MIK

	/* 指定フォルダにバックアップを作成する詳細設定 */ // 20051107 aroka
	m_Common.m_sBackup.m_bBackUpPathAdvanced = (BST_CHECKED==::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_ADVANCED ));
	/* バックアップを作成するフォルダ */ // 20051107 aroka
	::DlgItem_GetText( hwndDlg, IDC_EDIT_BACKUPFILE, m_Common.m_sBackup.m_szBackUpPathAdvanced, _countof2( m_Common.m_sBackup.m_szBackUpPathAdvanced ) - 1);

	// 20051128 aroka 詳細設定の日付のタイプ
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_DATETYPE1A ) ){
		m_Common.m_sBackup.SetBackupTypeAdv(2);	// 現時刻
	}
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_DATETYPE2A ) ){
		m_Common.m_sBackup.SetBackupTypeAdv(4);	// 前回の保存時刻
	}

	//	From Here Aug. 16, 2000 genta
	//	世代数の取得
	int	 nN;
	nN = ::GetDlgItemInt( hwndDlg, IDC_EDIT_BACKUP_3, NULL, FALSE );	//	Oct. 29, 2001 genta

//	for( nN = 0, pDigit = szNumBuf; *pDigit != '\0'; pDigit++ ){
//		if( '0' <= *pDigit && *pDigit <= '9' ){
//			nN = nN * 10 + *pDigit - '0';
//		}
//		else
//			break;
//	}
	nN = nN < 1  ?  1 : nN;
	nN = nN > 99 ? 99 : nN;
	m_Common.m_sBackup.SetBackupCount( nN );
	//	To Here Aug. 16, 2000 genta

	return TRUE;
}

//	From Here Aug. 16, 2000 genta
/*!	チェック状態に応じてダイアログボックス要素のEnable/Disableを
	適切に設定する

	@date 2004.06.05 genta 元の拡張子を残す設定を追加．
		日時指定でチェックボックスが空欄で残ると設定されない問題を避けるため，
		IDC_RADIO_BACKUP_TYPE2
		を廃止してレイアウト変更
	@date 2005.11.07 aroka レイアウトに合わせて順序を入れ替え、インデントを整理
	@date 2005.11.21 aroka 詳細設定モードの制御を追加
	@date 2009.02.20 ryoji IDC_LABEL_BACKUP_HELPによる別コントロール隠しを廃止、if文制御をShowEnableフラグ制御に置き換えて簡素化して下記問題修正。
	                       ・Vista Aeroだと詳細設定ONにしても詳細設定OFF項目が画面から消えない
	                       ・詳細設定OFF項目が非表示ではなかったので隠れていてもTooltipヘルプが表示される
	                       ・詳細設定ONなのにバックアップ作成OFFだと詳細設定OFF項目のほうが表示される
*/
static inline void ShowEnable(HWND hWnd, BOOL bShow, BOOL bEnable)
{
	::ShowWindow( hWnd, bShow? SW_SHOW: SW_HIDE );
	::EnableWindow( hWnd, bEnable && bShow );		// bShow=false,bEnable=trueの場合ショートカットキーが変な動きをするので修正	2010/5/27 Uchi
}

void CPropBackup::EnableBackupInput(HWND hwndDlg)
{
	#define SHOWENABLE(id, show, enable) ShowEnable( ::GetDlgItem( hwndDlg, id ), show, enable )

	BOOL bBackup = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP );
	BOOL bAdvanced = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_ADVANCED );
	BOOL bType1 = ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_TYPE1 );
	//BOOL bType2 = ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_TYPE2 );
	BOOL bType3 = ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_TYPE3 );
	BOOL bDate1 = ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_DATETYPE1 );
	BOOL bDate2 = ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_DATETYPE2 );
	BOOL bFolder = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUPFOLDER );

	SHOWENABLE( IDC_CHECK_BACKUP_ADVANCED,	TRUE, bBackup );	// 20050628 aroka
	SHOWENABLE( IDC_RADIO_BACKUP_TYPE1,		!bAdvanced, bBackup );
	SHOWENABLE( IDC_RADIO_BACKUP_TYPE3,		!bAdvanced, bBackup );
	SHOWENABLE( IDC_RADIO_BACKUP_DATETYPE1,	!bAdvanced, bBackup );
	SHOWENABLE( IDC_RADIO_BACKUP_DATETYPE2,	!bAdvanced, bBackup );
	SHOWENABLE( IDC_LABEL_BACKUP_3,			!bAdvanced, bBackup && bType3);
	SHOWENABLE( IDC_EDIT_BACKUP_3,			!bAdvanced, bBackup && bType3);
	SHOWENABLE( IDC_SPIN_BACKUP_GENS,		!bAdvanced, bBackup && bType3 );	//	20051107 aroka 追加
	SHOWENABLE( IDC_CHECK_BACKUP_RETAINEXT,	!bAdvanced, bBackup && (bType1 || bType3) );	//	Jun.  5, 2005 genta 追加
	SHOWENABLE( IDC_CHECK_BACKUP_YEAR,		!bAdvanced, bBackup && (bDate1 || bDate2) );
	SHOWENABLE( IDC_CHECK_BACKUP_MONTH,		!bAdvanced, bBackup && (bDate1 || bDate2) );
	SHOWENABLE( IDC_CHECK_BACKUP_DAY,		!bAdvanced, bBackup && (bDate1 || bDate2) );
	SHOWENABLE( IDC_CHECK_BACKUP_HOUR,		!bAdvanced, bBackup && (bDate1 || bDate2) );
	SHOWENABLE( IDC_CHECK_BACKUP_MIN,		!bAdvanced, bBackup && (bDate1 || bDate2) );
	SHOWENABLE( IDC_CHECK_BACKUP_SEC,		!bAdvanced, bBackup && (bDate1 || bDate2) );

	// 詳細設定
	SHOWENABLE( IDC_EDIT_BACKUPFILE,		TRUE, bBackup && bAdvanced );
//	SHOWENABLE( IDC_LABEL_BACKUP_HELP,		bAdvanced, bBackup );	// 不可視のまま放置（他コントロール隠しの方式は廃止） 2009.02.20 ryoji
	SHOWENABLE( IDC_LABEL_BACKUP_HELP2,		bAdvanced, bBackup );
	SHOWENABLE( IDC_RADIO_BACKUP_DATETYPE1A,	bAdvanced, bBackup );
	SHOWENABLE( IDC_RADIO_BACKUP_DATETYPE2A,	bAdvanced, bBackup );

	SHOWENABLE( IDC_CHECK_BACKUPFOLDER,		TRUE, bBackup );
	SHOWENABLE( IDC_LABEL_BACKUP_4,			TRUE, bBackup && bFolder );	// added Sept. 6, JEPRO フォルダ指定したときだけEnableになるように変更
	SHOWENABLE( IDC_CHECK_BACKUP_FOLDER_RM,	TRUE, bBackup && bFolder );	// 2010/5/27 Uchi
	SHOWENABLE( IDC_EDIT_BACKUPFOLDER,		TRUE, bBackup && bFolder );
	SHOWENABLE( IDC_BUTTON_BACKUP_FOLDER_REF,	TRUE, bBackup && bFolder );
	SHOWENABLE( IDC_CHECK_BACKUP_DUSTBOX,	TRUE, bBackup );	//@@@ 2001.12.11 add MIK

	// 作成前に確認
	SHOWENABLE( IDC_CHECK_BACKUPDIALOG,		TRUE, bBackup );

	#undef SHOWENABLE
}
//	To Here Aug. 16, 2000 genta


/*!	バックアップファイルの詳細設定エディットボックスを適切に更新する

	@date 2005.11.07 aroka 新規追加

	@note 詳細設定切り替え時のデフォルトをオプションに合わせるため、
		m_szBackUpPathAdvanced を更新する
*/
void CPropBackup::UpdateBackupFile(HWND hwndDlg)	//	バックアップファイルの詳細設定
{
	wchar_t temp[MAX_PATH];
	/* バックアップを作成するファイル */ // 20051107 aroka
	if( !m_Common.m_sBackup.m_bBackUp ){
		temp[0] = LTEXT('\0');
	}
	else{
		if( m_Common.m_sBackup.m_bBackUpFolder ){
			temp[0] = LTEXT('\0');
		}
		else if( m_Common.m_sBackup.m_bBackUpDustBox  ){
			auto_sprintf( temp, LTEXT("%ls\\"), LSW(STR_PROPCOMBK_DUSTBOX) );
		}
		else{
			auto_sprintf( temp, LTEXT(".\\") );
		}

		switch( m_Common.m_sBackup.GetBackupType() ){
		case 1: // .bak
			wcscat( temp, LTEXT("$0.bak") );
			break;
		case 5: // .*.bak
			wcscat( temp, LTEXT("$0.*.bak") );
			break;
		case 3: // .b??
			wcscat( temp, LTEXT("$0.b??") );
			break;
		case 6: // .*.b??
			wcscat( temp, LTEXT("$0.*.b??") );
			break;
		case 2:	//	日付，時刻
		case 4:	//	日付，時刻
			wcscat( temp, LTEXT("$0_") );

			if( m_Common.m_sBackup.GetBackupOpt(BKUP_YEAR) ){	/* バックアップファイル名：日付の年 */
				wcscat( temp, LTEXT("%Y") );
			}
			if( m_Common.m_sBackup.GetBackupOpt(BKUP_MONTH) ){	/* バックアップファイル名：日付の月 */
				wcscat( temp, LTEXT("%m") );
			}
			if( m_Common.m_sBackup.GetBackupOpt(BKUP_DAY) ){	/* バックアップファイル名：日付の日 */
				wcscat( temp, LTEXT("%d") );
			}
			if( m_Common.m_sBackup.GetBackupOpt(BKUP_HOUR) ){	/* バックアップファイル名：日付の時 */
				wcscat( temp, LTEXT("%H") );
			}
			if( m_Common.m_sBackup.GetBackupOpt(BKUP_MIN) ){	/* バックアップファイル名：日付の分 */
				wcscat( temp, LTEXT("%M") );
			}
			if( m_Common.m_sBackup.GetBackupOpt(BKUP_SEC) ){	/* バックアップファイル名：日付の秒 */
				wcscat( temp, LTEXT("%S") );
			}

			wcscat( temp, LTEXT(".*") );
			break;
		default:
			break;
		}
	}
	if( !m_Common.m_sBackup.m_bBackUpPathAdvanced ){	// 詳細設定モードでないときだけ自動更新する
		auto_sprintf( m_Common.m_sBackup.m_szBackUpPathAdvanced, _T("%ls"), temp );
		::DlgItem_SetText( hwndDlg, IDC_EDIT_BACKUPFILE, m_Common.m_sBackup.m_szBackUpPathAdvanced );
	}
	return;
}

