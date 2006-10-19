/*!	@file
	@brief アウトライン解析ダイアログボックス

	@author Norio Nakatani

	@date 2001/06/23 N.Nakatani Visual Basicのアウトライン解析
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, Stonee, JEPRO, genta, hor
	Copyright (C) 2002, MIK, aroka, hor, genta, YAZAKI, Moca, frozen
	Copyright (C) 2003, zenryaku, Moca, naoh, little YOSHI, genta,
	Copyright (C) 2004, zenryaku, Moca, novice
	Copyright (C) 2005, genta, zenryaku, ぜっと, D.S.Koba
	Copyright (C) 2006, genta, aroka, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include <windows.h>
#include <commctrl.h>
#include "sakura_rc.h"
#include "CDlgFuncList.h"
#include "etc_uty.h"
#include "debug.h"
#include "global.h"
#include "CEditView.h"
#include "funccode.h"		//Stonee, 2001/03/12
#include "CFuncInfoArr.h"// 2002/2/3 aroka
#include "mymessage.h"// 2002/2/3 aroka
#include "Keycode.h"// 2002/2/10 aroka ヘッダ整理
#include "CEditDoc.h"	//	2002/5/13 YAZAKI ヘッダ整理
#include "CEditWnd.h"	//	2006/2/11 aroka 追加

//アウトライン解析 CDlgFuncList.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
const DWORD p_helpids[] = {	//12200
	IDC_BUTTON_COPY,				HIDC_FL_BUTTON_COPY,	//コピー
	IDOK,							HIDOK_FL,				//ジャンプ
	IDCANCEL,						HIDCANCEL_FL,			//キャンセル
	IDC_BUTTON_HELP,				HIDC_FL_BUTTON_HELP,	//ヘルプ
	IDC_CHECK_bAutoCloseDlgFuncList,	HIDC_FL_CHECK_bAutoCloseDlgFuncList,	//自動的に閉じる
	IDC_LIST1,						HIDC_FL_LIST1,			//トピックリスト
	IDC_TREE1,						HIDC_FL_TREE1,			//トピックツリー
	IDC_CHECK_bFunclistSetFocusOnJump	,HIDC_FL_CHECK_bFunclistSetFocusOnJump,	//ジャンプでフォーカス移動する
	IDC_CHECK_bMarkUpBlankLineEnable	,HIDC_FL_CHECK_bMarkUpBlankLineEnable,	//空行を無視する
	IDC_COMBO_nSortType,			HIDC_COMBO_nSortType,	//順序
	IDC_BUTTON_WINSIZE,				HIDC_FL_BUTTON_WINSIZE,	//ウィンドウ位置保存	// 2006.08.06 ryoji
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

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
	//	Apr. 23, 2005 genta 行番号を左端へ
	if( 1 == pcDlgFuncList->m_nSortCol){	/* ソートする列番号 */
		return strcmp( pcFuncInfo1->m_cmemFuncName.GetPtr(), pcFuncInfo2->m_cmemFuncName.GetPtr() );
	}
	//	Apr. 23, 2005 genta 行番号を左端へ
	if( 0 == pcDlgFuncList->m_nSortCol){	/* ソートする列番号 */
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
	//	Apr. 23, 2005 genta 行番号を左端へ
	m_nSortCol = 0;				/* ソートする列番号 2004.04.06 zenryaku 標準は行番号(1列目) */
	m_bLineNumIsCRLF = FALSE;	/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
	m_bWaitTreeProcess = false;	// 2002.02.16 hor Treeのダブルクリックでフォーカス移動できるように 2/4
	m_nSortType = 0;
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

	//2002.02.08 hor 隠しといてアイテム削除→あとで表示
	::ShowWindow( hwndList, SW_HIDE );
	::ShowWindow( hwndTree, SW_HIDE );
	ListView_DeleteAllItems( hwndList );
	TreeView_DeleteAllItems( hwndTree );

	m_cmemClipText.SetDataSz( "" );	/* クリップボードコピー用テキスト */

	if( OUTLINE_CPP == m_nListType ){	/* C++メソッドリスト */
		//	May 18, 2001 genta
		//	Windowがいなくなると後で都合が悪いので、表示しないだけにしておく
		//	::DestroyWindow( hwndList );
//		::ShowWindow( hwndList, SW_HIDE );
		m_nViewType = 1;
		/* ツリーコントロールの初期化：C++メソッドツリー */
		//SetTreeCpp( m_hWnd );
		//	Jan. 04, 2002 genta Java Method Treeに統合

		SetTreeJava( m_hWnd, TRUE );

		::SetWindowText( m_hWnd, "C++ メソッドツリー" );
	}else
	if( OUTLINE_FILE == m_nListType ){	//@@@ 2002.04.01 YAZAKI アウトライン解析にルールファイル導入
		m_nViewType = 1;
		SetTree();
		::SetWindowText( m_hWnd, "ルールファイル" );
	}else
	if( OUTLINE_WZTXT == m_nListType ){ //@@@ 2003.05.20 zenryaku 階層付テキストアウトライン解析
		m_nViewType = 1;
		SetTree();
		//	2003.06.22 Moca 名前変更
		::SetWindowText( m_hWnd, "WZ階層付テキスト" );
	}else
	if( OUTLINE_HTML == m_nListType ){ //@@@ 2003.05.20 zenryaku HTMLアウトライン解析
		m_nViewType = 1;
		SetTree();
		::SetWindowText( m_hWnd, "HTML" );
	}else
	if( OUTLINE_TEX == m_nListType ){ //@@@ 2003.07.20 naoh TeXアウトライン解析
		m_nViewType = 1;
		SetTree();
		::SetWindowText( m_hWnd, "TeX" );
	}else
	if( OUTLINE_TEXT == m_nListType ){ /* テキスト・トピックリスト */
		//	May 18, 2001 genta
		//	Windowがいなくなると後で都合が悪いので、表示しないだけにしておく
		//::DestroyWindow( hwndList );
//		::ShowWindow( hwndList, SW_HIDE );
		m_nViewType = 1;
		/* ツリーコントロールの初期化：テキストトピックツリー */
//		SetTreeTxt();
		SetTree();	//@@@ 2002.04.01 YAZAKI テキストトピックツリーも、汎用SetTreeを呼ぶように変更。
		::SetWindowText( m_hWnd, "テキスト トピックツリー" );
	}else
	if( OUTLINE_JAVA == m_nListType ){ /* Javaメソッドツリー */
		//	May 18, 2001 genta
		//	Windowがいなくなると後で都合が悪いので、表示しないだけにしておく
		//	::DestroyWindow( hwndList );
//		::ShowWindow( hwndList, SW_HIDE );
		m_nViewType = 1;
		/* ツリーコントロールの初期化：Javaメソッドツリー */
		SetTreeJava( m_hWnd, TRUE );
		::SetWindowText( m_hWnd, "Java メソッドツリー" );
	}else
	if( OUTLINE_COBOL == m_nListType ){ /* COBOL アウトライン */
		//	May 18, 2001 genta
		//	Windowがいなくなると後で都合が悪いので、表示しないだけにしておく
		//	::DestroyWindow( hwndList );
//		::ShowWindow( hwndList, SW_HIDE );
		m_nViewType = 1;
		/* ツリーコントロールの初期化：COBOL アウトライン */
		SetTreeJava( m_hWnd, FALSE );
		::SetWindowText( m_hWnd, "COBOL アウトライン" );
	}else
	if( OUTLINE_VB == m_nListType ){	/* VisualBasic アウトライン */
		// Jul 10, 2003  little YOSHI   処理が長くなったので独立
		m_nViewType = 0;
		/* リストビューコントロールの初期化：Visual Basic アウトライン */
		SetListVB();
		::SetWindowText( m_hWnd, "Visual Basic アウトライン" );
	}else{
		m_nViewType = 0;
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
// Jul 10, 2003  little YOSHI  上に移動しました--->>
//		case OUTLINE_VB:	// 2001/06/23 N.Nakatani for Visual Basic
//			::SetWindowText( m_hWnd, "Visual Basic アウトライン" );
//			break;
// <<---ここまで
		case OUTLINE_BOOKMARK:
			LV_COLUMN col;
			col.mask = LVCF_TEXT;
			col.pszText = "テキスト";
			col.iSubItem = 0;
			//	Apr. 23, 2005 genta 行番号を左端へ
			ListView_SetColumn( hwndList, 1, &col );
			::SetWindowText( m_hWnd, "ブックマーク" );
			break;
//		case OUTLINE_COBOL:
//			::SetWindowText( m_hWnd, "COBOLアウトライン" );
//			break;
		}
		//	May 18, 2001 genta
		//	Windowがいなくなると後で都合が悪いので、表示しないだけにしておく
		//::DestroyWindow( hwndTree );
//		::ShowWindow( hwndTree, SW_HIDE );
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

			//	From Here Apr. 23, 2005 genta 行番号を左端へ
			/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
			if(m_bLineNumIsCRLF ){
				wsprintf( szText, "%d", pcFuncInfo->m_nFuncLineCRLF );
			}else{
				wsprintf( szText, "%d", pcFuncInfo->m_nFuncLineLAYOUT );
			}
			item.mask = LVIF_TEXT | LVIF_PARAM;
			item.pszText = szText;
			item.iItem = i;
			item.lParam	= i;
			item.iSubItem = 0;
			ListView_InsertItem( hwndList, &item);

			item.mask = LVIF_TEXT;
			item.pszText = pcFuncInfo->m_cmemFuncName.GetPtr();
			item.iItem = i;
			item.iSubItem = 1;
			ListView_SetItem( hwndList, &item);
			//	To Here Apr. 23, 2005 genta 行番号を左端へ

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
				// Jul 10, 2003  little YOSHI
				// ここにあったVB関係の処理はSetListVB()メソッドに移動しました。

				item.pszText = "";
			}
			item.iItem = i;
			item.iSubItem = 2;
			ListView_SetItem( hwndList, &item);

			/* クリップボードにコピーするテキストを編集 */
			if(lstrlen(item.pszText)){
				// 検出結果の種類(関数,,,)があるとき
				wsprintf( szText, "%s(%d): %s(%s)\r\n",
					m_pcFuncInfoArr->m_szFilePath,				/* 解析対象ファイル名 */
					pcFuncInfo->m_nFuncLineCRLF,				/* 検出行番号 */
					pcFuncInfo->m_cmemFuncName.GetPtr(),	/* 検出結果 */
					item.pszText								/* 検出結果の種類 */
				);
			}else{
				// 検出結果の種類(関数,,,)がないとき
				wsprintf( szText, "%s(%d): %s\r\n",
					m_pcFuncInfoArr->m_szFilePath,				/* 解析対象ファイル名 */
					pcFuncInfo->m_nFuncLineCRLF,				/* 検出行番号 */
					pcFuncInfo->m_cmemFuncName.GetPtr()	/* 検出結果 */
				);
			}
			m_cmemClipText.AppendSz( (const char *)szText );					/* クリップボードコピー用テキスト */
		}
		//2002.02.08 hor Listは列幅調整とかを実行する前に表示しとかないと変になる
		::ShowWindow( hwndList, SW_SHOW );
		/* 列の幅をデータに合わせて調整 */
		ListView_SetColumnWidth( hwndList, 0, LVSCW_AUTOSIZE );
		ListView_SetColumnWidth( hwndList, 1, LVSCW_AUTOSIZE );
		ListView_SetColumnWidth( hwndList, 2, LVSCW_AUTOSIZE );
		ListView_SetColumnWidth( hwndList, 0, ListView_GetColumnWidth( hwndList, 0 ) + 16 );
		ListView_SetColumnWidth( hwndList, 1, ListView_GetColumnWidth( hwndList, 1 ) + 16 );
		ListView_SetColumnWidth( hwndList, 2, ListView_GetColumnWidth( hwndList, 2 ) + 16 );

		// 2005.07.05 ぜっと
		DWORD dwExStyle  = ListView_GetExtendedListViewStyle( hwndList );
		dwExStyle |= LVS_EX_FULLROWSELECT;
		ListView_SetExtendedListViewStyle( hwndList, dwExStyle );

		if( bSelected ){
			ListView_GetItemRect( hwndList, 0, &rc, LVIR_BOUNDS );
			ListView_Scroll( hwndList, 0, nSelectedLine * ( rc.bottom - rc.top ) );
			ListView_SetItemState( hwndList, nSelectedLine, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
		}
	}
	/* アウトライン ダイアログを自動的に閉じる */
	::CheckDlgButton( m_hWnd, IDC_CHECK_bAutoCloseDlgFuncList, m_pShareData->m_Common.m_bAutoCloseDlgFuncList );
	/* アウトライン ブックマーク一覧で空行を無視する */
	::CheckDlgButton( m_hWnd, IDC_CHECK_bMarkUpBlankLineEnable, m_pShareData->m_Common.m_bMarkUpBlankLineEnable );
	/* アウトライン ジャンプしたらフォーカスを移す */
	::CheckDlgButton( m_hWnd, IDC_CHECK_bFunclistSetFocusOnJump, m_pShareData->m_Common.m_bFunclistSetFocusOnJump );

	/* アウトライン ■位置とサイズを記憶する */ // 20060201 aroka
	::CheckDlgButton( m_hWnd, IDC_BUTTON_WINSIZE, m_pShareData->m_Common.m_bRememberOutlineWindowPos );

	/* ダイアログを自動的に閉じるならフォーカス移動オプションは関係ない */
	if(m_pShareData->m_Common.m_bAutoCloseDlgFuncList){
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHECK_bFunclistSetFocusOnJump ), FALSE );
	}else{
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHECK_bFunclistSetFocusOnJump ), TRUE );
	}

	//2002.02.08 hor
	//（IDC_LIST1もIDC_TREE1も常に存在していて、m_nViewTypeによって、どちらを表示するかを選んでいる）
	if(m_nViewType){
		::ShowWindow( hwndTree, SW_SHOW );
		::SetFocus( hwndTree );
	}else{
//		::ShowWindow( hwndList, SW_SHOW );
		::SetFocus( hwndList );
	}
	//2002.02.08 hor
	//空行をどう扱うかのチェックボックスはブックマーク一覧のときだけ表示する
	if(OUTLINE_BOOKMARK == m_nListType){
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHECK_bMarkUpBlankLineEnable ), TRUE );
		::ShowWindow( GetDlgItem( m_hWnd, IDC_CHECK_bMarkUpBlankLineEnable ), SW_SHOW );
	}else{
		::ShowWindow( GetDlgItem( m_hWnd, IDC_CHECK_bMarkUpBlankLineEnable ), SW_HIDE );
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHECK_bMarkUpBlankLineEnable ), FALSE );
	}
	// 2002/11/1 frozen 項目のソート基準を設定するコンボボックスはブックマーク一覧の以外の時に表示する
	// Nov. 5, 2002 genta ツリー表示の時だけソート基準コンボボックスを表示
	if( m_nViewType == 1 ){
		HWND hWnd_Combo_Sort = ::GetDlgItem( m_hWnd, IDC_COMBO_nSortType );
		::EnableWindow( hWnd_Combo_Sort , TRUE );
		::ShowWindow( hWnd_Combo_Sort , SW_SHOW );
		::SendMessage( hWnd_Combo_Sort , CB_RESETCONTENT, 0, 0 ); // 2002.11.10 Moca 追加
		::SendMessage( hWnd_Combo_Sort , CB_ADDSTRING, 0, (LPARAM)(_T("デフォルト")));
		::SendMessage( hWnd_Combo_Sort , CB_ADDSTRING, 0, (LPARAM)(_T("アルファベット順")));
		::SendMessage( hWnd_Combo_Sort , CB_SETCURSEL, m_nSortType, 0L);
		::ShowWindow( GetDlgItem( m_hWnd, IDC_STATIC_nSortType ), SW_SHOW );
		// 2002.11.10 Moca 追加 ソートする
		if( 1 == m_nSortType ){
			SortTree(::GetDlgItem( m_hWnd , IDC_TREE1),TVI_ROOT);
		}
	}
	else {
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_COMBO_nSortType ), FALSE );
		::ShowWindow( GetDlgItem( m_hWnd, IDC_COMBO_nSortType ), SW_HIDE );
		::ShowWindow( GetDlgItem( m_hWnd, IDC_STATIC_nSortType ), SW_HIDE );
		//ListView_SortItems( hwndList, _CompareFunc_, (LPARAM)this );  // 2005.04.05 zenryaku ソート状態を保持
		SortListView( hwndList, m_nSortCol );	// 2005.04.23 genta 関数化(ヘッダ書き換えのため)
	}

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

//2002.02.08 hor OnBnClicked内に移動
//	/* アウトライン ダイアログを自動的に閉じる */
//	m_pShareData->m_Common.m_bAutoCloseDlgFuncList = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_bAutoCloseDlgFuncList );

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
				// lParamが-1以下は pcFuncInfoArrには含まれない項目
				if( 0 <= tvi.lParam ){
					pcFuncInfo = m_pcFuncInfoArr->GetAt( tvi.lParam );
					nLineTo = pcFuncInfo->m_nFuncLineCRLF;
				}
			}
		}
	}
	return nLineTo;
}

#if 0
Jan. 04, 2001 genta Java Treeに統合
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

		pWork = pcFuncInfo->m_cmemFuncName.GetPtr();

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
				pcFuncInfo->m_cmemFuncName.GetPtr() 	/* 検出結果 */
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
					pcFuncInfo->m_cmemFuncName.GetPtr() 	/* 検出結果 */
				);
//				m_cmemClipText.Append( (const char *)szText, lstrlen( szText ) );	/* クリップボードコピー用テキスト */
				m_cmemClipText.AppendSz( (const char *)szText );					/* クリップボードコピー用テキスト */
			}else{
				/* クリップボードにコピーするテキストを編集 */
				wsprintf( szText, "%s(%d): %s\r\n",
					m_pcFuncInfoArr->m_szFilePath,				/* 解析対象ファイル名 */
					pcFuncInfo->m_nFuncLineCRLF,				/* 検出行番号 */
					pcFuncInfo->m_cmemFuncName.GetPtr() 	/* 検出結果 */
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
#endif


/*! ツリーコントロールの初期化：Javaメソッドツリー

	Java Method Treeの構築: 関数リストを元にTreeControlを初期化する。

	@date 2002.01.04 genta C++ツリーを統合
*/
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
    char            szLabel[64+6];  // Jan. 07, 2001 genta クラス名エリアの拡大
	HTREEITEM		htiGlobal = NULL;	// Jan. 04, 2001 genta C++と統合
	HTREEITEM		htiClass;
	HTREEITEM		htiItem;
	HTREEITEM		htiItemOld;
	HTREEITEM		htiSelected;
	TV_ITEM			tvi;
	int				nClassNest;
	int				nDummylParam = -64000;	// 2002.11.10 Moca クラス名のダミーlParam ソートのため
	char			szClassArr[16][64];	// Jan. 04, 2001 genta クラス名エリアの拡大

	::EnableWindow( ::GetDlgItem( m_hWnd , IDC_BUTTON_COPY ), TRUE );

	hwndTree = ::GetDlgItem( m_hWnd, IDC_TREE1 );

	nFuncLineOld = 0;
	bSelected = FALSE;
	htiItemOld = NULL;
	for( i = 0; i < m_pcFuncInfoArr->GetNum(); ++i ){
		pcFuncInfo = m_pcFuncInfoArr->GetAt( i );
		pWork = pcFuncInfo->m_cmemFuncName.GetPtr();
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
				// 2005-09-02 D.S.Koba GetSizeOfChar
				nCharChars = CMemory::GetSizeOfChar( pWork, nWorkLen, k );
				if( 1 == nCharChars && ':' == pWork[k] ){
					//	Jan. 04, 2001 genta
					//	C++の統合のため、\に加えて::をクラス区切りとみなすように
					if( k < nWorkLen - 1 && ':' == pWork[k+1] ){
						memcpy( szClassArr[nClassNest], &pWork[m], k - m );
						szClassArr[nClassNest][k - m] = '\0';
						++nClassNest;
						m = k + 2;
						++k;
					}
					else 
						break;
				}
				else if( 1 == nCharChars && '\\' == pWork[k] ){
					memcpy( szClassArr[nClassNest], &pWork[m], k - m );
					szClassArr[nClassNest][k - m] = '\0';
					++nClassNest;
					m = k + 1;
				}
				if( 2 == nCharChars ){
					++k;
				}
			}
			//	Jan. 04, 2001 genta
			//	::もクラス区切りとみなすので、最後の文字列は関数名として残しておく
			/*
			if( 0 < k - m ){
				memcpy( szClassArr[nClassNest], &pWork[m], k - m );
				szClassArr[nClassNest][k - m] = '\0';
				++nClassNest;
			}
			*/
//			for( k = 0; k < nClassNest; ++k ){
//				MYTRACE( "%d [%s]\n", k, szClassArr[k] );
//			}
//			MYTRACE( "\n" );
			//	Jan. 04, 2001 genta
			//	関数先頭のセット(ツリー構築で使う)
			pWork = pWork + m; // 2 == lstrlen( "::" );

			/* クラス名のアイテムが登録されているか */
			htiClass = TreeView_GetFirstVisible( hwndTree );
			HTREEITEM htiParent = TVI_ROOT;
			for( k = 0; k < nClassNest; ++k ){
				//	Apr. 1, 2001 genta
				//	追加文字列を全角にしたのでメモリもそれだけ必要
				//	6 == strlen( "クラス" ), 1 == strlen( '\0' )

				// 2002/10/30 frozen
				// bAddClass == true の場合の仕様変更
				// 既存の項目は　「(クラス名)(半角スペース一個)(追加文字列)」
				// となっているとみなし、szClassArr[k] が 「クラス名」と一致すれば、それを親ノードに設定。
				// ただし、一致する項目が複数ある場合は最初の項目を親ノードにする。
				// 一致しない場合は「(クラス名)(半角スペース一個)クラス」のノードを作成する。
				size_t nClassNameLen = strlen( szClassArr[k] );
				for( ; NULL != htiClass ; htiClass = TreeView_GetNextSibling( hwndTree, htiClass ))
				{
					tvi.mask = TVIF_HANDLE | TVIF_TEXT;
					tvi.hItem = htiClass;
					tvi.pszText = szLabel;
					tvi.cchTextMax = sizeof(szLabel);
					if( TreeView_GetItem( hwndTree, &tvi ) )
					{
						if( 0 == strncmp( szClassArr[k],szLabel,nClassNameLen) )
						{
							if(sizeof(szLabel) < (nClassNameLen +1) )
								break;// バッファ不足では無条件にマッチする
							else
							{
								if(bAddClass)
								{
									if(szLabel[nClassNameLen]==' ')
										break;
								}
								else
								{
									if(szLabel[nClassNameLen]=='\0')
										break;
								}
							}
						}
					}
				}
//				pClassName = new char[ lstrlen( szClassArr[k] ) + 1 + 6 ];
//				strcpy( pClassName, szClassArr[k] );
//				if( bAddClass ){
//					strcat( pClassName, "クラス" );
//				}
//				while( NULL != htiClass ){
//					tvi.mask = TVIF_HANDLE | TVIF_TEXT;
//					tvi.hItem = htiClass;
//					tvi.pszText = szLabel;
//					tvi.cchTextMax = sizeof(szLabel);
//					if( TreeView_GetItem( hwndTree, &tvi ) ){
//						if( 0 == strcmp( pClassName, szLabel ) ){
//							break;
//						}
//					}
//					htiClass = TreeView_GetNextSibling( hwndTree, htiClass );
//				}
				/* クラス名のアイテムが登録されていないので登録 */
				if( NULL == htiClass ){
					// 2002/10/28 frozen 上からここへ移動
					pClassName = new char[ lstrlen( szClassArr[k] ) + 1 + 9 ]; // 2002/10/28 frozen +9は追加する文字列の最大長（" 名前空間"が最大）
					strcpy( pClassName, szClassArr[k] );

					tvis.item.lParam = -1;
					if( bAddClass )
					{
						if( pcFuncInfo->m_nInfo == 7 )
						{
							strcat( pClassName, " 名前空間" );
							tvis.item.lParam = i;
						}
						else
							strcat( pClassName, " クラス" );
							tvis.item.lParam = nDummylParam;
							nDummylParam++;
					}

					tvis.hParent = htiParent;
					tvis.hInsertAfter = TVI_LAST;
					tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
					tvis.item.pszText = pClassName;

					htiClass = TreeView_InsertItem( hwndTree, &tvis );
					//	Jan. 04, 2001 genta
					//	不要になったらさっさと削除
					delete [] pClassName; // 2002/10/28 frozen 下からここへ移動

				}else{
					//none
				}
				htiParent = htiClass;
				//if( k + 1 >= nClassNest ){
				//	break;
				//}
				htiClass = TreeView_GetChild( hwndTree, htiClass );
			}
			htiClass = htiParent;
		}else{
			//	Jan. 04, 2001 genta
			//	Global空間の場合 (C++のみ)

			// 2002/10/27 frozen ここから
			if( 3 <= pcFuncInfo->m_nInfo  && pcFuncInfo->m_nInfo <= 7 )
				htiClass = TVI_ROOT;
			else
			{
			// 2002/10/27 frozen ここまで
				if( htiGlobal == NULL ){
					TV_INSERTSTRUCT	tvg;
					
					::ZeroMemory( &tvg, sizeof(tvg));
					tvg.hParent = TVI_ROOT;
					tvg.hInsertAfter = TVI_LAST;
					tvg.item.mask = TVIF_TEXT | TVIF_PARAM;
					tvg.item.pszText = "グローバル";
//					tvg.item.lParam = -1;
					tvg.item.lParam = nDummylParam;
					htiGlobal = TreeView_InsertItem( hwndTree, &tvg );
					nDummylParam++;
				}
				htiClass = htiGlobal;
			}
		}
		pFuncName = new char[ strlen(pWork) + 1 + 7 ];	// +6 は追加する文字列の最大長
		strcpy( pFuncName, pWork );

		// 2002/10/27 frozen 追加文字列の種類を増やした
		switch(pcFuncInfo->m_nInfo)
		{// case 4以上の各追加文字列の最初にある半角スペースを省略することはできない。
		case 1: strcat( pFuncName, "(宣言)" );break;
		case 3: strcat( pFuncName, " クラス" );break;
		case 4: strcat( pFuncName, " 構造体" );break;
		case 5: strcat( pFuncName, " 列挙体" );break;
		case 6: strcat( pFuncName, " 共用体" );break;
//		case 7: strcat( pFuncName, " 名前空間" );break;
		};
//		}
		/* 該当クラス名のアイテムの子として、メソッドのアイテムを登録 */
		tvis.hParent = htiClass;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
		tvis.item.pszText = pFuncName;
		tvis.item.lParam = i;
		htiItem = TreeView_InsertItem( hwndTree, &tvis );

		/* クリップボードにコピーするテキストを編集 */
		wsprintf( szText, "%s(%d): %s %s\r\n",
			m_pcFuncInfoArr->m_szFilePath,				/* 解析対象ファイル名 */
			pcFuncInfo->m_nFuncLineCRLF,				/* 検出行番号 */
			pcFuncInfo->m_cmemFuncName.GetPtr(), 	/* 検出結果 */
			( 1 == pcFuncInfo->m_nInfo ? "(宣言)" : "" ) 	//	Jan. 04, 2001 genta C++で使用
		);
		m_cmemClipText.AppendSz( (const char *)szText ); /* クリップボードコピー用テキスト */
		delete [] pFuncName;

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
		htiItemOld = htiItem;
		//	Jan. 04, 2001 genta
		//	deleteはその都度行うのでここでは不要
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


/*! リストビューコントロールの初期化：VisualBasic

  長くなったので独立させました。

  @date Jul 10, 2003  little YOSHI
*/
void CDlgFuncList::SetListVB (void)
{
	int				i;
	char			szText[2048], szType[128], szOption[256];
	CFuncInfo*		pcFuncInfo;
	LV_ITEM			item;
	HWND			hwndList;
	int				bSelected;
	int				nFuncLineOld;
	int				nSelectedLine;
	RECT			rc;

	::EnableWindow( ::GetDlgItem( m_hWnd , IDC_BUTTON_COPY ), TRUE );

	hwndList = ::GetDlgItem( m_hWnd, IDC_LIST1 );

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

		//	From Here Apr. 23, 2005 genta 行番号を左端へ
		/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
		if(m_bLineNumIsCRLF ){
			wsprintf( szText, "%d", pcFuncInfo->m_nFuncLineCRLF );
		}else{
			wsprintf( szText, "%d", pcFuncInfo->m_nFuncLineLAYOUT );
		}
		item.mask = LVIF_TEXT | LVIF_PARAM;
		item.pszText = szText;
		item.iItem = i;
		item.iSubItem = 0;
		item.lParam	= i;
		ListView_InsertItem( hwndList, &item);

		item.mask = LVIF_TEXT;
		item.pszText = pcFuncInfo->m_cmemFuncName.GetPtr();
		item.iItem = i;
		item.iSubItem = 1;
		ListView_SetItem( hwndList, &item);
		//	To Here Apr. 23, 2005 genta 行番号を左端へ

		item.mask = LVIF_TEXT;

		// 2001/06/23 N.Nakatani for Visual Basic
		//	Jun. 26, 2001 genta 半角かな→全角に
		memset(szText, '\0', sizeof(szText));
		memset(szType, '\0', sizeof(szType));
		memset(szOption, '\0', sizeof(szOption));
		if( 1 == ((pcFuncInfo->m_nInfo >> 8) & 0x01) ){
			// スタティック宣言(Static)
			strcpy(szOption, "静的");
		}
		switch ((pcFuncInfo->m_nInfo >> 4) & 0x0f) {
			case 2  :	// プライベート(Private)
				strncat(szOption, "プライベート", sizeof(szText) - strlen(szText));
				break;

			case 3  :	// フレンド(Friend)
				strncat(szOption, "フレンド", sizeof(szText) - strlen(szText));
				break;

			default :	// パブリック(Public)
				strncat(szOption, "パブリック", sizeof(szText) - strlen(szText));
		}
		switch (pcFuncInfo->m_nInfo & 0x0f) {
			case 1:		// 関数(Function)
				strcpy(szType, "関数");
				break;

			case 2:		// ステータス(Sub)
				strcpy(szType, "ステータス");
				break;

			case 3:		// プロパティ 取得(Property Get)
				strcpy(szType, "プロパティ 取得");
				break;

			case 4:		// プロパティ 設定(Property Let)
				strcpy(szType, "プロパティ 設定");
				break;

			case 5:		// プロパティ 参照(Property Set)
				strcpy(szType, "プロパティ 参照");
				break;

			case 6:		// 定数(Const)
				strcpy(szType, "定数");
				break;

			case 7:		// 列挙型(Enum)
				strcpy(szType, "列挙型");
				break;

			case 8:		// ユーザ定義型(Type)
				strcpy(szType, "ユーザ定義型");
				break;

			case 9:		// イベント(Event)
				strcpy(szType, "イベント");
				break;

			default:	// 未定義なのでクリア
				pcFuncInfo->m_nInfo	= 0;

		}
		if ( 2 == ((pcFuncInfo->m_nInfo >> 8) & 0x02) ) {
			// 宣言(Declareなど)
			strncat(szType, "宣言", sizeof(szType) - strlen(szType));
		}

		if ( 0 == pcFuncInfo->m_nInfo ) {
			memset(szText, '\0', sizeof(szText));
		} else
		if ( 0 == strlen(szOption) ) {
			wsprintf(szText, "%s", szType);
		} else {
			wsprintf(szText, "%s（%s）", szType, szOption);
		}
		item.pszText = szText;
		item.iItem = i;
		item.iSubItem = 2;
		ListView_SetItem( hwndList, &item);

		/* クリップボードにコピーするテキストを編集 */
		if(lstrlen(item.pszText)){
			// 検出結果の種類(関数,,,)があるとき
			wsprintf( szText, "%s(%d): %s(%s)\r\n",
				m_pcFuncInfoArr->m_szFilePath,				/* 解析対象ファイル名 */
				pcFuncInfo->m_nFuncLineCRLF,				/* 検出行番号 */
				pcFuncInfo->m_cmemFuncName.GetPtr(),		/* 検出結果 */
				item.pszText								/* 検出結果の種類 */
			);
		}else{
			// 検出結果の種類(関数,,,)がないとき
			wsprintf( szText, "%s(%d): %s\r\n",
				m_pcFuncInfoArr->m_szFilePath,				/* 解析対象ファイル名 */
				pcFuncInfo->m_nFuncLineCRLF,				/* 検出行番号 */
				pcFuncInfo->m_cmemFuncName.GetPtr()			/* 検出結果 */
			);
		}
		m_cmemClipText.AppendSz( (const char *)szText );	/* クリップボードコピー用テキスト */
	}

	//2002.02.08 hor Listは列幅調整とかを実行する前に表示しとかないと変になる
	::ShowWindow( hwndList, SW_SHOW );
	/* 列の幅をデータに合わせて調整 */
	ListView_SetColumnWidth( hwndList, 0, LVSCW_AUTOSIZE );
	ListView_SetColumnWidth( hwndList, 1, LVSCW_AUTOSIZE );
	ListView_SetColumnWidth( hwndList, 2, LVSCW_AUTOSIZE );
	ListView_SetColumnWidth( hwndList, 0, ListView_GetColumnWidth( hwndList, 0 ) + 16 );
	ListView_SetColumnWidth( hwndList, 1, ListView_GetColumnWidth( hwndList, 1 ) + 16 );
	ListView_SetColumnWidth( hwndList, 2, ListView_GetColumnWidth( hwndList, 2 ) + 16 );
	if( bSelected ){
		ListView_GetItemRect( hwndList, 0, &rc, LVIR_BOUNDS );
		ListView_Scroll( hwndList, 0, nSelectedLine * ( rc.bottom - rc.top ) );
		ListView_SetItemState( hwndList, nSelectedLine, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	}

	return;
}

#if 0
2002.04.01 YAZAKI もともと使用されていなかったため
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
#endif

/*! 汎用ツリーコントロールの初期化：CFuncInfo::m_nDepthを利用して親子を設定

	@date 2002.04.01 YAZAKI
	@date 2002.11.10 Moca 階層の制限をなくした
*/
void CDlgFuncList::SetTree()
{
	HTREEITEM hItemSelected = NULL;
	HWND hwndTree = ::GetDlgItem( m_hWnd, IDC_TREE1 );

	int i;
	int nFuncInfoArrNum = m_pcFuncInfoArr->GetNum();
	int nStackPointer = 0;
	int nStackDepth = 32; // phParentStack の確保している数
	HTREEITEM* phParentStack;
	phParentStack = (HTREEITEM*)malloc( nStackDepth * sizeof( HTREEITEM ) );
	phParentStack[ nStackPointer ] = TVI_ROOT;

	for (i = 0; i < nFuncInfoArrNum; i++){
		CFuncInfo* pcFuncInfo = m_pcFuncInfoArr->GetAt(i);

		/*	新しいアイテムを作成
			現在の親の下にぶら下げる形で、最後に追加する。
		*/
		HTREEITEM hItem;
		TV_INSERTSTRUCT cTVInsertStruct;
		cTVInsertStruct.hParent = phParentStack[ nStackPointer ];
		cTVInsertStruct.hInsertAfter = TVI_LAST;	//	必ず最後に追加。
		cTVInsertStruct.item.mask = TVIF_TEXT | TVIF_PARAM;
		cTVInsertStruct.item.pszText = pcFuncInfo->m_cmemFuncName.GetPtr();
		cTVInsertStruct.item.lParam = i;	//	あとでこの数値（＝m_pcFuncInfoArrの何番目のアイテムか）を見て、目的地にジャンプするぜ!!。

		/*	親子関係をチェック
		*/
		if (nStackPointer != pcFuncInfo->m_nDepth){
			//	レベルが変わりました!!
			//	※が、2段階深くなることは考慮していないので注意。
			//	　もちろん、2段階以上浅くなることは考慮済み。

			// 2002.11.10 Moca 追加 確保したサイズでは足りなくなった。再確保
			if( nStackDepth <= pcFuncInfo->m_nDepth + 1 ){
				nStackDepth = pcFuncInfo->m_nDepth + 4; // 多めに確保しておく
				HTREEITEM* phTi;
				phTi = (HTREEITEM*)realloc( phParentStack, nStackDepth * sizeof( HTREEITEM ) );
				if( NULL != phTi ){
					phParentStack = phTi;
				}else{
					goto end_of_func;
				}
			}
			nStackPointer = pcFuncInfo->m_nDepth;
			cTVInsertStruct.hParent = phParentStack[ nStackPointer ];
		}
		hItem = TreeView_InsertItem( hwndTree, &cTVInsertStruct );
		phParentStack[ nStackPointer+1 ] = hItem;

		/*	pcFuncInfoに登録されている行数を確認して、選択するアイテムを考える
		*/
		if ( pcFuncInfo->m_nFuncLineLAYOUT <= m_nCurLine ){
			hItemSelected = hItem;
		}

		/* クリップボードコピー用テキストを作成する */
		//	2003.06.22 Moca dummy要素はツリーに入れるがTAGJUMPには加えない
		if( pcFuncInfo->IsAddClipText() ){
			int j;
			for( j = 0; j < nStackPointer; ++j ){
				m_cmemClipText.AppendSz( "  " );
			}
			m_cmemClipText.AppendSz( (const char *)pcFuncInfo->m_cmemFuncName.GetPtr() );
			m_cmemClipText.AppendSz( (const char *)"\r\n" );
		}
	}

end_of_func:;

	::EnableWindow( ::GetDlgItem( m_hWnd , IDC_BUTTON_COPY ), TRUE );

	if( NULL != hItemSelected ){
		/* 現在カーソル位置のメソッドを選択状態にする */
		TreeView_SelectItem( hwndTree, hItemSelected );
	}

	free( phParentStack );
}

#if 0
2002.04.01 YAZAKI SetTreeTxt()、SetTreeTxtNest()は廃止。汎用のSetTree()を使うようにしました。
/* ツリーコントロールの初期化：テキスト トピックツリー */
void CDlgFuncList::SetTreeTxt()
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
		pWork = (unsigned char*)pcFuncInfo->m_cmemFuncName.GetPtr();
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
			pWork2 = (unsigned char*)pcFuncInfo2->m_cmemFuncName.GetPtr();
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
#endif


BOOL CDlgFuncList::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	m_hWnd = hwndDlg;
	HWND		hwndList;
	int			nCxVScroll;
	int			nColWidthArr[3] = { 0, 46, 80 };
	RECT		rc;
	LV_COLUMN	col;
	hwndList = ::GetDlgItem( hwndDlg, IDC_LIST1 );
	// 2005.10.21 zenryaku 1行選択
	SendMessage(hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
		SendMessage(hwndList, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0) | LVS_EX_FULLROWSELECT);

	::GetWindowRect( hwndList, &rc );
	nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = rc.right - rc.left - ( nColWidthArr[1] + nColWidthArr[2] ) - nCxVScroll - 8;
	//	Apr. 23, 2005 genta 行番号を左端へ
	col.pszText = "行 *";
	col.iSubItem = 0;
	ListView_InsertColumn( hwndList, 0, &col);

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = nColWidthArr[1];
	//	Apr. 23, 2005 genta 行番号を左端へ
	col.pszText = "関数名";
	col.iSubItem = 1;
	ListView_InsertColumn( hwndList, 1, &col);

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = nColWidthArr[2];
	col.pszText = " ";
	col.iSubItem = 2;
	ListView_InsertColumn( hwndList, 2, &col);

	/* アウトライン位置とサイズを初期化する */ // 20060201 aroka
	if( m_lParam != NULL ){
		CEditView* pcEditView=(CEditView*)m_lParam;
		if( m_pShareData->m_Common.m_bRememberOutlineWindowPos ){
			WINDOWPLACEMENT cWindowPlacement;
			cWindowPlacement.length = sizeof( WINDOWPLACEMENT );
			if (::GetWindowPlacement( pcEditView->m_pcEditDoc->m_pcEditWnd->m_hWnd, &cWindowPlacement )){
				/* ウィンドウ位置・サイズを-1以外の値にしておくと、CDialogで使用される． */
				m_xPos = m_pShareData->m_Common.m_xOutlineWindowPos + cWindowPlacement.rcNormalPosition.left;
				m_yPos = m_pShareData->m_Common.m_yOutlineWindowPos + cWindowPlacement.rcNormalPosition.top;
				m_nWidth =  m_pShareData->m_Common.m_widthOutlineWindow;
				m_nHeight = m_pShareData->m_Common.m_heightOutlineWindow;
			}
		}
	}
	/* 基底クラスメンバ */
	CreateSizeBox();
	return CDialog::OnInitDialog( hwndDlg, wParam, lParam );
}


BOOL CDlgFuncList::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* 「アウトライン解析」のヘルプ */
		//Apr. 5, 2001 JEPRO 修正漏れを追加 (Stonee, 2001/03/12 第四引数を、機能番号からヘルプトピック番号を調べるようにした)
		MyWinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_OUTLINE) );	// 2006.10.10 ryoji MyWinHelpに変更に変更
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
		// Windowsクリップボードにコピー 
		// 2004.02.17 Moca 関数化
		SetClipboardText( m_hWnd, m_cmemClipText.GetPtr(), m_cmemClipText.GetLength() );
		return TRUE;
	case IDC_BUTTON_WINSIZE:
		{// ウィンドウの位置とサイズを記憶 // 20060201 aroka
			m_pShareData->m_Common.m_bRememberOutlineWindowPos = ::IsDlgButtonChecked( m_hWnd, IDC_BUTTON_WINSIZE );
		}
		return TRUE;
	//2002.02.08 オプション切替後List/Treeにフォーカス移動
	case IDC_CHECK_bAutoCloseDlgFuncList:
	case IDC_CHECK_bMarkUpBlankLineEnable:
	case IDC_CHECK_bFunclistSetFocusOnJump:
		m_pShareData->m_Common.m_bAutoCloseDlgFuncList = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_bAutoCloseDlgFuncList );
		m_pShareData->m_Common.m_bMarkUpBlankLineEnable = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_bMarkUpBlankLineEnable );
		m_pShareData->m_Common.m_bFunclistSetFocusOnJump = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_bFunclistSetFocusOnJump );
		if(m_pShareData->m_Common.m_bAutoCloseDlgFuncList){
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHECK_bFunclistSetFocusOnJump ), FALSE );
		}else{
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHECK_bFunclistSetFocusOnJump ), TRUE );
		}
		if(wID==IDC_CHECK_bMarkUpBlankLineEnable&&m_nListType==OUTLINE_BOOKMARK){
			CEditView* pcEditView=(CEditView*)m_lParam;
			pcEditView->HandleCommand( F_BOOKMARK_VIEW, TRUE, TRUE, 0, 0, 0 );
			m_nCurLine=pcEditView->m_nCaretPosY + 1;
			SetData();
		}else
		if(m_nViewType){
			::SetFocus( ::GetDlgItem( m_hWnd, IDC_TREE1 ) );
		}else{
			::SetFocus( ::GetDlgItem( m_hWnd, IDC_LIST1 ) );
		}
		return TRUE;
	}
	/* 基底クラスメンバ */
	return CDialog::OnBnClicked( wID );
}

BOOL CDlgFuncList::OnNotify( WPARAM wParam, LPARAM lParam )
{
//	int				idCtrl;
	LPNMHDR			pnmh;
	NM_LISTVIEW*	pnlv;
	HWND			hwndList;
	HWND			hwndTree;
	NM_TREEVIEW*	pnmtv;
//	int				nLineTo;

//	idCtrl = (int) wParam;
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
				// 2002.02.16 hor Treeのダブルクリックでフォーカス移動できるように 3/4
				OnJump();
				m_bWaitTreeProcess=true;
				return TRUE;
				//return OnJump();
			case TVN_KEYDOWN:
				Key2Command( ((LV_KEYDOWN *)lParam)->wVKey );
				return TRUE;
			case TVN_SELCHANGED:
				// 2005.11.04 zenryaku 画面外の選択項目を追う
				::SendMessage(hwndTree, TVM_ENSUREVISIBLE, 0, (LPARAM)((NMTREEVIEW*)lParam)->itemNew.hItem);
				break;
//			case NM_CLICK:
			case NM_KILLFOCUS:
				// 2002.02.16 hor Treeのダブルクリックでフォーカス移動できるように 4/4
				if(m_bWaitTreeProcess){
					if(m_pShareData->m_Common.m_bFunclistSetFocusOnJump){
						::SetFocus(m_hwndParent);
					}
					m_bWaitTreeProcess=false;
				}
				return TRUE;
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
			//	Apr. 23, 2005 genta 関数として独立させた
			SortListView( hwndList, m_nSortCol );
			return TRUE;
		case NM_DBLCLK:
			return OnJump();
		case LVN_KEYDOWN:
			Key2Command( ((LV_KEYDOWN *)lParam)->wVKey );
			return TRUE;
		}
	}
	return FALSE;
}
/*!
	指定されたカラムでリストビューをソートする．
	同時にヘッダも書き換える．

	ソート後はフォーカスが画面内に現れるように表示位置を調整する．

	@par 表示位置調整の小技
	EnsureVisibleの結果は，上スクロールの場合は上端に，下スクロールの場合は
	下端に目的の項目が現れる．端から少し離したい場合はオフセットを与える必要が
	あるが，スクロール方向がわからないと±がわからない
	そのため最初に一番下に一回スクロールさせることでEnsureVisibleでは
	かならず上スクロールになるようにすることで，ソート後の表示位置を
	固定する

	@param[in] hwndList	リストビューのウィンドウハンドル
	@param[in] sortcol	ソートするカラム番号(0-2)

	@date 2005.04.23 genta 関数として独立させた
	@date 2005.04.29 genta ソート後の表示位置調整
*/
void CDlgFuncList::SortListView(HWND hwndList, int sortcol)
{
	LV_COLUMN		col;

	//	Apr. 23, 2005 genta 行番号を左端へ
	if( sortcol == 1 ){
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
		ListView_SetColumn( hwndList, 1, &col );
		col.mask = LVCF_TEXT;
		col.pszText = "行";
		col.iSubItem = 0;
		ListView_SetColumn( hwndList, 0, &col );
	// From Here 2001.12.07 hor
		col.mask = LVCF_TEXT;
		col.pszText = "";
		col.iSubItem = 0;
		ListView_SetColumn( hwndList, 2, &col );
	// To Here 2001.12.07 hor
		ListView_SortItems( hwndList, _CompareFunc_, (LPARAM)this );
	}else
	if( sortcol == 0 ){
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
		ListView_SetColumn( hwndList, 1, &col );
		col.mask = LVCF_TEXT;
		col.pszText = "行 *";
		col.iSubItem = 0;
		ListView_SetColumn( hwndList, 0, &col );
	// From Here 2001.12.07 hor
		col.mask = LVCF_TEXT;
		col.pszText = "";
		col.iSubItem = 0;
		ListView_SetColumn( hwndList, 2, &col );
	// To Here 2001.12.03 hor
		ListView_SortItems( hwndList, _CompareFunc_, (LPARAM)this );
	// From Here 2001.12.07 hor
	}else
	if( sortcol == 2 ){
		col.mask = LVCF_TEXT;
		if(OUTLINE_BOOKMARK == m_nListType){
			col.pszText = "テキスト";
		}else{
			col.pszText = "関数名";
		}
		col.iSubItem = 0;
		ListView_SetColumn( hwndList, 1, &col );
		col.mask = LVCF_TEXT;
		col.pszText = "行";
		col.iSubItem = 0;
		ListView_SetColumn( hwndList, 0, &col );
		col.mask = LVCF_TEXT;
		col.pszText = "*";
		col.iSubItem = 0;
		ListView_SetColumn( hwndList, 2, &col );
		ListView_SortItems( hwndList, _CompareFunc_, (LPARAM)this );
	// To Here 2001.12.07 hor
	}
	//	2005.04.23 zenryaku 選択された項目が見えるようにする

	//	Apr. 29, 2005 genta 一旦一番下にスクロールさせる
	ListView_EnsureVisible( hwndList,
		ListView_GetItemCount(hwndList) - 1,
		FALSE );
	
	//	Jan.  9, 2006 genta 先頭から1つ目と2つ目の関数が
	//	選択された場合にスクロールされなかった
	int keypos = ListView_GetNextItem(hwndList, -1, LVNI_FOCUSED) - 2;
	ListView_EnsureVisible( hwndList,
		keypos >= 0 ? keypos : 0,
		FALSE );
}

/*!	ウィンドウサイズが変更された

	@date 2003.06.22 Moca コードの整理(コントロールの処理方法をテーブルに持たせる)
	@date 2003.08.16 genta 配列はstaticに(無駄な初期化を行わないため)
*/
BOOL CDlgFuncList::OnSize( WPARAM wParam, LPARAM lParam )
{
	/* 基底クラスメンバ */
	CDialog::OnSize( wParam, lParam );

	static const int Controls[][2] = {
		{IDC_CHECK_bFunclistSetFocusOnJump, 1},
		{IDC_CHECK_bMarkUpBlankLineEnable , 1},
		{IDC_CHECK_bAutoCloseDlgFuncList, 1},
		{IDC_BUTTON_WINSIZE, 2}, // 20060201 aroka
		{IDC_BUTTON_COPY, 2},
		{IDOK, 2},
		{IDCANCEL, 2},
		{IDC_BUTTON_HELP, 2},
		{IDC_LIST1, 3},
		{IDC_TREE1, 3},
	};
	int		nControls = sizeof( Controls ) / sizeof( Controls[0] );
//	int		fwSizeType;
	int		nWidth;
	int		nHeight;
	int		i;
	int		nHeightCheckBox;
	int		nHeightButton;
	const int	nHeightMargin = 3;
	RECT	rc;
	HWND	hwndCtrl;
	POINT	po;

	nWidth = LOWORD(lParam);	// width of client area
	nHeight = HIWORD(lParam);	// height of client area


	::GetWindowRect( ::GetDlgItem( m_hWnd, IDC_CHECK_bAutoCloseDlgFuncList ), &rc );
	nHeightCheckBox = rc.bottom -  rc.top;
	::GetWindowRect( ::GetDlgItem( m_hWnd, IDOK ), &rc );	
	nHeightButton = rc.bottom - rc.top;

	for ( i = 0; i < nControls; ++i ){
		hwndCtrl = ::GetDlgItem( m_hWnd, Controls[i][0] );
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
		//	2003.06.22 Moca テーブル上の種別によって処理方法を変える
		switch( Controls[i][1] ){
		case 1:
			::SetWindowPos( hwndCtrl, NULL, 
				rc.left,
				nHeight - nHeightCheckBox - nHeightMargin,
				0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER );
			break;
// 2002/11/1 frozen ここから
		case 2:
			::SetWindowPos( hwndCtrl, NULL,
				rc.left,
				nHeight - nHeightCheckBox - nHeightButton - nHeightMargin * 2,
				0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER );
			break;
		case 3:
			::SetWindowPos( hwndCtrl, NULL, 0, 0, 
				nWidth - 2 * rc.left,
				nHeight - rc.top - nHeightCheckBox - nHeightButton - 3 * nHeightMargin,
				SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );
			break;
		}
// 2002/11/1 frozen ここまで
		::InvalidateRect( hwndCtrl, NULL, TRUE );
	}
	return TRUE;
}

int CALLBACK Compare_by_ItemData(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	if( lParam1< lParam2 )
		return -1;
	if( lParam1 > lParam2 )
		return 1;
	else
		return 0;
}

BOOL CDlgFuncList::OnDestroy( void )
{
	CDialog::OnDestroy();

	/* アウトライン ■位置とサイズを記憶する */ // 20060201 aroka
	// 前提条件：m_lParam が CDialog::OnDestroy でクリアされないこと
	CEditView* pcEditView=(CEditView*)m_lParam;
	if( m_pShareData->m_Common.m_bRememberOutlineWindowPos ){
		/* 親のウィンドウ位置・サイズを記憶 */
		WINDOWPLACEMENT cWindowPlacement;
		cWindowPlacement.length = sizeof( WINDOWPLACEMENT );
		if (::GetWindowPlacement( pcEditView->m_pcEditDoc->m_pcEditWnd->m_hWnd, &cWindowPlacement )){
			/* ウィンドウ位置・サイズを記憶 */
			m_pShareData->m_Common.m_xOutlineWindowPos = m_xPos - cWindowPlacement.rcNormalPosition.left;
			m_pShareData->m_Common.m_yOutlineWindowPos = m_yPos - cWindowPlacement.rcNormalPosition.top;
			m_pShareData->m_Common.m_widthOutlineWindow = m_nWidth;
			m_pShareData->m_Common.m_heightOutlineWindow = m_nHeight;
		}

	}
	return TRUE;
}


BOOL CDlgFuncList::OnCbnSelChange( HWND hwndCtl, int wID )
{
	int nSelect = ::SendMessage(hwndCtl,CB_GETCURSEL, 0, 0L);
	switch(wID)
	{
	case IDC_COMBO_nSortType:
		if( m_nSortType != nSelect )
		{
			m_nSortType = nSelect;
			SortTree(::GetDlgItem( m_hWnd , IDC_TREE1),TVI_ROOT);
		}
		return TRUE;
	};
	return FALSE;

}
void  CDlgFuncList::SortTree(HWND hWndTree,HTREEITEM htiParent)
{
	if( m_nSortType == 1 )
		TreeView_SortChildren(hWndTree,htiParent,TRUE);
	else
	{
		TVSORTCB sort;
		sort.hParent =  htiParent;
		sort.lpfnCompare = Compare_by_ItemData;
		sort.lParam = 0;
		TreeView_SortChildrenCB(hWndTree , &sort , TRUE);
	}
	for(HTREEITEM htiItem = TreeView_GetChild( hWndTree, htiParent ); NULL != htiItem ; htiItem = TreeView_GetNextSibling( hWndTree, htiItem ))
		SortTree(hWndTree,htiItem);
}



BOOL CDlgFuncList::OnJump( bool bCheckAutoClose )	//2002.02.08 hor 引数追加
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
			//	2006.07.09 genta 移動時に選択状態を保持するように
			::SendMessage( ::GetParent( ::GetParent( m_hwndParent ) ),
				MYWM_SETCARETPOS, 0, PM_SETCARETPOS_KEEPSELECT );
			if( bCheckAutoClose ){
				/* アウトライン ダイアログを自動的に閉じる */
				if( m_pShareData->m_Common.m_bAutoCloseDlgFuncList ){
					::DestroyWindow( m_hWnd );
				}else
				if( m_pShareData->m_Common.m_bFunclistSetFocusOnJump ){
					::SetFocus( m_hwndParent );
				}
			}
		}
	}
	return TRUE;
}


//@@@ 2002.01.18 add start
LPVOID CDlgFuncList::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end


/*!	キー操作をコマンドに変換するヘルパー関数
	
*/
void CDlgFuncList::Key2Command(WORD KeyCode)
{
	CEditView*	pcEditView;
	int nIdx, nFuncCode;
// novice 2004/10/10
	/* Shift,Ctrl,Altキーが押されていたか */
	nIdx = getCtrlKeyState();
	nFuncCode=CKeyBind::GetFuncCode(
			((WORD)(((BYTE)(KeyCode)) | ((WORD)((BYTE)(nIdx))) << 8)),
			m_pShareData->m_nKeyNameArrNum,
			m_pShareData->m_pKeyNameArr
	);
	switch( nFuncCode ){
	case F_REDRAW:
		nFuncCode=(m_nListType==OUTLINE_BOOKMARK)?F_BOOKMARK_VIEW:F_OUTLINE;
		/*FALLTHROUGH*/
	case F_OUTLINE:
	case F_OUTLINE_TOGGLE: // 20060201 aroka フォーカスがあるときはリロード
	case F_BOOKMARK_VIEW:
		pcEditView=(CEditView*)m_lParam;
		pcEditView->HandleCommand( nFuncCode, TRUE, SHOW_RELOAD, 0, 0, 0 ); // 引数の変更 20060201 aroka

		// 2002.11.11 Moca CEditView::HandleCommand→Readrawと回ってくるため更新しなくてよい
//		m_nListType=(nFuncCode==F_BOOKMARK_VIEW)?OUTLINE_BOOKMARK:pcEditView->m_pcEditDoc->GetDocumentAttribute().m_nDefaultOutline;
//		m_nCurLine=pcEditView->m_nCaretPosY + 1;
//		SetData(); 
		break;
	case F_BOOKMARK_SET:
		OnJump( false );
		pcEditView=(CEditView*)m_lParam;
		pcEditView->HandleCommand( nFuncCode, TRUE, 0, 0, 0, 0 );
	//	if( m_pShareData->m_Common.m_bAutoCloseDlgFuncList ){
	//		OnBnClicked( IDCANCEL );
	//	}else
	//	if( m_pShareData->m_Common.m_bFunclistSetFocusOnJump ){
	//		::SetFocus( m_hwndParent );
	//	}
		break;
	case F_COPY:
	case F_CUT:
		OnBnClicked( IDC_BUTTON_COPY );
		break;
	}
}

/*!
	@date 2002.10.05 genta
*/
void CDlgFuncList::Redraw( int nOutLineType, CFuncInfoArr* pcFuncInfoArr, int nCurLine )
{
	m_nListType = nOutLineType;
	m_pcFuncInfoArr = pcFuncInfoArr;	/* 関数情報配列 */
	m_nCurLine = nCurLine;				/* 現在行 */
	SetData();
}

/*[EOF]*/
