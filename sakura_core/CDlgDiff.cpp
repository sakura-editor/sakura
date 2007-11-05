/*!	@file
	@brief DIFF差分表示ダイアログボックス

	@author MIK
	@date 2002.5.27
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, Stonee, genta, JEPRO, YAZAKI
	Copyright (C) 2002, aroka, MIK, Moca
	Copyright (C) 2003, MIK, genta
	Copyright (C) 2004, MIK, genta, じゅうじ
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include <string.h>
#include "sakura_rc.h"
#include "CDlgOpenFile.h"
#include "CDlgDiff.h"
#include "debug.h"
#include "CEditDoc.h"
#include "global.h"
#include "funccode.h"
#include "mymessage.h"
#include "util/shell.h"

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
	IDC_CHECK_NOTIFYNOTFOUND,	HIDC_CHECK_DIFF_NOTIFYNOTFOUND,	// 見つからないときにメッセージを表示	// 2006.10.10 ryoji
	IDC_CHECK_SEARCHALL,		HIDC_CHECK_DIFF_SEARCHALL,		// 先頭（末尾）から再検索する	// 2006.10.10 ryoji
//	IDC_FRAME_DIFF_SEARCH_MSG,	HIDC_FRAME_DIFF_SEARCH_MSG,
//	IDC_STATIC,						-1,
	0, 0
};

CDlgDiff::CDlgDiff()
	: m_nIndexSave( 0 )
{
	m_szFile1[0] = 0;
	m_szFile2[0] = 0;
	//m_nDiffFlgFile12 = 1;
	m_nDiffFlgOpt    = 0;
	m_bIsModified    = FALSE;
	m_bIsModifiedDst = FALSE;
	m_hWnd_Dst       = NULL;
	return;
}

/* モーダルダイアログの表示 */
int CDlgDiff::DoModal(
	HINSTANCE			hInstance,
	HWND				hwndParent,
	LPARAM				lParam,
	const TCHAR*		pszPath,		//自ファイル
	BOOL				bIsModified		//自ファイル編集中？
)
{
	_tcscpy(m_szFile1, pszPath);
	m_bIsModified = bIsModified;

	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_DIFF, lParam );
}

BOOL CDlgDiff::OnBnClicked( int wID )
{
	switch( wID )
	{
	case IDC_BUTTON_HELP:
		/* ヘルプ */
		MyWinHelp( GetHwnd(), m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_DIFF_DIALOG) );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;

	case IDC_BUTTON_DIFF_DST:	/* 参照 */
		{
			CDlgOpenFile	cDlgOpenFile;
			TCHAR			szPath[_MAX_PATH];
			_tcscpy( szPath, m_szFile2 );
			/* ファイルオープンダイアログの初期化 */
			cDlgOpenFile.Create(
				m_hInstance,
				GetHwnd(),
				_T("*.*"),
				m_szFile1 /*m_szFile2*/
			);
			if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) )
			{
				_tcscpy( m_szFile2, szPath );
				::DlgItem_SetText( GetHwnd(), IDC_EDIT_DIFF_DST, m_szFile2 );
				//外部ファイルを選択状態に
				::CheckDlgButton( GetHwnd(), IDC_RADIO_DIFF_DST1, TRUE );
				::CheckDlgButton( GetHwnd(), IDC_RADIO_DIFF_DST2, FALSE );
				::SendMessageAny( ::GetDlgItem( GetHwnd(), IDC_LIST_DIFF_FILES), LB_SETCURSEL, (WPARAM)-1, 0 );
			}
		}
		return TRUE;

	case IDOK:			/* 左右に表示 */
		/* ダイアログデータの取得 */
		::EndDialog( GetHwnd(), GetData() );
		return TRUE;

	case IDCANCEL:
		::EndDialog( GetHwnd(), FALSE );
		return TRUE;

	case IDC_RADIO_DIFF_DST1:
		::CheckDlgButton( GetHwnd(), IDC_RADIO_DIFF_DST2, FALSE );
		//::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_EDIT_DIFF_DST ), TRUE );
		//::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_DIFF_DST ), TRUE );
		//::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LIST_DIFF_FILES ), FALSE );
		//	Feb. 28, 2004 genta 選択解除前に前回の位置を記憶
		{
			int n = ::SendMessageAny( GetDlgItem( GetHwnd(), IDC_LIST_DIFF_FILES ), LB_GETCURSEL, 0, 0 );
			if( n != LB_ERR ){
				m_nIndexSave = n;
			}
		}
		::SendMessageAny( ::GetDlgItem( GetHwnd(), IDC_LIST_DIFF_FILES), LB_SETCURSEL, (WPARAM)-1, 0 );
		return TRUE;

	case IDC_RADIO_DIFF_DST2:
		::CheckDlgButton( GetHwnd(), IDC_RADIO_DIFF_DST1, FALSE );
		//::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_EDIT_DIFF_DST ), FALSE );
		//::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_DIFF_DST ), FALSE );
		//::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LIST_DIFF_FILES ), TRUE );
		{
			//	Aug. 9, 2003 genta
			//	ListBoxが選択されていなかったら，先頭のファイルを選択する．
			HWND hwndList = GetDlgItem( GetHwnd(), IDC_LIST_DIFF_FILES );
			if( ::SendMessageAny( hwndList, LB_GETCURSEL, 0, 0 ) == LB_ERR )
			{
				::SendMessageAny( hwndList, LB_SETCURSEL, m_nIndexSave, 0 );
			}
		}
		return TRUE;

	case IDC_RADIO_DIFF_FILE1:
		::CheckDlgButton( GetHwnd(), IDC_RADIO_DIFF_FILE2, FALSE );
		return TRUE;

	case IDC_RADIO_DIFF_FILE2:
		::CheckDlgButton( GetHwnd(), IDC_RADIO_DIFF_FILE1, FALSE );
		return TRUE;
	}

	/* 基底クラスメンバ */
	return CDialog::OnBnClicked( wID );
}

/* ダイアログデータの設定 */
void CDlgDiff::SetData( void )
{
	//自ファイル
	::DlgItem_SetText( GetHwnd(), IDC_STATIC_DIFF_SRC, m_szFile1 );

	//オプション
	m_nDiffFlgOpt = m_pShareData->m_nDiffFlgOpt;
	if( m_nDiffFlgOpt & 0x0001 ) ::CheckDlgButton( GetHwnd(), IDC_CHECK_DIFF_OPT_CASE,   TRUE );
	if( m_nDiffFlgOpt & 0x0002 ) ::CheckDlgButton( GetHwnd(), IDC_CHECK_DIFF_OPT_SPACE,  TRUE );
	if( m_nDiffFlgOpt & 0x0004 ) ::CheckDlgButton( GetHwnd(), IDC_CHECK_DIFF_OPT_SPCCHG, TRUE );
	if( m_nDiffFlgOpt & 0x0008 ) ::CheckDlgButton( GetHwnd(), IDC_CHECK_DIFF_OPT_BLINE,  TRUE );
	if( m_nDiffFlgOpt & 0x0010 ) ::CheckDlgButton( GetHwnd(), IDC_CHECK_DIFF_OPT_TABSPC, TRUE );

	//新旧ファイル
	if( m_nDiffFlgOpt & 0x0020 )
	{
		::CheckDlgButton( GetHwnd(), IDC_RADIO_DIFF_FILE1, FALSE );
		::CheckDlgButton( GetHwnd(), IDC_RADIO_DIFF_FILE2, TRUE );
	}
	else
	{
		::CheckDlgButton( GetHwnd(), IDC_RADIO_DIFF_FILE1, TRUE );
		::CheckDlgButton( GetHwnd(), IDC_RADIO_DIFF_FILE2, FALSE );
	}
	//::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_FRAME_DIFF_FILE12 ), FALSE );
	//::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_RADIO_DIFF_FILE1 ), FALSE );
	//::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_RADIO_DIFF_FILE2 ), FALSE );

	//DIFF差分が見つからないときにメッセージを表示 2003.05.12 MIK
	if( m_nDiffFlgOpt & 0x0040 ) ::CheckDlgButton( GetHwnd(), IDC_CHECK_DIFF_EXEC_STATE, TRUE );

	/* 見つからないときメッセージを表示 */
	::CheckDlgButton( GetHwnd(), IDC_CHECK_NOTIFYNOTFOUND, m_pShareData->m_Common.m_sSearch.m_bNOTIFYNOTFOUND );
	
	/* 先頭（末尾）から再検索 */
	::CheckDlgButton( GetHwnd(), IDC_CHECK_SEARCHALL, m_pShareData->m_Common.m_sSearch.m_bSearchAll );

	/* 編集中のファイル一覧を作成する */
	{
		HWND		hwndList;
		int			nRowNum;
		EditNode	*pEditNode;
		FileInfo	*pFileInfo;
		int			i;
		int			nItem;
		WIN_CHAR	szName[_MAX_PATH];
		CEditDoc*	pCEditDoc = (CEditDoc*)m_lParam;
		int			count = 0;

		/* リストのハンドル取得 */
		hwndList = :: GetDlgItem( GetHwnd(), IDC_LIST_DIFF_FILES );

		/* 現在開いている編集窓のリストをメニューにする */
		nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNode, TRUE );
		if( nRowNum > 0 )
		{
			for( i = 0; i < nRowNum; i++ )
			{
				/* トレイからエディタへの編集ファイル名要求通知 */
				::SendMessageAny( pEditNode[i].GetHwnd(), MYWM_GETFILEINFO, 0, 0 );
				pFileInfo = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;

				/* 自分ならスキップ */
				if ( pEditNode[i].GetHwnd() == pCEditDoc->GetOwnerHwnd() )
				{
					continue;
				}

				/* ファイル名を作成する */
				auto_sprintf(
					szName,
					_T("%ts %ts"),
					( _tcslen( pFileInfo->m_szPath ) ) ? pFileInfo->m_szPath : _T("(無題)"),
					pFileInfo->m_bIsModified ? _T("*") : _T(" ")
				);

				// gm_pszCodeNameArr_Bracket からコピーするように変更
				if(IsValidCodeTypeExceptSJIS(pFileInfo->m_nCharCode)){
					_tcscat( szName, gm_pszCodeNameArr_Bracket[pFileInfo->m_nCharCode] );
				}

				/* リストに登録する */
				nItem = ::List_AddString( hwndList, szName );
				::SendMessageAny( hwndList, LB_SETITEMDATA, nItem, (LPARAM)pEditNode[i].GetHwnd() );
				count++;
			}

			delete [] pEditNode;
			// 2002/11/01 Moca 追加 リストビューの横幅を設定。これをやらないと水平スクロールバーが使えない
			::SendMessageAny( hwndList, LB_SETHORIZONTALEXTENT, (WPARAM)1000, 0 );

			/* 最初を選択 */
			//::SendMessageAny( hwndList, LB_SETCURSEL, (WPARAM)0, 0 );
		}

		//	From Here 2004.02.22 じゅうじ
		//	開いているファイルがある場合には初期状態でそちらを優先
		if( count == 0 )
		{
			/* 相手ファイルの選択 */
			::CheckDlgButton( GetHwnd(), IDC_RADIO_DIFF_DST1, TRUE );
			::CheckDlgButton( GetHwnd(), IDC_RADIO_DIFF_DST2, FALSE );
			/* その他の編集中リストはなし */
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_RADIO_DIFF_DST2 ), FALSE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LIST_DIFF_FILES ), FALSE );
		}
		else
		{
			/* 相手ファイルの選択 */
			::CheckDlgButton( GetHwnd(), IDC_RADIO_DIFF_DST1, FALSE );
			::CheckDlgButton( GetHwnd(), IDC_RADIO_DIFF_DST2, TRUE );
			//	ListBoxが選択されていなかったら，先頭のファイルを選択する．
			HWND hwndList = GetDlgItem( GetHwnd(), IDC_LIST_DIFF_FILES );
			if( ::SendMessageAny( hwndList, LB_GETCURSEL, 0, 0 ) == LB_ERR )
			{
			    ::SendMessageAny( hwndList, LB_SETCURSEL, 0 /*先頭アイテム*/, 0 );
			}
		}
		//	To Here 2004.02.22 じゅうじ
		//	Feb. 28, 2004 genta 一番上を選択位置とする．
		m_nIndexSave = 0;
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
	if( ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_DIFF_OPT_CASE   ) == BST_CHECKED ) m_nDiffFlgOpt |= 0x0001;
	if( ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_DIFF_OPT_SPACE  ) == BST_CHECKED ) m_nDiffFlgOpt |= 0x0002;
	if( ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_DIFF_OPT_SPCCHG ) == BST_CHECKED ) m_nDiffFlgOpt |= 0x0004;
	if( ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_DIFF_OPT_BLINE  ) == BST_CHECKED ) m_nDiffFlgOpt |= 0x0008;
	if( ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_DIFF_OPT_TABSPC ) == BST_CHECKED ) m_nDiffFlgOpt |= 0x0010;
	//ファイル新旧
	if( ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_DIFF_FILE2      ) == BST_CHECKED ) m_nDiffFlgOpt |= 0x0020;
	//DIFF差分が見つからないときにメッセージを表示 2003.05.12 MIK
	if( ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_DIFF_EXEC_STATE ) == BST_CHECKED ) m_nDiffFlgOpt |= 0x0040;
	m_pShareData->m_nDiffFlgOpt = m_nDiffFlgOpt;

	//相手ファイル名
	_tcscpy( m_szFile2, _T("") );
	m_hWnd_Dst = NULL;
	m_bIsModifiedDst = FALSE;
	if( ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_DIFF_DST1 ) == BST_CHECKED )
	{
		::DlgItem_GetText( GetHwnd(), IDC_EDIT_DIFF_DST, m_szFile2, _countof2(m_szFile2) );
		//	2004.05.19 MIK 外部ファイルが指定されていない場合はキャンセル
		//相手ファイルが指定されてなければキャンセル
		if( m_szFile2[0] == '\0' ) ret = FALSE;

	}
	else if( ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_DIFF_DST2 ) == BST_CHECKED )
	{
		HWND		hwndList;
		int			nItem;
		FileInfo	*pFileInfo;

		/* リストから相手のウインドウハンドルを取得 */
		hwndList = :: GetDlgItem( GetHwnd(), IDC_LIST_DIFF_FILES );
		nItem = ::SendMessageAny( hwndList, LB_GETCURSEL, 0, 0 );
		if( nItem != LB_ERR )
		{
			m_hWnd_Dst = (HWND)::SendMessageAny( hwndList, LB_GETITEMDATA, nItem, 0 );

			/* トレイからエディタへの編集ファイル名要求通知 */
			::SendMessageAny( m_hWnd_Dst, MYWM_GETFILEINFO, 0, 0 );
			pFileInfo = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;

			_tcscpy( m_szFile2, pFileInfo->m_szPath );
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
	if( ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_NOTIFYNOTFOUND ) == BST_CHECKED )
		m_pShareData->m_Common.m_sSearch.m_bNOTIFYNOTFOUND = TRUE;
	else
		m_pShareData->m_Common.m_sSearch.m_bNOTIFYNOTFOUND = FALSE;

	/* 先頭（末尾）から再検索 */
	if( ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_SEARCHALL ) == BST_CHECKED )
		m_pShareData->m_Common.m_sSearch.m_bSearchAll = TRUE;
	else
		m_pShareData->m_Common.m_sSearch.m_bSearchAll = FALSE;

	//相手ファイルが指定されてなければキャンセル
	//	2004.02.21 MIK 相手が無題だと比較できないので判定削除
	//if( m_szFile2[0] == '\0' ) ret = FALSE;

	return ret;
}

BOOL CDlgDiff::OnCbnSelChange( HWND hwndCtl, int wID )
{
	HWND	hwndList;

	hwndList = GetDlgItem( GetHwnd(), IDC_LIST_DIFF_FILES );

	if( hwndList == hwndCtl )
	{
		::CheckDlgButton( GetHwnd(), IDC_RADIO_DIFF_DST1, FALSE );
		::CheckDlgButton( GetHwnd(), IDC_RADIO_DIFF_DST2, TRUE );
		return TRUE;
	}

	/* 基底クラスメンバ */
	return CDialog::OnCbnSelChange( hwndCtl, wID );
}

BOOL CDlgDiff::OnEditChange( HWND hwndCtl, int wID )
{
	HWND	hwndEdit;

	hwndEdit = GetDlgItem( GetHwnd(), IDC_EDIT_DIFF_DST );

	if( hwndEdit == hwndCtl )
	{
		::CheckDlgButton( GetHwnd(), IDC_RADIO_DIFF_DST1, TRUE );
		::CheckDlgButton( GetHwnd(), IDC_RADIO_DIFF_DST2, FALSE );
		//	Feb. 28, 2004 genta 選択解除前に前回の位置を記憶して選択解除
		int n = ::SendMessageAny( GetDlgItem( GetHwnd(), IDC_LIST_DIFF_FILES ), LB_GETCURSEL, 0, 0 );
		if( n != LB_ERR ){
			m_nIndexSave = n;
		}
		::SendMessageAny( ::GetDlgItem( GetHwnd(), IDC_LIST_DIFF_FILES), LB_SETCURSEL, (WPARAM)-1, 0 );
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
