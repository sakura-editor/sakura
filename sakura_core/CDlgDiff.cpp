//	$Id$
/*!	@file
	@brief DIFF差分表示ダイアログボックス

	@author MIK
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, Stonee, genta, JEPRO, YAZAKI
	Copyright (C) 2002, aroka, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include <string.h>
#include "sakura_rc.h"
#include "CDlgOpenFile.h"
#include "CDlgDiff.h"
#include "etc_uty.h"
#include "debug.h"
#include "CEditDoc.h"
#include "global.h"
#include "funccode.h"
#include "mymessage.h"

#include "sakura.hh"
const DWORD p_helpids[] = {	//13200
	IDC_BUTTON_DIFF_DST,		HIDC_BUTTON_DIFF_DST,
	IDC_CHECK_DIFF_OPT_BLINE,	HIDC_CHECK_DIFF_OPT_BLINE,
	IDC_CHECK_DIFF_OPT_CASE,	HIDC_CHECK_DIFF_OPT_CASE,
	IDC_CHECK_DIFF_OPT_SPACE,	HIDC_CHECK_DIFF_OPT_SPACE,
	IDC_CHECK_DIFF_OPT_SPCCHG,	HIDC_CHECK_DIFF_OPT_SPCCHG,
	IDC_CHECK_DIFF_OPT_TABSPC,	HIDC_CHECK_DIFF_OPT_TABSPC,
	IDC_EDIT_DIFF_DST,			HIDC_EDIT_DIFF_DST,
	IDC_FRAME_DIFF_FILE12,		HIDC_RADIO_DIFF_FILE1,
	IDC_RADIO_DIFF_FILE1,		HIDC_RADIO_DIFF_FILE1,
	IDC_RADIO_DIFF_FILE2,		HIDC_RADIO_DIFF_FILE2,
	IDC_FRAME_DIFF_DST,			HIDC_RADIO_DIFF_DST1,
	IDC_RADIO_DIFF_DST1,		HIDC_RADIO_DIFF_DST1,
	IDC_RADIO_DIFF_DST2,		HIDC_RADIO_DIFF_DST2,
	IDC_LIST_DIFF_FILES,		HIDC_LIST_DIFF_FILES,
	IDC_STATIC_DIFF_SRC,		HIDC_STATIC_DIFF_SRC,
	IDOK,						HIDC_DIFF_IDOK,
	IDCANCEL,					HIDC_DIFF_IDCANCEL,
	IDC_BUTTON_HELP,			HIDC_BUTTON_DIFF_HELP,
	IDC_CHECK_DIFF_EXEC_STATE,	HIDC_CHECK_DIFF_EXEC_STATE,	//DIFF差分が見つからないときにメッセージを表示  2003.05.12 MIK
//	IDC_FRAME_DIFF_SEARCH_MSG,	HIDC_FRAME_DIFF_SEARCH_MSG,
//	IDC_STATIC,						-1,
	0, 0
};

CDlgDiff::CDlgDiff()
{
	strcpy( m_szFile1, "" );
	strcpy( m_szFile2, "" );
	//m_nDiffFlgFile12 = 1;
	m_nDiffFlgOpt    = 0;
	m_bIsModified    = FALSE;
	m_bIsModifiedDst = FALSE;
	m_hWnd_Dst       = NULL;
	return;
}

/* モーダルダイアログの表示 */
int CDlgDiff::DoModal(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	LPARAM		lParam,
	const char*	pszPath,		//自ファイル
	BOOL		bIsModified		//自ファイル編集中？
)
{
	strcpy( m_szFile1, pszPath );
	m_bIsModified = bIsModified;

	return CDialog::DoModal( hInstance, hwndParent, IDD_DIFF, lParam );
}

BOOL CDlgDiff::OnBnClicked( int wID )
{
	switch( wID )
	{
	case IDC_BUTTON_HELP:
		/* ヘルプ */
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_DIFF_DIALOG) );
		return TRUE;

	case IDC_BUTTON_DIFF_DST:	/* 参照 */
		{
			CDlgOpenFile	cDlgOpenFile;
			char*			pszMRU = NULL;;
			char*			pszOPENFOLDER = NULL;;
			char			szPath[_MAX_PATH];
			strcpy( szPath, m_szFile2 );
			/* ファイルオープンダイアログの初期化 */
			cDlgOpenFile.Create(
				m_hInstance,
				m_hWnd,
				"*.*",
				m_szFile1 /*m_szFile2*/,
				(const char **)&pszMRU,
				(const char **)&pszOPENFOLDER
			);
			if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) )
			{
				strcpy( m_szFile2, szPath );
				::SetDlgItemText( m_hWnd, IDC_EDIT_DIFF_DST, m_szFile2 );
				//外部ファイルを選択状態に
				::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_DST1, TRUE );
				::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_DST2, FALSE );
				::SendMessage( ::GetDlgItem( m_hWnd, IDC_LIST_DIFF_FILES), LB_SETCURSEL, (WPARAM)-1, 0 );
			}
		}
		return TRUE;

	case IDOK:			/* 左右に表示 */
		/* ダイアログデータの取得 */
		::EndDialog( m_hWnd, GetData() );
		return TRUE;

	case IDCANCEL:
		::EndDialog( m_hWnd, FALSE );
		return TRUE;

	case IDC_RADIO_DIFF_DST1:
		::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_DST2, FALSE );
		//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_EDIT_DIFF_DST ), TRUE );
		//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_DIFF_DST ), TRUE );
		//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_LIST_DIFF_FILES ), FALSE );
		::SendMessage( ::GetDlgItem( m_hWnd, IDC_LIST_DIFF_FILES), LB_SETCURSEL, (WPARAM)-1, 0 );
		return TRUE;

	case IDC_RADIO_DIFF_DST2:
		::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_DST1, FALSE );
		//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_EDIT_DIFF_DST ), FALSE );
		//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_DIFF_DST ), FALSE );
		//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_LIST_DIFF_FILES ), TRUE );
		return TRUE;

	case IDC_RADIO_DIFF_FILE1:
		::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_FILE2, FALSE );
		return TRUE;

	case IDC_RADIO_DIFF_FILE2:
		::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_FILE1, FALSE );
		return TRUE;
	}

	/* 基底クラスメンバ */
	return CDialog::OnBnClicked( wID );
}

/* ダイアログデータの設定 */
void CDlgDiff::SetData( void )
{
	//自ファイル
	::SetDlgItemText( m_hWnd, IDC_STATIC_DIFF_SRC, m_szFile1 );

	//オプション
	m_nDiffFlgOpt = m_pShareData->m_nDiffFlgOpt;
	if( m_nDiffFlgOpt & 0x0001 ) ::CheckDlgButton( m_hWnd, IDC_CHECK_DIFF_OPT_CASE,   TRUE );
	if( m_nDiffFlgOpt & 0x0002 ) ::CheckDlgButton( m_hWnd, IDC_CHECK_DIFF_OPT_SPACE,  TRUE );
	if( m_nDiffFlgOpt & 0x0004 ) ::CheckDlgButton( m_hWnd, IDC_CHECK_DIFF_OPT_SPCCHG, TRUE );
	if( m_nDiffFlgOpt & 0x0008 ) ::CheckDlgButton( m_hWnd, IDC_CHECK_DIFF_OPT_BLINE,  TRUE );
	if( m_nDiffFlgOpt & 0x0010 ) ::CheckDlgButton( m_hWnd, IDC_CHECK_DIFF_OPT_TABSPC, TRUE );

	//新旧ファイル
	if( m_nDiffFlgOpt & 0x0020 )
	{
		::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_FILE1, FALSE );
		::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_FILE2, TRUE );
	}
	else
	{
		::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_FILE1, TRUE );
		::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_FILE2, FALSE );
	}
	//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_FRAME_DIFF_FILE12 ), FALSE );
	//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_RADIO_DIFF_FILE1 ), FALSE );
	//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_RADIO_DIFF_FILE2 ), FALSE );

	//DIFF差分が見つからないときにメッセージを表示 2003.05.12 MIK
	if( m_nDiffFlgOpt & 0x0040 ) ::CheckDlgButton( m_hWnd, IDC_CHECK_DIFF_EXEC_STATE, TRUE );

	/* 相手ファイルの選択 */
	::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_DST1, TRUE );
	::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_DST2, FALSE );

	/* 見つからないときメッセージを表示 */
	::CheckDlgButton( m_hWnd, IDC_CHECK_NOTIFYNOTFOUND, m_pShareData->m_Common.m_bNOTIFYNOTFOUND );
	
	/* 先頭（末尾）から再検索 */
	::CheckDlgButton( m_hWnd, IDC_CHECK_SEARCHALL, m_pShareData->m_Common.m_bSearchAll );

	/* 編集中のファイル一覧を作成する */
	{
		HWND		hwndList;
		int			nRowNum;
		EditNode	*pEditNode;
		FileInfo	*pFileInfo;
		int			i;
		int			nItem;
		char		szName[_MAX_PATH];
		CEditDoc*	pCEditDoc = (CEditDoc*)m_lParam;
		int			count = 0;

		/* リストのハンドル取得 */
		hwndList = :: GetDlgItem( m_hWnd, IDC_LIST_DIFF_FILES );

		/* 現在開いている編集窓のリストをメニューにする */
		nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNode, TRUE );
		if( nRowNum > 0 )
		{
			for( i = 0; i < nRowNum; i++ )
			{
				/* トレイからエディタへの編集ファイル名要求通知 */
				::SendMessage( pEditNode[i].m_hWnd, MYWM_GETFILEINFO, 0, 0 );
				pFileInfo = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;

				/* 自分ならスキップ */
				if ( pEditNode[i].m_hWnd == pCEditDoc->m_hwndParent )
				{
					continue;
				}

				/* ファイル名を作成する */
				wsprintf( szName, "%s %s",
					( strlen( pFileInfo->m_szPath ) ) ? pFileInfo->m_szPath : "(無題)",
					pFileInfo->m_bIsModified ? "*" : " "
				);

				// gm_pszCodeNameArr_3 からコピーするように変更
				if( 0 < pFileInfo->m_nCharCode && pFileInfo->m_nCharCode < CODE_CODEMAX ){
					strcat( szName, gm_pszCodeNameArr_3[pFileInfo->m_nCharCode] );
				}
#if 0
				/* 文字コードを付与する */
				switch( pFileInfo->m_nCharCode )
				{
				case CODE_JIS:	/* JIS */
					strcat( szName, "  [JIS]" );
					break;
				case CODE_EUC:	/* EUC */
					strcat( szName, "  [EUC]" );
					break;
				case CODE_UNICODE:	/* Unicode */
					strcat( szName, "  [Unicode]" );
					break;
				case CODE_UTF8:	/* UTF-8 */
					strcat( szName, "  [UTF-8]" );
					break;
				case CODE_UTF7:	/* UTF-7 */
					strcat( szName, "  [UTF-7]" );
					break;
				default:	/* SJIS */
					break;
				}
#endif

				/* リストに登録する */
				nItem = ::SendMessage( hwndList, LB_ADDSTRING, 0, (LPARAM)(char*)szName );
				::SendMessage( hwndList, LB_SETITEMDATA, nItem, (LPARAM)pEditNode[i].m_hWnd );
				count++;
			}

			delete [] pEditNode;
			// 2002/11/01 Moca 追加 リストビューの横幅を設定。これをやらないと水平スクロールバーが使えない
			::SendMessage( hwndList, LB_SETHORIZONTALEXTENT, (WPARAM)1000, 0 );

			/* 最初を選択 */
			//::SendMessage( hwndList, LB_SETCURSEL, (WPARAM)0, 0 );
		}

		if( count == 0 )
		{
			/* その他の編集中リストはなし */
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_RADIO_DIFF_DST2 ), FALSE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_LIST_DIFF_FILES ), FALSE );
		}
	}

	return;
}

/* ダイアログデータの取得 */
/* TRUE==正常  FALSE==入力エラー */
int CDlgDiff::GetData( void )
{
	BOOL	ret = TRUE;

	//DIFFオプション
	m_nDiffFlgOpt = 0;
	if( ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_DIFF_OPT_CASE   ) == BST_CHECKED ) m_nDiffFlgOpt |= 0x0001;
	if( ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_DIFF_OPT_SPACE  ) == BST_CHECKED ) m_nDiffFlgOpt |= 0x0002;
	if( ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_DIFF_OPT_SPCCHG ) == BST_CHECKED ) m_nDiffFlgOpt |= 0x0004;
	if( ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_DIFF_OPT_BLINE  ) == BST_CHECKED ) m_nDiffFlgOpt |= 0x0008;
	if( ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_DIFF_OPT_TABSPC ) == BST_CHECKED ) m_nDiffFlgOpt |= 0x0010;
	//ファイル新旧
	if( ::IsDlgButtonChecked( m_hWnd, IDC_RADIO_DIFF_FILE2      ) == BST_CHECKED ) m_nDiffFlgOpt |= 0x0020;
	//DIFF差分が見つからないときにメッセージを表示 2003.05.12 MIK
	if( ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_DIFF_EXEC_STATE ) == BST_CHECKED ) m_nDiffFlgOpt |= 0x0040;
	m_pShareData->m_nDiffFlgOpt = m_nDiffFlgOpt;

	//相手ファイル名
	strcpy( m_szFile2, "" );
	m_hWnd_Dst = NULL;
	m_bIsModifiedDst = FALSE;
	if( ::IsDlgButtonChecked( m_hWnd, IDC_RADIO_DIFF_DST1 ) == BST_CHECKED )
	{
		::GetDlgItemText( m_hWnd, IDC_EDIT_DIFF_DST, m_szFile2, sizeof( m_szFile2 ) );
	}
	else if( ::IsDlgButtonChecked( m_hWnd, IDC_RADIO_DIFF_DST2 ) == BST_CHECKED )
	{
		HWND		hwndList;
		int			nItem;
		FileInfo	*pFileInfo;

		/* リストから相手のウインドウハンドルを取得 */
		hwndList = :: GetDlgItem( m_hWnd, IDC_LIST_DIFF_FILES );
		nItem = ::SendMessage( hwndList, LB_GETCURSEL, 0, 0 );
		if( nItem != LB_ERR )
		{
			m_hWnd_Dst = (HWND)::SendMessage( hwndList, LB_GETITEMDATA, nItem, 0 );

			/* トレイからエディタへの編集ファイル名要求通知 */
			::SendMessage( m_hWnd_Dst, MYWM_GETFILEINFO, 0, 0 );
			pFileInfo = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;

			strcpy( m_szFile2, pFileInfo->m_szPath );
			m_bIsModifiedDst = pFileInfo->m_bIsModified;
		}
		else
		{
			ret = FALSE;
		}
	}
	else
	{
		ret = FALSE;
	}

	/* 見つからないときメッセージを表示 */
	if( ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_NOTIFYNOTFOUND ) == BST_CHECKED )
		m_pShareData->m_Common.m_bNOTIFYNOTFOUND = TRUE;
	else
		m_pShareData->m_Common.m_bNOTIFYNOTFOUND = FALSE;

	/* 先頭（末尾）から再検索 */
	if( ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_SEARCHALL ) == BST_CHECKED )
		m_pShareData->m_Common.m_bSearchAll = TRUE;
	else
		m_pShareData->m_Common.m_bSearchAll = FALSE;

	//相手ファイルが指定されてなければキャンセル
	if( m_szFile2[0] == '\0' ) ret = FALSE;

	return ret;
}

BOOL CDlgDiff::OnCbnSelChange( HWND hwndCtl, int wID )
{
	HWND	hwndList;

	hwndList = GetDlgItem( m_hWnd, IDC_LIST_DIFF_FILES );

	if( hwndList == hwndCtl )
	{
		::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_DST1, FALSE );
		::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_DST2, TRUE );
		return TRUE;
	}

	/* 基底クラスメンバ */
	return CDialog::OnCbnSelChange( hwndCtl, wID );
}

BOOL CDlgDiff::OnEditChange( HWND hwndCtl, int wID )
{
	HWND	hwndEdit;

	hwndEdit = GetDlgItem( m_hWnd, IDC_EDIT_DIFF_DST );

	if( hwndEdit == hwndCtl )
	{
		::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_DST1, TRUE );
		::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_DST2, FALSE );
		return TRUE;
	}

	/* 基底クラスメンバ */
	return CDialog::OnCbnSelChange( hwndCtl, wID );
}

LPVOID CDlgDiff::GetHelpIdTable( void )
{
	return (LPVOID)p_helpids;
}

/*[EOF]*/
