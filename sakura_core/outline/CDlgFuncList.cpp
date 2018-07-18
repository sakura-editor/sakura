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
	Copyright (C) 2006, genta, aroka, ryoji, Moca
	Copyright (C) 2006, genta, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2010, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <limits.h>
#include "outline/CDlgFuncList.h"
#include "outline/CFuncInfo.h"
#include "outline/CFuncInfoArr.h"// 2002/2/3 aroka
#include "outline/CDlgFileTree.h"
#include "window/CEditWnd.h"	//	2006/2/11 aroka 追加
#include "doc/CEditDoc.h"
#include "uiparts/CGraphics.h"
#include "util/shell.h"
#include "util/os.h"
#include "util/input.h"
#include "util/window.h"
#include "env/CAppNodeManager.h"
#include "env/CDocTypeManager.h"
#include "env/CFileNameManager.h"
#include "env/CShareData.h"
#include "env/CShareData_IO.h"
#include "extmodule/CUxTheme.h"
#include "CGrepEnumKeys.h"
#include "CGrepEnumFilterFiles.h"
#include "CGrepEnumFilterFolders.h"
#include "CDataProfile.h"
#include "dlg/CDlgTagJumpList.h"
#include "typeprop/CImpExpManager.h"
#include "sakura_rc.h"
#include "sakura.hh"

// 画面ドッキング用の定義	// 2010.06.05 ryoji
#define DEFINE_SYNCCOLOR
#define DOCK_SPLITTER_WIDTH		DpiScaleX(5)
#define DOCK_MIN_SIZE			DpiScaleX(60)
#define DOCK_BUTTON_NUM			(3)

// ビューの種別
#define VIEWTYPE_LIST	0
#define VIEWTYPE_TREE	1

//アウトライン解析 CDlgFuncList.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//12200
	IDC_BUTTON_COPY,					HIDC_FL_BUTTON_COPY,	//コピー
	IDOK,								HIDOK_FL,				//ジャンプ
	IDCANCEL,							HIDCANCEL_FL,			//キャンセル
	IDC_BUTTON_HELP,					HIDC_FL_BUTTON_HELP,	//ヘルプ
	IDC_CHECK_bAutoCloseDlgFuncList,	HIDC_FL_CHECK_bAutoCloseDlgFuncList,	//自動的に閉じる
	IDC_LIST_FL,						HIDC_FL_LIST1,			//トピックリスト	IDC_LIST1->IDC_LIST_FL	2008/7/3 Uchi
	IDC_TREE_FL,						HIDC_FL_TREE1,			//トピックツリー	IDC_TREE1->IDC_TREE_FL	2008/7/3 Uchi
	IDC_CHECK_bFunclistSetFocusOnJump,	HIDC_FL_CHECK_bFunclistSetFocusOnJump,	//ジャンプでフォーカス移動する
	IDC_CHECK_bMarkUpBlankLineEnable,	HIDC_FL_CHECK_bMarkUpBlankLineEnable,	//空行を無視する
	IDC_COMBO_nSortType,				HIDC_COMBO_nSortType,	//順序
	IDC_BUTTON_WINSIZE,					HIDC_FL_BUTTON_WINSIZE,	//ウィンドウ位置保存	// 2006.08.06 ryoji
	IDC_BUTTON_MENU,					HIDC_FL_BUTTON_MENU,	//ウィンドウの位置メニュー
	IDC_BUTTON_SETTING,					HIDC_FL_BUTTON_SETTING,	//設定
//	IDC_STATIC,							-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

static const SAnchorList anchorList[] = {
	{IDC_BUTTON_COPY, ANCHOR_BOTTOM},
	{IDOK, ANCHOR_BOTTOM},
	{IDCANCEL, ANCHOR_BOTTOM},
	{IDC_BUTTON_HELP, ANCHOR_BOTTOM},
	{IDC_CHECK_bAutoCloseDlgFuncList, ANCHOR_BOTTOM},
	{IDC_LIST_FL, ANCHOR_ALL},
	{IDC_TREE_FL, ANCHOR_ALL},
	{IDC_CHECK_bFunclistSetFocusOnJump, ANCHOR_BOTTOM},
	{IDC_CHECK_bMarkUpBlankLineEnable , ANCHOR_BOTTOM},
	{IDC_COMBO_nSortType, ANCHOR_TOP},
	{IDC_BUTTON_WINSIZE, ANCHOR_BOTTOM}, // 20060201 aroka
	{IDC_BUTTON_MENU, ANCHOR_BOTTOM},
};

//関数リストの列
enum EFuncListCol {
	FL_COL_ROW		= 0,	//行
	FL_COL_COL		= 1,	//桁
	FL_COL_NAME		= 2,	//関数名
	FL_COL_REMARK	= 3		//備考
};

/*! ソート比較用プロシージャ */
int CALLBACK CDlgFuncList::CompareFunc_Asc( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort )
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
	if( FL_COL_NAME == pcDlgFuncList->m_nSortCol){	/* 名前でソート */
		return auto_stricmp( pcFuncInfo1->m_cmemFuncName.GetStringPtr(), pcFuncInfo2->m_cmemFuncName.GetStringPtr() );
	}
	//	Apr. 23, 2005 genta 行番号を左端へ
	if( FL_COL_ROW == pcDlgFuncList->m_nSortCol){	/* 行（＋桁）でソート */
		if( pcFuncInfo1->m_nFuncLineCRLF < pcFuncInfo2->m_nFuncLineCRLF ){
			return -1;
		}else
		if( pcFuncInfo1->m_nFuncLineCRLF == pcFuncInfo2->m_nFuncLineCRLF ){
			if( pcFuncInfo1->m_nFuncColCRLF < pcFuncInfo2->m_nFuncColCRLF ){
				return -1;
			}else
			if( pcFuncInfo1->m_nFuncColCRLF == pcFuncInfo2->m_nFuncColCRLF ){
				return 0;
			}else{
				return 1;
			}
		}else{
			return 1;
		}
	}
	if( FL_COL_COL == pcDlgFuncList->m_nSortCol){	/* 桁でソート */
		if( pcFuncInfo1->m_nFuncColCRLF < pcFuncInfo2->m_nFuncColCRLF ){
			return -1;
		}else
		if( pcFuncInfo1->m_nFuncColCRLF == pcFuncInfo2->m_nFuncColCRLF ){
			return 0;
		}else{
			return 1;
		}
	}
	// From Here 2001.12.07 hor
	if( FL_COL_REMARK == pcDlgFuncList->m_nSortCol){	/* 備考でソート */
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

int CALLBACK CDlgFuncList::CompareFunc_Desc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	return -1 * CompareFunc_Asc(lParam1, lParam2, lParamSort);
}

EFunctionCode CDlgFuncList::GetFuncCodeRedraw(int outlineType)
{
	if( outlineType == OUTLINE_BOOKMARK ){
		return F_BOOKMARK_VIEW;
	}else if( outlineType == OUTLINE_FILETREE ){
		return F_FILETREE;
	}
	return F_OUTLINE;
}

static EOutlineType GetOutlineTypeRedraw(int outlineType)
{
	if( outlineType == OUTLINE_BOOKMARK ){
		return OUTLINE_BOOKMARK;
	}else if( outlineType == OUTLINE_FILETREE ){
		return OUTLINE_FILETREE;
	}
	return OUTLINE_DEFAULT;
}

LPDLGTEMPLATE CDlgFuncList::m_pDlgTemplate = NULL;
DWORD CDlgFuncList::m_dwDlgTmpSize = 0;
HINSTANCE CDlgFuncList::m_lastRcInstance = 0;

CDlgFuncList::CDlgFuncList() : CDialog(true)
{
	/* サイズ変更時に位置を制御するコントロール数 */
	assert( _countof(anchorList) == _countof(m_rcItems) );

	m_pcFuncInfoArr = NULL;		/* 関数情報配列 */
	m_nCurLine = CLayoutInt(0);				/* 現在行 */
	m_nOutlineType = OUTLINE_DEFAULT;
	m_nListType = OUTLINE_DEFAULT;
	//	Apr. 23, 2005 genta 行番号を左端へ
	m_nSortCol = 0;				/* ソートする列番号 2004.04.06 zenryaku 標準は行番号(1列目) */
	m_nSortColOld = -1;
	m_bLineNumIsCRLF = false;	/* 行番号の表示 false=折り返し単位／true=改行単位 */
	m_bWaitTreeProcess = false;	// 2002.02.16 hor Treeのダブルクリックでフォーカス移動できるように 2/4
	m_nSortType = SORTTYPE_DEFAULT;
	m_cFuncInfo = NULL;			/* 現在の関数情報 */
	m_bEditWndReady = false;	/* エディタ画面の準備完了 */
	m_bInChangeLayout = false;
	m_pszTimerJumpFile = NULL;
	m_ptDefaultSize.x = -1;
	m_ptDefaultSize.y = -1;
	m_bDummyLParamMode = false;
}


/*!
	標準以外のメッセージを捕捉する

	@date 2007.11.07 ryoji 新規
*/
INT_PTR CDlgFuncList::DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
	INT_PTR result;
	result = CDialog::DispatchEvent( hWnd, wMsg, wParam, lParam );

	switch( wMsg ){
	case WM_ACTIVATEAPP:
		if( IsDocking() )
			break;

		// 自分が最初にアクティブ化された場合は一旦編集ウィンドウをアクティブ化して戻す
		//
		// Note. このダイアログは他とは異なるウィンドウスタイルのため閉じたときの挙動が異なる．
		// 他はスレッド内最近アクティブなウィンドウがアクティブになるが，このダイアログでは
		// セッション内全体での最近アクティブウィンドウがアクティブになってしまう．
		// それでは都合が悪いので，特別に以下の処理を行って他と同様な挙動が得られるようにする．
		if( (BOOL)wParam ){
			CEditView* pcEditView = (CEditView*)m_lParam;
			CEditWnd* pcEditWnd = pcEditView->m_pcEditWnd;
			if( ::GetActiveWindow() == GetHwnd() ){
				::SetActiveWindow( pcEditWnd->GetHwnd() );
				BlockingHook( NULL );	// キュー内に溜まっているメッセージを処理
				::SetActiveWindow( GetHwnd() );
				return 0L;
			}
		}
		break;

	case WM_NCPAINT:
		return OnNcPaint( hWnd, wMsg, wParam, lParam );
	case WM_NCCALCSIZE:
		return OnNcCalcSize( hWnd, wMsg, wParam, lParam );
	case WM_NCHITTEST:
		return OnNcHitTest( hWnd, wMsg, wParam, lParam );
	case WM_NCMOUSEMOVE:
		return OnNcMouseMove( hWnd, wMsg, wParam, lParam );
	case WM_MOUSEMOVE:
		return OnMouseMove( hWnd, wMsg, wParam, lParam );
	case WM_NCLBUTTONDOWN:
		return OnNcLButtonDown( hWnd, wMsg, wParam, lParam );
	case WM_LBUTTONUP:
		return OnLButtonUp( hWnd, wMsg, wParam, lParam );
	case WM_NCRBUTTONUP:
		if( IsDocking() && wParam == HTCAPTION ){
			// ドッキングのときはコンテキストメニューを明示的に呼び出す必要があるらしい
			::SendMessage( GetHwnd(), WM_CONTEXTMENU, (WPARAM)GetHwnd(), lParam );
			return 1L;
		}
		break;
	case WM_TIMER:
		return OnTimer( hWnd, wMsg, wParam, lParam );
	case WM_GETMINMAXINFO:
		return OnMinMaxInfo( lParam );
	case WM_SETTEXT:
		if( IsDocking() ){
			// キャプションを再描画する
			// ※ この時点ではまだテキスト設定されていないので RDW_UPDATENOW では NG
			::RedrawWindow( hWnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_NOINTERNALPAINT );
		}
		break;
	case WM_MOUSEACTIVATE:
		if( IsDocking() ){
			// 分割バー以外の場所ならフォーカス移動
			if( !(HTLEFT <= LOWORD(lParam) && LOWORD(lParam) <= HTBOTTOMRIGHT) ){
				::SetFocus( GetHwnd() );
			}
		}
		break;
	case WM_COMMAND:
		if( IsDocking() ){
			// コンボボックスのフォーカスが変化したらキャプションを再描画する（アクティブ／非アクティブ切替）
			if( LOWORD(wParam) == IDC_COMBO_nSortType ){
				if( HIWORD(wParam) == CBN_SETFOCUS || HIWORD(wParam) == CBN_KILLFOCUS ){
					::RedrawWindow( hWnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOINTERNALPAINT );
				}
			}
		}
		break;
	case WM_NOTIFY:
		if( IsDocking() ){
			// ツリーやリストのフォーカスが変化したらキャプションを再描画する（アクティブ／非アクティブ切替）
			NMHDR* pNMHDR = (NMHDR*)lParam;
			if( pNMHDR->code == NM_SETFOCUS || pNMHDR->code == NM_KILLFOCUS ){
				::RedrawWindow( hWnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOINTERNALPAINT );
			}
		}
		{
			NMHDR* pNMHDR = (NMHDR*)lParam;
			if( pNMHDR->code == TVN_ITEMEXPANDING ){
				NMTREEVIEW* pNMTREEVIEW = (NMTREEVIEW*)lParam;
				TVITEM* pItem = &(pNMTREEVIEW->itemNew);
				if( m_nListType == OUTLINE_FILETREE ){
					SetTreeFileSub( pItem->hItem, NULL );
				}
			}
		}
		break;
	}

	return result;
}


/* モードレスダイアログの表示 */
/*
 * @note 2011.06.25 syat nOutlineTypeを追加
 *   nOutlineTypeとnListTypeはほとんどの場合同じ値だが、プラグインの場合は例外で、
 *   nOutlineTypeはアウトライン解析のID、nListTypeはプラグイン内で指定するリスト形式となる。
 */
HWND CDlgFuncList::DoModeless(
	HINSTANCE		hInstance,
	HWND			hwndParent,
	LPARAM			lParam,
	CFuncInfoArr*	pcFuncInfoArr,
	CLayoutInt		nCurLine,
	CLayoutInt		nCurCol,
	int				nOutlineType,		
	int				nListType,
	bool			bLineNumIsCRLF		/* 行番号の表示 false=折り返し単位／true=改行単位 */
)
{
	CEditView* pcEditView=(CEditView*)lParam;
	if( !pcEditView ) return NULL;
	m_pcFuncInfoArr = pcFuncInfoArr;	/* 関数情報配列 */
	m_nCurLine = nCurLine;				/* 現在行 */
	m_nCurCol = nCurCol;				/* 現在桁 */
	m_nOutlineType = nOutlineType;		/* アウトライン解析の種別 */
	m_nListType = nListType;			/* 一覧の種類 */
	m_bLineNumIsCRLF = bLineNumIsCRLF;	/* 行番号の表示 false=折り返し単位／true=改行単位 */
	m_nDocType = pcEditView->GetDocument()->m_cDocType.GetDocumentType().GetIndex();
	CDocTypeManager().GetTypeConfig(CTypeConfig(m_nDocType), m_type);
	m_nSortCol = m_type.m_nOutlineSortCol;
	m_nSortColOld = m_nSortCol;
	m_bSortDesc = m_type.m_bOutlineSortDesc;
	m_nSortType = m_type.m_nOutlineSortType;

	bool bType = (ProfDockSet() != 0);
	if( bType ){
		m_type.m_nDockOutline = m_nOutlineType;
		SetTypeConfig( CTypeConfig(m_nDocType), m_type );
	}else{
		CommonSet().m_nDockOutline = m_nOutlineType;
	}

	// 2007.04.18 genta : 「フォーカスを移す」と「自動的に閉じる」がチェックされている場合に
	// ダブルクリックを行うと，trueのまま残ってしまうので，ウィンドウを開いたときにリセットする．
	m_bWaitTreeProcess = false;

	m_eDockSide = ProfDockSide();
	HWND hwndRet;
	if( IsDocking() ){
		// ドッキング用にダイアログテンプレートに手を加えてから表示する（WS_CHILD化）
		HINSTANCE hInstance2 = CSelectLang::getLangRsrcInstance();
		if( !m_pDlgTemplate || m_lastRcInstance != hInstance2 ){
			HRSRC hResInfo = ::FindResource( hInstance2, MAKEINTRESOURCE(IDD_FUNCLIST), RT_DIALOG );
			if( !hResInfo ) return NULL;
			HGLOBAL hResData = ::LoadResource( hInstance2, hResInfo );
			if( !hResData ) return NULL;
			m_pDlgTemplate = (LPDLGTEMPLATE)::LockResource( hResData );
			if( !m_pDlgTemplate ) return NULL;
			m_dwDlgTmpSize = ::SizeofResource( hInstance2, hResInfo );
			// 言語切り替えでリソースがアンロードされていないか確認するためインスタンスを記憶する
			m_lastRcInstance = hInstance2;
		}
		LPDLGTEMPLATE pDlgTemplate = (LPDLGTEMPLATE)::GlobalAlloc( GMEM_FIXED, m_dwDlgTmpSize );
		if( !pDlgTemplate ) return NULL;
		::CopyMemory( pDlgTemplate, m_pDlgTemplate, m_dwDlgTmpSize );
		pDlgTemplate->style = (WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | DS_SETFONT);
		hwndRet = CDialog::DoModeless( hInstance, MyGetAncestor(hwndParent, GA_ROOT), pDlgTemplate, lParam, SW_HIDE );
		::GlobalFree( pDlgTemplate );
		pcEditView->m_pcEditWnd->EndLayoutBars( m_bEditWndReady );	// 画面の再レイアウト
	}else{
		hwndRet = CDialog::DoModeless( hInstance, MyGetAncestor(hwndParent, GA_ROOT), IDD_FUNCLIST, lParam, SW_SHOW );
	}
	return hwndRet;
}

/* モードレス時：検索対象となるビューの変更 */
void CDlgFuncList::ChangeView( LPARAM pcEditView )
{
	m_lParam = pcEditView;
	return;
}

/*! ダイアログデータの設定 */
void CDlgFuncList::SetData()
{
	HWND			hwndList;
	HWND			hwndTree;
	hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_FL );
	hwndTree = ::GetDlgItem( GetHwnd(), IDC_TREE_FL );

	m_bDummyLParamMode = false;
	m_vecDummylParams.clear();

	//2002.02.08 hor 隠しといてアイテム削除→あとで表示
	::ShowWindow( hwndList, SW_HIDE );
	::ShowWindow( hwndTree, SW_HIDE );
	ListView_DeleteAllItems( hwndList );
	TreeView_DeleteAllItems( hwndTree );
	::ShowWindow( GetItemHwnd(IDC_BUTTON_SETTING), SW_HIDE );


	SetDocLineFuncList();
	if( OUTLINE_C_CPP == m_nListType || OUTLINE_CPP == m_nListType ){	/* C++メソッドリスト */
		m_nViewType = VIEWTYPE_TREE;
		SetTreeJava( GetHwnd(), TRUE );	// Jan. 04, 2002 genta Java Method Treeに統合
		::SetWindowText( GetHwnd(), LS(STR_DLGFNCLST_TITLE_CPP) );
	}
	else if( OUTLINE_FILE == m_nListType ){	//@@@ 2002.04.01 YAZAKI アウトライン解析にルールファイル導入
		m_nViewType = VIEWTYPE_TREE;
		SetTree();
		::SetWindowText( GetHwnd(), LS(STR_DLGFNCLST_TITLE_RULE) );
	}
	else if( OUTLINE_WZTXT == m_nListType ){ //@@@ 2003.05.20 zenryaku 階層付テキストアウトライン解析
		m_nViewType = VIEWTYPE_TREE;
		SetTree();
		::SetWindowText( GetHwnd(), LS(STR_DLGFNCLST_TITLE_WZ) ); //	2003.06.22 Moca 名前変更
	}
	else if( OUTLINE_HTML == m_nListType ){ //@@@ 2003.05.20 zenryaku HTMLアウトライン解析
		m_nViewType = VIEWTYPE_TREE;
		SetTree();
		::SetWindowText( GetHwnd(), _T("HTML") );
	}
	else if( OUTLINE_TEX == m_nListType ){ //@@@ 2003.07.20 naoh TeXアウトライン解析
		m_nViewType = VIEWTYPE_TREE;
		SetTree();
		::SetWindowText( GetHwnd(), _T("TeX") );
	}
	else if( OUTLINE_TEXT == m_nListType ){ /* テキスト・トピックリスト */
		m_nViewType = VIEWTYPE_TREE;
		SetTree();	//@@@ 2002.04.01 YAZAKI テキストトピックツリーも、汎用SetTreeを呼ぶように変更。
		::SetWindowText( GetHwnd(), LS(STR_DLGFNCLST_TITLE_TEXT) );
	}
	else if( OUTLINE_JAVA == m_nListType ){ /* Javaメソッドツリー */
		m_nViewType = VIEWTYPE_TREE;
		SetTreeJava( GetHwnd(), TRUE );
		::SetWindowText( GetHwnd(), LS(STR_DLGFNCLST_TITLE_JAVA) );
	}
	//	2007.02.08 genta Python追加
	else if( OUTLINE_PYTHON == m_nListType ){ /* Python メソッドツリー */
		m_nViewType = VIEWTYPE_TREE;
		SetTree( true );
		::SetWindowText( GetHwnd(), LS(STR_DLGFNCLST_TITLE_PYTHON) );
	}
	else if( OUTLINE_COBOL == m_nListType ){ /* COBOL アウトライン */
		m_nViewType = VIEWTYPE_TREE;
		SetTreeJava( GetHwnd(), FALSE );
		::SetWindowText( GetHwnd(), LS(STR_DLGFNCLST_TITLE_COBOL) );
	}
	else if( OUTLINE_VB == m_nListType ){	/* VisualBasic アウトライン */
		m_nViewType = VIEWTYPE_LIST;
		SetListVB();
		::SetWindowText( GetHwnd(), LS(STR_DLGFNCLST_TITLE_VB) );
	}
	else if( OUTLINE_XML == m_nListType ){ // XMLツリー
		m_nViewType = VIEWTYPE_TREE;
		SetTree();
		::SetWindowText( GetHwnd(), _T("XML") );
	}
	else if ( OUTLINE_FILETREE == m_nListType ){
		m_nViewType = VIEWTYPE_TREE;
		SetTreeFile();
		::SetWindowText( GetHwnd(), LS(F_FILETREE) );	// ファイルツリー
	}
	else if( OUTLINE_TREE == m_nListType ){ /* 汎用ツリー */
		m_nViewType = VIEWTYPE_TREE;
		SetTree();
		::SetWindowText( GetHwnd(), _T("") );
	}
	else if( OUTLINE_TREE_TAGJUMP == m_nListType ){ /* 汎用ツリー(タグジャンプ付き) */
		m_nViewType = VIEWTYPE_TREE;
		SetTree( true );
		::SetWindowText( GetHwnd(), _T("") );
	}
	else if( OUTLINE_CLSTREE == m_nListType ){ /* 汎用クラスツリー */
		m_nViewType = VIEWTYPE_TREE;
		SetTreeJava( GetHwnd(), TRUE );
		::SetWindowText( GetHwnd(), _T("") );
	}
	else{
		m_nViewType = VIEWTYPE_LIST;
		switch( m_nListType ){
		case OUTLINE_C:
			::SetWindowText( GetHwnd(), LS(STR_DLGFNCLST_TITLE_C) );
			break;
		case OUTLINE_PLSQL:
			::SetWindowText( GetHwnd(), LS(STR_DLGFNCLST_TITLE_PLSQL) );
			break;
		case OUTLINE_ASM:
			::SetWindowText( GetHwnd(), LS(STR_DLGFNCLST_TITLE_ASM) );
			break;
		case OUTLINE_PERL:	//	Sep. 8, 2000 genta
			::SetWindowText( GetHwnd(), LS(STR_DLGFNCLST_TITLE_PERL) );
			break;
// Jul 10, 2003  little YOSHI  上に移動しました--->>
//		case OUTLINE_VB:	// 2001/06/23 N.Nakatani for Visual Basic
//			::SetWindowText( GetHwnd(), "Visual Basic アウトライン" );
//			break;
// <<---ここまで
		case OUTLINE_ERLANG:	//	2009.08.10 genta
			::SetWindowText( GetHwnd(), LS(STR_DLGFNCLST_TITLE_ERLANG) );
			break;
		case OUTLINE_BOOKMARK:
			LV_COLUMN col;
			col.mask = LVCF_TEXT;
			col.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_LIST_TEXT));
			col.iSubItem = 0;
			//	Apr. 23, 2005 genta 行番号を左端へ
			ListView_SetColumn( hwndList, FL_COL_NAME, &col );
			::SetWindowText( GetHwnd(), LS(STR_DLGFNCLST_TITLE_BOOK) );
			break;
		case OUTLINE_LIST:	// 汎用リスト 2010.03.28 syat
			::SetWindowText( GetHwnd(), _T("") );
			break;
		}
		//	May 18, 2001 genta
		//	Windowがいなくなると後で都合が悪いので、表示しないだけにしておく
		//::DestroyWindow( hwndTree );
//		::ShowWindow( hwndTree, SW_HIDE );
		int				i;
		TCHAR			szText[2048];
		const CFuncInfo*	pcFuncInfo;
		LV_ITEM			item;
		bool			bSelected;
		CLayoutInt		nFuncLineOld(-1);
		CLayoutInt		nFuncColOld(-1);
		CLayoutInt		nFuncLineTop(INT_MAX);
		CLayoutInt		nFuncColTop(INT_MAX);
		int				nSelectedLineTop = 0;
		int				nSelectedLine = 0;
		RECT			rc;

		m_cmemClipText.SetString(L"");	/* クリップボードコピー用テキスト */
		{
			const int nBuffLenTag = 13 + wcslen(to_wchar(m_pcFuncInfoArr->m_szFilePath));
			const int nNum = m_pcFuncInfoArr->GetNum();
			int nBuffLen = 0;
			for(int i = 0; i < nNum; ++i ){
				const CFuncInfo* pcFuncInfo = m_pcFuncInfoArr->GetAt(i);
				nBuffLen += pcFuncInfo->m_cmemFuncName.GetStringLength();
			}
			m_cmemClipText.AllocStringBuffer( nBuffLen + nBuffLenTag * nNum );
		}

		::EnableWindow( ::GetDlgItem( GetHwnd() , IDC_BUTTON_COPY ), TRUE );
		bSelected = false;
		for( i = 0; i < m_pcFuncInfoArr->GetNum(); ++i ){
			pcFuncInfo = m_pcFuncInfoArr->GetAt( i );
			if( !bSelected ){
				if( pcFuncInfo->m_nFuncLineLAYOUT < nFuncLineTop
					|| (pcFuncInfo->m_nFuncLineLAYOUT == nFuncLineTop && pcFuncInfo->m_nFuncColLAYOUT <= nFuncColTop) ){
					nFuncLineTop = pcFuncInfo->m_nFuncLineLAYOUT;
					nFuncColTop = pcFuncInfo->m_nFuncColLAYOUT;
					nSelectedLineTop = i;
				}
			}
			{
				if( (nFuncLineOld	 < pcFuncInfo->m_nFuncLineLAYOUT
					|| (nFuncLineOld == pcFuncInfo->m_nFuncColLAYOUT && nFuncColOld <= pcFuncInfo->m_nFuncColLAYOUT))
				  && (pcFuncInfo->m_nFuncLineLAYOUT < m_nCurLine
					|| (pcFuncInfo->m_nFuncLineLAYOUT == m_nCurLine && pcFuncInfo->m_nFuncColLAYOUT <= m_nCurCol)) ){
					nFuncLineOld = pcFuncInfo->m_nFuncLineLAYOUT;
					nFuncColOld = pcFuncInfo->m_nFuncColLAYOUT;
					bSelected = true;
					nSelectedLine = i;
				}
			}
		}
		if( 0 < m_pcFuncInfoArr->GetNum() && !bSelected ){
			bSelected = true;
			nSelectedLine =  nSelectedLineTop;
		}
		for( i = 0; i < m_pcFuncInfoArr->GetNum(); ++i ){
			/* 現在の解析結果要素 */
			pcFuncInfo = m_pcFuncInfoArr->GetAt( i );

			//	From Here Apr. 23, 2005 genta 行番号を左端へ
			/* 行番号の表示 false=折り返し単位／true=改行単位 */
			if(m_bLineNumIsCRLF ){
				auto_sprintf( szText, _T("%d"), pcFuncInfo->m_nFuncLineCRLF );
			}else{
				auto_sprintf( szText, _T("%d"), pcFuncInfo->m_nFuncLineLAYOUT );
			}
			item.mask = LVIF_TEXT | LVIF_PARAM;
			item.pszText = szText;
			item.iItem = i;
			item.lParam	= i;
			item.iSubItem = FL_COL_ROW;
			ListView_InsertItem( hwndList, &item);

			// 2010.03.17 syat 桁追加
			/* 行番号の表示 false=折り返し単位／true=改行単位 */
			if(m_bLineNumIsCRLF ){
				auto_sprintf( szText, _T("%d"), pcFuncInfo->m_nFuncColCRLF );
			}else{
				auto_sprintf( szText, _T("%d"), pcFuncInfo->m_nFuncColLAYOUT );
			}
			item.mask = LVIF_TEXT;
			item.pszText = szText;
			item.iItem = i;
			item.iSubItem = FL_COL_COL;
			ListView_SetItem( hwndList, &item);

			item.mask = LVIF_TEXT;
			item.pszText = const_cast<TCHAR*>(pcFuncInfo->m_cmemFuncName.GetStringPtr());
			item.iItem = i;
			item.iSubItem = FL_COL_NAME;
			ListView_SetItem( hwndList, &item);
			//	To Here Apr. 23, 2005 genta 行番号を左端へ

			item.mask = LVIF_TEXT;
			if(  1 == pcFuncInfo->m_nInfo ){item.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_REMARK01));}else
			if( 10 == pcFuncInfo->m_nInfo ){item.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_REMARK02));}else
			if( 20 == pcFuncInfo->m_nInfo ){item.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_REMARK03));}else
			if( 11 == pcFuncInfo->m_nInfo ){item.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_REMARK04));}else
			if( 21 == pcFuncInfo->m_nInfo ){item.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_REMARK05));}else
			if( 31 == pcFuncInfo->m_nInfo ){item.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_REMARK06));}else
			if( 41 == pcFuncInfo->m_nInfo ){item.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_REMARK07));}else
			if( 50 == pcFuncInfo->m_nInfo ){item.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_REMARK08));}else
			if( 51 == pcFuncInfo->m_nInfo ){item.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_REMARK09));}else
			if( 52 == pcFuncInfo->m_nInfo ){item.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_REMARK10));}else{
				// Jul 10, 2003  little YOSHI
				// ここにあったVB関係の処理はSetListVB()メソッドに移動しました。

				item.pszText = const_cast<TCHAR*>(_T(""));
			}
			item.iItem = i;
			item.iSubItem = FL_COL_REMARK;
			ListView_SetItem( hwndList, &item);

			/* クリップボードにコピーするテキストを編集 */
			if(item.pszText[0] != _T('\0')){
				// 検出結果の種類(関数,,,)があるとき
				auto_sprintf(
					szText,
					_T("%ts(%d,%d): "),
					m_pcFuncInfoArr->m_szFilePath.c_str(),		/* 解析対象ファイル名 */
					pcFuncInfo->m_nFuncLineCRLF,		/* 検出行番号 */
					pcFuncInfo->m_nFuncColCRLF		/* 検出桁番号 */
				);
				m_cmemClipText.AppendStringT(szText);
				// "%ts(%ts)\r\n"
				m_cmemClipText.AppendNativeDataT(pcFuncInfo->m_cmemFuncName);
				m_cmemClipText.AppendString(L"(");
				m_cmemClipText.AppendStringT(item.pszText);
				m_cmemClipText.AppendString(L")\r\n");
			}else{
				// 検出結果の種類(関数,,,)がないとき
				auto_sprintf(
					szText,
					_T("%ts(%d,%d): "),
					m_pcFuncInfoArr->m_szFilePath.c_str(),		/* 解析対象ファイル名 */
					pcFuncInfo->m_nFuncLineCRLF,		/* 検出行番号 */
					pcFuncInfo->m_nFuncColCRLF		/* 検出桁番号 */
				);
				m_cmemClipText.AppendStringT(szText);
				m_cmemClipText.AppendNativeDataT(pcFuncInfo->m_cmemFuncName);
				m_cmemClipText.AppendString(L"\r\n");
			}
		}
		//2002.02.08 hor Listは列幅調整とかを実行する前に表示しとかないと変になる
		::ShowWindow( hwndList, SW_SHOW );
		/* 列の幅をデータに合わせて調整 */
		ListView_SetColumnWidth( hwndList, FL_COL_ROW, LVSCW_AUTOSIZE );
		ListView_SetColumnWidth( hwndList, FL_COL_COL, LVSCW_AUTOSIZE );
		ListView_SetColumnWidth( hwndList, FL_COL_NAME, LVSCW_AUTOSIZE );
		ListView_SetColumnWidth( hwndList, FL_COL_REMARK, LVSCW_AUTOSIZE );
		ListView_SetColumnWidth( hwndList, FL_COL_ROW, ListView_GetColumnWidth( hwndList, FL_COL_ROW ) + 16 );
		ListView_SetColumnWidth( hwndList, FL_COL_COL, ListView_GetColumnWidth( hwndList, FL_COL_COL ) + 16 );
		ListView_SetColumnWidth( hwndList, FL_COL_NAME, ListView_GetColumnWidth( hwndList, FL_COL_NAME ) + 16 );
		ListView_SetColumnWidth( hwndList, FL_COL_REMARK, ListView_GetColumnWidth( hwndList, FL_COL_REMARK ) + 16 );

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
	::CheckDlgButton( GetHwnd(), IDC_CHECK_bAutoCloseDlgFuncList, m_pShareData->m_Common.m_sOutline.m_bAutoCloseDlgFuncList );
	/* アウトライン ブックマーク一覧で空行を無視する */
	::CheckDlgButton( GetHwnd(), IDC_CHECK_bMarkUpBlankLineEnable, m_pShareData->m_Common.m_sOutline.m_bMarkUpBlankLineEnable );
	/* アウトライン ジャンプしたらフォーカスを移す */
	::CheckDlgButton( GetHwnd(), IDC_CHECK_bFunclistSetFocusOnJump, m_pShareData->m_Common.m_sOutline.m_bFunclistSetFocusOnJump );

	/* アウトライン ■位置とサイズを記憶する */ // 20060201 aroka
	::CheckDlgButton( GetHwnd(), IDC_BUTTON_WINSIZE, m_pShareData->m_Common.m_sOutline.m_bRememberOutlineWindowPos );
	// ボタンが押されているかはっきりさせる 2008/6/5 Uchi
	::DlgItem_SetText( GetHwnd(), IDC_BUTTON_WINSIZE, 
		m_pShareData->m_Common.m_sOutline.m_bRememberOutlineWindowPos ? _T("■") : _T("□") );

	/* ダイアログを自動的に閉じるならフォーカス移動オプションは関係ない */
	if(m_pShareData->m_Common.m_sOutline.m_bAutoCloseDlgFuncList){
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHECK_bFunclistSetFocusOnJump ), FALSE );
	}else{
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHECK_bFunclistSetFocusOnJump ), TRUE );
	}

	//2002.02.08 hor
	//（IDC_LIST_FLもIDC_TREE_FLも常に存在していて、m_nViewTypeによって、どちらを表示するかを選んでいる）
	HWND hwndShow = (VIEWTYPE_LIST == m_nViewType)? hwndList: hwndTree;
	::ShowWindow( hwndShow, SW_SHOW );
	if( ::GetForegroundWindow() == MyGetAncestor( GetHwnd(), GA_ROOT ) && IsChild( GetHwnd(), GetFocus()) )
		::SetFocus( hwndShow );

	//2002.02.08 hor
	//空行をどう扱うかのチェックボックスはブックマーク一覧のときだけ表示する
	if(OUTLINE_BOOKMARK == m_nListType){
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHECK_bMarkUpBlankLineEnable ), TRUE );
		if( !IsDocking() ) ::ShowWindow( GetDlgItem( GetHwnd(), IDC_CHECK_bMarkUpBlankLineEnable ), SW_SHOW );
	}else{
		::ShowWindow( GetDlgItem( GetHwnd(), IDC_CHECK_bMarkUpBlankLineEnable ), SW_HIDE );
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHECK_bMarkUpBlankLineEnable ), FALSE );
	}
	// 2002/11/1 frozen 項目のソート基準を設定するコンボボックスはブックマーク一覧の以外の時に表示する
	// Nov. 5, 2002 genta ツリー表示の時だけソート基準コンボボックスを表示
	CEditView* pcEditView = (CEditView*)m_lParam;
	int nDocType = pcEditView->GetDocument()->m_cDocType.GetDocumentType().GetIndex();
	if( nDocType != m_nDocType ){
		// 以前とはドキュメントタイプが変わったので初期化する
		m_nDocType = nDocType;
		m_nSortCol = m_type.m_nOutlineSortCol;
		m_nSortColOld = m_nSortCol;
		m_bSortDesc = m_type.m_bOutlineSortDesc;
		m_nSortType = m_type.m_nOutlineSortType;
	}
	if( m_nViewType == VIEWTYPE_TREE && m_nListType != OUTLINE_FILETREE ){
		HWND hWnd_Combo_Sort = ::GetDlgItem( GetHwnd(), IDC_COMBO_nSortType );
		if( m_nListType == OUTLINE_FILETREE ){
			::EnableWindow( hWnd_Combo_Sort , FALSE );
		}else{
			::EnableWindow( hWnd_Combo_Sort , TRUE );
		}
		::ShowWindow( hWnd_Combo_Sort , SW_SHOW );
		Combo_ResetContent( hWnd_Combo_Sort ); // 2002.11.10 Moca 追加
		Combo_AddString( hWnd_Combo_Sort , LS(STR_DLGFNCLST_SORTTYPE1));	// SORTTYPE_DEFAULT
		Combo_AddString( hWnd_Combo_Sort , LS(STR_DLGFNCLST_SORTTYPE1_2));	// SORTTYPE_DEFAULT_DESC
		Combo_AddString( hWnd_Combo_Sort , LS(STR_DLGFNCLST_SORTTYPE2));    // SORTTYPE_ATOZ
		Combo_AddString( hWnd_Combo_Sort , LS(STR_DLGFNCLST_SORTTYPE2_2));  // SORTTYPE_ZTOA
		Combo_SetCurSel( hWnd_Combo_Sort , m_nSortType );
		::ShowWindow( GetDlgItem( GetHwnd(), IDC_STATIC_nSortType ), SW_SHOW );
		// 2002.11.10 Moca 追加 ソートする
		SortTree(::GetDlgItem( GetHwnd() , IDC_TREE_FL),TVI_ROOT);
	}else if( m_nListType == OUTLINE_FILETREE ){
		::ShowWindow( GetItemHwnd(IDC_COMBO_nSortType), SW_HIDE );
		::ShowWindow( GetItemHwnd(IDC_STATIC_nSortType), SW_HIDE );
		::ShowWindow( GetItemHwnd(IDC_BUTTON_SETTING), SW_SHOW );
	}else {
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_COMBO_nSortType ), FALSE );
		::ShowWindow( GetDlgItem( GetHwnd(), IDC_COMBO_nSortType ), SW_HIDE );
		::ShowWindow( GetDlgItem( GetHwnd(), IDC_STATIC_nSortType ), SW_HIDE );
		//ListView_SortItems( hwndList, CompareFunc_Asc, (LPARAM)this );  // 2005.04.05 zenryaku ソート状態を保持
		SortListView( hwndList, m_nSortCol );	// 2005.04.23 genta 関数化(ヘッダ書き換えのため)
	}
}




bool CDlgFuncList::GetTreeFileFullName(HWND hwndTree, HTREEITEM target, std::tstring* pPath, int* pnItem)
{
	*pPath = _T("");
	*pnItem = -1;
	do{
		TVITEM tvItem;
		TCHAR szFileName[_MAX_PATH];
		tvItem.mask = TVIF_HANDLE | TVIF_TEXT;
		tvItem.pszText = szFileName;
		tvItem.cchTextMax = _countof(szFileName);
		tvItem.hItem = target;
		TreeView_GetItem( hwndTree, &tvItem );
		if( ((-tvItem.lParam) % 10) == 3 ){
			*pnItem = (-tvItem.lParam) / 10;
			*pPath = std::tstring(m_pcFuncInfoArr->GetAt(*pnItem)->m_cmemFileName.GetStringPtr()) + _T("\\") + *pPath;
			return true;
		}
		if( tvItem.lParam != -1 && tvItem.lParam != -2 ){
			return false;
		}
		if( *pPath != _T("") ){
			*pPath = std::tstring(szFileName) + _T("\\") + *pPath;
		}else{
			*pPath = szFileName;
		}
		target = TreeView_GetParent( hwndTree, target );
	}while( target != NULL );
	return false;
}


/*! lParamからFuncInfoの番号を算出
	vecにはダミーのlParam番号が入っているのでずれている数を数える
*/
static int TreeDummylParamToFuncInfoIndex(std::vector<int>& vec, LPARAM lParam)
{
	// vec = { 3,6,7 }
	// lParam 0,1,2,3,4,5,6,7,8
	// return 0 1 2-1 3 4-1-1 5
	int nCount = (int)vec.size();
	int nDiff = 0;
	for( int i = 0; i < nCount; i++ ){
		if( vec[i] < lParam ){
			nDiff++;
		}else if( vec[i] == lParam ){
			return -1;
		}else{
			break;
		}
	}
	return lParam - nDiff;
}



/* ダイアログデータの取得 */
/* 0==条件未入力   0より大きい==正常   0より小さい==入力エラー */
int CDlgFuncList::GetData( void )
{
	HWND			hwndList;
	HWND			hwndTree;
	int				nItem;
	LV_ITEM			item;
	HTREEITEM		htiItem;
	TV_ITEM			tvi;

	m_cFuncInfo = NULL;
	m_sJumpFile = _T("");
	hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_FL );
	if( m_nViewType == VIEWTYPE_LIST ){
		//	List
		nItem = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
		if( -1 == nItem ){
			return -1;
		}
		item.mask = LVIF_PARAM;
		item.iItem = nItem;
		item.iSubItem = 0;
		ListView_GetItem( hwndList, &item );
		m_cFuncInfo = m_pcFuncInfoArr->GetAt( item.lParam );
	}else{
		hwndTree = ::GetDlgItem( GetHwnd(), IDC_TREE_FL );
		if( NULL != hwndTree ){
			htiItem = TreeView_GetSelection( hwndTree );

			tvi.mask = TVIF_HANDLE | TVIF_PARAM;
			tvi.hItem = htiItem;
			tvi.pszText = NULL;
			tvi.cchTextMax = 0;
			if( TreeView_GetItem( hwndTree, &tvi ) ){
				// lParamが-1以下は pcFuncInfoArrには含まれない項目
				if( 0 <= tvi.lParam ){
					int nIndex;
					if( m_bDummyLParamMode ){
						// ダミー要素を排除:SetTreeJava
						nIndex = TreeDummylParamToFuncInfoIndex(m_vecDummylParams, tvi.lParam);
					}else{
						nIndex = tvi.lParam;
					}
					if( 0 <= nIndex ){
						m_cFuncInfo = m_pcFuncInfoArr->GetAt(nIndex);
					}
				}else{
					if( m_nListType == OUTLINE_FILETREE ){
						if( tvi.lParam == -1 ){
							int nItem;
							if( !GetTreeFileFullName( hwndTree, htiItem, &m_sJumpFile, &nItem ) ){
								m_sJumpFile = _T(""); // error
							}
						}
					}
				}
			}
		}
	}
	return 1;
}

/* Java/C++メソッドツリーの最大ネスト深さ */
// 2016.03.06 vector化で16 -> 32 まで増やしておく
#define MAX_JAVA_TREE_NEST 32

/*! ツリーコントロールの初期化：Javaメソッドツリー

	Java Method Treeの構築: 関数リストを元にTreeControlを初期化する。

	@date 2002.01.04 genta C++ツリーを統合
*/
void CDlgFuncList::SetTreeJava( HWND hwndDlg, BOOL bAddClass )
{
	int				i;
	const CFuncInfo*	pcFuncInfo;
	HWND			hwndTree;
	int				bSelected;
	CLayoutInt		nFuncLineOld;
	CLayoutInt		nFuncColOld;
	CLayoutInt		nFuncLineTop(INT_MAX);
	CLayoutInt		nFuncColTop(INT_MAX);
	TV_INSERTSTRUCT	tvis;
	const TCHAR*	pPos;
	HTREEITEM		htiGlobal = NULL;	// Jan. 04, 2001 genta C++と統合
	HTREEITEM		htiClass;
	HTREEITEM		htiItem;
	HTREEITEM		htiSelectedTop = NULL;
	HTREEITEM		htiSelected = NULL;
	TV_ITEM			tvi;
	int				nClassNest;
	std::vector<std::tstring> vStrClasses;

	::EnableWindow( ::GetDlgItem( GetHwnd() , IDC_BUTTON_COPY ), TRUE );
	m_bDummyLParamMode = true;
	m_vecDummylParams.clear();
	int nlParamCount = 0;

	hwndTree = ::GetDlgItem( GetHwnd(), IDC_TREE_FL );

	m_cmemClipText.SetString( L"" );
	{
		const int nBuffLenTag = 13 + wcslen(to_wchar(m_pcFuncInfoArr->m_szFilePath));
		const int nNum = m_pcFuncInfoArr->GetNum();
		int nBuffLen = 0;
		for( int i = 0; i < nNum; i++ ){
			const CFuncInfo* pcFuncInfo = m_pcFuncInfoArr->GetAt(i);
			nBuffLen += pcFuncInfo->m_cmemFuncName.GetStringLength();
		}
		m_cmemClipText.AllocStringBuffer( nBuffLen + nBuffLenTag * nNum );
	}
	// 追加文字列の初期化（プラグインで指定済みの場合は上書きしない）
	m_pcFuncInfoArr->SetAppendText( FL_OBJ_DECLARE,		LSW(STR_DLGFNCLST_APND_DECLARE),	false );
	m_pcFuncInfoArr->SetAppendText( FL_OBJ_CLASS,		LSW(STR_DLGFNCLST_APND_CLASS),		false );
	m_pcFuncInfoArr->SetAppendText( FL_OBJ_STRUCT,		LSW(STR_DLGFNCLST_APND_STRUCT),		false );
	m_pcFuncInfoArr->SetAppendText( FL_OBJ_ENUM,		LSW(STR_DLGFNCLST_APND_ENUM),		false );
	m_pcFuncInfoArr->SetAppendText( FL_OBJ_UNION,		LSW(STR_DLGFNCLST_APND_UNION),		false );
	m_pcFuncInfoArr->SetAppendText( FL_OBJ_NAMESPACE,	LSW(STR_DLGFNCLST_APND_NAMESPACE),	false );
	m_pcFuncInfoArr->SetAppendText( FL_OBJ_INTERFACE,	LSW(STR_DLGFNCLST_APND_INTERFACE),	false );
	m_pcFuncInfoArr->SetAppendText( FL_OBJ_GLOBAL,		LSW(STR_DLGFNCLST_APND_GLOBAL),		false );

	nFuncLineOld = CLayoutInt(-1);
	nFuncColOld = CLayoutInt(-1);
	bSelected = FALSE;
	for( i = 0; i < m_pcFuncInfoArr->GetNum(); ++i ){
		pcFuncInfo = m_pcFuncInfoArr->GetAt( i );
		const TCHAR*		pWork;
		pWork = pcFuncInfo->m_cmemFuncName.GetStringPtr();
		int m = 0;
		vStrClasses.clear();
		nClassNest = 0;
		/* クラス名::メソッドの場合 */
		if( NULL != ( pPos = _tcsstr( pWork, _T("::") ) )
			&& auto_strncmp( _T("operator "), pWork, 9) != 0 ){
			/* インナークラスのネストレベルを調べる */
			int	k;
			int	nWorkLen;
			int	nCharChars;
			int	nNestTemplate = 0;
			nWorkLen = _tcslen( pWork );
			for( k = 0; k < nWorkLen; ++k ){
				//2009.9.21 syat ネストが深すぎる際のBOF対策
				if( nClassNest == MAX_JAVA_TREE_NEST ){
					k = nWorkLen;
					break;
				}
				nCharChars = CNativeT::GetSizeOfChar( pWork, nWorkLen, k );
				if( 1 == nCharChars && 0 == nNestTemplate && _T(':') == pWork[k] ){
					//	Jan. 04, 2001 genta
					//	C++の統合のため、\に加えて::をクラス区切りとみなすように
					if( k < nWorkLen - 1 && _T(':') == pWork[k+1] ){
						std::tstring strClass(&pWork[m], k - m);
						vStrClasses.push_back(strClass);
						++nClassNest;
						m = k + 2;
						++k;
						// Klass::operator std::string
						if( auto_strncmp( _T("operator "), pWork + m, 9) == 0 ){
							break;
						}
					}
					else 
						break;
				}
				else if( 1 == nCharChars && _T('\\') == pWork[k] ){
					std::tstring strClass(&pWork[m], k - m);
					vStrClasses.push_back(strClass);
					++nClassNest;
					m = k + 1;
				}
				else if( 1 == nCharChars && _T('<') == pWork[k] ){
					// namesp::function<std::string> のようなものを処理する
					nNestTemplate++;
				}
				else if( 1 == nCharChars && _T('>') == pWork[k] ){
					if( 0 < nNestTemplate ){
						nNestTemplate--;
					}
				}
				if( 2 == nCharChars ){
					++k;
				}
			}
		}
		if( 0 < nClassNest ){
			int	k;
			//	Jan. 04, 2001 genta
			//	関数先頭のセット(ツリー構築で使う)
			pWork = pWork + m; // 2 == lstrlen( "::" );

			/* クラス名のアイテムが登録されているか */
			htiClass = TreeView_GetFirstVisible( hwndTree );
			HTREEITEM htiParent = TVI_ROOT;
			for( k = 0; k < nClassNest; ++k ){
				//	Apr. 1, 2001 genta
				//	追加文字列を全角にしたのでメモリもそれだけ必要
				//	6 == strlen( "クラス" ), 1 == strlen( L'\0' )

				// 2002/10/30 frozen
				// bAddClass == true の場合の仕様変更
				// 既存の項目は　「(クラス名)(半角スペース一個)(追加文字列)」
				// となっているとみなし、szClassArr[k] が 「クラス名」と一致すれば、それを親ノードに設定。
				// ただし、一致する項目が複数ある場合は最初の項目を親ノードにする。
				// 一致しない場合は「(クラス名)(半角スペース一個)クラス」のノードを作成する。
				size_t nClassNameLen = vStrClasses[k].size();
				for( ; NULL != htiClass ; htiClass = TreeView_GetNextSibling( hwndTree, htiClass ))
				{
					tvi.mask = TVIF_HANDLE | TVIF_TEXT;
					tvi.hItem = htiClass;

					std::vector<TCHAR> vecStr;
					if( TreeView_GetItemTextVector(hwndTree, tvi, vecStr) ){
						const TCHAR* pszLabel = &vecStr[0];
						if( 0 == _tcsncmp(vStrClasses[k].c_str(), pszLabel, nClassNameLen) ){
							if( bAddClass ){
								if( pszLabel[nClassNameLen]==L' ' ){
									break;
								}
							}else{
								if( pszLabel[nClassNameLen]==L'\0' ){
									break;
								}
							}
						}
					}
				}

				/* クラス名のアイテムが登録されていないので登録 */
				if( NULL == htiClass ){
					// 2002/10/28 frozen 上からここへ移動
					std::tstring strClassName = vStrClasses[k];
					
					if( bAddClass )
					{
						if( pcFuncInfo->m_nInfo == FL_OBJ_NAMESPACE )
						{
							//_tcscat( pClassName, _T(" 名前空間") );
							strClassName += to_tchar(m_pcFuncInfoArr->GetAppendText(FL_OBJ_NAMESPACE).c_str());
						}
						else
							//_tcscat( pClassName, _T(" クラス") );
							strClassName += to_tchar(m_pcFuncInfoArr->GetAppendText(FL_OBJ_CLASS).c_str());
					}
					tvis.hParent = htiParent;
					tvis.hInsertAfter = TVI_LAST;
					tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
					tvis.item.pszText = const_cast<TCHAR*>(strClassName.c_str());
					// 2016.03.06 item.lParamは登録順の連番に変更
					tvis.item.lParam = nlParamCount;
					m_vecDummylParams.push_back(nlParamCount);
					nlParamCount++;

					htiClass = TreeView_InsertItem( hwndTree, &tvis );
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
			// 2007.05.26 genta "__interface" をクラスに類する扱いにする
			// 2011.09.25 syat プラグインで追加された要素をクラスに類する扱いにする
			if( FL_OBJ_CLASS <= pcFuncInfo->m_nInfo  && pcFuncInfo->m_nInfo <= FL_OBJ_ELEMENT_MAX )
				htiClass = TVI_ROOT;
			else
			{
			// 2002/10/27 frozen ここまで
				if( htiGlobal == NULL ){
					TV_INSERTSTRUCT	tvg;
					std::tstring sGlobal = to_tchar(m_pcFuncInfoArr->GetAppendText( FL_OBJ_GLOBAL ).c_str());

					::ZeroMemory( &tvg, sizeof(tvg));
					tvg.hParent = TVI_ROOT;
					tvg.hInsertAfter = TVI_LAST;
					tvg.item.mask = TVIF_TEXT | TVIF_PARAM;
					//tvg.item.pszText = const_cast<TCHAR*>(_T("グローバル"));
					tvg.item.pszText = const_cast<TCHAR*>(sGlobal.c_str());
					tvg.item.lParam = nlParamCount;
					m_vecDummylParams.push_back(nlParamCount);
					nlParamCount++;
					htiGlobal = TreeView_InsertItem( hwndTree, &tvg );
				}
				htiClass = htiGlobal;
			}
		}
		std::tstring strFuncName = pWork;

		// 2002/10/27 frozen 追加文字列の種類を増やした
		switch(pcFuncInfo->m_nInfo)
		{
		case FL_OBJ_DEFINITION:		//「定義位置」に追加文字列は不要なため除外
		case FL_OBJ_NAMESPACE:		//「名前空間」は別の場所で処理してるので除外
		case FL_OBJ_GLOBAL:			//「グローバル」は別の場所で処理してるので除外
			break;
		default:
			strFuncName += to_tchar(m_pcFuncInfoArr->GetAppendText(pcFuncInfo->m_nInfo).c_str());
		}

/* 該当クラス名のアイテムの子として、メソッドのアイテムを登録 */
		tvis.hParent = htiClass;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
		tvis.item.pszText = const_cast<TCHAR*>(strFuncName.c_str());
		tvis.item.lParam = nlParamCount;
		nlParamCount++;
		htiItem = TreeView_InsertItem( hwndTree, &tvis );

		/* クリップボードにコピーするテキストを編集 */
		WCHAR szText[2048];
		auto_sprintf(
			szText,
			L"%ts(%d,%d): ",
			m_pcFuncInfoArr->m_szFilePath.c_str(),		/* 解析対象ファイル名 */
			pcFuncInfo->m_nFuncLineCRLF,		/* 検出行番号 */
			pcFuncInfo->m_nFuncColCRLF		/* 検出桁番号 */
		);
		m_cmemClipText.AppendString( szText ); /* クリップボードコピー用テキスト */
		// "%ts%ls\r\n"
		m_cmemClipText.AppendNativeDataT(pcFuncInfo->m_cmemFuncName);
		m_cmemClipText.AppendString(FL_OBJ_DECLARE == pcFuncInfo->m_nInfo ? m_pcFuncInfoArr->GetAppendText( FL_OBJ_DECLARE ).c_str() : L"" ); 	//	Jan. 04, 2001 genta C++で使用
		m_cmemClipText.AppendString(L"\r\n");

		/* 現在カーソル位置のメソッドかどうか調べる */
		if( !bSelected ){
			if( pcFuncInfo->m_nFuncLineLAYOUT < nFuncLineTop
				|| (pcFuncInfo->m_nFuncLineLAYOUT == nFuncLineTop && pcFuncInfo->m_nFuncColLAYOUT <= nFuncColTop) ){
				nFuncLineTop = pcFuncInfo->m_nFuncLineLAYOUT;
				nFuncColTop = pcFuncInfo->m_nFuncColLAYOUT;
				htiSelectedTop = htiItem;
			}
		}
		{
			if( (nFuncLineOld < pcFuncInfo->m_nFuncLineLAYOUT
				|| (nFuncLineOld == pcFuncInfo->m_nFuncColLAYOUT && nFuncColOld <= pcFuncInfo->m_nFuncColLAYOUT))
			  && (pcFuncInfo->m_nFuncLineLAYOUT < m_nCurLine
				|| (pcFuncInfo->m_nFuncLineLAYOUT == m_nCurLine && pcFuncInfo->m_nFuncColLAYOUT <= m_nCurCol)) ){
				nFuncLineOld = pcFuncInfo->m_nFuncLineLAYOUT;
				nFuncColOld = pcFuncInfo->m_nFuncColLAYOUT;
				bSelected = TRUE;
				htiSelected = htiItem;
			}
		}
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
		TreeView_SelectItem( hwndTree, htiSelectedTop );
	}
//	GetTreeTextNext( hwndTree, NULL, 0 );
	m_nTreeItemCount = nlParamCount;
	return;
}


/*! リストビューコントロールの初期化：VisualBasic

  長くなったので独立させました。

  @date Jul 10, 2003  little YOSHI
*/
void CDlgFuncList::SetListVB (void)
{
	int				i;
	TCHAR			szType[64];
	TCHAR			szOption[64];
	const CFuncInfo*	pcFuncInfo;
	LV_ITEM			item;
	HWND			hwndList;
	int				bSelected;
	CLayoutInt		nFuncLineOld;
	CLayoutInt		nFuncColOld;
	int				nSelectedLine = 0;
	RECT			rc;

	::EnableWindow( ::GetDlgItem( GetHwnd() , IDC_BUTTON_COPY ), TRUE );

	hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_FL );

	
	m_cmemClipText.SetString( L"" );
	{
		const int nBuffLenTag = 17 + wcslen(to_wchar(m_pcFuncInfoArr->m_szFilePath));
		const int nNum = m_pcFuncInfoArr->GetNum();
		int nBuffLen = 0;
		for( int i = 0; i < nNum; i++ ){
			const CFuncInfo* pcFuncInfo = m_pcFuncInfoArr->GetAt(i);
			nBuffLen += pcFuncInfo->m_cmemFuncName.GetStringLength();
		}
		m_cmemClipText.AllocStringBuffer( nBuffLen + nBuffLenTag * nNum );
	}

	nFuncLineOld = CLayoutInt(-1);
	nFuncColOld = CLayoutInt(-1);
	CLayoutInt nFuncLineTop(INT_MAX);
	CLayoutInt nFuncColTop(INT_MAX);
	int nSelectedLineTop = 0;
	bSelected = FALSE;
	for( i = 0; i < m_pcFuncInfoArr->GetNum(); ++i ){
		pcFuncInfo = m_pcFuncInfoArr->GetAt( i );
		if( !bSelected ){
			if( pcFuncInfo->m_nFuncLineLAYOUT < nFuncLineTop
				|| (pcFuncInfo->m_nFuncLineLAYOUT == nFuncLineTop && pcFuncInfo->m_nFuncColLAYOUT <= nFuncColTop) ){
				nFuncLineTop = pcFuncInfo->m_nFuncLineLAYOUT;
				nFuncColTop = pcFuncInfo->m_nFuncColLAYOUT;
				nSelectedLineTop = i;
			}
		}
		{
			if( (nFuncLineOld < pcFuncInfo->m_nFuncLineLAYOUT
				|| (nFuncLineOld == pcFuncInfo->m_nFuncColLAYOUT && nFuncColOld <= pcFuncInfo->m_nFuncColLAYOUT))
			  && (pcFuncInfo->m_nFuncLineLAYOUT < m_nCurLine
				|| (pcFuncInfo->m_nFuncLineLAYOUT == m_nCurLine && pcFuncInfo->m_nFuncColLAYOUT <= m_nCurCol)) ){
				nFuncLineOld = pcFuncInfo->m_nFuncLineLAYOUT;
				nFuncColOld = pcFuncInfo->m_nFuncColLAYOUT;
				bSelected = TRUE;
				nSelectedLine = i;
			}
		}
	}
	if( 0 < m_pcFuncInfoArr->GetNum() && !bSelected ){
		bSelected = TRUE;
		nSelectedLine =  nSelectedLineTop;
	}

	TCHAR			szText[2048];
	for( i = 0; i < m_pcFuncInfoArr->GetNum(); ++i ){
		/* 現在の解析結果要素 */
		pcFuncInfo = m_pcFuncInfoArr->GetAt( i );

		//	From Here Apr. 23, 2005 genta 行番号を左端へ
		/* 行番号の表示 false=折り返し単位／true=改行単位 */
		if(m_bLineNumIsCRLF ){
			auto_sprintf( szText, _T("%d"), pcFuncInfo->m_nFuncLineCRLF );
		}else{
			auto_sprintf( szText, _T("%d"), pcFuncInfo->m_nFuncLineLAYOUT );
		}
		item.mask = LVIF_TEXT | LVIF_PARAM;
		item.pszText = szText;
		item.iItem = i;
		item.iSubItem = FL_COL_ROW;
		item.lParam	= i;
		ListView_InsertItem( hwndList, &item);

		// 2010.03.17 syat 桁追加
		/* 行番号の表示 false=折り返し単位／true=改行単位 */
		if(m_bLineNumIsCRLF ){
			auto_sprintf( szText, _T("%d"), pcFuncInfo->m_nFuncColCRLF );
		}else{
			auto_sprintf( szText, _T("%d"), pcFuncInfo->m_nFuncColLAYOUT );
		}
		item.mask = LVIF_TEXT;
		item.pszText = szText;
		item.iItem = i;
		item.iSubItem = FL_COL_COL;
		ListView_SetItem( hwndList, &item);

		item.mask = LVIF_TEXT;
		item.pszText = const_cast<TCHAR*>(pcFuncInfo->m_cmemFuncName.GetStringPtr());
		item.iItem = i;
		item.iSubItem = FL_COL_NAME;
		ListView_SetItem( hwndList, &item);
		//	To Here Apr. 23, 2005 genta 行番号を左端へ

		item.mask = LVIF_TEXT;

		// 2001/06/23 N.Nakatani for Visual Basic
		//	Jun. 26, 2001 genta 半角かな→全角に
		auto_memset(szText, _T('\0'), _countof(szText));
		auto_memset(szType, _T('\0'), _countof(szType));
		auto_memset(szOption, _T('\0'), _countof(szOption));
		if( 1 == ((pcFuncInfo->m_nInfo >> 8) & 0x01) ){
			// スタティック宣言(Static)
			// 2006.12.12 Moca 末尾にスペース追加
			_tcscpy(szOption, LS(STR_DLGFNCLST_VB_STATIC));
		}
		switch ((pcFuncInfo->m_nInfo >> 4) & 0x0f) {
			case 2  :	// プライベート(Private)
				_tcsncat(szOption, LS(STR_DLGFNCLST_VB_PRIVATE), _countof(szOption) - _tcslen(szOption)); //	2006.12.17 genta サイズ誤り修正
				break;

			case 3  :	// フレンド(Friend)
				_tcsncat(szOption, LS(STR_DLGFNCLST_VB_FRIEND), _countof(szOption) - _tcslen(szOption)); //	2006.12.17 genta サイズ誤り修正
				break;

			default :	// パブリック(Public)
				_tcsncat(szOption, LS(STR_DLGFNCLST_VB_PUBLIC), _countof(szOption) - _tcslen(szOption)); //	2006.12.17 genta サイズ誤り修正
		}
		int nInfo = pcFuncInfo->m_nInfo;
		switch (nInfo & 0x0f) {
			case 1:		// 関数(Function)
				_tcscpy(szType, LS(STR_DLGFNCLST_VB_FUNCTION));
				break;

			// 2006.12.12 Moca ステータス→プロシージャに変更
			case 2:		// プロシージャ(Sub)
				_tcscpy(szType, LS(STR_DLGFNCLST_VB_PROC));
				break;

			case 3:		// プロパティ 取得(Property Get)
				_tcscpy(szType, LS(STR_DLGFNCLST_VB_PROPGET));
				break;

			case 4:		// プロパティ 設定(Property Let)
				_tcscpy(szType, LS(STR_DLGFNCLST_VB_PROPLET));
				break;

			case 5:		// プロパティ 参照(Property Set)
				_tcscpy(szType, LS(STR_DLGFNCLST_VB_PROPSET));
				break;

			case 6:		// 定数(Const)
				_tcscpy(szType, LS(STR_DLGFNCLST_VB_CONST));
				break;

			case 7:		// 列挙型(Enum)
				_tcscpy(szType, LS(STR_DLGFNCLST_VB_ENUM));
				break;

			case 8:		// ユーザ定義型(Type)
				_tcscpy(szType, LS(STR_DLGFNCLST_VB_TYPE));
				break;

			case 9:		// イベント(Event)
				_tcscpy(szType, LS(STR_DLGFNCLST_VB_EVENT));
				break;

			default:	// 未定義なのでクリア
				nInfo	= 0;

		}
		if ( 2 == ((nInfo >> 8) & 0x02) ) {
			// 宣言(Declareなど)
			_tcsncat(szType, LS(STR_DLGFNCLST_VB_DECL), _countof(szType) - _tcslen(szType));
		}

		TCHAR szTypeOption[256]; // 2006.12.12 Moca auto_sprintfの入出力で同一変数を使わないための作業領域追加
		if ( 0 == nInfo ) {
			szTypeOption[0] = _T('\0');	//	2006.12.17 genta 全体を0で埋める必要はない
		} else
		if ( szOption[0] == _T('\0') ) {
			auto_sprintf(szTypeOption, _T("%ts"), szType);
		} else {
			auto_sprintf(szTypeOption, _T("%ts（%ts）"), szType, szOption);
		}
		item.pszText = szTypeOption;
		item.iItem = i;
		item.iSubItem = FL_COL_REMARK;
		ListView_SetItem( hwndList, &item);

		/* クリップボードにコピーするテキストを編集 */
		if(item.pszText[0] != _T('\0')){
			// 検出結果の種類(関数,,,)があるとき
			// 2006.12.12 Moca szText を自分自身にコピーしていたバグを修正
			auto_sprintf(
				szText,
				_T("%ts(%d,%d): "),
				m_pcFuncInfoArr->m_szFilePath.c_str(),		/* 解析対象ファイル名 */
				pcFuncInfo->m_nFuncLineCRLF,		/* 検出行番号 */
				pcFuncInfo->m_nFuncColCRLF		/* 検出桁番号 */
			);
			m_cmemClipText.AppendStringT(szText);
			// "%ts(%ts)\r\n"
			m_cmemClipText.AppendNativeDataT(pcFuncInfo->m_cmemFuncName);
			m_cmemClipText.AppendString(L"(");
			m_cmemClipText.AppendStringT(item.pszText);
			m_cmemClipText.AppendString(L")\r\n");
		}else{
			// 検出結果の種類(関数,,,)がないとき
			auto_sprintf(
				szText,
				_T("%ts(%d,%d): "),
				m_pcFuncInfoArr->m_szFilePath.c_str(),		/* 解析対象ファイル名 */
				pcFuncInfo->m_nFuncLineCRLF,		/* 検出行番号 */
				pcFuncInfo->m_nFuncColCRLF		/* 検出桁番号 */
			);
			m_cmemClipText.AppendStringT(szText);
			// "%ts\r\n"
			m_cmemClipText.AppendNativeDataT(pcFuncInfo->m_cmemFuncName);
			m_cmemClipText.AppendString(L"\r\n");
		}
	}

	//2002.02.08 hor Listは列幅調整とかを実行する前に表示しとかないと変になる
	::ShowWindow( hwndList, SW_SHOW );
	/* 列の幅をデータに合わせて調整 */
	ListView_SetColumnWidth( hwndList, FL_COL_ROW, LVSCW_AUTOSIZE );
	ListView_SetColumnWidth( hwndList, FL_COL_COL, LVSCW_AUTOSIZE );
	ListView_SetColumnWidth( hwndList, FL_COL_NAME, LVSCW_AUTOSIZE );
	ListView_SetColumnWidth( hwndList, FL_COL_REMARK, LVSCW_AUTOSIZE );
	ListView_SetColumnWidth( hwndList, FL_COL_ROW, ListView_GetColumnWidth( hwndList, FL_COL_ROW ) + 16 );
	ListView_SetColumnWidth( hwndList, FL_COL_COL, ListView_GetColumnWidth( hwndList, FL_COL_COL ) + 16 );
	ListView_SetColumnWidth( hwndList, FL_COL_NAME, ListView_GetColumnWidth( hwndList, FL_COL_NAME ) + 16 );
	ListView_SetColumnWidth( hwndList, FL_COL_REMARK, ListView_GetColumnWidth( hwndList, FL_COL_REMARK ) + 16 );
	if( bSelected ){
		ListView_GetItemRect( hwndList, 0, &rc, LVIR_BOUNDS );
		ListView_Scroll( hwndList, 0, nSelectedLine * ( rc.bottom - rc.top ) );
		ListView_SetItemState( hwndList, nSelectedLine, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	}

	return;
}

/*! 汎用ツリーコントロールの初期化：CFuncInfo::m_nDepthを利用して親子を設定

	@param[in] tagjump タグジャンプ形式で出力する

	@date 2002.04.01 YAZAKI
	@date 2002.11.10 Moca 階層の制限をなくした
	@date 2007.02.25 genta クリップボード出力をタブジャンプ可能な書式に変更
	@date 2007.03.04 genta タブジャンプ可能な書式に変更するかどうかのフラグを追加
	@date 2014.06.06 Moca 他ファイルへのタグジャンプ機能を追加
*/
void CDlgFuncList::SetTree(bool tagjump, bool nolabel)
{
	HTREEITEM hItemSelected = NULL;
	HTREEITEM hItemSelectedTop = NULL;
	HWND hwndTree = ::GetDlgItem( GetHwnd(), IDC_TREE_FL );

	int i;
	int nFuncInfoArrNum = m_pcFuncInfoArr->GetNum();
	int nStackPointer = 0;
	int nStackDepth = 32; // phParentStack の確保している数
	HTREEITEM* phParentStack;
	phParentStack = (HTREEITEM*)malloc( nStackDepth * sizeof( HTREEITEM ) );
	phParentStack[ nStackPointer ] = TVI_ROOT;
	CLayoutInt nFuncLineOld(-1);
	CLayoutInt nFuncColOld(-1);
	CLayoutInt nFuncLineTop(INT_MAX);
	CLayoutInt nFuncColTop(INT_MAX);
	bool bSelected = false;

	m_cmemClipText.SetString(L"");
	{
		int nCount = 0;
		int nBuffLen = 0;
		int nBuffLenTag = 3; // " \r\n"
		if( tagjump ){
			nBuffLenTag = 10 + wcslen(to_wchar(m_pcFuncInfoArr->m_szFilePath));
		}
		for( int i = 0; i < nFuncInfoArrNum; i++ ){
			const CFuncInfo* pcFuncInfo = m_pcFuncInfoArr->GetAt(i);
			if( pcFuncInfo->IsAddClipText() ){
				nBuffLen += pcFuncInfo->m_cmemFuncName.GetStringLength() + pcFuncInfo->m_nDepth * 2;
				nCount++;
			}
		}
		m_cmemClipText.AllocStringBuffer( nBuffLen + nBuffLenTag * nCount );
	}

	for (i = 0; i < nFuncInfoArrNum; i++){
		CFuncInfo* pcFuncInfo = m_pcFuncInfoArr->GetAt(i);

		/*	新しいアイテムを作成
			現在の親の下にぶら下げる形で、最後に追加する。
		*/
		HTREEITEM hItem;
		TV_INSERTSTRUCT cTVInsertStruct;
		cTVInsertStruct.hParent = phParentStack[ nStackPointer ];
		// 2016.04.24 TVI_LASTは要素数が多いとすごく遅い。TVI_FIRSTを使い後でソートしなおす
		cTVInsertStruct.hInsertAfter = TVI_FIRST;
		cTVInsertStruct.item.mask = TVIF_TEXT | TVIF_PARAM;
		cTVInsertStruct.item.pszText = pcFuncInfo->m_cmemFuncName.GetStringPtr();
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

		/*	pcFuncInfoに登録されている行数、桁を確認して、選択するアイテムを考える
		*/
		bool bFileSelect = false;
		if( pcFuncInfo->m_cmemFileName.GetStringPtr() && m_pcFuncInfoArr->m_szFilePath[0] ){
			if( 0 == auto_stricmp( pcFuncInfo->m_cmemFileName.GetStringPtr(), m_pcFuncInfoArr->m_szFilePath.c_str() ) ){
				bFileSelect = true;
			}
		}else{
			bFileSelect = true;
		}
		if( bFileSelect ){
			if( !bSelected ){
				if( pcFuncInfo->m_nFuncLineLAYOUT < nFuncLineTop
					|| (pcFuncInfo->m_nFuncLineLAYOUT == nFuncLineTop && pcFuncInfo->m_nFuncColLAYOUT <= nFuncColTop) ){
					nFuncLineTop = pcFuncInfo->m_nFuncLineLAYOUT;
					nFuncColTop = pcFuncInfo->m_nFuncColLAYOUT;
					hItemSelectedTop = hItem;
				}
			}
			if( (nFuncLineOld < pcFuncInfo->m_nFuncLineLAYOUT
				|| (nFuncLineOld == pcFuncInfo->m_nFuncColLAYOUT && nFuncColOld <= pcFuncInfo->m_nFuncColLAYOUT))
			  && (pcFuncInfo->m_nFuncLineLAYOUT < m_nCurLine
				|| (pcFuncInfo->m_nFuncLineLAYOUT == m_nCurLine && pcFuncInfo->m_nFuncColLAYOUT <= m_nCurCol)) ){
				nFuncLineOld = pcFuncInfo->m_nFuncLineLAYOUT;
				nFuncColOld = pcFuncInfo->m_nFuncColLAYOUT;
				hItemSelected = hItem;
				bSelected = true;
			}
		}

		/* クリップボードコピー用テキストを作成する */
		//	2003.06.22 Moca dummy要素はツリーに入れるがTAGJUMPには加えない
		if( pcFuncInfo->IsAddClipText() ){
			CNativeT text;
			if( tagjump ){
				const TCHAR* pszFileName = pcFuncInfo->m_cmemFileName.GetStringPtr();
				if( pszFileName == NULL ){
					pszFileName = m_pcFuncInfoArr->m_szFilePath;
				}
				text.AllocStringBuffer(
					  pcFuncInfo->m_cmemFuncName.GetStringLength()
					+ nStackPointer * 2 + 1
					+ _tcslen( pszFileName )
					+ 20
				);
				//	2007.03.04 genta タグジャンプできる形式で書き込む
				text.AppendString( pszFileName );
				
				if( 0 < pcFuncInfo->m_nFuncLineCRLF ){
					TCHAR linenum[32];
					int len = auto_sprintf( linenum, _T("(%d,%d): "),
						pcFuncInfo->m_nFuncLineCRLF,				/* 検出行番号 */
						pcFuncInfo->m_nFuncColCRLF					/* 検出桁番号 */
					);
					text.AppendString( linenum );
				}
			}

			if( !nolabel ){
				for( int cnt = 0; cnt < nStackPointer; cnt++ ){
					text.AppendString(_T("  "));
				}
				text.AppendString(_T(" "));
				
				text.AppendNativeData( pcFuncInfo->m_cmemFuncName );
			}
			text.AppendString( _T("\r\n") );
			m_cmemClipText.AppendNativeDataT( text );	/* クリップボードコピー用テキスト */
		}
	}

end_of_func:;

	::EnableWindow( ::GetDlgItem( GetHwnd() , IDC_BUTTON_COPY ), TRUE );

	if( NULL != hItemSelected ){
		/* 現在カーソル位置のメソッドを選択状態にする */
		TreeView_SelectItem( hwndTree, hItemSelected );
	}else if( NULL != hItemSelectedTop ){
		TreeView_SelectItem( hwndTree, hItemSelectedTop );
	}

	free( phParentStack );
}



void CDlgFuncList::SetDocLineFuncList()
{
	if( m_nOutlineType == OUTLINE_BOOKMARK ){
		return;
	}
	if( m_nOutlineType == OUTLINE_FILETREE ){
		return;
	}
	CEditView* pcEditView=(CEditView*)m_lParam;
	CDocLineMgr* pcDocLineMgr = &pcEditView->GetDocument()->m_cDocLineMgr;
	
	CFuncListManager().ResetAllFucListMark(pcDocLineMgr, false);
	int i;
	int num = m_pcFuncInfoArr->GetNum();
	for( i = 0; i < num; ++i ){
		const CFuncInfo* pcFuncInfo = m_pcFuncInfoArr->GetAt(i);
		if( 0 < pcFuncInfo->m_nFuncLineCRLF ){
			CDocLine* pcDocLine = pcDocLineMgr->GetLine( pcFuncInfo->m_nFuncLineCRLF - 1 );
			if( pcDocLine ){
				CFuncListManager().SetLineFuncList( pcDocLine, true );
			}
		}
	}
}



/*! ファイルツリー作成
	@note m_pcFuncInfoArrにフルパス情報を書き込みつつツリーを作成
*/
void CDlgFuncList::SetTreeFile()
{
	HWND hwndTree = ::GetDlgItem( GetHwnd(), IDC_TREE_FL );

	m_cmemClipText.SetString(L"");
	SFilePath IniDirPath;
	LoadFileTreeSetting( m_fileTreeSetting, IniDirPath );
	m_pcFuncInfoArr->Empty();
	int nFuncInfo = 0;
	std::vector<HTREEITEM> hParentTree;
	hParentTree.push_back(TVI_ROOT);
	for( int i = 0; i < (int)m_fileTreeSetting.m_aItems.size(); i++ ){
		TCHAR szPath[_MAX_PATH];
		TCHAR szPath2[_MAX_PATH];
		const SFileTreeItem& item = m_fileTreeSetting.m_aItems[i];
		// item.m_szTargetPath => szPath メタ文字の展開
		if( !CFileNameManager::ExpandMetaToFolder(item.m_szTargetPath, szPath, _countof(szPath)) ){
			auto_strcpy_s(szPath, _countof(szPath), _T("<Error:Long Path>"));
		}
		// szPath => szPath2 <iniroot>展開
		const TCHAR* pszFrom = szPath;
		if( m_fileTreeSetting.m_szLoadProjectIni[0] != _T('\0')){
			CNativeT strTemp(pszFrom);
			strTemp.Replace(_T("<iniroot>"), IniDirPath);
			if( _countof(szPath2) <= strTemp.GetStringLength() ){
				auto_strcpy_s(szPath2, _countof(szPath), _T("<Error:Long Path>"));
			}else{
				auto_strcpy_s(szPath2, _countof(szPath), strTemp.GetStringPtr());
			}
		}else{
			auto_strcpy(szPath2, pszFrom);
		}
		// szPath2 => szPath 「.」やショートパス等の展開
		pszFrom = szPath2;
		if( ::GetLongFileName(pszFrom, szPath) ){
		}else{
			auto_strcpy(szPath, pszFrom);
		}
		while( item.m_nDepth < (int)hParentTree.size() - 1 ){
			hParentTree.resize(hParentTree.size() - 1);
		}
		const TCHAR* pszLabel = szPath;
		if( item.m_szLabelName[0] != _T('\0') ){
			pszLabel = item.m_szLabelName;
		}
		// lvis.item.lParam
		// 0 以下(nFuncInfo): m_pcFuncInfoArr->At(nFuncInfo)にファイル名
		// -1: Grepのファイル名要素
		// -2: Grepのサブフォルダ要素
		// -(nFuncInfo * 10 + 3): Grepルートフォルダ要素
		// -4: データ・追加操作なし
		TVINSERTSTRUCT tvis;
		tvis.hParent      = hParentTree.back();
		tvis.item.mask    = TVIF_TEXT | TVIF_PARAM;
		if( item.m_eFileTreeItemType == EFileTreeItemType_Grep ){
			m_pcFuncInfoArr->AppendData( CLogicInt(-1), CLogicInt(-1), CLayoutInt(-1), CLayoutInt(-1), _T(""), szPath, 0, 0 );
			tvis.item.pszText = const_cast<TCHAR*>(pszLabel);
			tvis.item.lParam  = -(nFuncInfo * 10 + 3);
			HTREEITEM hParent = TreeView_InsertItem(hwndTree, &tvis);
			nFuncInfo++;
			SetTreeFileSub( hParent, NULL );
		}else if( item.m_eFileTreeItemType == EFileTreeItemType_File ){
			m_pcFuncInfoArr->AppendData( CLogicInt(-1), CLogicInt(-1), CLayoutInt(-1), CLayoutInt(-1), _T(""), szPath, 0, 0 );
			tvis.item.pszText = const_cast<TCHAR*>(pszLabel);
			tvis.item.lParam  = nFuncInfo;
			TreeView_InsertItem(hwndTree, &tvis);
			nFuncInfo++;
		}else if( item.m_eFileTreeItemType == EFileTreeItemType_Folder ){
			pszLabel = item.m_szLabelName;
			if( pszLabel[0] == _T('\0') ){
				pszLabel = _T("Folder");
			}
			tvis.item.pszText = const_cast<TCHAR*>(pszLabel);
			tvis.item.lParam  = -4;
			HTREEITEM hParent = TreeView_InsertItem(hwndTree, &tvis);
			hParentTree.push_back(hParent);
		}
	}
}


void CDlgFuncList::SetTreeFileSub( HTREEITEM hParent, const TCHAR* pszFile )
{
	HWND hwndTree = ::GetDlgItem( GetHwnd(), IDC_TREE_FL );

	if( NULL != TreeView_GetChild( hwndTree, hParent ) ){
		return;
	}

	HTREEITEM hItemSelected = NULL;

	std::tstring basePath;
	int nItem = 0; // 設定Item番号
	if( !GetTreeFileFullName( hwndTree, hParent, &basePath, &nItem ) ){
		return; // error
	}

	int count = 0;
	CGrepEnumKeys cGrepEnumKeys;
	int errNo = cGrepEnumKeys.SetFileKeys( m_fileTreeSetting.m_aItems[nItem].m_szTargetFile );
	if( errNo != 0 ){
		TVINSERTSTRUCT tvis;
		tvis.hParent      = hParent;
		tvis.item.mask    = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
		tvis.item.pszText = const_cast<TCHAR*>(_T("<Wild Card Error>"));
		tvis.item.lParam = -4;
		TreeView_InsertItem(hwndTree, &tvis);
		return;
	}
	CGrepEnumOptions cGrepEnumOptions;
	cGrepEnumOptions.m_bIgnoreHidden   = m_fileTreeSetting.m_aItems[nItem].m_bIgnoreHidden;
	cGrepEnumOptions.m_bIgnoreReadOnly = m_fileTreeSetting.m_aItems[nItem].m_bIgnoreReadOnly;
	cGrepEnumOptions.m_bIgnoreSystem   = m_fileTreeSetting.m_aItems[nItem].m_bIgnoreSystem;
	CGrepEnumFiles cGrepExceptAbsFiles;
	cGrepExceptAbsFiles.Enumerates(_T(""), cGrepEnumKeys.m_vecExceptAbsFileKeys, cGrepEnumOptions);
	CGrepEnumFolders cGrepExceptAbsFolders;
	cGrepExceptAbsFolders.Enumerates(_T(""), cGrepEnumKeys.m_vecExceptAbsFolderKeys, cGrepEnumOptions);

	//フォルダ一覧作成
	CGrepEnumFilterFolders cGrepEnumFilterFolders;
	cGrepEnumFilterFolders.Enumerates( basePath.c_str(), cGrepEnumKeys, cGrepEnumOptions, cGrepExceptAbsFolders );
	int nItemCount = cGrepEnumFilterFolders.GetCount();
	count = nItemCount;
	for( int i = 0; i < nItemCount; i++ ){
		TVINSERTSTRUCT tvis;
		tvis.hParent      = hParent;
		tvis.item.mask    = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
		tvis.item.pszText = const_cast<TCHAR*>(cGrepEnumFilterFolders.GetFileName(i));
		tvis.item.lParam  = -2;
		tvis.item.cChildren = 1; // ダミーの子要素を持たせて[+]を表示
		TreeView_InsertItem(hwndTree, &tvis);
	}

	//ファイル一覧作成
	CGrepEnumFilterFiles cGrepEnumFilterFiles;
	cGrepEnumFilterFiles.Enumerates( basePath.c_str(), cGrepEnumKeys, cGrepEnumOptions, cGrepExceptAbsFiles );
	nItemCount = cGrepEnumFilterFiles.GetCount();
	count += nItemCount;
	for( int i = 0; i < nItemCount; i ++ ){
		const TCHAR* pFile = cGrepEnumFilterFiles.GetFileName(i);
		TVINSERTSTRUCT tvis;
		tvis.hParent      = hParent;
		tvis.item.mask    = TVIF_TEXT | TVIF_PARAM;
		tvis.item.pszText = const_cast<TCHAR*>(pFile);
		tvis.item.lParam  = -1;
		HTREEITEM hItem = TreeView_InsertItem(hwndTree, &tvis);
		if( pszFile && auto_stricmp(pszFile, pFile) == 0 ){
			hItemSelected = hItem;
		}
	}
	if( hItemSelected ){
		TreeView_SelectItem( hwndTree, hItemSelected );
	}
	if( count == 0 ){
		// [+]記号削除
		TVITEM item;
		item.mask  = TVIF_HANDLE | TVIF_CHILDREN;
		item.cChildren = 0;
		item.hItem = hParent;
		TreeView_SetItem(hwndTree, &item);
	}
}

BOOL CDlgFuncList::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	m_bStretching = false;
	m_bHovering = false;
	m_nHilightedBtn = -1;
	m_nCapturingBtn = -1;

	_SetHwnd( hwndDlg );

	HWND		hwndList;
	int			nCxVScroll;
	int			nColWidthArr[] = { 0, 10, 46, 80 };
	RECT		rc;
	LV_COLUMN	col;
	hwndList = ::GetDlgItem( hwndDlg, IDC_LIST_FL );
	::SetWindowLongPtr(hwndList, GWL_STYLE, ::GetWindowLongPtr(hwndList, GWL_STYLE) | LVS_SHOWSELALWAYS );
	// 2005.10.21 zenryaku 1行選択
	ListView_SetExtendedListViewStyle(hwndList,
		ListView_GetExtendedListViewStyle(hwndList) | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	::GetWindowRect( hwndList, &rc );
	nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = rc.right - rc.left - ( nColWidthArr[1] + nColWidthArr[2] + nColWidthArr[3] ) - nCxVScroll - 8;
	//	Apr. 23, 2005 genta 行番号を左端へ
	col.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_LIST_LINE_M));
	col.iSubItem = FL_COL_ROW;
	ListView_InsertColumn( hwndList, FL_COL_ROW, &col);

	// 2010.03.17 syat 桁追加
	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = nColWidthArr[FL_COL_COL];
	col.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_LIST_COL));
	col.iSubItem = FL_COL_COL;
	ListView_InsertColumn( hwndList, FL_COL_COL, &col);

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = nColWidthArr[FL_COL_NAME];
	//	Apr. 23, 2005 genta 行番号を左端へ
	col.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_LIST_FUNC));
	col.iSubItem = FL_COL_NAME;
	ListView_InsertColumn( hwndList, FL_COL_NAME, &col);

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = nColWidthArr[FL_COL_REMARK];
	col.pszText = const_cast<TCHAR*>(_T(" "));
	col.iSubItem = FL_COL_REMARK;
	ListView_InsertColumn( hwndList, FL_COL_REMARK, &col);

	/* アウトライン位置とサイズを初期化する */ // 20060201 aroka
	CEditView* pcEditView=(CEditView*)m_lParam;
	if( pcEditView != NULL ){
		if( !IsDocking() && m_pShareData->m_Common.m_sOutline.m_bRememberOutlineWindowPos ){
			WINDOWPLACEMENT cWindowPlacement;
			cWindowPlacement.length = sizeof( cWindowPlacement );
			if (::GetWindowPlacement( pcEditView->m_pcEditWnd->GetHwnd(), &cWindowPlacement )){
				/* ウィンドウ位置・サイズを-1以外の値にしておくと、CDialogで使用される． */
				m_xPos = m_pShareData->m_Common.m_sOutline.m_xOutlineWindowPos + cWindowPlacement.rcNormalPosition.left;
				m_yPos = m_pShareData->m_Common.m_sOutline.m_yOutlineWindowPos + cWindowPlacement.rcNormalPosition.top;
				m_nWidth =  m_pShareData->m_Common.m_sOutline.m_widthOutlineWindow;
				m_nHeight = m_pShareData->m_Common.m_sOutline.m_heightOutlineWindow;
			}
		}else if( IsDocking() ){
			m_xPos = 0;
			m_yPos = 0;
			m_nShowCmd = SW_HIDE;
			::GetWindowRect( ::GetParent(pcEditView->GetHwnd()), &rc );	// ここではまだ GetDockSpaceRect() は使えない
			EDockSide eDockSide = GetDockSide();
			switch( eDockSide ){
			case DOCKSIDE_LEFT:		m_nWidth = ProfDockLeft();		break;
			case DOCKSIDE_TOP:		m_nHeight = ProfDockTop();		break;
			case DOCKSIDE_RIGHT:	m_nWidth = ProfDockRight();		break;
			case DOCKSIDE_BOTTOM:	m_nHeight = ProfDockBottom();	break;
			}
			if( eDockSide == DOCKSIDE_LEFT || eDockSide == DOCKSIDE_RIGHT ){
				if( m_nWidth == 0 )	// 初回
					m_nWidth = (rc.right - rc.left) / 3;
				if( m_nWidth > rc.right - rc.left - DOCK_MIN_SIZE ) m_nWidth = rc.right - rc.left - DOCK_MIN_SIZE;
				if( m_nWidth < DOCK_MIN_SIZE ) m_nWidth = DOCK_MIN_SIZE;
			}else{
				if( m_nHeight == 0 )	// 初回
					m_nHeight = (rc.bottom - rc.top) / 3;
				if( m_nHeight > rc.bottom - rc.top - DOCK_MIN_SIZE ) m_nHeight = rc.bottom - rc.top - DOCK_MIN_SIZE;
				if( m_nHeight < DOCK_MIN_SIZE ) m_nHeight = DOCK_MIN_SIZE;
			}
		}
	}

	if( !m_bInChangeLayout ){	// ChangeLayout() 処理中は設定変更しない
		bool bType = (ProfDockSet() != 0);
		if( bType ){
			CDocTypeManager().GetTypeConfig(CTypeConfig(m_nDocType), m_type);
		}
		ProfDockDisp() = TRUE;
		if( bType ){
			SetTypeConfig( CTypeConfig(m_nDocType), m_type );

		}
		// 他ウィンドウに変更を通知する
		if( ProfDockSync() ){
			HWND hwndEdit = pcEditView->m_pcEditWnd->GetHwnd();
			PostOutlineNotifyToAllEditors( (WPARAM)0, (LPARAM)hwndEdit );
		}
	}

	if( !IsDocking() ){
		/* 基底クラスメンバ */
		CreateSizeBox();

		LONG_PTR lStyle = ::GetWindowLongPtr( GetHwnd(), GWL_STYLE );
		::SetWindowLongPtr( GetHwnd(), GWL_STYLE, lStyle | WS_THICKFRAME );
		::SetWindowPos( GetHwnd(), NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED );
	}

	m_hwndToolTip = NULL;
	if( IsDocking() ){
		//ツールチップを作成する。（「閉じる」などのボタン用）
		m_hwndToolTip = ::CreateWindowEx(
			0,
			TOOLTIPS_CLASS,
			NULL,
			WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			GetHwnd(),
			NULL,
			m_hInstance,
			NULL
			);

		// ツールチップをマルチライン可能にする（SHRT_MAX: Win95でINT_MAXだと表示されない）
		Tooltip_SetMaxTipWidth( m_hwndToolTip, SHRT_MAX );

		// アウトラインにツールチップを追加する
		TOOLINFO	ti;
		ti.cbSize      = CCSIZEOF_STRUCT(TOOLINFO, lpszText);
		ti.uFlags      = TTF_SUBCLASS | TTF_IDISHWND;	// TTF_IDISHWND: uId は HWND で rect は無視（HWND 全体）
		ti.hwnd        = GetHwnd();
		ti.hinst       = m_hInstance;
		ti.uId         = (UINT_PTR)GetHwnd();
		ti.lpszText    = NULL;
		ti.rect.left   = 0;
		ti.rect.top    = 0;
		ti.rect.right  = 0;
		ti.rect.bottom = 0;
		Tooltip_AddTool( m_hwndToolTip, &ti );

		// 不要なコントロールを隠す
		HWND hwndPrev;
		HWND hwnd = ::GetWindow( GetHwnd(), GW_CHILD );
		while( hwnd ){
			int nId = ::GetDlgCtrlID( hwnd );
			hwndPrev = hwnd;
			hwnd = ::GetWindow( hwnd, GW_HWNDNEXT );
			switch( nId ){
			case IDC_STATIC_nSortType:
			case IDC_COMBO_nSortType:
			case IDC_LIST_FL:
			case IDC_TREE_FL:
				continue;
			}
			ShowWindow( hwndPrev, SW_HIDE );
		}
	}

	SyncColor();

	::GetWindowRect( hwndDlg, &rc );
	m_ptDefaultSize.x = rc.right - rc.left;
	m_ptDefaultSize.y = rc.bottom - rc.top;
	
	::GetClientRect( hwndDlg, &rc );
	m_ptDefaultSizeClient.x = rc.right;
	m_ptDefaultSizeClient.y = rc.bottom;

	for( int i = 0; i < _countof(anchorList); i++ ){
		GetItemClientRect( anchorList[i].id, m_rcItems[i] );
		// ドッキング中はウィンドウ幅いっぱいまで伸ばす
		if( IsDocking() ){
			if( anchorList[i].anchor == ANCHOR_ALL ){
				::GetClientRect( hwndDlg, &rc );
				m_rcItems[i].right = rc.right;
				m_rcItems[i].bottom = rc.bottom;
			}
		}
	}


	return CDialog::OnInitDialog( hwndDlg, wParam, lParam );
}


BOOL CDlgFuncList::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_MENU:
		RECT rcMenu;
		GetWindowRect( ::GetDlgItem( GetHwnd(), IDC_BUTTON_MENU ), &rcMenu );
		POINT ptMenu;
		ptMenu.x = rcMenu.left;
		ptMenu.y = rcMenu.bottom;
		DoMenu( ptMenu, GetHwnd() );
		return TRUE;
	case IDC_BUTTON_HELP:
		/* 「アウトライン解析」のヘルプ */
		//Apr. 5, 2001 JEPRO 修正漏れを追加 (Stonee, 2001/03/12 第四引数を、機能番号からヘルプトピック番号を調べるようにした)
		MyWinHelp( GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_OUTLINE) );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
	case IDOK:
		return OnJump();
	case IDCANCEL:
		if( m_bModal ){		/* モーダル ダイアログか */
			::EndDialog( GetHwnd(), 0 );
		}else{
			if( IsDocking() ){
				::SetFocus( ((CEditView*)m_lParam)->GetHwnd() );
			}else{
				::DestroyWindow( GetHwnd() );
			}
		}
		return TRUE;
	case IDC_BUTTON_COPY:
		// Windowsクリップボードにコピー 
		// 2004.02.17 Moca 関数化
		SetClipboardText( GetHwnd(), m_cmemClipText.GetStringPtr(), m_cmemClipText.GetStringLength() );
		return TRUE;
	case IDC_BUTTON_WINSIZE:
		{// ウィンドウの位置とサイズを記憶 // 20060201 aroka
			m_pShareData->m_Common.m_sOutline.m_bRememberOutlineWindowPos = ::IsDlgButtonChecked( GetHwnd(), IDC_BUTTON_WINSIZE );
		}
		// ボタンが押されているかはっきりさせる 2008/6/5 Uchi
		::DlgItem_SetText( GetHwnd(), IDC_BUTTON_WINSIZE,
			m_pShareData->m_Common.m_sOutline.m_bRememberOutlineWindowPos ? _T("■") : _T("□") );
		return TRUE;
	//2002.02.08 オプション切替後List/Treeにフォーカス移動
	case IDC_CHECK_bAutoCloseDlgFuncList:
	case IDC_CHECK_bMarkUpBlankLineEnable:
	case IDC_CHECK_bFunclistSetFocusOnJump:
		m_pShareData->m_Common.m_sOutline.m_bAutoCloseDlgFuncList = ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_bAutoCloseDlgFuncList );
		m_pShareData->m_Common.m_sOutline.m_bMarkUpBlankLineEnable = ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_bMarkUpBlankLineEnable );
		m_pShareData->m_Common.m_sOutline.m_bFunclistSetFocusOnJump = ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_bFunclistSetFocusOnJump );
		if(m_pShareData->m_Common.m_sOutline.m_bAutoCloseDlgFuncList){
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHECK_bFunclistSetFocusOnJump ), FALSE );
		}else{
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHECK_bFunclistSetFocusOnJump ), TRUE );
		}
		if(wID==IDC_CHECK_bMarkUpBlankLineEnable&&m_nListType==OUTLINE_BOOKMARK){
			CEditView* pcEditView=(CEditView*)m_lParam;
			pcEditView->GetCommander().HandleCommand( F_BOOKMARK_VIEW, true, TRUE, 0, 0, 0 );
			m_nCurLine=pcEditView->GetCaret().GetCaretLayoutPos().GetY2() + CLayoutInt(1);
			CDocTypeManager().GetTypeConfig(pcEditView->GetDocument()->m_cDocType.GetDocumentType(), m_type);
			SetData();
		}else
		if(m_nViewType == VIEWTYPE_TREE){
			::SetFocus( ::GetDlgItem( GetHwnd(), IDC_TREE_FL ) );
		}else{
			::SetFocus( ::GetDlgItem( GetHwnd(), IDC_LIST_FL ) );
		}
		return TRUE;
	case IDC_BUTTON_SETTING:
		{
			CDlgFileTree cDlgFileTree;
			int nRet = cDlgFileTree.DoModal( G_AppInstance(), GetHwnd(), (LPARAM)this );
			if( nRet == TRUE ){
				EFunctionCode nFuncCode = GetFuncCodeRedraw(m_nOutlineType);
				CEditView* pcEditView = (CEditView*)m_lParam;
				pcEditView->GetCommander().HandleCommand( nFuncCode, true, SHOW_RELOAD, 0, 0, 0 );
			}
		}
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

	CEditView* pcEditView=(CEditView*)m_lParam;
	hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_FL );
	hwndTree = ::GetDlgItem( GetHwnd(), IDC_TREE_FL );

	if( hwndTree == pnmh->hwndFrom ){
		pnmtv = (NM_TREEVIEW *) lParam;
		switch( pnmtv->hdr.code ){
		case NM_CLICK:
			if( IsDocking() ){
				// この時点ではまだ選択変更されていないが OnJump() の予備動作として先に選択変更しておく
				TVHITTESTINFO tvht = {0};
				::GetCursorPos( &tvht.pt );
				::ScreenToClient( hwndTree, &tvht.pt );
				TreeView_HitTest( hwndTree, &tvht );
				if( (tvht.flags & TVHT_ONITEM) && tvht.hItem ){
					TreeView_SelectItem( hwndTree, tvht.hItem );
					OnJump( false, false );
					return TRUE;
				}
			}
			break;
		case NM_DBLCLK:
			// 2002.02.16 hor Treeのダブルクリックでフォーカス移動できるように 3/4
			OnJump();
			m_bWaitTreeProcess=true;
			::SetWindowLongPtr( GetHwnd(), DWLP_MSGRESULT, TRUE );	// ツリーの展開／縮小をしない
			return TRUE;
			//return OnJump();
		case TVN_KEYDOWN:
			if( ((TV_KEYDOWN *)lParam)->wVKey == VK_SPACE ){
				OnJump( false );
				return TRUE;
			}
			Key2Command( ((TV_KEYDOWN *)lParam)->wVKey );
			return TRUE;
		case NM_KILLFOCUS:
			// 2002.02.16 hor Treeのダブルクリックでフォーカス移動できるように 4/4
			if(m_bWaitTreeProcess){
				if(m_pShareData->m_Common.m_sOutline.m_bFunclistSetFocusOnJump){
					::SetFocus( pcEditView->GetHwnd() );
				}
				m_bWaitTreeProcess=false;
			}
			return TRUE;
		}
	}else
	if( hwndList == pnmh->hwndFrom ){
		switch( pnmh->code ){
		case LVN_COLUMNCLICK:
//			MYTRACE( _T("LVN_COLUMNCLICK\n") );
			m_nSortCol =  pnlv->iSubItem;
			if( m_nSortCol == m_nSortColOld ){
				m_bSortDesc = !m_bSortDesc;
			}
			m_nSortColOld = m_nSortCol;
			{
				STypeConfig* type = new STypeConfig();
				CDocTypeManager().GetTypeConfig( CTypeConfig(m_nDocType), *type );
				type->m_nOutlineSortCol = m_nSortCol;
				type->m_bOutlineSortDesc = m_bSortDesc;
				SetTypeConfig( CTypeConfig(m_nDocType), *type );
				delete type;
			}
			//	Apr. 23, 2005 genta 関数として独立させた
			SortListView( hwndList, m_nSortCol );
			return TRUE;
		case NM_CLICK:
			if( IsDocking() ){
				OnJump( false, false );
				return TRUE;
			}
			break;
		case NM_DBLCLK:
				OnJump();
			return TRUE;
		case LVN_KEYDOWN:
			if( ((LV_KEYDOWN *)lParam)->wVKey == VK_SPACE ){
				OnJump( false );
				return TRUE;
			}
			Key2Command( ((LV_KEYDOWN *)lParam)->wVKey );
			return TRUE;
		}
	}

#ifdef DEFINE_SYNCCOLOR
	if( IsDocking() ){
		if( hwndList == pnmh->hwndFrom || hwndTree == pnmh->hwndFrom ){
			if( pnmh->code == NM_CUSTOMDRAW ){
				LPNMCUSTOMDRAW lpnmcd = (LPNMCUSTOMDRAW)lParam;
				switch( lpnmcd->dwDrawStage ){
				case CDDS_PREPAINT:
					::SetWindowLongPtr( GetHwnd(), DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW );
					break;
				case CDDS_ITEMPREPAINT:
					{	// 選択アイテムを反転表示にする
						const STypeConfig	*TypeDataPtr = &(pcEditView->m_pcEditDoc->m_cDocType.GetDocumentAttribute());
						COLORREF clrText = TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cTEXT;
						COLORREF clrTextBk = TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cBACK;
						if( hwndList == pnmh->hwndFrom ){
							//if( lpnmcd->uItemState & CDIS_SELECTED ){	// 非選択のアイテムもすべて CDIS_SELECTED で来る？
							if( ListView_GetItemState( hwndList, lpnmcd->dwItemSpec, LVIS_SELECTED ) ){
								((LPNMLVCUSTOMDRAW)lpnmcd)->clrText = clrText ^ RGB(255, 255, 255);
								((LPNMLVCUSTOMDRAW)lpnmcd)->clrTextBk = clrTextBk ^ RGB(255, 255, 255);
								lpnmcd->uItemState = 0;	// リストビューには選択としての描画をさせないようにする？
							}
						}else{
							if( lpnmcd->uItemState & CDIS_SELECTED ){
								((LPNMTVCUSTOMDRAW)lpnmcd)->clrText = clrText ^ RGB(255, 255, 255);
								((LPNMTVCUSTOMDRAW)lpnmcd)->clrTextBk = clrTextBk ^ RGB(255, 255, 255);
							}
						}
					}
					::SetWindowLongPtr( GetHwnd(), DWLP_MSGRESULT, CDRF_DODEFAULT );
					break;
				}

				return TRUE;
			}
		}
	}
#endif

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
	@date 2010.03.17 syat 桁追加
*/
void CDlgFuncList::SortListView(HWND hwndList, int sortcol)
{
	LV_COLUMN		col;
	int col_no;

	//	Apr. 23, 2005 genta 行番号を左端へ

//	if( sortcol == 1 ){
	{
		col_no = FL_COL_NAME;
		col.mask = LVCF_TEXT;
	// From Here 2001.12.03 hor
	//	col.pszText = _T("関数名 *");
		if(OUTLINE_BOOKMARK == m_nListType){
			col.pszText = const_cast<TCHAR*>( sortcol == col_no ? LS(STR_DLGFNCLST_LIST_TEXT_M) : LS(STR_DLGFNCLST_LIST_TEXT) );
		}else{
			col.pszText = const_cast<TCHAR*>( sortcol == col_no ? LS(STR_DLGFNCLST_LIST_FUNC_M) : LS(STR_DLGFNCLST_LIST_FUNC) );
		}
	// To Here 2001.12.03 hor
		col.iSubItem = 0;
		ListView_SetColumn( hwndList, col_no, &col );

		col_no = FL_COL_ROW;
		col.mask = LVCF_TEXT;
		col.pszText = const_cast<TCHAR*>( sortcol == col_no ? LS(STR_DLGFNCLST_LIST_LINE_M) : LS(STR_DLGFNCLST_LIST_LINE) );
		col.iSubItem = 0;
		ListView_SetColumn( hwndList, col_no, &col );

		// 2010.03.17 syat 桁追加
		col_no = FL_COL_COL;
		col.mask = LVCF_TEXT;
		col.pszText = const_cast<TCHAR*>( sortcol == col_no ? LS(STR_DLGFNCLST_LIST_COL_M) : LS(STR_DLGFNCLST_LIST_COL) );
		col.iSubItem = 0;
		ListView_SetColumn( hwndList, col_no, &col );

		col_no = FL_COL_REMARK;
	// From Here 2001.12.07 hor
		col.mask = LVCF_TEXT;
		col.pszText = const_cast<TCHAR*>( sortcol == col_no ? LS(STR_DLGFNCLST_LIST_M) : _T("") );
		col.iSubItem = 0;
		ListView_SetColumn( hwndList, col_no, &col );
	// To Here 2001.12.07 hor

		ListView_SortItems( hwndList, (m_bSortDesc ? CompareFunc_Desc : CompareFunc_Asc), (LPARAM)this );
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
	// 今のところ CEditWnd::OnSize() からの呼び出しでは lParam は CEditWnd 側 の lParam のまま渡される	// 2010.06.05 ryoji
	RECT rcDlg;
	::GetClientRect( GetHwnd(), &rcDlg );
	lParam = MAKELONG(rcDlg.right - rcDlg.left, rcDlg.bottom -  rcDlg.top);	// 自前で補正

	/* 基底クラスメンバ */
	CDialog::OnSize( wParam, lParam );

	RECT  rc;
	POINT ptNew;
	ptNew.x = rcDlg.right - rcDlg.left;
	ptNew.y = rcDlg.bottom - rcDlg.top;

	for( int i = 0 ; i < _countof(anchorList); i++ ){
		HWND hwndCtrl = GetItemHwnd(anchorList[i].id);
		ResizeItem( hwndCtrl, m_ptDefaultSizeClient, ptNew, m_rcItems[i], anchorList[i].anchor, (anchorList[i].anchor != ANCHOR_ALL));
//	2013.2.6 aroka ちらつき防止用の試行錯誤
		if(anchorList[i].anchor == ANCHOR_ALL){
			::UpdateWindow( hwndCtrl );
		}
	}

//	if( IsDocking() )
	{
		// ダイアログ部分を再描画（ツリー／リストの範囲はちらつかないように除外）
		::InvalidateRect( GetHwnd(), NULL, FALSE );
		POINT pt;
		::GetWindowRect( ::GetDlgItem( GetHwnd(), IDC_TREE_FL ), &rc );
		pt.x = rc.left;
		pt.y = rc.top;
		::ScreenToClient( GetHwnd(), &pt );
		::OffsetRect( &rc, pt.x - rc.left, pt.y - rc.top );
		::ValidateRect( GetHwnd(), &rc );
	}
	return TRUE;
}

BOOL CDlgFuncList::OnMinMaxInfo( LPARAM lParam )
{
	LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;
	if( m_ptDefaultSize.x < 0 ){
		return 0;
	}
	lpmmi->ptMinTrackSize.x = m_ptDefaultSize.x/2;
	lpmmi->ptMinTrackSize.y = m_ptDefaultSize.y/3;
	lpmmi->ptMaxTrackSize.x = m_ptDefaultSize.x*2;
	lpmmi->ptMaxTrackSize.y = m_ptDefaultSize.y*2;
	return 0;
}
static inline int CALLBACK Compare_by_ItemData(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	if( lParam1< lParam2 )
		return -1;
	if( lParam1 > lParam2 )
		return 1;
	else
		return 0;
}

static int CALLBACK Compare_by_ItemDataDesc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	return Compare_by_ItemData(lParam2, lParam1, lParamSort);
}

struct STreeViewSortData{
	std::vector<std::tstring> m_vecText;
};

static int CALLBACK Compare_by_ItemText(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	STreeViewSortData* pData = (STreeViewSortData*)lParamSort;
	std::tstring* pText1 = &pData->m_vecText[lParam1];
	std::tstring* pText2 = &pData->m_vecText[lParam2];
	int result = ::lstrcmpi(pText1->c_str(), pText2->c_str());
	if( result == 0 ){
		// 同じ名前は登録順
		return Compare_by_ItemData(lParam1, lParam2, lParamSort);
	}
	return result;
}

static int CALLBACK Compare_by_ItemTextDesc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	return Compare_by_ItemText(lParam2, lParam1, lParamSort);
}

BOOL CDlgFuncList::OnDestroy( void )
{
	CDialog::OnDestroy();

	/* アウトライン ■位置とサイズを記憶する */ // 20060201 aroka
	// 前提条件：m_lParam が CDialog::OnDestroy でクリアされないこと
	CEditView* pcEditView=(CEditView*)m_lParam;
	HWND hwndEdit = pcEditView->m_pcEditWnd->GetHwnd();
	if( !IsDocking() && m_pShareData->m_Common.m_sOutline.m_bRememberOutlineWindowPos ){
		/* 親のウィンドウ位置・サイズを記憶 */
		WINDOWPLACEMENT cWindowPlacement;
		cWindowPlacement.length = sizeof( cWindowPlacement );
		if (::GetWindowPlacement( hwndEdit, &cWindowPlacement )){
			/* ウィンドウ位置・サイズを記憶 */
			m_pShareData->m_Common.m_sOutline.m_xOutlineWindowPos = m_xPos - cWindowPlacement.rcNormalPosition.left;
			m_pShareData->m_Common.m_sOutline.m_yOutlineWindowPos = m_yPos - cWindowPlacement.rcNormalPosition.top;
			m_pShareData->m_Common.m_sOutline.m_widthOutlineWindow = m_nWidth;
			m_pShareData->m_Common.m_sOutline.m_heightOutlineWindow = m_nHeight;
		}
	}

	// ドッキング画面を閉じるときは画面を再レイアウトする
	// ドッキングでアプリ終了時には hwndEdit は NULL になっている（親に先に WM_DESTROY が送られるため）
	if( IsDocking() && hwndEdit )
		pcEditView->m_pcEditWnd->EndLayoutBars();

	// 明示的にアウトライン画面を閉じたときだけアウトライン表示フラグを OFF にする
	// フローティングでアプリ終了時やタブモードで裏にいる場合は ::IsWindowVisible( hwndEdit ) が FALSE を返す
	if( hwndEdit && ::IsWindowVisible( hwndEdit ) && !m_bInChangeLayout ){	// ChangeLayout() 処理中は設定変更しない
		bool bType = (ProfDockSet() != 0);
		if( bType ){
			CDocTypeManager().GetTypeConfig(CTypeConfig(m_nDocType), m_type);
		}
		ProfDockDisp() = FALSE;
		if( bType ){
			SetTypeConfig( CTypeConfig(m_nDocType), m_type );
		}
		// 他ウィンドウに変更を通知する
		if( ProfDockSync() ){
			PostOutlineNotifyToAllEditors( (WPARAM)0, (LPARAM)hwndEdit );
		}
	}

	if( m_hwndToolTip ){
		::DestroyWindow( m_hwndToolTip );
		m_hwndToolTip = NULL;
	}
	::KillTimer( GetHwnd(), 1 );

	return TRUE;
}


/*!
	@date 2016.03.04 Moca OnCbnSelChange -> OnCbnSelEndOk マウスで一覧から選択中にソートされないように変更
*/
BOOL CDlgFuncList::OnCbnSelEndOk( HWND hwndCtl, int wID )
{
	int nSelect = Combo_GetCurSel( hwndCtl );
	switch(wID)
	{
	case IDC_COMBO_nSortType:
		if( m_nSortType != nSelect )
		{
			m_nSortType = nSelect;
			STypeConfig* type = new STypeConfig();
			CDocTypeManager().GetTypeConfig( CTypeConfig(m_nDocType), *type );
			type->m_nOutlineSortType = m_nSortType;
			SetTypeConfig( CTypeConfig(m_nDocType), *type );
			delete type;
			SortTree(::GetDlgItem( GetHwnd() , IDC_TREE_FL),TVI_ROOT);
		}
		return TRUE;
	}
	return FALSE;

}

static void SortTree_Sub(HWND hWndTree,HTREEITEM htiParent, STreeViewSortData& data, int nSortType)
{
	if( SORTTYPE_ATOZ == nSortType || SORTTYPE_ZTOA == nSortType ){
		for(HTREEITEM htiItem = TreeView_GetChild( hWndTree, htiParent ); NULL != htiItem ; htiItem = TreeView_GetNextSibling( hWndTree, htiItem )){
			TVITEM item;
			item.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
			item.hItem = htiItem;
			std::vector<TCHAR> vecStr;
			if( TreeView_GetItemTextVector(hWndTree, item, vecStr) ){
				data.m_vecText[item.lParam].assign(&vecStr[0]);
			}
		}
	}
	TVSORTCB sort;
	sort.hParent = htiParent;
	switch( nSortType ){
	case SORTTYPE_DEFAULT:
		sort.lpfnCompare = Compare_by_ItemData;
		sort.lParam = 0;
		TreeView_SortChildrenCB(hWndTree , &sort , FALSE);
		// TreeView_SortChildren(hWndTree,htiParent,FALSE);
		break;
	case SORTTYPE_DEFAULT_DESC:
		sort.lpfnCompare = Compare_by_ItemDataDesc;
		sort.lParam = 0;
		TreeView_SortChildrenCB(hWndTree , &sort , FALSE);
		break;
	case SORTTYPE_ATOZ:
		sort.lpfnCompare = Compare_by_ItemText;
		sort.lParam = (LPARAM)&data;
		TreeView_SortChildrenCB(hWndTree , &sort , FALSE);
		break;
	case SORTTYPE_ZTOA:
		sort.lpfnCompare = Compare_by_ItemTextDesc;
		sort.lParam = (LPARAM)&data;
		TreeView_SortChildrenCB(hWndTree , &sort , FALSE);
		break;
	default:
		assert(0);
		break;
	}

	for(HTREEITEM htiItem = TreeView_GetChild( hWndTree, htiParent ); NULL != htiItem ; htiItem = TreeView_GetNextSibling( hWndTree, htiItem )){
		SortTree_Sub(hWndTree, htiItem, data, nSortType);
	}
}



void CDlgFuncList::SortTree(HWND hWndTree,HTREEITEM htiParent)
{
	STreeViewSortData data;
	int size = m_pcFuncInfoArr->GetNum();
	if( m_bDummyLParamMode ){
		size = m_nTreeItemCount;
	}
	data.m_vecText.resize(size);
	::SendMessageAny(hWndTree, WM_SETREDRAW, (WPARAM)FALSE, 0);
	SortTree_Sub(hWndTree, htiParent, data, m_nSortType);
	::SendMessageAny(hWndTree, WM_SETREDRAW, (WPARAM)TRUE, 0);
}



bool CDlgFuncList::TagJumpTimer( const TCHAR* pFile, CMyPoint point, bool bCheckAutoClose )
{
	CEditView* pcView = reinterpret_cast<CEditView*>(m_lParam);

	// ファイルを開いていない場合は自分で開く
	if( pcView->GetDocument()->IsAcceptLoad() ){
		std::wstring strFile = to_wchar(pFile);
		pcView->GetCommander().Command_FILEOPEN( strFile.c_str(), CODE_AUTODETECT, CAppMode::getInstance()->IsViewMode(), NULL );
		if( point.y != -1 ){
			if( pcView->GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
				CLogicPoint pt;
				pt.x = CLogicInt(point.GetX() - 1);
				pt.y = CLogicInt(point.GetY() - 1);
				if( pt.x < 0 ){
					pt.x = 0;
				}
				pcView->GetCommander().Command_MOVECURSOR( pt, 0 );
			}
		}
		return true;
	}
	m_pszTimerJumpFile = pFile;
	m_pointTimerJump = point;
	m_bTimerJumpAutoClose = bCheckAutoClose;
	::SetTimer( GetHwnd(), 2, 200, NULL ); // id == 2
	return false;
}


BOOL CDlgFuncList::OnJump( bool bCheckAutoClose, bool bFileJump )	//2002.02.08 hor 引数追加
{
	int				nLineTo;
	int				nColTo;
	/* ダイアログデータの取得 */
	if( 0 < GetData() && (m_cFuncInfo != NULL || 0 < m_sJumpFile.size() ) ){
		if( m_bModal ){		/* モーダル ダイアログか */
			//モーダル表示する場合は、m_cFuncInfoを取得するアクセサを実装して結果取得すること。
			::EndDialog( GetHwnd(), 1 );
		}else{
			bool bFileJumpSelf = true;
			if( 0 < m_sJumpFile.size() ){
				if( bFileJump ){
					// ファイルツリーの場合
					if( m_bModal ){		/* モーダル ダイアログか */
						//モーダル表示する場合は、m_cFuncInfoを取得するアクセサを実装して結果取得すること。
						::EndDialog( GetHwnd(), 1 );
					}
					CMyPoint poCaret;
					poCaret.x = -1;
					poCaret.y = -1;
					bFileJumpSelf = TagJumpTimer(m_sJumpFile.c_str(), poCaret, bCheckAutoClose);
				}
			}else
			if( m_cFuncInfo != NULL && 0 < m_cFuncInfo->m_cmemFileName.GetStringLength() ){
				if( bFileJump ){
					nLineTo = m_cFuncInfo->m_nFuncLineCRLF;
					nColTo = m_cFuncInfo->m_nFuncColCRLF;
					// 別のファイルへジャンプ
					CMyPoint poCaret; // TagJumpSubも1開始
					poCaret.x = nColTo;
					poCaret.y = nLineTo;
					bFileJumpSelf = TagJumpTimer(m_cFuncInfo->m_cmemFileName.GetStringPtr(), poCaret, bCheckAutoClose);
				}
			}else{
				nLineTo = m_cFuncInfo->m_nFuncLineCRLF;
				nColTo = m_cFuncInfo->m_nFuncColCRLF;
				/* カーソルを移動させる */
				CLogicPoint	poCaret;
				poCaret.x = nColTo - 1;
				poCaret.y = nLineTo - 1;

				m_pShareData->m_sWorkBuffer.m_LogicPoint = poCaret;

				//	2006.07.09 genta 移動時に選択状態を保持するように
				::SendMessageAny( ((CEditView*)m_lParam)->m_pcEditWnd->GetHwnd(),
					MYWM_SETCARETPOS, 0, PM_SETCARETPOS_KEEPSELECT );
			}
			if( bCheckAutoClose && bFileJumpSelf ){
				/* アウトライン ダイアログを自動的に閉じる */
				if( IsDocking() ){
					::PostMessageAny( ((CEditView*)m_lParam)->GetHwnd(), MYWM_SETACTIVEPANE, 0, 0 );
				}
				else if( m_pShareData->m_Common.m_sOutline.m_bAutoCloseDlgFuncList ){
					::DestroyWindow( GetHwnd() );
				}
				else if( m_pShareData->m_Common.m_sOutline.m_bFunclistSetFocusOnJump ){
					::SetFocus( ((CEditView*)m_lParam)->GetHwnd() );
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
// novice 2004/10/10
	/* Shift,Ctrl,Altキーが押されていたか */
	int nIdx = getCtrlKeyState();
	EFunctionCode nFuncCode=CKeyBind::GetFuncCode(
			((WORD)(((BYTE)(KeyCode)) | ((WORD)((BYTE)(nIdx))) << 8)),
			m_pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum,
			m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr
	);
	switch( nFuncCode ){
	case F_REDRAW:
		nFuncCode=GetFuncCodeRedraw(m_nOutlineType);
		/*FALLTHROUGH*/
	case F_OUTLINE:
	case F_OUTLINE_TOGGLE: // 20060201 aroka フォーカスがあるときはリロード
	case F_BOOKMARK_VIEW:
	case F_FILETREE:
		pcEditView=(CEditView*)m_lParam;
		pcEditView->GetCommander().HandleCommand( nFuncCode, true, SHOW_RELOAD, 0, 0, 0 ); // 引数の変更 20060201 aroka

		break;
	case F_BOOKMARK_SET:
		OnJump( false );
		pcEditView=(CEditView*)m_lParam;
		pcEditView->GetCommander().HandleCommand( nFuncCode, true, 0, 0, 0, 0 );

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
void CDlgFuncList::Redraw( int nOutLineType, int nListType, CFuncInfoArr* pcFuncInfoArr, CLayoutInt nCurLine, CLayoutInt nCurCol )
{
	CEditView* pcEditView = (CEditView*)m_lParam;
	m_nDocType = pcEditView->GetDocument()->m_cDocType.GetDocumentType().GetIndex();
	CDocTypeManager().GetTypeConfig(CTypeConfig(m_nDocType), m_type);
	SyncColor();

	m_nOutlineType = nOutLineType;
	m_nListType = nListType;
	m_pcFuncInfoArr = pcFuncInfoArr;	/* 関数情報配列 */
	m_nCurLine = nCurLine;				/* 現在行 */
	m_nCurCol = nCurCol;				/* 現在桁 */

	bool bType = (ProfDockSet() != 0);
	if( bType ){
		m_type.m_nDockOutline = m_nOutlineType;
		SetTypeConfig( CTypeConfig(m_nDocType), m_type );
	}else{
		CommonSet().m_nDockOutline = m_nOutlineType;
	}

	SetData();
}

//ダイアログタイトルの設定
void CDlgFuncList::SetWindowText( const TCHAR* szTitle )
{
	::SetWindowText( GetHwnd(), szTitle );
}

/** 配色適用処理
	@date 2010.06.05 ryoji 新規作成
*/
void CDlgFuncList::SyncColor( void )
{
	if( !IsDocking() )
		return;
#ifdef DEFINE_SYNCCOLOR
	// テキスト色・背景色をビューと同色にする
	CEditView* pcEditView = (CEditView*)m_lParam;
	const STypeConfig	*TypeDataPtr = &(pcEditView->m_pcEditDoc->m_cDocType.GetDocumentAttribute());
	COLORREF clrText = TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cTEXT;
	COLORREF clrBack = TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cBACK;

	HWND hwndTree = ::GetDlgItem( GetHwnd(), IDC_TREE_FL );
	TreeView_SetTextColor( hwndTree, clrText );
	TreeView_SetBkColor( hwndTree, clrBack );
	{
		// WinNT4.0 あたりではウィンドウスタイルを強制的に再設定しないと
		// ツリーアイテムの左側が真っ黒になる
		LONG lStyle = (LONG)GetWindowLongPtr(hwndTree, GWL_STYLE);
		SetWindowLongPtr( hwndTree, GWL_STYLE, lStyle & ~(TVS_HASBUTTONS|TVS_HASLINES|TVS_LINESATROOT) );
		SetWindowLongPtr( hwndTree, GWL_STYLE, lStyle );
	}
	::SetWindowPos( hwndTree, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED );	// なぜかこうしないと四辺１ドット幅分だけ色変更が即時適用されない（←スタイル再設定とは無関係）
	::InvalidateRect( hwndTree, NULL, TRUE );

	HWND hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_FL );
	ListView_SetTextColor( hwndList, clrText );
	ListView_SetTextBkColor( hwndList, clrBack );
	ListView_SetBkColor( hwndList, clrBack );
	::InvalidateRect( hwndList, NULL, TRUE );
#endif
}

/** ドッキング対象矩形の取得（スクリーン座標）
	@date 2010.06.05 ryoji 新規作成
*/
void CDlgFuncList::GetDockSpaceRect( LPRECT pRect )
{
	CEditView* pcEditView = (CEditView*)m_lParam;
	// CDlgFuncList と CSplitterWnd の外接矩形
	// 2014.12.02 ミニマップ対応
	HWND hwnd[3];
	RECT rc[3];
	hwnd[0] = ::GetParent( pcEditView->GetHwnd() );	// CSplitterWnd
	int nCount = 1;
	if( IsDocking() ){
		hwnd[nCount] = GetHwnd();
		nCount++;
	}
	hwnd[nCount] = pcEditView->m_pcEditWnd->GetMiniMap().GetHwnd();
	if( hwnd[nCount] != NULL ){
		nCount++;
	}
	for( int i = 0; i < nCount; i++ ){
		::GetWindowRect(hwnd[i], &rc[i]);
	}
	if( 1 == nCount ){
		*pRect = rc[0];
	}else if( 2 == nCount ){
		::UnionRect(pRect, &rc[0], &rc[1]);
	}else{
		RECT rcTemp;
		::UnionRect(&rcTemp, &rc[0], &rc[1]);
		::UnionRect(pRect, &rcTemp, &rc[2]);
	}
}

/**キャプション矩形取得（スクリーン座標）
	@date 2010.06.05 ryoji 新規作成
*/
void CDlgFuncList::GetCaptionRect( LPRECT pRect )
{
	RECT rc;
	::GetWindowRect( GetHwnd(), &rc );
	EDockSide eDockSide = GetDockSide();
	pRect->left = rc.left + ((eDockSide == DOCKSIDE_RIGHT)? DOCK_SPLITTER_WIDTH: 0);
	pRect->top = rc.top + ((eDockSide == DOCKSIDE_BOTTOM)? DOCK_SPLITTER_WIDTH: 0);
	pRect->right = rc.right - ((eDockSide == DOCKSIDE_LEFT)? DOCK_SPLITTER_WIDTH: 0);
	pRect->bottom = pRect->top + (::GetSystemMetrics( SM_CYSMCAPTION ) + 1);
}

/** キャプション上のボタン矩形取得（スクリーン座標）
	@date 2010.06.05 ryoji 新規作成
*/
bool CDlgFuncList::GetCaptionButtonRect( int nButton, LPRECT pRect )
{
	if( !IsDocking() )
		return false;
	if( nButton >= DOCK_BUTTON_NUM )
		return false;
	GetCaptionRect( pRect );
	::OffsetRect( pRect, 0, 1 );
	int cx = ::GetSystemMetrics( SM_CXSMSIZE );
	pRect->left = pRect->right - cx * (nButton + 1);
	pRect->right = pRect->left + cx;
	pRect->bottom = pRect->top + ::GetSystemMetrics( SM_CYSMSIZE );
	return true;
}

/** 分割バーへのヒットテスト（スクリーン座標）
	@date 2010.06.05 ryoji 新規作成
*/
bool CDlgFuncList::HitTestSplitter( int xPos, int yPos )
{
	if( !IsDocking() )
		return false;

	bool bRet = false;
	RECT rc;
	::GetWindowRect(GetHwnd(), &rc);

	EDockSide eDockSide = GetDockSide();
	switch( eDockSide ){
	case DOCKSIDE_LEFT:		bRet = (rc.right - xPos < DOCK_SPLITTER_WIDTH);		break;
	case DOCKSIDE_TOP:		bRet = (rc.bottom - yPos < DOCK_SPLITTER_WIDTH);	break;
	case DOCKSIDE_RIGHT:	bRet = (xPos - rc.left< DOCK_SPLITTER_WIDTH);		break;
	case DOCKSIDE_BOTTOM:	bRet = (yPos - rc.top < DOCK_SPLITTER_WIDTH);		break;
	}

	return bRet;
}

/** キャプション上のボタンへのヒットテスト（スクリーン座標）
	@date 2010.06.05 ryoji 新規作成
*/
int CDlgFuncList::HitTestCaptionButton( int xPos, int yPos )
{
	if( !IsDocking() )
		return -1;

	POINT pt;
	pt.x = xPos;
	pt.y = yPos;

	RECT rcBtn;
	GetCaptionRect( &rcBtn );
	::OffsetRect( &rcBtn, 0, 1 );
	rcBtn.left = rcBtn.right - ::GetSystemMetrics( SM_CXSMSIZE );
	rcBtn.bottom = rcBtn.top + ::GetSystemMetrics( SM_CYSMSIZE );
	int nBtn = -1;
	for( int i = 0; i < DOCK_BUTTON_NUM; i++ ){
		if( ::PtInRect( &rcBtn, pt ) ){
			nBtn = i;	// 右端から i 番目のボタン上
			break;
		}
		::OffsetRect( &rcBtn, -(rcBtn.right - rcBtn.left), 0 );
	}

	return nBtn;
}

/** WM_NCCALCSIZE 処理
	@date 2010.06.05 ryoji 新規作成
*/
INT_PTR CDlgFuncList::OnNcCalcSize( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsDocking() )
		return 0L;

	// 自ウィンドウのクライアント領域を定義する
	// これでキャプションや分割バーを非クライアント領域にすることができる
	NCCALCSIZE_PARAMS* pNCS = (NCCALCSIZE_PARAMS*)lParam;
	pNCS->rgrc[0].top += (::GetSystemMetrics( SM_CYSMCAPTION ) + 1);
	switch( GetDockSide() ){
	case DOCKSIDE_LEFT:		pNCS->rgrc[0].right -= DOCK_SPLITTER_WIDTH;		break;
	case DOCKSIDE_TOP:		pNCS->rgrc[0].bottom -= DOCK_SPLITTER_WIDTH;	break;
	case DOCKSIDE_RIGHT:	pNCS->rgrc[0].left += DOCK_SPLITTER_WIDTH;		break;
	case DOCKSIDE_BOTTOM:	pNCS->rgrc[0].top += DOCK_SPLITTER_WIDTH;		break;
	}
	return 1L;
}

/** WM_NCHITTEST 処理
	@date 2010.06.05 ryoji 新規作成
*/
INT_PTR CDlgFuncList::OnNcHitTest( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsDocking() )
		return 0L;

	INT_PTR nRet = HTERROR;
	POINT pt;
	pt.x = MAKEPOINTS(lParam).x;
	pt.y = MAKEPOINTS(lParam).y;
	if( HitTestSplitter(pt.x, pt.y) ){
		switch( GetDockSide() ){
		case DOCKSIDE_LEFT:		nRet = HTRIGHT;		break;
		case DOCKSIDE_TOP:		nRet = HTBOTTOM;	break;
		case DOCKSIDE_RIGHT:	nRet = HTLEFT;		break;
		case DOCKSIDE_BOTTOM:	nRet = HTTOP;		break;
		}
	}else {
		RECT rc;
		GetCaptionRect( &rc );
		nRet = ::PtInRect( &rc, pt )? HTCAPTION: HTCLIENT;
	}
	::SetWindowLongPtr( GetHwnd(), DWLP_MSGRESULT, nRet );

	return nRet;
}

/** WM_TIMER 処理
	@date 2010.06.05 ryoji 新規作成
*/
INT_PTR CDlgFuncList::OnTimer( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( wParam == 2 ){
		CEditView* pcView = reinterpret_cast<CEditView*>(m_lParam);
		if( m_pszTimerJumpFile ){
			const TCHAR* pszFile = m_pszTimerJumpFile;
			m_pszTimerJumpFile = NULL;
			bool bSelf = false;
			pcView->TagJumpSub( pszFile, m_pointTimerJump, false, false, &bSelf );
			if( m_bTimerJumpAutoClose ){
				if( IsDocking() ){
					if( bSelf ){
						::PostMessageAny( pcView->GetHwnd(), MYWM_SETACTIVEPANE, 0, 0 );
					}
				}
				else if( m_pShareData->m_Common.m_sOutline.m_bAutoCloseDlgFuncList ){
					::DestroyWindow( GetHwnd() );
				}
				else if( m_pShareData->m_Common.m_sOutline.m_bFunclistSetFocusOnJump ){
					if( bSelf ){
						::SetFocus( pcView->GetHwnd() );
					}
				}
			}
		}
		::KillTimer(hwnd, 2);
		return 0L;
	}else if( wParam == 3 ){
		::KillTimer(hwnd, 3);
		HWND hwndTree = ::GetDlgItem(hwnd, IDC_TREE_FL);
		TreeView_ExpandAll(hwndTree, true, 64);
	}else  if( wParam == 4 ){
		::KillTimer(hwnd, 4);
		HWND hwndTree = ::GetDlgItem(hwnd, IDC_TREE_FL);
		TreeView_ExpandAll(hwndTree, false, 64);
	}

	if( !IsDocking() )
		return 0L;

	if( wParam == 1 ){
		// カーソルがウィンドウ外にある場合にも WM_NCMOUSEMOVE を送る
		POINT pt;
		RECT rc;
		::GetCursorPos( &pt );
		::GetWindowRect( hwnd, &rc );
		if( !::PtInRect( &rc, pt ) ){
			::SendMessageAny( hwnd, WM_NCMOUSEMOVE, 0, MAKELONG( pt.x, pt.y ) );
		}
	}

	return 0L;
}

/** WM_NCMOUSEMOVE 処理
	@date 2010.06.05 ryoji 新規作成
*/
INT_PTR CDlgFuncList::OnNcMouseMove( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsDocking() )
		return 0L;

	POINT pt;
	pt.x = MAKEPOINTS(lParam).x;
	pt.y = MAKEPOINTS(lParam).y;

	// カーソルがウィンドウ内に入ったらタイマー起動
	// ウィンドウ外に出たらタイマー削除
	RECT rc;
	::GetWindowRect( GetHwnd(), &rc );
	bool bHovering = ::PtInRect( &rc, pt )? true: false;
	if( bHovering != m_bHovering )
	{
		m_bHovering = bHovering;
		if( m_bHovering )
			::SetTimer( hwnd, 1, 200, NULL );
		else
			::KillTimer( hwnd, 1 );
	}

	// マウスカーソルがボタン上にあればハイライト
	int nHilightedBtn = HitTestCaptionButton(pt.x, pt.y);
	if( nHilightedBtn != m_nHilightedBtn ){
		// ハイライト状態の変更を反映するために再描画する
		m_nHilightedBtn = nHilightedBtn;
		::RedrawWindow( GetHwnd(), NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOINTERNALPAINT );

		// ツールチップ更新
		TOOLINFO ti;
		::ZeroMemory( &ti, sizeof(ti) );
		ti.cbSize       = CCSIZEOF_STRUCT(TOOLINFO, lpszText);
		ti.hwnd         = GetHwnd();
		ti.hinst        = m_hInstance;
		ti.uId          = (UINT_PTR)GetHwnd();
		switch( m_nHilightedBtn ){
		case 0: ti.lpszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_TIP_CLOSE)); break;
		case 1: ti.lpszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_TIP_WIN)); break;
		case 2: ti.lpszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_TIP_UPDATE)); break;
		default: ti.lpszText = NULL;	// 消す
		}
		Tooltip_UpdateTipText( m_hwndToolTip, &ti );
	}

	return 0L;
}

/** WM_MOUSEMOVE 処理
	@date 2010.06.05 ryoji 新規作成
*/
INT_PTR CDlgFuncList::OnMouseMove( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsDocking() )
		return 0L;

	if( m_bStretching ){	// マウスのドラッグ位置にあわせてサイズを変更する
		POINT pt;
		pt.x = MAKEPOINTS(lParam).x;
		pt.y = MAKEPOINTS(lParam).y;
		::ClientToScreen( GetHwnd(), &pt );

		RECT rc;
		GetDockSpaceRect(&rc);

		// 画面サイズが小さすぎるときは何もしない
		EDockSide eDockSide = GetDockSide();
		if( eDockSide == DOCKSIDE_LEFT || eDockSide == DOCKSIDE_RIGHT ){
			if( rc.right - rc.left < DOCK_MIN_SIZE )
				return 0L;
		}else{
			if( rc.bottom - rc.top < DOCK_MIN_SIZE )
				return 0L;
		}

		// マウスが上下左右に行き過ぎなら範囲内に調整する
		if( pt.x > rc.right - DOCK_MIN_SIZE ) pt.x = rc.right - DOCK_MIN_SIZE;
		if( pt.x < rc.left + DOCK_MIN_SIZE ) pt.x = rc.left + DOCK_MIN_SIZE;
		if( pt.y > rc.bottom - DOCK_MIN_SIZE ) pt.y = rc.bottom - DOCK_MIN_SIZE;
		if( pt.y < rc.top + DOCK_MIN_SIZE ) pt.y = rc.top + DOCK_MIN_SIZE;

		// クライアント座標系に変換して新しい位置とサイズを計算する
		POINT ptLT;
		ptLT.x = rc.left;
		ptLT.y = rc.top;
		::ScreenToClient( m_hwndParent, &ptLT );
		::OffsetRect( &rc, ptLT.x - rc.left, ptLT.y - rc.top );
		::ScreenToClient( m_hwndParent, &pt );
		switch( eDockSide ){
		case DOCKSIDE_LEFT:		rc.right = pt.x - DOCK_SPLITTER_WIDTH / 2 + DOCK_SPLITTER_WIDTH;	break;
		case DOCKSIDE_TOP:		rc.bottom = pt.y - DOCK_SPLITTER_WIDTH / 2 + DOCK_SPLITTER_WIDTH;	break;
		case DOCKSIDE_RIGHT:	rc.left = pt.x - DOCK_SPLITTER_WIDTH / 2;	break;
		case DOCKSIDE_BOTTOM:	rc.top = pt.y - DOCK_SPLITTER_WIDTH / 2;	break;
		}

		// 以前と同じ配置なら無駄に移動しない
		RECT rcOld;
		::GetWindowRect( GetHwnd(), &rcOld );
		ptLT.x = rcOld.left;
		ptLT.y = rcOld.top;
		::ScreenToClient( m_hwndParent, &ptLT );
		::OffsetRect( &rcOld, ptLT.x - rcOld.left, ptLT.y - rcOld.top );
		if( ::EqualRect( &rcOld, &rc ) )
			return 0L;

		// 移動する
		::SetWindowPos( GetHwnd(), NULL,
			rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
			SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE );
		((CEditView*)m_lParam)->m_pcEditWnd->EndLayoutBars( m_bEditWndReady );

		// 移動後の配置情報を記憶する
		GetWindowRect( GetHwnd(), &rc );
		bool bType = (ProfDockSet() != 0);
		if( bType ){
			CDocTypeManager().GetTypeConfig(CTypeConfig(m_nDocType), m_type);
		}
		switch( GetDockSide() ){
		case DOCKSIDE_LEFT:		ProfDockLeft() = rc.right - rc.left;	break;
		case DOCKSIDE_TOP:		ProfDockTop() = rc.bottom - rc.top;		break;
		case DOCKSIDE_RIGHT:	ProfDockRight() = rc.right - rc.left;	break;
		case DOCKSIDE_BOTTOM:	ProfDockBottom() = rc.bottom - rc.top;	break;
		}
		if( bType ){
			SetTypeConfig(CTypeConfig(m_nDocType), m_type);
		}
		return 1L;
	}

	return 0L;
}

/** WM_NCLBUTTONDOWN 処理
	@date 2010.06.05 ryoji 新規作成
*/
INT_PTR CDlgFuncList::OnNcLButtonDown( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	POINT pt;
	pt.x = MAKEPOINTS(lParam).x;
	pt.y = MAKEPOINTS(lParam).y;

	if( !IsDocking() ){
		if( GetDockSide() == DOCKSIDE_FLOAT ){
			if( wParam == HTCAPTION  && !::IsZoomed(GetHwnd()) && !::IsIconic(GetHwnd()) ){
				::SetActiveWindow( GetHwnd() );
				// 上の SetActiveWindow() で WM_ACTIVATEAPP へ行くケースでは、WM_ACTIVATEAPP に入れた特殊処理（エディタ本体を一時的にアクティブ化して戻す）
				// に余計に時間がかかるため、上の SetActiveWindow() 後にはボタンが離されていることがある。その場合は Track() を開始せずに抜ける。
				if( (::GetAsyncKeyState( ::GetSystemMetrics(SM_SWAPBUTTON)? VK_RBUTTON: VK_LBUTTON ) & 0x8000) == 0 )
					return 1L;	// ボタンは既に離されている
				Track( pt );	// タイトルバーのドラッグ＆ドロップによるドッキング配置変更
				return 1L;
			}
		}
		return 0L;
	}

	int nBtn;
	if( HitTestSplitter(pt.x, pt.y) ){	// 分割バー
		m_bStretching = true;
		::SetCapture( GetHwnd() );	// OnMouseMoveでのサイズ制限のために自前のキャプチャが必要
	}else{
		if( (nBtn = HitTestCaptionButton(pt.x, pt.y)) >= 0 ){	// キャプション上のボタン
			if( nBtn == 1 ){	// メニュー
				RECT rcBtn;
				GetCaptionButtonRect( nBtn, &rcBtn );
				pt.x = rcBtn.left;
				pt.y = rcBtn.bottom;
				DoMenu( pt, GetHwnd() );
				// メニュー選択せずにリストやツリーをクリックしたらボタンがハイライトのままになるので更新
				::RedrawWindow( GetHwnd(), NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOINTERNALPAINT );
			}else{
				m_nCapturingBtn = nBtn;
				::SetCapture( GetHwnd() );
			}
		}else{	// 残りはタイトルバーのみ
			Track( pt );	// タイトルバーのドラッグ＆ドロップによるドッキング配置変更
		}
	}

	return 1L;
}

/** WM_LBUTTONUP 処理
	@date 2010.06.05 ryoji 新規作成
*/
INT_PTR CDlgFuncList::OnLButtonUp( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsDocking() )
		return 0L;

	if( m_bStretching ){
		::ReleaseCapture();
		m_bStretching = false;

		if( ProfDockSync() ){
			// 他ウィンドウに変更を通知する
			HWND hwndEdit = ((CEditView*)m_lParam)->m_pcEditWnd->GetHwnd();
			PostOutlineNotifyToAllEditors( (WPARAM)0, (LPARAM)hwndEdit );
		}
		return 1L;
	}

	if( m_nCapturingBtn >= 0 ){
		::ReleaseCapture();
		POINT pt;
		pt.x = MAKEPOINTS(lParam).x;
		pt.y = MAKEPOINTS(lParam).y;
		::ClientToScreen( GetHwnd(), &pt );
		int nBtn = HitTestCaptionButton( pt.x, pt.y);
		if( nBtn == m_nCapturingBtn ){
			if( nBtn == 0 ){	// 閉じる
				::DestroyWindow( GetHwnd() );
			}else if( m_nCapturingBtn == 2 ){	// 更新
				EFunctionCode nFuncCode = GetFuncCodeRedraw(m_nOutlineType);
				CEditView* pcEditView = (CEditView*)m_lParam;
				pcEditView->GetCommander().HandleCommand( nFuncCode, true, SHOW_RELOAD, 0, 0, 0 );
			}
		}
		m_nCapturingBtn = -1;
		return 1L;
	}

	return 0L;
}

/** WM_NCPAINT 処理
	@date 2010.06.05 ryoji 新規作成
*/
INT_PTR CDlgFuncList::OnNcPaint( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsDocking() )
		return 0L;

	EDockSide eDockSide = GetDockSide();

	HDC hdc;
	RECT rc, rcScr, rcWk;

	//描画対象
	hdc = ::GetWindowDC( hwnd );
	CGraphics gr(hdc);
	::GetWindowRect( hwnd, &rcScr );
	rc = rcScr;
	::OffsetRect( &rc, -rcScr.left, -rcScr.top );

	// 背景を描画する
	//::FillRect( gr, &rc, (HBRUSH)(COLOR_3DFACE + 1) );

	// 分割線を描画する
	rcWk = rc;
	switch( eDockSide ){
	case DOCKSIDE_LEFT:		rcWk.left = rcWk.right - DOCK_SPLITTER_WIDTH; break;
	case DOCKSIDE_TOP:		rcWk.top = rcWk.bottom - DOCK_SPLITTER_WIDTH; break;
	case DOCKSIDE_RIGHT:	rcWk.right = rcWk.left + DOCK_SPLITTER_WIDTH; break;
	case DOCKSIDE_BOTTOM:	rcWk.bottom = rcWk.top + DOCK_SPLITTER_WIDTH; break;
	}
	::FillRect( gr, &rcWk, (HBRUSH)(COLOR_3DFACE + 1) );
	::DrawEdge( gr, &rcWk, EDGE_ETCHED, BF_TOPLEFT );

	// タイトルを描画する
	BOOL bThemeActive = CUxTheme::getInstance()->IsThemeActive();
	BOOL bGradient = FALSE;
	::SystemParametersInfo( SPI_GETGRADIENTCAPTIONS, 0, &bGradient, 0 );
	if( !bThemeActive ) bGradient = FALSE;	// 適当に調整
	HWND hwndFocus = ::GetFocus();
	BOOL bActive = (GetHwnd() == hwndFocus || ::IsChild(GetHwnd(), hwndFocus));
	RECT rcCaption;
	GetCaptionRect( &rcCaption );
	::OffsetRect( &rcCaption, -rcScr.left, -rcScr.top );
	rcWk = rcCaption;
	rcWk.top += 1;
	rcWk.right -= DOCK_BUTTON_NUM * (::GetSystemMetrics( SM_CXSMSIZE ));
	// ↓DrawCaption() に DC_SMALLCAP を指定してはいけないっぽい
	// ↓DC_SMALLCAP 指定のものを Win7(64bit版) で動かしてみたら描画位置が下にずれて上半分しか見えなかった（x86ビルド/x64ビルドのどちらも NG）
	::DrawCaption( hwnd, gr, &rcWk, DC_TEXT | (bGradient? DC_GRADIENT: 0) /*| DC_SMALLCAP*/ | (bActive? DC_ACTIVE: 0) );
	rcWk.left = rcCaption.right;
	int nClrCaption;
	if( bGradient )
		nClrCaption = ( bActive? COLOR_GRADIENTACTIVECAPTION: COLOR_GRADIENTINACTIVECAPTION );
	else
		nClrCaption = ( bActive? COLOR_ACTIVECAPTION: COLOR_INACTIVECAPTION );
	::FillRect( gr, &rcWk, ::GetSysColorBrush( nClrCaption ) );
	::DrawEdge( gr, &rcCaption, BDR_SUNKENOUTER, BF_TOP );

	// タイトル上のボタンを描画する
	NONCLIENTMETRICS ncm;
	ncm.cbSize = CCSIZEOF_STRUCT( NONCLIENTMETRICS, lfMessageFont );	// 以前のプラットフォームに WINVER >= 0x0600 で定義される構造体のフルサイズを渡すと失敗する
	::SystemParametersInfo( SPI_GETNONCLIENTMETRICS, ncm.cbSize, (PVOID)&ncm, 0 );
	LOGFONT lf;
	memset( &lf, 0, sizeof(LOGFONT) );
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfHeight = ncm.lfCaptionFont.lfHeight;
	::lstrcpy( lf.lfFaceName, _T("Marlett") );
	HFONT hFont = ::CreateFontIndirect( &lf );
	::lstrcpy( lf.lfFaceName, _T("Wingdings") );
	HFONT hFont2 = ::CreateFontIndirect( &lf );
	gr.SetTextBackTransparent( true );

	static const TCHAR szBtn[DOCK_BUTTON_NUM] = { (TCHAR)0x72/* 閉じる */, (TCHAR)0x36/* メニュー */, (TCHAR)0xFF/* 更新 */ };
	HFONT hFontBtn[DOCK_BUTTON_NUM] = { hFont/* 閉じる */, hFont/* メニュー */, hFont2/* 更新 */ };
	POINT pt;
	::GetCursorPos( &pt );
	pt.x -= rcScr.left;
	pt.y -= rcScr.top;
	RECT rcBtn = rcCaption;
	::OffsetRect( &rcBtn, 0, 1 );
	rcBtn.left = rcBtn.right - ::GetSystemMetrics( SM_CXSMSIZE );
	rcBtn.bottom = rcBtn.top + ::GetSystemMetrics( SM_CYSMSIZE );
	for( int i = 0; i < DOCK_BUTTON_NUM; i++ ){
		int nClrCaptionText;
		// マウスカーソルがボタン上にあればハイライト
		if( ::PtInRect( &rcBtn, pt ) ){
			::FillRect( gr, &rcBtn, ::GetSysColorBrush( (bGradient && !bActive)? COLOR_INACTIVECAPTION: COLOR_ACTIVECAPTION ) );
			nClrCaptionText = ( (bGradient && !bActive)? COLOR_INACTIVECAPTIONTEXT: COLOR_CAPTIONTEXT );
		}else{
			nClrCaptionText = ( bActive? COLOR_CAPTIONTEXT: COLOR_INACTIVECAPTIONTEXT );
		}
		gr.PushMyFont( hFontBtn[i] );
		::SetTextColor( gr, ::GetSysColor( nClrCaptionText ) );
		::DrawText( gr, &szBtn[i], 1, &rcBtn, DT_SINGLELINE | DT_CENTER | DT_VCENTER );
		::OffsetRect( &rcBtn, -(rcBtn.right - rcBtn.left), 0 );
		gr.PopMyFont();
	}

	::DeleteObject( hFont );
	::DeleteObject( hFont2 );

	::ReleaseDC( hwnd, hdc );
	return 1L;
}

/** メニュー処理
	@date 2010.06.05 ryoji 新規作成
*/
void CDlgFuncList::DoMenu( POINT pt, HWND hwndFrom )
{
	// メニューを作成する
	CEditView* pcEditView = &CEditDoc::GetInstance(0)->m_pcEditWnd->GetActiveView();
	CDocTypeManager().GetTypeConfig( CTypeConfig(m_nDocType), m_type );
	EDockSide eDockSide = ProfDockSide();	// 設定上の配置
	UINT uFlags = MF_BYPOSITION | MF_STRING;
	const bool bDropDown = (hwndFrom == GetHwnd()); // true=ドロップダウン, false=右クリック
	HMENU hMenu = ::CreatePopupMenu();
	HMENU hMenuSub = bDropDown ? NULL : ::CreatePopupMenu();
	int iPos = 0;
	int iPosSub = 0;
	HMENU& hMenuRef = bDropDown ? hMenu : hMenuSub;
	int& iPosRef = bDropDown ? iPos : iPosSub;

	if( bDropDown == false ){
		// 将来、ここに hwndFrom に応じた状況依存メニューを追加するといいかも
		// （ツリーなら「すべて展開」／「すべて縮小」とか、そういうの）
		::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_STRING, 450, LS(STR_DLGFNCLST_MENU_UPDATE) );
		int flag = 0;
		if( FALSE == ::IsWindowEnabled( GetItemHwnd(IDC_BUTTON_COPY) ) ){
			flag |= MF_GRAYED;
		}
		::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_STRING | flag, 451, LS(STR_DLGFNCLST_MENU_COPY) );
		if( m_nViewType == VIEWTYPE_TREE ){
			::InsertMenu(hMenu, iPos++, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
			::InsertMenu(hMenu, iPos++, MF_BYPOSITION | MF_STRING, 500, LS(STR_DLGFNCLST_MENU_EXPAND));
			::InsertMenu(hMenu, iPos++, MF_BYPOSITION | MF_STRING, 501, LS(STR_DLGFNCLST_MENU_COLLAPSE));
		}else if( m_nListType == OUTLINE_BOOKMARK ){
			::InsertMenu(hMenu, iPos++, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
			flag = 0;
			HWND hwndList = ::GetDlgItem(GetHwnd(), IDC_LIST_FL);
			if( ListView_GetSelectedCount(hwndList) == 0 ){
				flag |= MF_GRAYED;
			}
			::InsertMenu(hMenu, iPos++, MF_BYPOSITION | MF_STRING | flag, 510, LS(STR_DLGFNCLST_MENU_BOOK_DEL));
			::InsertMenu(hMenu, iPos++, MF_BYPOSITION | MF_STRING, 511, LS(STR_DLGFNCLST_MENU_BOOK_ALL_DEL));
		}
		::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_SEPARATOR, 0,	NULL );
		::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuSub,	LS(STR_DLGFNCLST_MENU_WINPOS) );
	}

	int iFrom = iPosRef;
	::InsertMenu( hMenuRef, iPosRef++, uFlags, 100 + DOCKSIDE_LEFT,       LS(STR_DLGFNCLST_MENU_LEFTDOC) );
	::InsertMenu( hMenuRef, iPosRef++, uFlags, 100 + DOCKSIDE_RIGHT,      LS(STR_DLGFNCLST_MENU_RIGHTDOC) );
	::InsertMenu( hMenuRef, iPosRef++, uFlags, 100 + DOCKSIDE_TOP,        LS(STR_DLGFNCLST_MENU_TOPDOC) );
	::InsertMenu( hMenuRef, iPosRef++, uFlags, 100 + DOCKSIDE_BOTTOM,     LS(STR_DLGFNCLST_MENU_BOTDOC) );
	::InsertMenu( hMenuRef, iPosRef++, uFlags, 100 + DOCKSIDE_FLOAT,      LS(STR_DLGFNCLST_MENU_FLOATING) );
	::InsertMenu( hMenuRef, iPosRef++, uFlags, 100 + DOCKSIDE_UNDOCKABLE, LS(STR_DLGFNCLST_MENU_NODOCK) );
	int iTo = iPosRef - 1;
	for( int i = iFrom; i <= iTo; i++ ){
		if( ::GetMenuItemID( hMenuRef, i ) == (100 + eDockSide) ){
			::CheckMenuRadioItem( hMenuRef, iFrom, iTo, i, MF_BYPOSITION );
			break;
		}
	}
	::InsertMenu( hMenuRef, iPosRef++, MF_BYPOSITION | MF_SEPARATOR, 0,	NULL );
	::InsertMenu( hMenuRef, iPosRef++, uFlags, 200, LS(STR_DLGFNCLST_MENU_SYNC) );
	::CheckMenuItem( hMenuRef, 200, MF_BYCOMMAND | ProfDockSync()? MF_CHECKED: MF_UNCHECKED );
	::InsertMenu( hMenuRef, iPosRef++, MF_BYPOSITION | MF_SEPARATOR, 0,	NULL );
	::InsertMenu( hMenuRef, iPosRef++, MF_BYPOSITION | MF_STRING, 300, LS(STR_DLGFNCLST_MENU_INHERIT) );
	::InsertMenu( hMenuRef, iPosRef++, MF_BYPOSITION | MF_STRING, 301, LS(STR_DLGFNCLST_MENU_TYPE) );
	::CheckMenuRadioItem( hMenuRef, 300, 301, (ProfDockSet() == 0)? 300: 301, MF_BYCOMMAND );
	::InsertMenu( hMenuRef, iPosRef++, MF_BYPOSITION | MF_SEPARATOR, 0,	NULL );
	::InsertMenu( hMenuRef, iPosRef++, MF_BYPOSITION | MF_STRING, 305, LS(STR_DLGFNCLST_MENU_UNIFY) );

	if( bDropDown == false ){
		::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_SEPARATOR, 0,	NULL );
		::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_STRING, 452, LS(STR_DLGFNCLST_MENU_CLOSE) );
	}

	// メニューを表示する
	RECT rcWork;
	GetMonitorWorkRect( pt, &rcWork );	// モニタのワークエリア
	int nId = ::TrackPopupMenu( hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
								( pt.x > rcWork.left )? pt.x: rcWork.left,
								( pt.y < rcWork.bottom )? pt.y: rcWork.bottom,
								0, GetHwnd(), NULL);
	::DestroyMenu( hMenu );	// サブメニューは再帰的に破棄される

	// メニュー選択された状態に切り替える
	EFunctionCode nFuncCode = GetFuncCodeRedraw(m_nOutlineType);
	HWND hwndEdit = pcEditView->m_pcEditWnd->GetHwnd();
	if( nId == 450 ){	// 更新
		CEditView* pcEditView = (CEditView*)m_lParam;
		pcEditView->GetCommander().HandleCommand( nFuncCode, true, SHOW_RELOAD, 0, 0, 0 );
	}
	else if( nId == 451 ){	// コピー
		// Windowsクリップボードにコピー 
		SetClipboardText( GetHwnd(), m_cmemClipText.GetStringPtr(), m_cmemClipText.GetStringLength() );
	}
	else if( nId == 452 ){	// 閉じる
		::DestroyWindow( GetHwnd() );
	}else if( nId == 500 ){	// すべて展開
		::SetTimer(GetHwnd(), 3, 100, NULL);
	}else if( nId == 501 ){	// すべて縮小
		::SetTimer(GetHwnd(), 4, 100, NULL);
	}else if( nId == 510 ){	// ブックマーク削除
		HWND hwndList = ::GetDlgItem(GetHwnd(), IDC_LIST_FL);
		int nItem = ListView_GetNextItem(hwndList, -1, LVNI_ALL | LVNI_SELECTED);
		if( nItem != -1 ){
			LVITEM item;
			item.mask = LVIF_PARAM;
			item.iItem = nItem;
			item.iSubItem = 0;
			ListView_GetItem(hwndList, &item);
			const CFuncInfo* pFuncInfo = m_pcFuncInfoArr->GetAt(item.lParam);
			// FIXME: 行番号があってるとは限らない
			CDocLine* pCDocLine = pcEditView->GetDocument()->m_cDocLineMgr.GetLine(pFuncInfo->m_nFuncLineCRLF - 1);
			if( pCDocLine ){
				CBookmarkSetter cBookmark(pCDocLine);
				cBookmark.SetBookmark(false);
				pcEditView->m_pcEditWnd->Views_Redraw();
			}
		}
		pcEditView->GetCommander().HandleCommand(nFuncCode, true, SHOW_RELOAD, 0, 0, 0);
	}else if( nId == 511 ){	// ブックマークすべて削除
		HWND hwndList = ::GetDlgItem(GetHwnd(), IDC_LIST_FL);
		pcEditView->GetCommander().HandleCommand(F_BOOKMARK_RESET, TRUE, 0, 0, 0, 0);
		pcEditView->GetCommander().HandleCommand(nFuncCode, true, SHOW_RELOAD, 0, 0, 0);
	}
	else if( nId == 300 || nId == 301 ){	// ドッキング配置の継承方法
		ProfDockSet() = nId - 300;
		ChangeLayout( OUTLINE_LAYOUT_FOREGROUND );	// 自分自身への強制変更
		if( ProfDockSync() ){
			PostOutlineNotifyToAllEditors( (WPARAM)0, (LPARAM)hwndEdit );	// 他ウィンドウにドッキング配置変更を通知する
		}
	}
	else if( nId == 305 ){	// 設定コピー
		if( IDOK == ::MYMESSAGEBOX( hwndEdit,
						MB_OKCANCEL | MB_ICONINFORMATION, GSTR_APPNAME,
						LS(STR_DLGFNCLST_UNIFY) ) ){
			CommonSet().m_bOutlineDockDisp = GetHwnd()? TRUE: FALSE;
			CommonSet().m_eOutlineDockSide = GetDockSide();
			if( GetHwnd() ){
				RECT rc;
				GetWindowRect( GetHwnd(), &rc );
				switch( GetDockSide() ){	// 現在のドッキングモード
					case DOCKSIDE_LEFT:		CommonSet().m_cxOutlineDockLeft = rc.right - rc.left;	break;
					case DOCKSIDE_TOP:		CommonSet().m_cyOutlineDockTop = rc.bottom - rc.top;	break;
					case DOCKSIDE_RIGHT:	CommonSet().m_cxOutlineDockRight = rc.right - rc.left;	break;
					case DOCKSIDE_BOTTOM:	CommonSet().m_cyOutlineDockBottom = rc.bottom - rc.top;	break;
				}
			}
			STypeConfig* type = new STypeConfig();
			for( int i = 0; i < GetDllShareData().m_nTypesCount; i++ ){
				CDocTypeManager().GetTypeConfig( CTypeConfig(i), *type );
				type->m_bOutlineDockDisp = CommonSet().m_bOutlineDockDisp;
				type->m_eOutlineDockSide = CommonSet().m_eOutlineDockSide;
				type->m_cxOutlineDockLeft = CommonSet().m_cxOutlineDockLeft;
				type->m_cyOutlineDockTop = CommonSet().m_cyOutlineDockTop;
				type->m_cxOutlineDockRight = CommonSet().m_cxOutlineDockRight;
				type->m_cyOutlineDockBottom = CommonSet().m_cyOutlineDockBottom;
				CDocTypeManager().SetTypeConfig( CTypeConfig(i), *type );
			}
			delete type;
			ChangeLayout( OUTLINE_LAYOUT_FOREGROUND );	// 自分自身への強制変更
			PostOutlineNotifyToAllEditors( (WPARAM)0, (LPARAM)hwndEdit );	// 他ウィンドウにドッキング配置変更を通知する
		}
	}
	else if( nId == 200 ){	// ドッキング配置の同期をとる
		ProfDockSync() = !ProfDockSync();
		ChangeLayout( OUTLINE_LAYOUT_FOREGROUND );	// 自分自身への強制変更
		if( ProfDockSync() ){
			PostOutlineNotifyToAllEditors( (WPARAM)0, (LPARAM)hwndEdit );	// 他ウィンドウにドッキング配置変更を通知する
		}
	}
	else if( nId >= 100 - 1 ){	// ドッキングモード （※ DOCKSIDE_UNDOCKABLE は -1 です） */
		int* pnWidth = NULL;
		int* pnHeight = NULL;
		RECT rc;
		GetDockSpaceRect( &rc );
		eDockSide = EDockSide(nId - 100);	// 新しいドッキングモード
		bool bType = (ProfDockSet() != 0);
		if( bType ){
			CDocTypeManager().GetTypeConfig(CTypeConfig(m_nDocType), m_type);
		}
		if( eDockSide > DOCKSIDE_FLOAT ){
			switch( eDockSide ){
			case DOCKSIDE_LEFT:		pnWidth = &ProfDockLeft();		break;
			case DOCKSIDE_TOP:		pnHeight = &ProfDockTop();		break;
			case DOCKSIDE_RIGHT:	pnWidth = &ProfDockRight();		break;
			case DOCKSIDE_BOTTOM:	pnHeight = &ProfDockBottom();	break;
			}
			if( eDockSide == DOCKSIDE_LEFT || eDockSide == DOCKSIDE_RIGHT ){
				if( *pnWidth == 0 )	// 初回
					*pnWidth = (rc.right - rc.left) / 3;
				if( *pnWidth > rc.right - rc.left - DOCK_MIN_SIZE ) *pnWidth = rc.right - rc.left - DOCK_MIN_SIZE;
				if( *pnWidth < DOCK_MIN_SIZE ) *pnWidth = DOCK_MIN_SIZE;
			}else{
				if( *pnHeight == 0 )	// 初回
					*pnHeight = (rc.bottom - rc.top) / 3;
				if( *pnHeight > rc.bottom - rc.top - DOCK_MIN_SIZE ) *pnHeight = rc.bottom - rc.top - DOCK_MIN_SIZE;
				if( *pnHeight < DOCK_MIN_SIZE ) *pnHeight = DOCK_MIN_SIZE;
			}
		}

		// ドッキング配置変更
		ProfDockDisp() = GetHwnd()? TRUE: FALSE;
		ProfDockSide() = eDockSide;	// 新しいドッキングモードを適用
		if( bType ){
			SetTypeConfig(CTypeConfig(m_nDocType), m_type);
		}
		ChangeLayout( OUTLINE_LAYOUT_FOREGROUND );	// 自分自身への強制変更
		if( ProfDockSync() ){
			PostOutlineNotifyToAllEditors( (WPARAM)0, (LPARAM)hwndEdit );	// 他ウィンドウにドッキング配置変更を通知する
		}
	}
}

/** 現在の設定に応じて表示を刷新する
	@date 2010.06.05 ryoji 新規作成
*/
void CDlgFuncList::Refresh( void )
{
	CEditWnd* pcEditWnd = CEditDoc::GetInstance(0)->m_pcEditWnd;
	BOOL bReloaded = ChangeLayout( OUTLINE_LAYOUT_FILECHANGED );	// 現在設定に従ってアウトライン画面を再配置する
	if( !bReloaded && pcEditWnd->m_cDlgFuncList.GetHwnd() ){
		EOutlineType nOutlineType = GetOutlineTypeRedraw(m_nOutlineType);
		pcEditWnd->GetActiveView().GetCommander().Command_FUNCLIST( SHOW_RELOAD, nOutlineType );	// 開く	※ HandleCommand(F_OUTLINE,...) だと印刷プレビュー状態で実行されないので Command_FUNCLIST()
	}
	if( MyGetAncestor( ::GetForegroundWindow(), GA_ROOTOWNER2 ) == pcEditWnd->GetHwnd() )
		::SetFocus( pcEditWnd->GetActiveView().GetHwnd() );	// フォーカスを戻す
}

/** 現在の設定に応じて配置を変更する（できる限り再解析しない）

	@param nId [in] 動作指定．OUTLINE_LAYOUT_FOREGROUND: 前面用の動作 / OUTLINE_LAYOUT_BACKGROUND: 背後用の動作 / OUTLINE_LAYOUT_FILECHANGED: ファイル切替用の動作（前面だが特殊）
	@retval 解析を実行したかどうか．true: 実行した / false: 実行しなかった

	@date 2010.06.10 ryoji 新規作成
*/
bool CDlgFuncList::ChangeLayout( int nId )
{
	struct SAutoSwitch
	{
		SAutoSwitch( bool* pbSwitch ): m_pbSwitch( pbSwitch ) { *m_pbSwitch = true; }
		~SAutoSwitch() { *m_pbSwitch = false; }
		bool* m_pbSwitch;
	} SAutoSwitch( &m_bInChangeLayout );	// 処理中は m_bInChangeLayout フラグを ON にしておく

	CEditDoc* pDoc = CEditDoc::GetInstance(0);	// 今は非表示かもしれないので (CEditView*)m_lParam は使えない
	m_nDocType = pDoc->m_cDocType.GetDocumentType().GetIndex();
	CDocTypeManager().GetTypeConfig( CTypeConfig(m_nDocType), m_type );

	BOOL bDockDisp = ProfDockDisp();
	EDockSide eDockSideNew = ProfDockSide();

	if( !GetHwnd() ){	// 現在は非表示
		if( bDockDisp ){	// 新設定は表示
			if( eDockSideNew <= DOCKSIDE_FLOAT ){
				if( nId == OUTLINE_LAYOUT_BACKGROUND ) return false;	// 裏ではフローティングは開かない（従来互換）※無理に開くとタブモード時は画面が切り替わってしまう
				if( nId == OUTLINE_LAYOUT_FILECHANGED ) return false;	// ファイル切替ではフローティングは開かない（従来互換）
			}
			// ※ 裏では一時的に Disable 化しておいて開く（タブモードでの不正な画面切り替え抑止）
			CEditView* pcEditView = &pDoc->m_pcEditWnd->GetActiveView();
			if( nId == OUTLINE_LAYOUT_BACKGROUND ) ::EnableWindow( pcEditView->m_pcEditWnd->GetHwnd(), FALSE );
			if( m_nOutlineType == OUTLINE_DEFAULT ){
				bool bType = (ProfDockSet() != 0);
				if( bType ){
					m_nOutlineType = m_type.m_nDockOutline;
					SetTypeConfig( CTypeConfig(m_nDocType), m_type );
				}else{
					m_nOutlineType = CommonSet().m_nDockOutline;
				}
			}
			EOutlineType nOutlineType = GetOutlineTypeRedraw(m_nOutlineType);	// ブックマークかアウトライン解析かは最後に開いていた時の状態を引き継ぐ（初期状態はアウトライン解析）
			pcEditView->GetCommander().Command_FUNCLIST( SHOW_NORMAL, nOutlineType );	// 開く	※ HandleCommand(F_OUTLINE,...) だと印刷プレビュー状態で実行されないので Command_FUNCLIST()
			if( nId == OUTLINE_LAYOUT_BACKGROUND ) ::EnableWindow( pcEditView->m_pcEditWnd->GetHwnd(), TRUE );
			return true;	// 解析した
		}
	}else{	// 現在は表示
		EDockSide eDockSideOld = GetDockSide();

		CEditView* pcEditView = (CEditView*)m_lParam;
		if( !bDockDisp ){	// 新設定は非表示
			if( eDockSideOld <= DOCKSIDE_FLOAT ){	// 現在はフローティング
				if( nId == OUTLINE_LAYOUT_BACKGROUND ) return false;	// 裏ではフローティングは閉じない（従来互換）
				if( nId == OUTLINE_LAYOUT_FILECHANGED && eDockSideNew <= DOCKSIDE_FLOAT ) return false;	// ファイル切替では新設定もフローティングなら再利用（従来互換）
			}
			::DestroyWindow( GetHwnd() );	// 閉じる
			return false;
		}

		// ドッキング⇔フローティング切替では閉じて開く
		if( (eDockSideOld <= DOCKSIDE_FLOAT) != (eDockSideNew <= DOCKSIDE_FLOAT) ){
			::DestroyWindow( GetHwnd() );	// 閉じる
			if( eDockSideNew <= DOCKSIDE_FLOAT ){	// 新設定はフローティング
				m_xPos = m_yPos = -1;	// 画面位置を初期化する
				if( nId == OUTLINE_LAYOUT_BACKGROUND ) return false;	// 裏ではフローティングは開かない（従来互換）※無理に開くとタブモード時は画面が切り替わってしまう
				if( nId == OUTLINE_LAYOUT_FILECHANGED ) return false;	// ファイル切替ではフローティングは開かない（従来互換）
			}
			// ※ 裏では一時的に Disable 化しておいて開く（タブモードでの不正な画面切り替え抑止）
			if( nId == OUTLINE_LAYOUT_BACKGROUND ) ::EnableWindow( pcEditView->m_pcEditWnd->GetHwnd(), FALSE );
			if( m_nOutlineType == OUTLINE_DEFAULT ){
				bool bType = (ProfDockSet() != 0);
				if( bType ){
					m_nOutlineType = m_type.m_nDockOutline;
					SetTypeConfig( CTypeConfig(m_nDocType), m_type );
				}else{
					m_nOutlineType = CommonSet().m_nDockOutline;
				}
			}
			EOutlineType nOutlineType = GetOutlineTypeRedraw(m_nOutlineType);
			pcEditView->GetCommander().Command_FUNCLIST( SHOW_NORMAL, nOutlineType );	// 開く	※ HandleCommand(F_OUTLINE,...) だと印刷プレビュー状態で実行されないので Command_FUNCLIST()
			if( nId == OUTLINE_LAYOUT_BACKGROUND ) ::EnableWindow( pcEditView->m_pcEditWnd->GetHwnd(), TRUE );
			return true;	// 解析した
		}

		// フローティング→フローティングでは配置同期せずに現状維持
		if( eDockSideOld <= DOCKSIDE_FLOAT ){
			m_eDockSide = eDockSideNew;
			return false;
		}

		// ドッキング→ドッキングでは配置同期
		RECT rc;
		POINT ptLT;
		GetDockSpaceRect( &rc );
		ptLT.x = rc.left;
		ptLT.y = rc.top;
		::ScreenToClient( m_hwndParent, &ptLT );
		::OffsetRect( &rc, ptLT.x - rc.left, ptLT.y - rc.top );

		switch( eDockSideNew ){
		case DOCKSIDE_LEFT:		rc.right = rc.left + ProfDockLeft();	break;
		case DOCKSIDE_TOP:		rc.bottom = rc.top + ProfDockTop();		break;
		case DOCKSIDE_RIGHT:	rc.left = rc.right - ProfDockRight();	break;
		case DOCKSIDE_BOTTOM:	rc.top = rc.bottom - ProfDockBottom();	break;
		}

		// 以前と同じ配置なら無駄に移動しない
		RECT rcOld;
		::GetWindowRect( GetHwnd(), &rcOld );
		ptLT.x = rcOld.left;
		ptLT.y = rcOld.top;
		::ScreenToClient( m_hwndParent, &ptLT );
		::OffsetRect( &rcOld, ptLT.x - rcOld.left, ptLT.y - rcOld.top );
		if( eDockSideOld == eDockSideNew && ::EqualRect( &rcOld, &rc ) ){
			::InvalidateRect( GetHwnd(), NULL, TRUE );	// いちおう再描画だけ
			return false;	// 配置変更不要（例：別のファイルタイプからの通知）
		}

		// 移動する
		m_eDockSide = eDockSideNew;	// 自身のドッキング配置の記憶を更新
		::SetWindowPos( GetHwnd(), NULL,
			rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
			SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE | ((eDockSideOld == eDockSideNew)? 0: SWP_FRAMECHANGED) );	// SWP_FRAMECHANGED 指定で WM_NCCALCSIZE（非クライアント領域の再計算）に誘導する
		pcEditView->m_pcEditWnd->EndLayoutBars( m_bEditWndReady );
	}
	return false;
}

/** アウトライン通知(MYWM_OUTLINE_NOTIFY)処理

	wParam: 通知種別
	lParam: 種別毎のパラメータ

	@date 2010.06.07 ryoji 新規作成
*/
void CDlgFuncList::OnOutlineNotify( WPARAM wParam, LPARAM lParam )
{
	CEditDoc* pDoc = CEditDoc::GetInstance(0);	// 今は非表示かもしれないので (CEditView*)m_lParam は使えない
	switch( wParam ){
	case 0:	// 設定変更通知（ドッキングモード or サイズ）, lParam: 通知元の HWND
		if( (HWND)lParam == pDoc->m_pcEditWnd->GetHwnd() )
			return;	// 自分からの通知は無視
		ChangeLayout( OUTLINE_LAYOUT_BACKGROUND );	// アウトライン画面を再配置
		break;
	}
	return;
}

/** 他ウィンドウにアウトライン通知をポストする
	@date 2010.06.10 ryoji 新規作成
*/
BOOL CDlgFuncList::PostOutlineNotifyToAllEditors( WPARAM wParam, LPARAM lParam )
{
	return CAppNodeGroupHandle(0).PostMessageToAllEditors( MYWM_OUTLINE_NOTIFY, (WPARAM)wParam, (LPARAM)lParam, GetHwnd() );
}

void CDlgFuncList::SetTypeConfig( CTypeConfig docType, const STypeConfig& type )
{
	CDocTypeManager().SetTypeConfig(docType, type);
}

/** コンテキストメニュー処理
	@date 2010.06.07 ryoji 新規作成
*/
BOOL CDlgFuncList::OnContextMenu( WPARAM wParam, LPARAM lParam )
{
	// キャプションかリスト／ツリー上ならメニューを表示する
	HWND hwndFrom = (HWND)wParam;
	if( ::SendMessage( GetHwnd(), WM_NCHITTEST, 0, lParam ) == HTCAPTION
			|| hwndFrom == ::GetDlgItem( GetHwnd(), IDC_LIST_FL )
			|| hwndFrom == ::GetDlgItem( GetHwnd(), IDC_TREE_FL )
	){
		POINT pt;
		pt.x = MAKEPOINTS(lParam).x;
		pt.y = MAKEPOINTS(lParam).y;
		if( pt.x == -1 && pt.y == -1 ){	// キーボード（メニューキー や Shift F10）からの呼び出し
			RECT rc;
			::GetWindowRect( hwndFrom, &rc );
			pt.x = rc.left;
			pt.y = rc.top;
		}
		DoMenu( pt, hwndFrom );
		return TRUE;
	}

	return CDialog::OnContextMenu( wParam, lParam );	// その他のコントロール上ではポップアップヘルプを表示する
}

/** タイトルバーのドラッグ＆ドロップでドッキング配置する際の移動先矩形を求める
	@date 2010.06.17 ryoji 新規作成
*/
EDockSide CDlgFuncList::GetDropRect( POINT ptDrag, POINT ptDrop, LPRECT pRect, bool bForceFloat )
{
	struct CDockStretch{
		static int GetIdealStretch( int nStretch, int nMaxStretch )
		{
			if( nStretch == 0 )
				nStretch = nMaxStretch / 3;
			if( nStretch > nMaxStretch - DOCK_MIN_SIZE ) nStretch = nMaxStretch - DOCK_MIN_SIZE;
			if( nStretch < DOCK_MIN_SIZE ) nStretch = DOCK_MIN_SIZE;
			return nStretch;
		}
	};

	// 移動しない矩形を取得する
	RECT rcWnd;
	::GetWindowRect( GetHwnd(), &rcWnd );
	if( IsDocking() && !bForceFloat ){
		if( ::PtInRect( &rcWnd, ptDrop ) ){
			*pRect = rcWnd;
			return GetDockSide();	// 移動しない位置だった
		}
	}

	// ドッキング用の矩形を取得する
	EDockSide eDockSide = DOCKSIDE_FLOAT;	// フローティングに仮決め
	RECT rcDock;
	GetDockSpaceRect( &rcDock );
	if( !bForceFloat && ::PtInRect( &rcDock, ptDrop ) ){
		int cxLeft		= CDockStretch::GetIdealStretch( ProfDockLeft(), rcDock.right - rcDock.left );
		int cyTop		= CDockStretch::GetIdealStretch( ProfDockTop(), rcDock.bottom - rcDock.top );
		int cxRight		= CDockStretch::GetIdealStretch( ProfDockRight(), rcDock.right - rcDock.left );
		int cyBottom	= CDockStretch::GetIdealStretch( ProfDockBottom(), rcDock.bottom - rcDock.top );

		int nDock = ::GetSystemMetrics( SM_CXCURSOR );
		if( ptDrop.x - rcDock.left < nDock ){
			eDockSide = DOCKSIDE_LEFT;
			rcDock.right = rcDock.left + cxLeft;
		}
		else if( rcDock.right - ptDrop.x < nDock ){
			eDockSide = DOCKSIDE_RIGHT;
			rcDock.left = rcDock.right - cxRight;
		}
		else if( ptDrop.y - rcDock.top < nDock ){
			eDockSide = DOCKSIDE_TOP;
			rcDock.bottom = rcDock.top + cyTop;
		}
		else if( rcDock.bottom - ptDrop.y < nDock ){
			eDockSide = DOCKSIDE_BOTTOM;
			rcDock.top = rcDock.bottom - cyBottom;
		}
		if( eDockSide != DOCKSIDE_FLOAT ){
			*pRect = rcDock;
			return eDockSide;	// ドッキング位置だった
		}
	}

	// フローティング用の矩形を取得する
	if( !IsDocking() ){	// フローティング → フローティング
		::OffsetRect( &rcWnd, ptDrop.x - ptDrag.x, ptDrop.y - ptDrag.y );
		*pRect = rcWnd;
	}else{	// ドッキング → フローティング
		int cx, cy;
		RECT rcFloat;
		rcFloat.left = 0;
		rcFloat.top = 0;
		if( m_pShareData->m_Common.m_sOutline.m_bRememberOutlineWindowPos
				&& m_pShareData->m_Common.m_sOutline.m_widthOutlineWindow	// 初期値だと 0 になっている
				&& m_pShareData->m_Common.m_sOutline.m_heightOutlineWindow	// 初期値だと 0 になっている
		){
			// 記憶しているサイズ
			rcFloat.right = m_pShareData->m_Common.m_sOutline.m_widthOutlineWindow;
			rcFloat.bottom = m_pShareData->m_Common.m_sOutline.m_heightOutlineWindow;
			cx = ::GetSystemMetrics( SM_CXMIN );
			cy = ::GetSystemMetrics( SM_CYMIN );
			if( rcFloat.right < cx ) rcFloat.right = cx;
			if( rcFloat.bottom < cy ) rcFloat.bottom = cy;
		}
		else{
			HINSTANCE hInstance2 = CSelectLang::getLangRsrcInstance();
			if ( m_lastRcInstance != hInstance2 ) {
				HRSRC hResInfo = ::FindResource( hInstance2, MAKEINTRESOURCE(IDD_FUNCLIST), RT_DIALOG );
				if( !hResInfo ) return eDockSide;
				HGLOBAL hResData = ::LoadResource( hInstance2, hResInfo );
				if( !hResData ) return eDockSide;
				m_pDlgTemplate = (LPDLGTEMPLATE)::LockResource( hResData );
				if( !m_pDlgTemplate ) return eDockSide;
				m_dwDlgTmpSize = ::SizeofResource( hInstance2, hResInfo );
				// 言語切り替えでリソースがアンロードされていないか確認するためインスタンスを記憶する
				m_lastRcInstance = hInstance2;
			}
			// デフォルトのサイズ（ダイアログテンプレートで決まるサイズ）
			rcFloat.right = m_pDlgTemplate->cx;
			rcFloat.bottom = m_pDlgTemplate->cy;
			::MapDialogRect( GetHwnd(), &rcFloat );
			rcFloat.right += ::GetSystemMetrics( SM_CXDLGFRAME ) * 2;	// ※ Create 時のスタイル変更でサイズ変更不可からサイズ変更可能にしている
			rcFloat.bottom += ::GetSystemMetrics( SM_CYCAPTION ) + ::GetSystemMetrics( SM_CYDLGFRAME ) * 2;
		}
		cy = ::GetSystemMetrics( SM_CYCAPTION );
		::OffsetRect( &rcFloat, ptDrop.x - cy * 2, ptDrop.y - cy / 2 );
		*pRect = rcFloat;
	}

	return DOCKSIDE_FLOAT;	// フローティング位置だった
}

/** タイトルバーのドラッグ＆ドロップでドッキング配置を変更する
	@date 2010.06.17 ryoji 新規作成
*/
BOOL CDlgFuncList::Track( POINT ptDrag )
{
	if( ::GetCapture() )
		return FALSE;

	struct SLockWindowUpdate
	{	// 画面にゴミが残らないように
		SLockWindowUpdate(){ ::LockWindowUpdate( ::GetDesktopWindow() ); }
		~SLockWindowUpdate(){ ::LockWindowUpdate( NULL ); }
	} sLockWindowUpdate;

	const SIZE sizeFull = {8, 8};	// フローティング配置用の枠線の太さ
	const SIZE sizeHalf = {4, 4};	// ドッキング配置用の枠線の太さ
	const SIZE sizeClear = {0, 0};	// 枠線描画しない

	POINT pt;
	RECT rc;
	RECT rcDragLast;
	SIZE sizeLast = sizeClear;
	BOOL bDragging = false;	// まだ本格開始しない
	int cxDragSm = ::GetSystemMetrics( SM_CXDRAG );
	int cyDragSm = ::GetSystemMetrics( SM_CYDRAG );

	::SetCapture( GetHwnd() );	// キャプチャ開始

	while( ::GetCapture() == GetHwnd() )
	{
		MSG msg;
		if (!::GetMessage(&msg, NULL, 0, 0)){
			::PostQuitMessage( (int)msg.wParam );
			break;
		}

		switch (msg.message){
		case WM_MOUSEMOVE:
			::GetCursorPos( &pt );

			bool bStart;
			bStart = false;
			if( !bDragging ){
				// 押した位置からいくらか動いてからドラッグ開始にする
				if( abs(pt.x - ptDrag.x) >= cxDragSm || abs(pt.y - ptDrag.y) >= cyDragSm ){
					bDragging = bStart = true;	// ここから開始
				}
			}
			if( bDragging ){	// ドラッグ中
				// ドロップ先矩形を描画する
				EDockSide eDockSide = GetDropRect( ptDrag, pt, &rc, GetKeyState_Control() );
				SIZE sizeNew = (eDockSide <= DOCKSIDE_FLOAT)? sizeFull: sizeHalf;
				CGraphics::DrawDropRect( &rc, sizeNew, bStart? NULL: &rcDragLast, sizeLast );
				rcDragLast = rc;
				sizeLast = sizeNew;
			}
			break;

		case WM_LBUTTONUP:
			::GetCursorPos( &pt );

			::ReleaseCapture();
			if( bDragging ){
				// ドッキング配置を変更する
				EDockSide eDockSide = GetDropRect( ptDrag, pt, &rc, GetKeyState_Control() );
				CGraphics::DrawDropRect( NULL, sizeClear, &rcDragLast, sizeLast );

				bool bType = (ProfDockSet() != 0);
				if( bType ){
					CDocTypeManager().GetTypeConfig(CTypeConfig(m_nDocType), m_type);
				}
				ProfDockDisp() = GetHwnd()? TRUE: FALSE;
				ProfDockSide() = eDockSide;	// 新しいドッキングモードを適用
				switch( eDockSide ){
				case DOCKSIDE_LEFT:		ProfDockLeft() = rc.right - rc.left;	break;
				case DOCKSIDE_TOP:		ProfDockTop() = rc.bottom - rc.top;		break;
				case DOCKSIDE_RIGHT:	ProfDockRight() = rc.right - rc.left;	break;
				case DOCKSIDE_BOTTOM:	ProfDockBottom() = rc.bottom - rc.top;	break;
				}
				if( bType ){
					SetTypeConfig(CTypeConfig(m_nDocType), m_type);
				}
				ChangeLayout( OUTLINE_LAYOUT_FOREGROUND );	// 自分自身への強制変更
				if( !IsDocking() ){
					::MoveWindow( GetHwnd(), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE );
				}
				if( ProfDockSync() ){
					PostOutlineNotifyToAllEditors( (WPARAM)0, (LPARAM)((CEditView*)m_lParam)->m_pcEditWnd->GetHwnd() );	// 他ウィンドウにドッキング配置変更を通知する
				}
				return TRUE;
			}
			return FALSE;

		case WM_KEYUP:
			if( bDragging ){
				if( msg.wParam == VK_CONTROL ){
					// フローティングを強制するモードを抜ける
					::GetCursorPos( &pt );
					EDockSide eDockSide = GetDropRect( ptDrag, pt, &rc, false );
					SIZE sizeNew = (eDockSide <= DOCKSIDE_FLOAT)? sizeFull: sizeHalf;
					CGraphics::DrawDropRect( &rc, sizeNew, &rcDragLast, sizeLast );
					rcDragLast = rc;
					sizeLast = sizeNew;
				}
			}
			break;

		case WM_KEYDOWN:
			if( bDragging ){
				if( msg.wParam == VK_CONTROL ){
					// フローティングを強制するモードに入る
					::GetCursorPos( &pt );
					GetDropRect( ptDrag, pt, &rc, true );
					CGraphics::DrawDropRect( &rc, sizeFull, &rcDragLast, sizeLast );
					sizeLast = sizeFull;
					rcDragLast = rc;
				}
			}
			if( msg.wParam == VK_ESCAPE ){
				// キャンセル
				::ReleaseCapture();
				if( bDragging )
					CGraphics::DrawDropRect( NULL, sizeClear, &rcDragLast, sizeLast );
				return FALSE;
			}
			break;

		case WM_RBUTTONDOWN:
			// キャンセル
			::ReleaseCapture();
			if( bDragging )
				CGraphics::DrawDropRect( NULL, sizeClear, &rcDragLast, sizeLast );
			return FALSE;

		default:
			::DispatchMessage( &msg );
			break;
		}
	}

	::ReleaseCapture();
	return FALSE;
}
void CDlgFuncList::LoadFileTreeSetting( CFileTreeSetting& data, SFilePath& IniDirPath )
{
	const SFileTree* pFileTree;
	if( ProfDockSet() == 0 ){
		pFileTree = &(CommonSet().m_sFileTree);
		data.m_eFileTreeSettingOrgType = EFileTreeSettingFrom_Common;
	}else{
		CDocTypeManager().GetTypeConfig(CTypeConfig(m_nDocType), m_type);
		pFileTree = &(TypeSet().m_sFileTree);
		data.m_eFileTreeSettingOrgType = EFileTreeSettingFrom_Type;
	}
	data.m_eFileTreeSettingLoadType = data.m_eFileTreeSettingOrgType;
	data.m_bProject = pFileTree->m_bProject;
	data.m_szDefaultProjectIni = pFileTree->m_szProjectIni;
	data.m_szLoadProjectIni = _T("");
	if( data.m_bProject ){
		// 各フォルダのプロジェクトファイル読み込み
		TCHAR szPath[_MAX_PATH];
		::GetLongFileName( _T("."), szPath );
		auto_strcat( szPath, _T("\\") );
		int maxDir = CDlgTagJumpList::CalcMaxUpDirectory( szPath );
		for( int i = 0; i <= maxDir; i++ ){
			CDataProfile cProfile;
			cProfile.SetReadingMode();
			std::tstring strIniFileName;
			strIniFileName += szPath;
			strIniFileName += CommonSet().m_sFileTreeDefIniName;
			if( cProfile.ReadProfile(strIniFileName.c_str()) ){
				CImpExpFileTree::IO_FileTreeIni(cProfile, data.m_aItems);
				data.m_eFileTreeSettingLoadType = EFileTreeSettingFrom_File;
				IniDirPath = szPath;
				CutLastYenFromDirectoryPath( IniDirPath );
				data.m_szLoadProjectIni = strIniFileName.c_str();
				break;
			}
			CDlgTagJumpList::DirUp( szPath );
		}
	}
	if( data.m_szLoadProjectIni[0] == _T('\0') ){
		// デフォルトプロジェクトファイル読み込み
		bool bReadIni = false;
		if( pFileTree->m_szProjectIni[0] != _T('\0') ){
			CDataProfile cProfile;
			cProfile.SetReadingMode();
			const TCHAR* pszIniFileName;
			TCHAR szDir[_MAX_PATH * 2];
			if( _IS_REL_PATH( pFileTree->m_szProjectIni ) ){
				// sakura.iniからの相対パス
				GetInidirOrExedir( szDir, pFileTree->m_szProjectIni );
				pszIniFileName = szDir;
			}else{
				pszIniFileName = pFileTree->m_szProjectIni;
			}
			if( cProfile.ReadProfile(pszIniFileName) ){
				CImpExpFileTree::IO_FileTreeIni(cProfile, data.m_aItems);
				data.m_szLoadProjectIni = pszIniFileName;
				bReadIni = true;
			}
		}
		if( !bReadIni ){
			// 共通設定orタイプ別設定から読み込み
			//m_fileTreeSetting = *pFileTree;
			data.m_aItems.resize( pFileTree->m_nItemCount );
			for( int i = 0; i < pFileTree->m_nItemCount; i++ ){
				data.m_aItems[i] = pFileTree->m_aItems[i];
			}
		}
	}
}

