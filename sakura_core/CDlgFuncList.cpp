//	$Id$
/*!	@file
	アウトライン解析ダイアログボックス

	@author Norio Nakatani
	@date 2001/06/23 N.Nakatani Visual Basicのアウトライン解析
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <windows.h>
#include <commctrl.h>
//#include <stdio.h>
#include "sakura_rc.h"
#include "CDlgFuncList.h"
#include "etc_uty.h"
#include "debug.h"
#include "global.h"
#include "CEditView.h"
#include "funccode.h"		//Stonee, 2001/03/12

/*! ソート比較用プロシージャ */
int CALLBACK _CompareFunc_( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort )
{
	CFuncInfo*		pcFuncInfo1;
	CFuncInfo*		pcFuncInfo2;
	CDlgFuncList*	pcDlgFuncList;
	pcDlgFuncList = (CDlgFuncList*)lParamSort;

	pcFuncInfo1 = pcDlgFuncList->m_pcFuncInfoArr->GetAt( lParam1 );
	if( NULL == pcFuncInfo1 ){
		return -1;
	}
	pcFuncInfo2 = pcDlgFuncList->m_pcFuncInfoArr->GetAt( lParam2 );
	if( NULL == pcFuncInfo2 ){
		return -1;
	}
	if( 0 == pcDlgFuncList->m_nSortCol){	/* ソートする列番号 */
		return strcmp( pcFuncInfo1->m_cmemFuncName.GetPtr( NULL ), pcFuncInfo2->m_cmemFuncName.GetPtr( NULL ) );
	}
	if( 1 == pcDlgFuncList->m_nSortCol){	/* ソートする列番号 */
		if( pcFuncInfo1->m_nFuncLineCRLF < pcFuncInfo2->m_nFuncLineCRLF ){
			return -1;
		}else
		if( pcFuncInfo1->m_nFuncLineCRLF == pcFuncInfo2->m_nFuncLineCRLF ){
			return 0;
		}else{
			return 1;
		}
	}
	// From Here 2001.12.07 hor
	if( 2 == pcDlgFuncList->m_nSortCol){	/* ソートする列番号 */
		if( pcFuncInfo1->m_nInfo < pcFuncInfo2->m_nInfo ){
			return -1;
		}else
		if( pcFuncInfo1->m_nInfo == pcFuncInfo2->m_nInfo ){
			return 0;
		}else{
			return 1;
		}
	}
	// To Here 2001.12.07 hor
	return -1;
}



CDlgFuncList::CDlgFuncList()
{
	m_pcFuncInfoArr = NULL;		/* 関数情報配列 */
	m_nCurLine = 0;				/* 現在行 */
	m_nSortCol = 0;				/* ソートする列番号 */
	m_bLineNumIsCRLF = FALSE;	/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
	return;
}



//	/* モーダルダイアログの表示 */
//	int CDlgFuncList::DoModal(
//		HINSTANCE		hInstance,
//		HWND			hwndParent,
//		LPARAM			lParam,
//		CFuncInfoArr*	pcFuncInfoArr,
//		int				nCurLine,
//		int				nListType,
//		int				bLineNumIsCRLF	/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
//	)
//	{
//		m_pcFuncInfoArr = pcFuncInfoArr;	/* 関数情報配列 */
//		m_nCurLine = nCurLine;				/* 現在行 */
//		m_nListType = nListType;			/* 一覧の種類 */
//		m_bLineNumIsCRLF = bLineNumIsCRLF;	/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
//		return CDialog::DoModal( hInstance, hwndParent, IDD_FUNCLIST, lParam );
//	}


/* モードレスダイアログの表示 */
HWND CDlgFuncList::DoModeless(
	HINSTANCE		hInstance,
	HWND			hwndParent,
	LPARAM			lParam,
	CFuncInfoArr*	pcFuncInfoArr,
	int				nCurLine,
	int				nListType,
	int				bLineNumIsCRLF		/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
)
{
	m_pcFuncInfoArr = pcFuncInfoArr;	/* 関数情報配列 */
	m_nCurLine = nCurLine;				/* 現在行 */
	m_nListType = nListType;			/* 一覧の種類 */
	m_bLineNumIsCRLF = bLineNumIsCRLF;	/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
	return CDialog::DoModeless( hInstance, hwndParent, IDD_FUNCLIST, lParam, SW_SHOW );
}

/* モードレス時：検索対象となるビューの変更 */
void CDlgFuncList::ChangeView( LPARAM pcEditView )
{
	m_lParam = pcEditView;
	return;
}

/*! ダイアログデータの設定 */
void CDlgFuncList::SetData( void/*HWND hwndDlg*/ )
{
	int				i;
	char			szText[2048];
	CFuncInfo*		pcFuncInfo;
	LV_ITEM			item;
	HWND			hwndList;
	HWND			hwndTree;
	int				bSelected;
	int				nFuncLineOld;
	int				nSelectedLine;
	RECT			rc;
	hwndList = ::GetDlgItem( m_hWnd, IDC_LIST1 );
	hwndTree = ::GetDlgItem( m_hWnd, IDC_TREE1 );
	m_cmemClipText.SetDataSz( "" );	/* クリップボードコピー用テキスト */

	if( OUTLINE_CPP == m_nListType ){	/* C++メソッドリスト */
		//	May 18, 2001 genta
		//	Windowがいなくなると後で都合が悪いので、表示しないだけにしておく
		//	::DestroyWindow( hwndList );
		::ShowWindow( hwndList, SW_HIDE );
		m_nViewType = 1;
		/* ツリーコントロールの初期化：C++メソッドツリー */
		SetTreeCpp( m_hWnd );
		::SetWindowText( m_hWnd, "C++ メソッドツリー" );
	}else
	if( OUTLINE_TEXT == m_nListType ){ /* テキスト・トピックリスト */
		//	May 18, 2001 genta
		//	Windowがいなくなると後で都合が悪いので、表示しないだけにしておく
		//::DestroyWindow( hwndList );
		::ShowWindow( hwndList, SW_HIDE );
		m_nViewType = 1;
		/* ツリーコントロールの初期化：テキストトピックツリー */
		SetTreeTxt( m_hWnd );
		::SetWindowText( m_hWnd, "テキスト トピックツリー" );
	}else
	if( OUTLINE_JAVA == m_nListType ){ /* Javaメソッドツリー */
		//	May 18, 2001 genta
		//	Windowがいなくなると後で都合が悪いので、表示しないだけにしておく
		//	::DestroyWindow( hwndList );
		::ShowWindow( hwndList, SW_HIDE );
		m_nViewType = 1;
		/* ツリーコントロールの初期化：Javaメソッドツリー */
		SetTreeJava( m_hWnd, TRUE );
		::SetWindowText( m_hWnd, "Java メソッドツリー" );
	}else
	if( OUTLINE_COBOL == m_nListType ){ /* COBOL アウトライン */
		//	May 18, 2001 genta
		//	Windowがいなくなると後で都合が悪いので、表示しないだけにしておく
		//	::DestroyWindow( hwndList );
		::ShowWindow( hwndList, SW_HIDE );
		m_nViewType = 1;
		/* ツリーコントロールの初期化：COBOL アウトライン */
		SetTreeJava( m_hWnd, FALSE );
		::SetWindowText( m_hWnd, "COBOL アウトライン" );
// From Here 2001.12.03 hor
//	ブックマークの一覧を無理矢理つくる
	}else
	if( OUTLINE_BOOKMARK == m_nListType ){	/* ブックマークリスト */
		::ShowWindow( hwndTree, SW_HIDE );
		SetTreeBookMark( m_hWnd );
		m_nViewType = 0;
		::SetWindowText( m_hWnd, "ブックマーク" );
// To Here 2001.12.03 hor
	}else{
		switch( m_nListType ){
		case OUTLINE_C:
			::SetWindowText( m_hWnd, "C 関数一覧" );
			break;
		case OUTLINE_PLSQL:
			::SetWindowText( m_hWnd, "PL/SQL 関数一覧" );
			break;
		case OUTLINE_ASM:
			::SetWindowText( m_hWnd, "アセンブラ アウトライン" );
			break;
		case OUTLINE_PERL:	//	Sep. 8, 2000 genta
			::SetWindowText( m_hWnd, "Perl 関数一覧" );
			break;
		case OUTLINE_VB:	// 2001/06/23 N.Nakatani for Visual Basic
			::SetWindowText( m_hWnd, "Visual Basic アウトライン" );
			break;
//		case OUTLINE_COBOL:
//			::SetWindowText( m_hWnd, "COBOLアウトライン" );
//			break;
		}
		//	May 18, 2001 genta
		//	Windowがいなくなると後で都合が悪いので、表示しないだけにしておく
		//::DestroyWindow( hwndTree );
		::ShowWindow( hwndTree, SW_HIDE );
		m_nViewType = 0;
		::EnableWindow( ::GetDlgItem( m_hWnd , IDC_BUTTON_COPY ), TRUE );
		nFuncLineOld = 0;
		bSelected = FALSE;
		for( i = 0; i < m_pcFuncInfoArr->GetNum(); ++i ){
			pcFuncInfo = m_pcFuncInfoArr->GetAt( i );
			if( !bSelected ){
				if( i == 0 && m_nCurLine < pcFuncInfo->m_nFuncLineLAYOUT ){
					bSelected = TRUE;
					nSelectedLine = i;
				}else
				if( i > 0 && nFuncLineOld <= m_nCurLine && m_nCurLine < pcFuncInfo->m_nFuncLineLAYOUT ){
					bSelected = TRUE;
					nSelectedLine = i - 1;
				}
			}
			nFuncLineOld = pcFuncInfo->m_nFuncLineLAYOUT;
		}
		if( 0 < m_pcFuncInfoArr->GetNum() && !bSelected ){
			bSelected = TRUE;
			nSelectedLine =  m_pcFuncInfoArr->GetNum() - 1;
		}
		for( i = 0; i < m_pcFuncInfoArr->GetNum(); ++i ){
			/* 現在の解析結果要素 */
			pcFuncInfo = m_pcFuncInfoArr->GetAt( i );

			item.mask = LVIF_TEXT | LVIF_PARAM;
			item.pszText = pcFuncInfo->m_cmemFuncName.GetPtr( NULL );
			item.iItem = i;
			item.iSubItem = 0;
			item.lParam	= i;
			ListView_InsertItem( hwndList, &item);

			/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
			if(m_bLineNumIsCRLF ){
				wsprintf( szText, "%d", pcFuncInfo->m_nFuncLineCRLF );
			}else{
				wsprintf( szText, "%d", pcFuncInfo->m_nFuncLineLAYOUT );
			}
			item.mask = LVIF_TEXT;
			item.pszText = szText;
			item.iItem = i;
			item.iSubItem = 1;
			ListView_SetItem( hwndList, &item);

			item.mask = LVIF_TEXT;
			if(  1 == pcFuncInfo->m_nInfo ){item.pszText = "宣言";}else
			if( 10 == pcFuncInfo->m_nInfo ){item.pszText = "関数宣言";}else
			if( 20 == pcFuncInfo->m_nInfo ){item.pszText = "プロシージャ宣言";}else
			if( 11 == pcFuncInfo->m_nInfo ){item.pszText = "関数";}else
			if( 21 == pcFuncInfo->m_nInfo ){item.pszText = "プロシージャ";}else
			if( 31 == pcFuncInfo->m_nInfo ){item.pszText = "■パッケージ仕様部";}else
			if( 41 == pcFuncInfo->m_nInfo ){item.pszText = "■パッケージ本体部";}else
			if( 50 == pcFuncInfo->m_nInfo ){item.pszText = "PROC";}else
			if( 51 == pcFuncInfo->m_nInfo ){item.pszText = "ラベル";}else
			if( 52 == pcFuncInfo->m_nInfo ){item.pszText = "ENDP";}else{

			// 2001/06/23 N.Nakatani for Visual Basic
			//	Jun. 26, 2001 genta 半角かな→全角に
			if( 60 == pcFuncInfo->m_nInfo ){item.pszText = "ステートメント宣言";}else
			if( 61 == pcFuncInfo->m_nInfo ){item.pszText = "関数宣言";}else
			if( 62 == pcFuncInfo->m_nInfo ){item.pszText = "ステートメント";}else
			if( 63 == pcFuncInfo->m_nInfo ){item.pszText = "関数";}else

				item.pszText = "";
			}
			item.iItem = i;
			item.iSubItem = 2;
			ListView_SetItem( hwndList, &item);

			/* クリップボードにコピーするテキストを編集 */
			wsprintf( szText, "%s(%d): %s(%s)\r\n",
				m_pcFuncInfoArr->m_szFilePath,				/* 解析対象ファイル名 */
				pcFuncInfo->m_nFuncLineCRLF,				/* 検出行番号 */
				pcFuncInfo->m_cmemFuncName.GetPtr( NULL ),	/* 検出結果 */
				item.pszText								/* 検出結果の種類 */
			);
//			m_cmemClipText.Append( (const char *)szText, lstrlen( szText ) );	/* クリップボードコピー用テキスト */
			m_cmemClipText.AppendSz( (const char *)szText );					/* クリップボードコピー用テキスト */
		}
		if( bSelected ){
			ListView_GetItemRect( hwndList, 0, &rc, LVIR_BOUNDS );
			ListView_Scroll( hwndList, 0, nSelectedLine * ( rc.bottom - rc.top ) );
			ListView_SetItemState( hwndList, nSelectedLine, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
//			ListView_GetItemRect( hwndList, nSelectedLine, &rc, LVIR_BOUNDS );
//			::PostMessage( hwndList, WM_LBUTTONDOWN, 0, MAKELONG( rc.left + 2, rc.bottom - 2) );
		}
		/* 列の幅をデータに合わせて調整 */
		ListView_SetColumnWidth( hwndList, 0, LVSCW_AUTOSIZE );
		ListView_SetColumnWidth( hwndList, 1, LVSCW_AUTOSIZE );
		ListView_SetColumnWidth( hwndList, 2, LVSCW_AUTOSIZE );
		ListView_SetColumnWidth( hwndList, 0, ListView_GetColumnWidth( hwndList, 0 ) + 16 );
		ListView_SetColumnWidth( hwndList, 1, ListView_GetColumnWidth( hwndList, 1 ) + 16 );
		ListView_SetColumnWidth( hwndList, 2, ListView_GetColumnWidth( hwndList, 2 ) + 16 );
	}
	/* アウトライン ダイアログを自動的に閉じる */
	::CheckDlgButton( m_hWnd, IDC_CHECK_bAutoCloseDlgFuncList, m_pShareData->m_Common.m_bAutoCloseDlgFuncList );
	return;
}




/* ダイアログデータの取得 */
/* 0==条件未入力   0より大きい==正常   0より小さい==入力エラー */
int CDlgFuncList::GetData( void )
{
	HWND			hwndList;
	HWND			hwndTree;
	int				nItem;
	CFuncInfo*		pcFuncInfo;
	LV_ITEM			item;
	int				nLineTo;
	HTREEITEM		htiItem;
	TV_ITEM			tvi;
	char			szLabel[32];

	/* アウトライン ダイアログを自動的に閉じる */
	m_pShareData->m_Common.m_bAutoCloseDlgFuncList = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_bAutoCloseDlgFuncList );

	nLineTo = -1;
	hwndList = ::GetDlgItem( m_hWnd, IDC_LIST1 );
	if( m_nViewType == 0 ){
		//	List
		nItem = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
		if( -1 == nItem ){
			return -1;
		}
		item.mask = LVIF_PARAM;
		item.iItem = nItem;
		item.iSubItem = 0;
		ListView_GetItem( hwndList, &item );
		pcFuncInfo = m_pcFuncInfoArr->GetAt( item.lParam );
		nLineTo = pcFuncInfo->m_nFuncLineCRLF;
	}else{
		hwndTree = ::GetDlgItem( m_hWnd, IDC_TREE1 );
		if( NULL != hwndTree ){
			htiItem = TreeView_GetSelection( hwndTree );

			tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
			tvi.hItem = htiItem;
			tvi.pszText = szLabel;
			tvi.cchTextMax = sizeof( szLabel );
			if( TreeView_GetItem( hwndTree, &tvi ) ){
				if( -1 != tvi.lParam ){
					pcFuncInfo = m_pcFuncInfoArr->GetAt( tvi.lParam );
					nLineTo = pcFuncInfo->m_nFuncLineCRLF;
				}
			}
		}
	}
	return nLineTo;
}


/*! ツリーコントロールの初期化：C++メソッドツリー */
void CDlgFuncList::SetTreeCpp( HWND hwndDlg )
{
	int				i;
	char			szText[2048];
	CFuncInfo*		pcFuncInfo;
	HWND			hwndTree;
	int				bSelected;
	int				nFuncLineOld;
	int				nSelectedLine;
	TV_INSERTSTRUCT	tvis;
	char*			pWork;
	char*			pPos;
	char*			pClassName;
	char*			pFuncName;
	//	Jul. 7, 2001 genta
	char			szLabel[64];
	HTREEITEM		htiGlobal;
	HTREEITEM		htiClass;
	HTREEITEM		htiItem;
	HTREEITEM		htiItemOld;
	HTREEITEM		htiSelected;
	TV_ITEM			tvi;
//	char			szText[2048];

	::EnableWindow( ::GetDlgItem( m_hWnd , IDC_BUTTON_COPY ), TRUE );

	hwndTree = ::GetDlgItem( m_hWnd, IDC_TREE1 );

	tvis.hParent = TVI_ROOT;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
	tvis.item.pszText = "グローバル";
	tvis.item.lParam = -1;
	htiGlobal = TreeView_InsertItem( hwndTree, &tvis );

	nFuncLineOld = 0;
	bSelected = FALSE;
	htiItemOld = NULL;
	for( i = 0; i < m_pcFuncInfoArr->GetNum(); ++i ){
		pcFuncInfo = m_pcFuncInfoArr->GetAt( i );

		pWork = pcFuncInfo->m_cmemFuncName.GetPtr( NULL );

		/* クラス名::メソッドの場合 */
		if( NULL != ( pPos = strstr( pWork, "::" ) ) ){
			//	Apr. 1, 2000 genta
			//	追加文字列を全角にしたのでメモリもそれだけ必要
			//	6 == strlen( "クラス" ), 1 == strlen( '\0' )
			pClassName = new char[pPos - pWork + 6 + 1 ];
			memcpy( pClassName, pWork, pPos - pWork );
			strcpy( &pClassName[pPos - pWork], "クラス" );
			pFuncName = new char[ lstrlen( pPos + lstrlen( "::" ) ) + 1 ];
			strcpy( pFuncName, pPos + lstrlen( "::" ) );

			/* クラス名のアイテムが登録されているか */
			htiClass = TreeView_GetFirstVisible( hwndTree );
			while( NULL != htiClass ){
				tvi.mask = TVIF_HANDLE | TVIF_TEXT;
				tvi.hItem = htiClass;
				tvi.pszText = szLabel;
				tvi.cchTextMax = sizeof(szLabel);
				if( TreeView_GetItem( hwndTree, &tvi ) ){
					if( 0 == strcmp( pClassName, szLabel ) ){
						break;
					}
				}
				htiClass = TreeView_GetNextSibling( hwndTree, htiClass );
			}
			/* クラス名のアイテムが登録されていないので登録 */
			if( NULL == htiClass ){
				tvis.hParent = TVI_ROOT;
				tvis.hInsertAfter = TVI_LAST;
				tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
				tvis.item.pszText = pClassName;
				tvis.item.lParam = -1;
				htiClass = TreeView_InsertItem( hwndTree, &tvis );
			}

			/* 該当クラス名のアイテムの子として、メソッドのアイテムを登録 */
			tvis.hParent = htiClass;
			tvis.hInsertAfter = TVI_LAST;
			tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
			tvis.item.pszText = pFuncName;
			tvis.item.lParam = i;
			htiItem = TreeView_InsertItem( hwndTree, &tvis );

			/* クリップボードにコピーするテキストを編集 */
			wsprintf( szText, "%s(%d): %s\r\n",
				m_pcFuncInfoArr->m_szFilePath,				/* 解析対象ファイル名 */
				pcFuncInfo->m_nFuncLineCRLF,				/* 検出行番号 */
				pcFuncInfo->m_cmemFuncName.GetPtr( NULL ) 	/* 検出結果 */
			);
//			m_cmemClipText.Append( (const char *)szText, lstrlen( szText ) );	/* クリップボードコピー用テキスト */
			m_cmemClipText.AppendSz( (const char *)szText );					/* クリップボードコピー用テキスト */
		}else{
			/* グローバル関数の場合 */
			pClassName = NULL;
			pFuncName = new char[ lstrlen( pWork ) + 1 ];
			strcpy( pFuncName, pWork );

			strcpy( szText, pFuncName );
			/* 関数宣言か */
			if( 1 == pcFuncInfo->m_nInfo ){
				strcat( szText, "(宣言)" );
			}

			/* グローバル関数のアイテムを登録 */
			tvis.hParent = htiGlobal;
			tvis.hInsertAfter = TVI_LAST;
			tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
			tvis.item.pszText = szText;
			tvis.item.lParam = i;
			htiItem = TreeView_InsertItem( hwndTree, &tvis );

			/* 関数宣言か */
			if( 1 == pcFuncInfo->m_nInfo ){
				/* クリップボードにコピーするテキストを編集 */
				wsprintf( szText, "%s(%d): %s(宣言)\r\n",
					m_pcFuncInfoArr->m_szFilePath,				/* 解析対象ファイル名 */
					pcFuncInfo->m_nFuncLineCRLF,				/* 検出行番号 */
					pcFuncInfo->m_cmemFuncName.GetPtr( NULL ) 	/* 検出結果 */
				);
//				m_cmemClipText.Append( (const char *)szText, lstrlen( szText ) );	/* クリップボードコピー用テキスト */
				m_cmemClipText.AppendSz( (const char *)szText );					/* クリップボードコピー用テキスト */
			}else{
				/* クリップボードにコピーするテキストを編集 */
				wsprintf( szText, "%s(%d): %s\r\n",
					m_pcFuncInfoArr->m_szFilePath,				/* 解析対象ファイル名 */
					pcFuncInfo->m_nFuncLineCRLF,				/* 検出行番号 */
					pcFuncInfo->m_cmemFuncName.GetPtr( NULL ) 	/* 検出結果 */
				);
//				m_cmemClipText.Append( (const char *)szText, lstrlen( szText ) );	/* クリップボードコピー用テキスト */
				m_cmemClipText.AppendSz( (const char *)szText );					/* クリップボードコピー用テキスト */
			}
		}
		/* 現在カーソル位置のメソッドかどうか調べる */
		if( !bSelected ){
			if( i == 0 && m_nCurLine < pcFuncInfo->m_nFuncLineLAYOUT ){
				bSelected = TRUE;
				nSelectedLine = i;
				htiSelected = htiItem;
			}else
			if( i > 0 && nFuncLineOld <= m_nCurLine && m_nCurLine < pcFuncInfo->m_nFuncLineLAYOUT ){
				bSelected = TRUE;
				nSelectedLine = i - 1;
				htiSelected = htiItemOld;
			}
		}
		nFuncLineOld = pcFuncInfo->m_nFuncLineLAYOUT;

		if( NULL != pClassName ){
			delete [] pClassName;
			pClassName = NULL;
		}
		if( NULL != pFuncName ){
			delete [] pFuncName;
			pFuncName = NULL;
		}
		htiItemOld = htiItem;
	}
	/* ソート、ノードの展開をする */
//	TreeView_SortChildren( hwndTree, TVI_ROOT, 0 );
	htiClass = TreeView_GetFirstVisible( hwndTree );
	while( NULL != htiClass ){
//		TreeView_SortChildren( hwndTree, htiClass, 0 );
		TreeView_Expand( hwndTree, htiClass, TVE_EXPAND );
		htiClass = TreeView_GetNextSibling( hwndTree, htiClass );
	}
	/* 現在カーソル位置のメソッドを選択状態にする */
	if( bSelected ){
		TreeView_SelectItem( hwndTree, htiSelected );
	}else{
		if( NULL != htiItemOld ){
			TreeView_SelectItem( hwndTree, htiItemOld );
		}
	}
//	GetTreeTextNext( hwndTree, NULL, 0 );
	return;
}



/* ツリーコントロールの初期化：Javaメソッドツリー */
void CDlgFuncList::SetTreeJava( HWND hwndDlg, BOOL bAddClass )
{
	int				i;
	char			szText[2048];
	CFuncInfo*		pcFuncInfo;
	HWND			hwndTree;
	int				bSelected;
	int				nFuncLineOld;
	int				nSelectedLine;
	TV_INSERTSTRUCT	tvis;
	char*			pWork;
	char*			pPos;
	char*			pClassName;
	char*			pFuncName;
	char			szLabel[32];
//	HTREEITEM		htiGlobal;
	HTREEITEM		htiClass;
	HTREEITEM		htiItem;
	HTREEITEM		htiItemOld;
	HTREEITEM		htiSelected;
	TV_ITEM			tvi;
	int				nClassNest;
	char			szClassArr[16][48];
//	char			szText[2048];

	::EnableWindow( ::GetDlgItem( m_hWnd , IDC_BUTTON_COPY ), TRUE );

	hwndTree = ::GetDlgItem( m_hWnd, IDC_TREE1 );

//	tvis.hParent = TVI_ROOT;
//	tvis.hInsertAfter = TVI_LAST;
//	tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
//	tvis.item.pszText = "グローバル";
//	tvis.item.lParam = -1;
//	htiGlobal = TreeView_InsertItem( hwndTree, &tvis );
	nFuncLineOld = 0;
	bSelected = FALSE;
	htiItemOld = NULL;
	for( i = 0; i < m_pcFuncInfoArr->GetNum(); ++i ){
		pcFuncInfo = m_pcFuncInfoArr->GetAt( i );
		pWork = pcFuncInfo->m_cmemFuncName.GetPtr( NULL );
		/* クラス名::メソッドの場合 */
		if( NULL != ( pPos = strstr( pWork, "::" ) ) ){
			/* インナークラスのネストレベルを調べる */
			int	k, m;
			int	nWorkLen;
			int	nCharChars;
			nClassNest = 0;
			m = 0;
			nWorkLen = lstrlen( pWork );
			for( k = 0; k < nWorkLen; ++k ){
				nCharChars = CMemory::MemCharNext( pWork, nWorkLen, &pWork[k] ) - &pWork[k];
				if( 1 == nCharChars && ':' == pWork[k] ){
					break;
				}
				if( 1 == nCharChars && '\\' == pWork[k] ){
					memcpy( szClassArr[nClassNest], &pWork[m], k - m );
					szClassArr[nClassNest][k - m] = '\0';
					++nClassNest;
					m = k + 1;
				}
				if( 2 == nCharChars ){
					++k;
				}
			}
			if( 0 < k - m ){
				memcpy( szClassArr[nClassNest], &pWork[m], k - m );
				szClassArr[nClassNest][k - m] = '\0';
				++nClassNest;
			}
//			for( k = 0; k < nClassNest; ++k ){
//				MYTRACE( "%d [%s]\n", k, szClassArr[k] );
//			}
//			MYTRACE( "\n" );
			pFuncName = new char[ lstrlen( pPos + lstrlen( "::" ) ) + 1 ];
			strcpy( pFuncName, pPos + lstrlen( "::" ) );

			/* クラス名のアイテムが登録されているか */
			htiClass = TreeView_GetFirstVisible( hwndTree );
			HTREEITEM htiParent = TVI_ROOT;
			for( k = 0; k < nClassNest; ++k ){
				//	Apr. 1, 2001 genta
				//	追加文字列を全角にしたのでメモリもそれだけ必要
				//	6 == strlen( "クラス" ), 1 == strlen( '\0' )
				pClassName = new char[ lstrlen( szClassArr[k] ) + 1 + 6 ];
				strcpy( pClassName, szClassArr[k] );
				if( bAddClass ){
					strcat( pClassName, "クラス" );
				}
				while( NULL != htiClass ){
					tvi.mask = TVIF_HANDLE | TVIF_TEXT;
					tvi.hItem = htiClass;
					tvi.pszText = szLabel;
					tvi.cchTextMax = sizeof(szLabel);
					if( TreeView_GetItem( hwndTree, &tvi ) ){
						if( 0 == strcmp( pClassName, szLabel ) ){
							break;
						}
					}
					htiClass = TreeView_GetNextSibling( hwndTree, htiClass );
				}
				/* クラス名のアイテムが登録されていないので登録 */
				if( NULL == htiClass ){
					tvis.hParent = htiParent;
					tvis.hInsertAfter = TVI_LAST;
					tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
					tvis.item.pszText = pClassName;
					tvis.item.lParam = -1;
					htiClass = TreeView_InsertItem( hwndTree, &tvis );
				}else{

				}
				htiParent = htiClass;
				if( k + 1 >= nClassNest ){
					break;
				}
				htiClass = TreeView_GetChild( hwndTree, htiClass );
			}
			/* 該当クラス名のアイテムの子として、メソッドのアイテムを登録 */
			tvis.hParent = htiClass;
			tvis.hInsertAfter = TVI_LAST;
			tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
			tvis.item.pszText = pFuncName;
			tvis.item.lParam = i;
			htiItem = TreeView_InsertItem( hwndTree, &tvis );

			/* クリップボードにコピーするテキストを編集 */
			wsprintf( szText, "%s(%d): %s\r\n",
				m_pcFuncInfoArr->m_szFilePath,				/* 解析対象ファイル名 */
				pcFuncInfo->m_nFuncLineCRLF,				/* 検出行番号 */
				pcFuncInfo->m_cmemFuncName.GetPtr( NULL ) 	/* 検出結果 */
			);
//			m_cmemClipText.Append( (const char *)szText, lstrlen( szText ) );	/* クリップボードコピー用テキスト */
			m_cmemClipText.AppendSz( (const char *)szText );					/* クリップボードコピー用テキスト */
//		}else{
//			/* グローバル関数の場合 */
//			pClassName = NULL;
//			pFuncName = new char[ lstrlen( pWork ) + 1 ];
//			strcpy( pFuncName, pWork );
//
//			strcpy( szText, pFuncName );
//			/* 関数宣言か */
//			if( 1 == pcFuncInfo->m_nInfo ){
//				strcat( szText, "(宣言)" );
//			}
//
//			/* グローバル関数のアイテムを登録 */
//			tvis.hParent = htiGlobal;
//			tvis.hInsertAfter = TVI_LAST;
//			tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
//			tvis.item.pszText = szText;
//			tvis.item.lParam = i;
//			htiItem = TreeView_InsertItem( hwndTree, &tvis );
//
//			/* 関数宣言か */
//			if( 1 == pcFuncInfo->m_nInfo ){
//				/* クリップボードにコピーするテキストを編集 */
//				sprintf( szText, "%s(%d): %s(宣言)\r\n",
//					m_pcFuncInfoArr->m_szFilePath,				/* 解析対象ファイル名 */
//					pcFuncInfo->m_nFuncLineCRLF,				/* 検出行番号 */
//					pcFuncInfo->m_cmemFuncName.GetPtr( NULL ) 	/* 検出結果 */
//				);
//				m_cmemClipText.Append( (const char *)szText, lstrlen( szText ) );	/* クリップボードコピー用テキスト */
//			}else{
//				/* クリップボードにコピーするテキストを編集 */
//				sprintf( szText, "%s(%d): %s\r\n",
//					m_pcFuncInfoArr->m_szFilePath,				/* 解析対象ファイル名 */
//					pcFuncInfo->m_nFuncLineCRLF,				/* 検出行番号 */
//					pcFuncInfo->m_cmemFuncName.GetPtr( NULL ) 	/* 検出結果 */
//				);
//				m_cmemClipText.Append( (const char *)szText, lstrlen( szText ) );	/* クリップボードコピー用テキスト */
//			}
		}
		/* 現在カーソル位置のメソッドかどうか調べる */
		if( !bSelected ){
			if( i == 0 && m_nCurLine < pcFuncInfo->m_nFuncLineLAYOUT ){
				bSelected = TRUE;
				nSelectedLine = i;
				htiSelected = htiItem;
			}else
			if( i > 0 && nFuncLineOld <= m_nCurLine && m_nCurLine < pcFuncInfo->m_nFuncLineLAYOUT ){
				bSelected = TRUE;
				nSelectedLine = i - 1;
				htiSelected = htiItemOld;
			}
		}
		nFuncLineOld = pcFuncInfo->m_nFuncLineLAYOUT;
		if( NULL != pClassName ){
			delete [] pClassName;
			pClassName = NULL;
		}
		if( NULL != pFuncName ){
			delete [] pFuncName;
			pFuncName = NULL;
		}
		htiItemOld = htiItem;
	}
	/* ソート、ノードの展開をする */
//	TreeView_SortChildren( hwndTree, TVI_ROOT, 0 );
	htiClass = TreeView_GetFirstVisible( hwndTree );
	while( NULL != htiClass ){
//		TreeView_SortChildren( hwndTree, htiClass, 0 );
		TreeView_Expand( hwndTree, htiClass, TVE_EXPAND );
		htiClass = TreeView_GetNextSibling( hwndTree, htiClass );
	}
	/* 現在カーソル位置のメソッドを選択状態にする */
	if( bSelected ){
		TreeView_SelectItem( hwndTree, htiSelected );
	}else{
		if( NULL != htiItemOld ){
			TreeView_SelectItem( hwndTree, htiItemOld );
		}
	}
//	GetTreeTextNext( hwndTree, NULL, 0 );
	return;
}


void CDlgFuncList::GetTreeTextNext(
		HWND		hwndTree,
		HTREEITEM	htiParent,
		int			nDepth
)
{
	HTREEITEM		htiItem;
	TV_ITEM			tvi;
	int				i;
	char			szWork[1024];

	if( NULL == htiParent ){
		htiItem = TreeView_GetRoot( hwndTree );
	}else{
		htiItem = TreeView_GetChild( hwndTree, htiParent );
	}
	while( NULL != htiItem ){
		tvi.mask = TVIF_HANDLE | TVIF_TEXT;
		tvi.hItem = htiItem;
		tvi.pszText = (LPSTR)szWork;
		tvi.cchTextMax = sizeof( szWork );
		TreeView_GetItem( hwndTree, &tvi );
		for( i = 0; i < nDepth; ++i ){
//			m_cmemClipText.Append( "  ", 2 );	/* クリップボードコピー用テキスト */
			m_cmemClipText.AppendSz( "  " );	/* クリップボードコピー用テキスト */
		}
//		m_cmemClipText.Append( (const char *)tvi.pszText, lstrlen( (const char *)tvi.pszText ) );	/* クリップボードコピー用テキスト */
		m_cmemClipText.AppendSz( (const char *)tvi.pszText );	/* クリップボードコピー用テキスト */
//		m_cmemClipText.Append( (const char *)"\r\n", 2 );		/* クリップボードコピー用テキスト */
		m_cmemClipText.AppendSz( (const char *)"\r\n" );		/* クリップボードコピー用テキスト */
		GetTreeTextNext( hwndTree, htiItem, nDepth + 1 );

		htiItem = TreeView_GetNextSibling( hwndTree, htiItem );
	}
	return;
}


/* ツリーコントロールの初期化：テキスト トピックツリー */
void CDlgFuncList::SetTreeTxt( HWND hwndDlg )
{
	HWND			hwndTree;
	int				nBgn;
	HTREEITEM		htiItem;
	HTREEITEM		htiItemSelected;
	TV_INSERTSTRUCT	tvis;
	hwndTree = ::GetDlgItem( m_hWnd, IDC_TREE1 );
	tvis.hParent = TVI_ROOT;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
	tvis.item.pszText = "トピック";
	tvis.item.lParam = -1;
	htiItem = TreeView_InsertItem( hwndTree, &tvis );
	::EnableWindow( ::GetDlgItem( m_hWnd , IDC_BUTTON_COPY ), TRUE );
	nBgn = 0;
	htiItemSelected = NULL;
	SetTreeTxtNest( hwndTree, htiItem, 0, m_pcFuncInfoArr->GetNum(), &htiItemSelected, 0 );
//	TreeView_Expand( hwndTree, htiItem, TVE_EXPAND );
	if( NULL != htiItemSelected ){
		/* 現在カーソル位置のメソッドを選択状態にする */
		TreeView_SelectItem( hwndTree, htiItemSelected );
	}
	return;
}





/* ツリーコントロールの初期化：テキスト トピックツリー  再帰サブ関数 */
int CDlgFuncList::SetTreeTxtNest(
	HWND			hwndTree,
	HTREEITEM		htiParent,
	int				nBgn,
	int				nEnd,
	HTREEITEM*		phtiItemSelected,
	int				nDepth
)
{
	CFuncInfo*		pcFuncInfoNext;
	CFuncInfo*		pcFuncInfo;
	CFuncInfo*		pcFuncInfo2;
	unsigned char*	pWork;
	unsigned char*	pWork2;
	int				nCharChars;
	int				nCharChars2;
	int				i;
	TV_INSERTSTRUCT	tvis;
	HTREEITEM		htiItem;
	while( nBgn < nEnd ){
		/* トピック情報取得 */
		pcFuncInfo = m_pcFuncInfoArr->GetAt( nBgn );
		pWork = (unsigned char*)pcFuncInfo->m_cmemFuncName.GetPtr( NULL );
		nCharChars = CMemory::MemCharNext( (char*)pWork, lstrlen( (char*)pWork ), (char*)pWork ) - (char*)pWork;

		tvis.hParent = htiParent;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
		tvis.item.pszText = (char*)pWork;
		tvis.item.lParam = nBgn;
		htiItem = TreeView_InsertItem( hwndTree, &tvis );

		for( i = 0; i < nDepth; ++i ){
//			m_cmemClipText.Append( "  ", 2 );	/* クリップボードコピー用テキスト */
			m_cmemClipText.AppendSz( "  " );	/* クリップボードコピー用テキスト */
		}
//		m_cmemClipText.Append( (const char *)pWork, lstrlen( (const char *)pWork ) );	/* クリップボードコピー用テキスト */
		m_cmemClipText.AppendSz( (const char *)pWork );		/* クリップボードコピー用テキスト */
//		m_cmemClipText.Append( (const char *)"\r\n", 2 );	/* クリップボードコピー用テキスト */
		m_cmemClipText.AppendSz( (const char *)"\r\n" );	/* クリップボードコピー用テキスト */

		/* 現在カーソル位置のメソッドかどうか調べる */
		if( nBgn == 0 && m_nCurLine < pcFuncInfo->m_nFuncLineLAYOUT ){
			/* 現在カーソル位置のメソッドを選択状態にする */
//			TreeView_SelectItem( hwndTree, htiItem );
			*phtiItemSelected = htiItem;
		}else{
			if( nBgn + 1 < m_pcFuncInfoArr->GetNum() ){
				pcFuncInfoNext = m_pcFuncInfoArr->GetAt( nBgn + 1 );
				if( m_nCurLine >= pcFuncInfo->m_nFuncLineLAYOUT &&
					m_nCurLine < pcFuncInfoNext->m_nFuncLineLAYOUT
				){
					/* 現在カーソル位置のメソッドを選択状態にする */
//					TreeView_SelectItem( hwndTree, htiItem );
					*phtiItemSelected = htiItem;
				}
			}else{
				if( m_nCurLine >= pcFuncInfo->m_nFuncLineLAYOUT ){
					/* 現在カーソル位置のメソッドを選択状態にする */
//					TreeView_SelectItem( hwndTree, htiItem );
					*phtiItemSelected = htiItem;
				}
			}
		}
		for( i = nBgn + 1; i < nEnd; ++i ){
			pcFuncInfo2 = m_pcFuncInfoArr->GetAt( i );
			pWork2 = (unsigned char*)pcFuncInfo2->m_cmemFuncName.GetPtr( NULL );
			nCharChars2 = CMemory::MemCharNext( (char*)pWork2, lstrlen( (char*)pWork2 ), (char*)pWork2 ) - (char*)pWork2;
			if( nCharChars == nCharChars2 ){
				if( nCharChars == 1 ){
					/* 半角数字 */
					if( pWork[0] >= '0' && pWork[0] <= '9' &&
						pWork2[0] >= '0' && pWork2[0] <= '9' ){
						break;
					}
				}else
				if( nCharChars == 2 ){
					/* 全角数字 */
					if( pWork[0] == 0x82 && ( pWork[1] >= 0x4f && pWork[1] <= 0x58 ) &&
						pWork2[0] == 0x82 && ( pWork2[1] >= 0x4f && pWork2[1] <= 0x58 ) ){
						break;
					}
					/* ①～⑳ */
					if( pWork[0] == 0x87 && ( pWork[1] >= 0x40 && pWork[1] <= 0x53 ) &&
						pWork2[0] == 0x87 && ( pWork2[1] >= 0x40 && pWork2[1] <= 0x53 ) ){
						break;
					}
					/* Ⅰ～Ⅹ */
					if( pWork[0] == 0x87 && ( pWork[1] >= 0x54 && pWork[1] <= 0x5d ) &&
						pWork2[0] == 0x87 && ( pWork2[1] >= 0x54 && pWork2[1] <= 0x5d ) ){
						break;
					}
				}
				if( 0 == memcmp( pWork, pWork2, nCharChars ) ){
					break;
				}
			}
		}
		SetTreeTxtNest( hwndTree, htiItem, nBgn + 1, i, phtiItemSelected, nDepth + 1 );
//		TreeView_Expand( hwndTree, htiItem, TVE_EXPAND );
		nBgn = i;
	}
	return 1;
}



BOOL CDlgFuncList::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	m_hWnd = hwndDlg;
	HWND		hwndList;
	int			nCxVScroll;
	int			nColWidthArr[3] = { 0, 46, 80 };
	RECT		rc;
	LV_COLUMN	col;
	hwndList = ::GetDlgItem( hwndDlg, IDC_LIST1 );
//	hwndTree = ::GetDlgItem( m_hWnd, IDC_TREE1 );
	::GetWindowRect( hwndList, &rc );
	nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = rc.right - rc.left - ( nColWidthArr[1] + nColWidthArr[2] ) - nCxVScroll - 8;
	col.pszText = "関数名";
	col.iSubItem = 0;
	ListView_InsertColumn( hwndList, 0, &col);

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = nColWidthArr[1];
	col.pszText = "行 *";
	col.iSubItem = 1;
	ListView_InsertColumn( hwndList, 1, &col);

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = nColWidthArr[2];
	col.pszText = " ";
	col.iSubItem = 2;
	ListView_InsertColumn( hwndList, 2, &col);

	/* 基底クラスメンバ */
	CreateSizeBox();
	return CDialog::OnInitDialog( hwndDlg, wParam, lParam );
}


BOOL CDlgFuncList::OnBnClicked( int wID )
{
	HGLOBAL			hgClip;
	char*			pszClip;
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* 「アウトライン解析」のヘルプ */
		//Apr. 5, 2001 JEPRO 修正漏れを追加 (Stonee, 2001/03/12 第四引数を、機能番号からヘルプトピック番号を調べるようにした)
//		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, 64 );
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_OUTLINE) );
		return TRUE;
	case IDOK:
		return OnJump();
	case IDCANCEL:
		if( m_bModal ){		/* モーダル ダイアログか */
			::EndDialog( m_hWnd, 0 );
		}else{
			::DestroyWindow( m_hWnd );
		}
		return TRUE;
	case IDC_BUTTON_COPY:
		/* Windowsクリップボードにコピー */
		hgClip = ::GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, m_cmemClipText.GetLength() + 1 );
		pszClip = (char*)::GlobalLock( hgClip );
		memcpy( pszClip, m_cmemClipText.GetPtr( NULL ), m_cmemClipText.GetLength() + 1 );
		::GlobalUnlock( hgClip );
		::OpenClipboard( m_hWnd );
		::EmptyClipboard();
		::SetClipboardData( CF_OEMTEXT, hgClip );
		::CloseClipboard();
		return TRUE;
	}
	/* 基底クラスメンバ */
	return CDialog::OnBnClicked( wID );
}

BOOL CDlgFuncList::OnNotify( WPARAM wParam, LPARAM lParam )
{
	int				idCtrl;
	LPNMHDR			pnmh;
	NM_LISTVIEW*	pnlv;
	HWND			hwndList;
	HWND			hwndTree;
	NM_TREEVIEW*	pnmtv;
//	int				nLineTo;
	LV_COLUMN		col;

	idCtrl = (int) wParam;
	pnmh = (LPNMHDR) lParam;
	pnlv = (NM_LISTVIEW*)lParam;

	hwndList = ::GetDlgItem( m_hWnd, IDC_LIST1 );
	hwndTree = ::GetDlgItem( m_hWnd, IDC_TREE1 );

	if( hwndTree == pnmh->hwndFrom ){
		pnmtv = (NM_TREEVIEW *) lParam;
//		switch( pnmh->code ){
//		case TVN_BEGINDRAG:
//		case TVN_BEGINLABELEDIT:
//		case TVN_BEGINRDRAG:
//		case TVN_DELETEITEM:
//		case TVN_ENDLABELEDIT:
//		case TVN_GETDISPINFO:
//		case TVN_ITEMEXPANDED:
//		case TVN_ITEMEXPANDING:
//		case TVN_KEYDOWN:
//		case TVN_SELCHANGED:
//		case TVN_SELCHANGING:
//		case TVN_SETDISPINFO:
//			break;
//		default:
			switch( pnmtv->hdr.code ){
			case NM_DBLCLK:
				return OnJump();
//			case NM_CLICK:
//			case NM_KILLFOCUS:
//			case NM_OUTOFMEMORY:
//			case NM_RCLICK:
//			case NM_RDBLCLK:
//			case NM_RETURN:
//			case NM_SETFOCUS:
//			default:
//				break;
			}
//			break;
//		}
	}else
	if( hwndList == pnmh->hwndFrom ){
		switch( pnmh->code ){
		case LVN_COLUMNCLICK:
//				MYTRACE( "LVN_COLUMNCLICK\n" );
			m_nSortCol =  pnlv->iSubItem;
			if( m_nSortCol == 0 ){
				col.mask = LVCF_TEXT;
			// From Here 2001.12.03 hor
			//	col.pszText = "関数名 *";
				if(OUTLINE_BOOKMARK == m_nListType){
					col.pszText = "テキスト *";
				}else{
					col.pszText = "関数名 *";
				}
			// To Here 2001.12.03 hor
				col.iSubItem = 0;
				ListView_SetColumn( hwndList, 0, &col );
				col.mask = LVCF_TEXT;
				col.pszText = "行";
				col.iSubItem = 0;
				ListView_SetColumn( hwndList, 1, &col );
			// From Here 2001.12.07 hor
				col.mask = LVCF_TEXT;
				col.pszText = "";
				col.iSubItem = 0;
				ListView_SetColumn( hwndList, 2, &col );
			// To Here 2001.12.07 hor
				ListView_SortItems( hwndList, _CompareFunc_, (LPARAM)this );
			}else
			if( m_nSortCol == 1 ){
				col.mask = LVCF_TEXT;
			// From Here 2001.12.03 hor
			//	col.pszText = "関数名";
				if(OUTLINE_BOOKMARK == m_nListType){
					col.pszText = "テキスト";
				}else{
					col.pszText = "関数名";
				}
			// To Here 2001.12.03 hor
				col.iSubItem = 0;
				ListView_SetColumn( hwndList, 0, &col );
				col.mask = LVCF_TEXT;
				col.pszText = "行 *";
				col.iSubItem = 0;
				ListView_SetColumn( hwndList, 1, &col );
			// From Here 2001.12.07 hor
				col.mask = LVCF_TEXT;
				col.pszText = "";
				col.iSubItem = 0;
				ListView_SetColumn( hwndList, 2, &col );
			// To Here 2001.12.03 hor
				ListView_SortItems( hwndList, _CompareFunc_, (LPARAM)this );
			// From Here 2001.12.07 hor
			}else
			if( m_nSortCol == 2 ){
				col.mask = LVCF_TEXT;
				if(OUTLINE_BOOKMARK == m_nListType){
					col.pszText = "テキスト";
				}else{
					col.pszText = "関数名";
				}
				col.iSubItem = 0;
				ListView_SetColumn( hwndList, 0, &col );
				col.mask = LVCF_TEXT;
				col.pszText = "行";
				col.iSubItem = 0;
				ListView_SetColumn( hwndList, 1, &col );
				col.mask = LVCF_TEXT;
				col.pszText = "*";
				col.iSubItem = 0;
				ListView_SetColumn( hwndList, 2, &col );
				ListView_SortItems( hwndList, _CompareFunc_, (LPARAM)this );
			// To Here 2001.12.07 hor
			}
			return TRUE;
		case NM_DBLCLK:
			return OnJump();
		}
	}
	return FALSE;
}



BOOL CDlgFuncList::OnSize( WPARAM wParam, LPARAM lParam )
{
	/* 基底クラスメンバ */
	CDialog::OnSize( wParam, lParam );

	int	Controls[] = {
		IDC_CHECK_bAutoCloseDlgFuncList,
		IDC_BUTTON_COPY,
		IDOK,
		IDCANCEL,
		IDC_BUTTON_HELP,
		IDC_LIST1,
		IDC_TREE1
	};
	int		nControls = sizeof( Controls ) / sizeof( Controls[0] );
	int		fwSizeType;
	int		nWidth;
	int		nHeight;
	int		i;
	int		nWork;
	RECT	rc;
	HWND	hwndCtrl;
	POINT	po;

	fwSizeType = wParam;		// resizing flag
	nWidth = LOWORD(lParam);	// width of client area
	nHeight = HIWORD(lParam);	// height of client area

	nWork = 48;
	for ( i = 0; i < nControls; ++i ){
		hwndCtrl = ::GetDlgItem( m_hWnd, Controls[i] );
		::GetWindowRect( hwndCtrl, &rc );
		po.x = rc.left;
		po.y = rc.top;
		::ScreenToClient( m_hWnd, &po );
		rc.left = po.x;
		rc.top  = po.y;
		po.x = rc.right;
		po.y = rc.bottom;
		::ScreenToClient( m_hWnd, &po );
		rc.right = po.x;
		rc.bottom  = po.y;
		if( Controls[i] == IDC_CHECK_bAutoCloseDlgFuncList ){
			::SetWindowPos( hwndCtrl, NULL, rc.left, nHeight - nWork + 32 /*- nWork*//*rc.top + nExtraSize*/, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER );
		}else
		if( Controls[i] != IDC_LIST1
		 && Controls[i] != IDC_TREE1
		){
			::SetWindowPos( hwndCtrl, NULL, rc.left, nHeight - nWork + 6/*rc.top + nExtraSize*/, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER );
//			::InvalidateRect( hwndCtrl, NULL, TRUE );
		}else{
			::SetWindowPos( hwndCtrl, NULL, 0, 0, nWidth - 2 * rc.left, nHeight - nWork + 5/*rc.bottom - rc.top + nExtraSize*/, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );
		}
		::InvalidateRect( hwndCtrl, NULL, TRUE );
	}
	return TRUE;
}




BOOL CDlgFuncList::OnJump( void )
{
	int				nLineTo;
	/* ダイアログデータの取得 */
	if( 0 < ( nLineTo = GetData() ) ){
		if( m_bModal ){		/* モーダル ダイアログか */
			::EndDialog( m_hWnd, nLineTo );
		}else{
			/* カーソルを移動させる */
			POINT	poCaret;
			poCaret.x = 0;
			poCaret.y = nLineTo - 1;
			memcpy( m_pShareData->m_szWork, (void*)&poCaret, sizeof(poCaret) );
			::SendMessage( ::GetParent( ::GetParent( m_hwndParent ) ), MYWM_SETCARETPOS, 0, 0 );
			/* アウトライン ダイアログを自動的に閉じる */
			if( m_pShareData->m_Common.m_bAutoCloseDlgFuncList ){
				::DestroyWindow( m_hWnd );
			}
		}
	}
	return TRUE;
}


// From Here 2001.12.03 hor
/* ブックマークリストの作成	*/
void CDlgFuncList::SetTreeBookMark( HWND hwndDlg )
{
	int				i;
	char			szText[2048];
	CFuncInfo*		pcFuncInfo;
	LV_ITEM			item;
	LV_COLUMN		col;
	HWND			hwndList;
	HWND			hwndTree;
	int				bSelected;
	int				nFuncLineOld;
	int				nSelectedLine;
	RECT			rc;
	hwndList = ::GetDlgItem( m_hWnd, IDC_LIST1 );
	::EnableWindow( ::GetDlgItem( m_hWnd , IDC_BUTTON_COPY ), TRUE );
	nFuncLineOld = 0;
	bSelected = FALSE;
	for( i = 0; i < m_pcFuncInfoArr->GetNum(); ++i ){
		pcFuncInfo = m_pcFuncInfoArr->GetAt( i );
		if( !bSelected ){
			if( i == 0 && m_nCurLine < pcFuncInfo->m_nFuncLineLAYOUT ){
				bSelected = TRUE;
				nSelectedLine = i;
			}else
			if( i > 0 && nFuncLineOld <= m_nCurLine && m_nCurLine < pcFuncInfo->m_nFuncLineLAYOUT ){
				bSelected = TRUE;
				nSelectedLine = i - 1;
			}
		}
		nFuncLineOld = pcFuncInfo->m_nFuncLineLAYOUT;
	}
	if( 0 < m_pcFuncInfoArr->GetNum() && !bSelected ){
		bSelected = TRUE;
		nSelectedLine =  m_pcFuncInfoArr->GetNum() - 1;
	}
	for( i = 0; i < m_pcFuncInfoArr->GetNum(); ++i ){
		/* 現在の解析結果要素 */
		pcFuncInfo = m_pcFuncInfoArr->GetAt( i );

		item.mask = LVIF_TEXT | LVIF_PARAM;
		item.pszText = pcFuncInfo->m_cmemFuncName.GetPtr( NULL );
		item.iItem = i;
		item.iSubItem = 0;
		item.lParam	= i;
		ListView_InsertItem( hwndList, &item);

		/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
		if(m_bLineNumIsCRLF ){
			wsprintf( szText, "%d", pcFuncInfo->m_nFuncLineCRLF );
		}else{
			wsprintf( szText, "%d", pcFuncInfo->m_nFuncLineLAYOUT );
		}
		item.mask = LVIF_TEXT;
		item.pszText = szText;
		item.iItem = i;
		item.iSubItem = 1;
		ListView_SetItem( hwndList, &item);

		/* クリップボードにコピーするテキストを編集 */
		wsprintf( szText, "%s(%d): %s\r\n",
			m_pcFuncInfoArr->m_szFilePath,				/* 解析対象ファイル名 */
			pcFuncInfo->m_nFuncLineCRLF,				/* 検出行番号 */
			pcFuncInfo->m_cmemFuncName.GetPtr( NULL )	/* 検出結果 */
		);
		m_cmemClipText.AppendSz( (const char *)szText );					/* クリップボードコピー用テキスト */

	}

	if( bSelected ){
		ListView_GetItemRect( hwndList, 0, &rc, LVIR_BOUNDS );
		ListView_Scroll( hwndList, 0, nSelectedLine * ( rc.bottom - rc.top ) );
		ListView_SetItemState( hwndList, nSelectedLine, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	}
	/* 列名変更 */
	col.mask = LVCF_TEXT;
	col.pszText = "テキスト";
	col.iSubItem = 0;
	ListView_SetColumn( hwndList, 0, &col );

	/* 列の幅をデータに合わせて調整 */
	ListView_SetColumnWidth( hwndList, 0, LVSCW_AUTOSIZE );
	ListView_SetColumnWidth( hwndList, 1, LVSCW_AUTOSIZE );
	ListView_SetColumnWidth( hwndList, 2, 0 );

	/* アウトライン ダイアログを自動的に閉じる */
	::CheckDlgButton( m_hWnd, IDC_CHECK_bAutoCloseDlgFuncList, m_pShareData->m_Common.m_bAutoCloseDlgFuncList );
	return;
}
// To Here 2001.12.03 hor


/*[EOF]*/
