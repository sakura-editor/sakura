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
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <string.h>
#include "CDlgOpenFile.h"
#include "CDlgDiff.h"
#include "etc_uty.h"
#include "Debug.h"
#include "CEditDoc.h"
#include "global.h"
#include "Funccode.h"
#include "mymessage.h"
#include "sakura_rc.h"
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
	m_bIsModified    = false;
	m_bIsModifiedDst = false;
	m_hWnd_Dst       = NULL;
	return;
}

/* モーダルダイアログの表示 */
int CDlgDiff::DoModal(
	HINSTANCE			hInstance,
	HWND				hwndParent,
	LPARAM				lParam,
	const TCHAR*		pszPath,		//自ファイル
	bool				bIsModified		//自ファイル編集中？
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
		MyWinHelp( m_hWnd, m_pszHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_DIFF_DIALOG) );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;

	case IDC_BUTTON_DIFF_DST:	/* 参照 */
		{
			CDlgOpenFile	cDlgOpenFile;
			TCHAR			szPath[_MAX_PATH];
			_tcscpy( szPath, m_szFile2 );
			/* ファイルオープンダイアログの初期化 */
			cDlgOpenFile.Create(
				m_hInstance,
				m_hWnd,
				_T("*.*"),
				m_szFile1 /*m_szFile2*/
			);
			if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) )
			{
				_tcscpy( m_szFile2, szPath );
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
		//	Feb. 28, 2004 genta 選択解除前に前回の位置を記憶
		{
			int n = ::SendMessage( GetDlgItem( m_hWnd, IDC_LIST_DIFF_FILES ), LB_GETCURSEL, 0, 0 );
			if( n != LB_ERR ){
				m_nIndexSave = n;
			}
		}
		::SendMessage( ::GetDlgItem( m_hWnd, IDC_LIST_DIFF_FILES), LB_SETCURSEL, (WPARAM)-1, 0 );
		return TRUE;

	case IDC_RADIO_DIFF_DST2:
		::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_DST1, FALSE );
		//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_EDIT_DIFF_DST ), FALSE );
		//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_DIFF_DST ), FALSE );
		//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_LIST_DIFF_FILES ), TRUE );
		{
			//	Aug. 9, 2003 genta
			//	ListBoxが選択されていなかったら，先頭のファイルを選択する．
			HWND hwndList = GetDlgItem( m_hWnd, IDC_LIST_DIFF_FILES );
			if( ::SendMessage( hwndList, LB_GETCURSEL, 0, 0 ) == LB_ERR )
			{
				::SendMessage( hwndList, LB_SETCURSEL, m_nIndexSave, 0 );
			}
		}
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

	/* 見つからないときメッセージを表示 */
	::CheckDlgButton( m_hWnd, IDC_CHECK_NOTIFYNOTFOUND, m_pShareData->m_Common.m_sSearch.m_bNOTIFYNOTFOUND );
	
	/* 先頭（末尾）から再検索 */
	::CheckDlgButton( m_hWnd, IDC_CHECK_SEARCHALL, m_pShareData->m_Common.m_sSearch.m_bSearchAll );

	/* 編集中のファイル一覧を作成する */
	{
		HWND		hwndList;
		int			nRowNum;
		EditNode	*pEditNode;
		EditInfo	*pFileInfo;
		int			i;
		int			nItem;
		TCHAR		szName[_MAX_PATH];
		CEditDoc*	pCEditDoc = (CEditDoc*)m_lParam;
		int			count = 0;

		/* リストのハンドル取得 */
		hwndList = :: GetDlgItem( m_hWnd, IDC_LIST_DIFF_FILES );

		/* 現在開いている編集窓のリストをメニューにする */
		nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNode, TRUE );
		if( nRowNum > 0 )
		{
			// 水平スクロール幅は実際に表示する文字列の幅を計測して決める	// 2009.09.26 ryoji
			HDC hDC = ::GetDC( hwndList );
			HFONT hFont = (HFONT)::SendMessage(hwndList, WM_GETFONT, 0, 0);
			HFONT hFontOld = (HFONT)::SelectObject(hDC, hFont);
			int nExtent = 0;	// 文字列の横幅
			for( i = 0; i < nRowNum; i++ )
			{
				/* トレイからエディタへの編集ファイル名要求通知 */
				::SendMessage( pEditNode[i].m_hWnd, MYWM_GETFILEINFO, 0, 0 );
				pFileInfo = (EditInfo*)&m_pShareData->m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO;

				/* 自分ならスキップ */
				if ( pEditNode[i].m_hWnd == pCEditDoc->m_hwndParent )
				{
					// 同じ形式にしておく。ただしアクセスキー番号はなし
					CShareData::getInstance()->GetMenuFullLabel_WinListNoEscape( szName, _countof(szName), pFileInfo, pEditNode[i].m_nId, -1 );
					::SetDlgItemText( m_hWnd, IDC_STATIC_DIFF_SRC, szName );
					continue;
				}

				// 番号はウィンドウ一覧と同じ番号を使う
				CShareData::getInstance()->GetMenuFullLabel_WinListNoEscape( szName, _countof(szName), pFileInfo, pEditNode[i].m_nId, i );


				/* リストに登録する */
				nItem = ::SendMessage( hwndList, LB_ADDSTRING, 0, (LPARAM)(char*)szName );
				::SendMessage( hwndList, LB_SETITEMDATA, nItem, (LPARAM)pEditNode[i].m_hWnd );
				count++;

				// 横幅を計算する
				SIZE sizeExtent;
				if( ::GetTextExtentPoint32( hDC, szName, _tcslen(szName), &sizeExtent ) && sizeExtent.cx > nExtent ){
					nExtent = sizeExtent.cx;
				}
			}

			delete [] pEditNode;
			// 2002/11/01 Moca 追加 リストビューの横幅を設定。これをやらないと水平スクロールバーが使えない
			::SelectObject(hDC, hFontOld);
			::ReleaseDC( hwndList, hDC );
			::SendMessage( hwndList, LB_SETHORIZONTALEXTENT, (WPARAM)(nExtent + 8), 0 );

			/* 最初を選択 */
			//::SendMessage( hwndList, LB_SETCURSEL, (WPARAM)0, 0 );
		}

		//	From Here 2004.02.22 じゅうじ
		//	開いているファイルがある場合には初期状態でそちらを優先
		if( count == 0 )
		{
			/* 相手ファイルの選択 */
			::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_DST1, TRUE );
			::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_DST2, FALSE );
			/* その他の編集中リストはなし */
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_RADIO_DIFF_DST2 ), FALSE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_LIST_DIFF_FILES ), FALSE );
		}
		else
		{
			/* 相手ファイルの選択 */
			::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_DST1, FALSE );
			::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_DST2, TRUE );
			//	ListBoxが選択されていなかったら，先頭のファイルを選択する．
			HWND hwndList = GetDlgItem( m_hWnd, IDC_LIST_DIFF_FILES );
			if( ::SendMessage( hwndList, LB_GETCURSEL, 0, 0 ) == LB_ERR )
			{
			    ::SendMessage( hwndList, LB_SETCURSEL, 0 /*先頭アイテム*/, 0 );
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
	_tcscpy( m_szFile2, _T("") );
	m_hWnd_Dst = NULL;
	m_bIsModifiedDst = false;
	if( ::IsDlgButtonChecked( m_hWnd, IDC_RADIO_DIFF_DST1 ) == BST_CHECKED )
	{
		::GetDlgItemText( m_hWnd, IDC_EDIT_DIFF_DST, m_szFile2, sizeof( m_szFile2 ) );
		//	2004.05.19 MIK 外部ファイルが指定されていない場合はキャンセル
		//相手ファイルが指定されてなければキャンセル
		if( m_szFile2[0] == '\0' ) ret = FALSE;

	}
	else if( ::IsDlgButtonChecked( m_hWnd, IDC_RADIO_DIFF_DST2 ) == BST_CHECKED )
	{
		HWND		hwndList;
		int			nItem;
		EditInfo	*pFileInfo;

		/* リストから相手のウインドウハンドルを取得 */
		hwndList = :: GetDlgItem( m_hWnd, IDC_LIST_DIFF_FILES );
		nItem = ::SendMessage( hwndList, LB_GETCURSEL, 0, 0 );
		if( nItem != LB_ERR )
		{
			m_hWnd_Dst = (HWND)::SendMessage( hwndList, LB_GETITEMDATA, nItem, 0 );

			/* トレイからエディタへの編集ファイル名要求通知 */
			::SendMessage( m_hWnd_Dst, MYWM_GETFILEINFO, 0, 0 );
			pFileInfo = (EditInfo*)&m_pShareData->m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO;

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
	if( ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_NOTIFYNOTFOUND ) == BST_CHECKED )
		m_pShareData->m_Common.m_sSearch.m_bNOTIFYNOTFOUND = TRUE;
	else
		m_pShareData->m_Common.m_sSearch.m_bNOTIFYNOTFOUND = FALSE;

	/* 先頭（末尾）から再検索 */
	if( ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_SEARCHALL ) == BST_CHECKED )
		m_pShareData->m_Common.m_sSearch.m_bSearchAll = TRUE;
	else
		m_pShareData->m_Common.m_sSearch.m_bSearchAll = FALSE;

	//相手ファイルが指定されてなければキャンセル
	//	2004.02.21 MIK 相手が無題だと比較できないので判定削除
	//if( m_szFile2[0] == '\0' ) ret = FALSE;

	return ret;
}

BOOL CDlgDiff::OnLbnSelChange( HWND hwndCtl, int wID )
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
	return CDialog::OnLbnSelChange( hwndCtl, wID );
}

BOOL CDlgDiff::OnEnChange( HWND hwndCtl, int wID )
{
	HWND	hwndEdit;

	hwndEdit = GetDlgItem( m_hWnd, IDC_EDIT_DIFF_DST );

	if( hwndEdit == hwndCtl )
	{
		::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_DST1, TRUE );
		::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_DST2, FALSE );
		//	Feb. 28, 2004 genta 選択解除前に前回の位置を記憶して選択解除
		int n = ::SendMessage( GetDlgItem( m_hWnd, IDC_LIST_DIFF_FILES ), LB_GETCURSEL, 0, 0 );
		if( n != LB_ERR ){
			m_nIndexSave = n;
		}
		::SendMessage( ::GetDlgItem( m_hWnd, IDC_LIST_DIFF_FILES), LB_SETCURSEL, (WPARAM)-1, 0 );
		return TRUE;
	}

	/* 基底クラスメンバ */
	return CDialog::OnEnChange( hwndCtl, wID );
}

LPVOID CDlgDiff::GetHelpIdTable( void )
{
	return (LPVOID)p_helpids;
}

/*[EOF]*/
