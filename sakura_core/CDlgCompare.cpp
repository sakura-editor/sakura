//	$Id$
/*!	@file
	@brief ファイル比較ダイアログボックス

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, Stonee, genta, JEPRO, YAZAKI
	Copyright (C) 2002, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "sakura_rc.h"
#include "CDlgCompare.h"
#include "etc_uty.h"
#include "debug.h"
#include "CEditDoc.h"
//#include "CEditView.h"	// Oct. 10, 2000 JEPRO added	//Oct. 10, 2000 JEPRO チェックボックスをボタン化すればこの行は必要？
#include "global.h"
#include "funccode.h"		// Stonee, 2001/03/12
#include "mymessage.h"
//#include <locale.h>	// 2002/2/10 aroka

// ファイル内容比較 CDlgCompare.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
const DWORD p_helpids[] = {	//12300
	IDC_BUTTON1,					HIDC_CMP_BUTTON1,			//上下に表示
	IDOK2,							HIDOK2_CMP,					//左右に表示
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
//	m_bCompareAndTileHorz = TRUE;	/* 左右に並べて表示 */	//Oct. 10, 2000 JEPRO チェックボックスをボタン化すればこの行は不要のはず
	return;
}


/* モーダルダイアログの表示 */
int CDlgCompare::DoModal(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	LPARAM		lParam,
	const char*	pszPath,
	BOOL		bIsModified,
	char*		pszComparePath,
	HWND*		phwndCompareWnd
)
{
	m_pszPath = pszPath;
	m_bIsModified = bIsModified;
	m_pszComparePath = pszComparePath;
	m_phwndCompareWnd = phwndCompareWnd;
	return CDialog::DoModal( hInstance, hwndParent, IDD_COMPARE, lParam );
}

BOOL CDlgCompare::OnBnClicked( int wID )
{
//	CEditView*	pcEditView = (CEditView*)m_lParam;	//	Oct. 10, 2000 JEPRO added	//Oct. 10, 2000 JEPRO チェックボックスをボタン化すればこの行は必要？
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* 「内容比較」のヘルプ */
		//Stonee, 2001/03/12 第四引数を、機能番号からヘルプトピック番号を調べるようにした
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_COMPARE) );
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
	FileInfo*		pfi;
	int				i;
	char			szMenu[512];
	int				nItem;

	hwndList = :: GetDlgItem( m_hWnd, IDC_LIST_FILES );

//	2002/2/10 aroka ファイル名で比較しないため不用 (2001.12.26 YAZAKIさん)
//	//	Oct. 15, 2001 genta ファイル名判定の stricmpをbccでも期待通り動かすため
//	setlocale ( LC_ALL, "C" );

	/* 現在開いている編集窓のリストをメニューにする */
	nRowNum = m_cShareData.GetOpenedWindowArr( &pEditNodeArr, TRUE );
	if( nRowNum > 0 ){
		for( i = 0; i < nRowNum; ++i ){
			/* トレイからエディタへの編集ファイル名要求通知 */
			::SendMessage( pEditNodeArr[i].m_hWnd, MYWM_GETFILEINFO, 0, 0 );
			pfi = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;

//@@@ 2001.12.26 YAZAKI ファイル名で比較すると(無題)だったときに問題同士の比較ができない
//			if( 0 == stricmp( pfi->m_szPath, m_pszPath ) ){
			if (pEditNodeArr[i].m_hWnd == pCEditDoc->m_hwndParent){
				continue;
			}
			wsprintf( szMenu, "%s %s",
				(0 < lstrlen(pfi->m_szPath))?pfi->m_szPath:"(無題)",
				pfi->m_bIsModified ? "*":" "
			);
			if( 0 != pfi->m_nCharCode ){		/* 文字コード種別 */
				switch( pfi->m_nCharCode ){
				case CODE_JIS:	/* JIS */
					strcat( szMenu, "  [JIS]" );
					break;
				case CODE_EUC:	/* EUC */
					strcat( szMenu, "  [EUC]" );
					break;
				case CODE_UNICODE:	/* Unicode */
					strcat( szMenu, "  [Unicode]" );
					break;
				case CODE_UTF8:	/* UTF-8 */
					strcat( szMenu, "  [UTF-8]" );
					break;
				case CODE_UTF7:	/* UTF-7 */
					strcat( szMenu, "  [UTF-7]" );
					break;
				}
			}
			nItem = ::SendMessage( hwndList, LB_ADDSTRING, 0, (LPARAM)(char*)szMenu );
			::SendMessage( hwndList, LB_SETITEMDATA, nItem, (LPARAM)pEditNodeArr[i].m_hWnd );
		}
		delete [] pEditNodeArr;
	}
	::SendMessage( hwndList, LB_SETCURSEL, (WPARAM)0, 0 );
	char	szWork[512];
	wsprintf( szWork, "%s %s",
		(0 < lstrlen( m_pszPath )?m_pszPath:"(無題)" ),
		m_bIsModified?"*":""
	);
	::SetDlgItemText( m_hWnd, IDC_STATIC_COMPARESRC, szWork );
	/* 左右に並べて表示 */
	::CheckDlgButton( m_hWnd, IDC_CHECK_TILE_H, m_bCompareAndTileHorz );
//	::CheckDlgButton( m_hWnd, IDC_CHECK_TILE_H, m_bCompareAndTileHorz );	//Oct. 10, 2000 JEPRO チェックボックスをボタン化すればこの行は不要のはず
	return;
}




/* ダイアログデータの取得 */
/* TRUE==正常  FALSE==入力エラー */
int CDlgCompare::GetData( void )
{
	HWND			hwndList;
	int				nItem;
//	HWND			hwndCompareFile;
	FileInfo*		pfi;
	hwndList = :: GetDlgItem( m_hWnd, IDC_LIST_FILES );
	nItem = ::SendMessage( hwndList, LB_GETCURSEL, 0, 0 );
	*m_phwndCompareWnd = (HWND)::SendMessage( hwndList, LB_GETITEMDATA, nItem, 0 );
	/* トレイからエディタへの編集ファイル名要求通知 */
	::SendMessage( *m_phwndCompareWnd, MYWM_GETFILEINFO, 0, 0 );
//	pfi = (FileInfo*)m_pShareData->m_szWork;
	pfi = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;

	strcpy( m_pszComparePath, pfi->m_szPath );

	/* 左右に並べて表示 */
	m_bCompareAndTileHorz = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_TILE_H );
//	m_bCompareAndTileHorz = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_TILE_H );	//Oct. 10, 2000 JEPRO チェックボックスをボタン化すればこの行は不要のはず

	return TRUE;
}

//@@@ 2002.01.18 add start
LPVOID CDlgCompare::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

/*[EOF]*/
