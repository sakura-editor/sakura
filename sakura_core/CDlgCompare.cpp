/*!	@file
	@brief ファイル比較ダイアログボックス

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, JEPRO
	Copyright (C) 2001, Stonee, genta, JEPRO, YAZAKI
	Copyright (C) 2002, aroka, MIK, Moca
	Copyright (C) 2003, MIK
	Copyright (C) 2006, ryoji
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CDlgCompare.h"
#include "etc_uty.h"
#include "Debug.h"
#include "CEditDoc.h"
#include "global.h"
#include "Funccode.h"		// Stonee, 2001/03/12
#include "mymessage.h"
#include "sakura_rc.h"
#include "sakura.hh"

// ファイル内容比較 CDlgCompare.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//12300
//	IDC_BUTTON1,					HIDC_CMP_BUTTON1,			//上下に表示
//	IDOK2,							HIDOK2_CMP,					//左右に表示
	IDOK,							HIDOK_CMP,					//OK
	IDCANCEL,						HIDCANCEL_CMP,				//キャンセル
	IDC_BUTTON_HELP,				HIDC_CMP_BUTTON_HELP,		//ヘルプ
	IDC_CHECK_TILE_H,				HIDC_CMP_CHECK_TILE_H,		//左右に表示
	IDC_LIST_FILES,					HIDC_CMP_LIST_FILES,		//ファイル一覧
	IDC_STATIC_COMPARESRC,			HIDC_CMP_STATIC_COMPARESRC,	//ソースファイル
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

CDlgCompare::CDlgCompare()
{
	m_bCompareAndTileHorz = TRUE;	/* 左右に並べて表示 */
	return;
}


/* モーダルダイアログの表示 */
int CDlgCompare::DoModal(
	HINSTANCE		hInstance,
	HWND			hwndParent,
	LPARAM			lParam,
	const TCHAR*	pszPath,
	bool			bIsModified,
	TCHAR*			pszComparePath,
	HWND*			phwndCompareWnd
)
{
	m_pszPath = pszPath;
	m_bIsModified = bIsModified;
	m_pszComparePath = pszComparePath;
	m_phwndCompareWnd = phwndCompareWnd;
	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_COMPARE, lParam );
}

BOOL CDlgCompare::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* 「内容比較」のヘルプ */
		//Stonee, 2001/03/12 第四引数を、機能番号からヘルプトピック番号を調べるようにした
		MyWinHelp( m_hWnd, m_pszHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_COMPARE) );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
//	From Here Oct. 10, 2000 JEPRO added  Ref. code はCDlgFind.cpp の OnBnClicked
//	チェックボックスをボタン化してCDlgCompare.cppに直接書き込んでみたが失敗
//	ダイアログのボタンは下に不可視化しておいてあります。
//	以下の追加コードは全部消して結構ですから誰か作ってください。水平スクロールも入れてくれるとなおうれしいです。
//	case IDC_BUTTON1:	/* 上下に表示 */
//		/* ダイアログデータの取得 */
//		return TRUE;
//	case IDOK:			/* 左右に表示 */
//		/* ダイアログデータの取得 */
//		HWND	hwndCompareWnd;
//		HWND*	phwndArr;
//		int		i;
//		phwndArr = new HWND[2];
//		phwndArr[0] = ::GetParent( m_hwndParent );
//		phwndArr[1] = hwndCompareWnd;
//		for( i = 0; i < 2; ++i ){
//			if( ::IsZoomed( phwndArr[i] ) ){
//				::ShowWindow( phwndArr[i], SW_RESTORE );
//			}
//		}
//		::TileWindows( NULL, MDITILE_VERTICAL, NULL, 2, phwndArr );
//		delete [] phwndArr;
//		CloseDialog( 0 );
//		return TRUE;
//	To Here Oct. 10, 2000
	case IDOK:			/* 左右に表示 */
		/* ダイアログデータの取得 */
		::EndDialog( m_hWnd, GetData() );
		return TRUE;
	case IDCANCEL:
		::EndDialog( m_hWnd, FALSE );
		return TRUE;
	}
	/* 基底クラスメンバ */
	return CDialog::OnBnClicked( wID );
}


/* ダイアログデータの設定 */
void CDlgCompare::SetData( void )
{
	CEditDoc*		pCEditDoc = (CEditDoc*)m_lParam;
	HWND			hwndList;
	int				nRowNum;
	EditNode*		pEditNodeArr;
	EditInfo*		pfi;
	int				i;
	TCHAR			szMenu[512];
	int				nItem;

	hwndList = :: GetDlgItem( m_hWnd, IDC_LIST_FILES );

//	2002/2/10 aroka ファイル名で比較しないため不用 (2001.12.26 YAZAKIさん)
//	//	Oct. 15, 2001 genta ファイル名判定の stricmpをbccでも期待通り動かすため
//	setlocale ( LC_ALL, "C" );

	/* 現在開いている編集窓のリストをメニューにする */
	nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
	if( nRowNum > 0 ){
		// 水平スクロール幅は実際に表示する文字列の幅を計測して決める	// 2009.09.26 ryoji
		HDC hDC = ::GetDC( hwndList );
		HFONT hFont = (HFONT)::SendMessage(hwndList, WM_GETFONT, 0, 0);
		HFONT hFontOld = (HFONT)::SelectObject(hDC, hFont);
		int nExtent = 0;	// 文字列の横幅
		for( i = 0; i < nRowNum; ++i ){
			/* トレイからエディタへの編集ファイル名要求通知 */
			::SendMessage( pEditNodeArr[i].m_hWnd, MYWM_GETFILEINFO, 0, 0 );
			pfi = (EditInfo*)&m_pShareData->m_EditInfo_MYWM_GETFILEINFO;

//@@@ 2001.12.26 YAZAKI ファイル名で比較すると(無題)だったときに問題同士の比較ができない
//			if( 0 == stricmp( pfi->m_szPath, m_pszPath ) ){
			if (pEditNodeArr[i].m_hWnd == pCEditDoc->m_hwndParent){
				continue;
			}
			wsprintf(
				szMenu,
				_T("%s %s"),
				(0 < lstrlen(pfi->m_szPath))?pfi->m_szPath : _T("(無題)"),
				pfi->m_bIsModified ? _T("*") : _T(" ")
			);
			// gm_pszCodeNameArr_3 からコピーするように変更
			if( IsValidCodeTypeExceptSJIS(pfi->m_nCharCode) ){
				strcat( szMenu, gm_pszCodeNameArr_3[pfi->m_nCharCode] );
			}
			nItem = ::SendMessage( hwndList, LB_ADDSTRING, 0, (LPARAM)(char*)szMenu );
			::SendMessage( hwndList, LB_SETITEMDATA, nItem, (LPARAM)pEditNodeArr[i].m_hWnd );

			// 横幅を計算する
			SIZE sizeExtent;
			if( ::GetTextExtentPoint32( hDC, szMenu, _tcslen(szMenu), &sizeExtent ) && sizeExtent.cx > nExtent ){
				nExtent = sizeExtent.cx;
			}
		}
		delete [] pEditNodeArr;
		// 2002/11/01 Moca 追加 リストビューの横幅を設定。これをやらないと水平スクロールバーが使えない
		::SelectObject(hDC, hFontOld);
		::ReleaseDC( hwndList, hDC );
		::SendMessage( hwndList, LB_SETHORIZONTALEXTENT, (WPARAM)(nExtent + 8), 0 );
	}
	::SendMessage( hwndList, LB_SETCURSEL, (WPARAM)0, 0 );
	TCHAR	szWork[512];
	wsprintf( szWork, _T("%s %s"),
		(0 < _tcslen( m_pszPath )?m_pszPath:_T("(無題)") ),
		m_bIsModified?_T("*"):_T("")
	);
	::SetDlgItemText( m_hWnd, IDC_STATIC_COMPARESRC, szWork );
	/* 左右に並べて表示 */
	//@@@ 2003.06.12 MIK
	// TAB 1ウィンドウ表示のときは並べて比較できなくする
	if( TRUE  == m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd
	 && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin )
	{
		m_bCompareAndTileHorz = FALSE;
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHECK_TILE_H ), FALSE );
	}
	::CheckDlgButton( m_hWnd, IDC_CHECK_TILE_H, m_bCompareAndTileHorz );
	return;
}




/* ダイアログデータの取得 */
/* TRUE==正常  FALSE==入力エラー */
int CDlgCompare::GetData( void )
{
	HWND			hwndList;
	int				nItem;
	EditInfo*		pfi;
	hwndList = :: GetDlgItem( m_hWnd, IDC_LIST_FILES );
	nItem = ::SendMessage( hwndList, LB_GETCURSEL, 0, 0 );
	*m_phwndCompareWnd = (HWND)::SendMessage( hwndList, LB_GETITEMDATA, nItem, 0 );
	/* トレイからエディタへの編集ファイル名要求通知 */
	::SendMessage( *m_phwndCompareWnd, MYWM_GETFILEINFO, 0, 0 );
	pfi = (EditInfo*)&m_pShareData->m_EditInfo_MYWM_GETFILEINFO;

	_tcscpy( m_pszComparePath, pfi->m_szPath );

	/* 左右に並べて表示 */
	m_bCompareAndTileHorz = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_TILE_H );

	return TRUE;
}

//@@@ 2002.01.18 add start
LPVOID CDlgCompare::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

/*[EOF]*/
