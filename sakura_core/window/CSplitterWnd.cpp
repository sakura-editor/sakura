/*!	@file
	@brief 分割線ウィンドウクラス

	@author Norio Nakatani
	@date 1998/07/07 新規作成
	@date 2002/2/3 aroka 未使用コード除去
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka, YAZAKI
	Copyright (C) 2003, MIK
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "window/CSplitterWnd.h"
#include "window/CSplitBoxWnd.h"
#include "window/CEditWnd.h"
#include "view/CEditView.h"
#include "outline/CDlgFuncList.h"
#include "env/DLLSHAREDATA.h"


//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
CSplitterWnd::CSplitterWnd()
: CWnd(_T("::CSplitterWnd"))
, m_pcEditWnd(NULL)
, m_nAllSplitRows(1)					/* 分割行数 */
, m_nAllSplitCols(1)					/* 分割桁数 */
, m_nVSplitPos(0)					/* 垂直分割位置 */
, m_nHSplitPos(0)					/* 水平分割位置 */
, m_nChildWndCount(0)
, m_bDragging(0)						/* 分割バーをドラッグ中か */
, m_nDragPosX(0)						/* ドラッグ位置Ｘ */
, m_nDragPosY(0)						/* ドラッグ位置Ｙ */
, m_nActivePane(0)					/* アクティブなペイン 0-3 */
{
	/* 共有データ構造体のアドレスを返す */
	m_pShareData = &GetDllShareData();

	m_hcurOld = NULL;						/* もとのマウスカーソル */

	for( int v=0; v < MAXCOUNTOFVIEW; v++ ){
		m_ChildWndArr[v] = NULL;				/* 子ウィンドウ配列 */
	}
	return;
}




CSplitterWnd::~CSplitterWnd()
{
}




/* 初期化 */
HWND CSplitterWnd::Create( HINSTANCE hInstance, HWND hwndParent, void* pCEditWnd )
{
	LPCTSTR pszClassName = _T("SplitterWndClass");

	/* 初期化 */
	m_pcEditWnd	= pCEditWnd;

	/* ウィンドウクラス作成 */
	ATOM atWork;
	atWork = RegisterWC(
		hInstance,
		NULL,// Handle to the class icon.
		NULL,	//Handle to a small icon
		NULL,// Handle to the class cursor.
		(HBRUSH)NULL,// Handle to the class background brush.
		NULL/*MAKEINTRESOURCE( MYDOCUMENT )*/,// Pointer to a null-terminated 
				//character string that specifies the resource name of the class menu,
				//as the name appears in the resource file.
		pszClassName// Pointer to a null-terminated string or is an atom.
	);
	if( 0 == atWork ){
		ErrorMessage( NULL, LS(STR_ERR_CSPLITTER01) );
	}

	/* 基底クラスメンバ呼び出し */
	return CWnd::Create(
		hwndParent,
		0, // extended window style
		pszClassName,	// Pointer to a null-terminated string or is an atom.
		pszClassName, // pointer to window name
		WS_CHILD | WS_VISIBLE, // window style
		CW_USEDEFAULT, // horizontal position of window
		0, // vertical position of window
		CW_USEDEFAULT, // window width
		0, // window height
		NULL // handle to menu, or child-window identifier
	);
}





/* 子ウィンドウの設定
	@param hwndEditViewArr [in] HWND配列 NULL終端
*/
void CSplitterWnd::SetChildWndArr( HWND* hwndEditViewArr )
{
	int v=0;
	for( ; v < MAXCOUNTOFVIEW && hwndEditViewArr[v]; v++ ){
		m_ChildWndArr[v] = hwndEditViewArr[v];				/* 子ウィンドウ配列 */
	}
	m_nChildWndCount = v;
	// 残りはNULLで埋める
	for( ; v < MAXCOUNTOFVIEW; v++ ){
		m_ChildWndArr[v] = NULL;
	}

	// 2002/05/11 YAZAKI 不要な処理と思われる
	/* ウィンドウの分割 */
//	DoSplit( m_nHSplitPos, m_nVSplitPos );
//	DoSplit( 0, 0 );
	return;
}




/* 分割フレーム描画 */
void CSplitterWnd::DrawFrame( HDC hdc, RECT* prc )
{
	CSplitBoxWnd::Draw3dRect( hdc, prc->left, prc->top, prc->right, prc->bottom,
		::GetSysColor( COLOR_3DSHADOW ),
		::GetSysColor( COLOR_3DHILIGHT )
	);
	CSplitBoxWnd::Draw3dRect( hdc, prc->left + 1, prc->top + 1, prc->right - 2, prc->bottom - 2,
		RGB( 0, 0, 0 ),
		::GetSysColor( COLOR_3DFACE )
	);
	return;
}




/* 分割トラッカーの表示 */
void CSplitterWnd::DrawSplitter( int xPos, int yPos, int bEraseOld )
{
	HDC			hdc;
	HBRUSH		hBrush;
	HBRUSH		hBrushOld;
	RECT		rc;
	RECT		rc2;
	int			nTrackerWidth = 6;

	hdc = ::GetDC( GetHwnd() );
	hBrush = ::CreateSolidBrush( RGB(255,255,255) );
	hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
	::SetROP2( hdc, R2_XORPEN );
	::SetBkMode( hdc, TRANSPARENT );
	::GetClientRect( GetHwnd(), &rc );

	if( bEraseOld ){
		if( m_bDragging & 1 ){	/* 分割バーをドラッグ中か */
			rc2.left = -1;
			rc2.top = m_nDragPosY;
			rc2.right = rc.right;
			rc2.bottom = rc2.top + nTrackerWidth;
			::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );
		}
		if( m_bDragging & 2 ){	/* 分割バーをドラッグ中か */
			rc2.left = m_nDragPosX;
			rc2.top = 0;
			rc2.right = rc2.left + nTrackerWidth;
			rc2.bottom = rc.bottom;
			::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );
		}
	}

	m_nDragPosX = xPos;
	m_nDragPosY = yPos;
	if( m_bDragging & 1 ){	/* 分割バーをドラッグ中か */
		rc2.left = -1;
		rc2.top = m_nDragPosY;
		rc2.right = rc.right;
		rc2.bottom = rc2.top + nTrackerWidth;
		::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );
	}
	if( m_bDragging & 2 ){	/* 分割バーをドラッグ中か */
		rc2.left = m_nDragPosX;
		rc2.top = 0;
		rc2.right = rc2.left + nTrackerWidth;
		rc2.bottom = rc.bottom;
		::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );
	}

	::SelectObject( hdc, hBrushOld );
	::DeleteObject( hBrush );
	::ReleaseDC( GetHwnd(), hdc );
	return;
}




/* 分割バーへのヒットテスト */
int CSplitterWnd::HitTestSplitter( int xPos, int yPos )
{
	int			nFrameWidth = 3;
	int			nMargin = 2;

	if( m_nAllSplitRows == 1 && m_nAllSplitCols == 1 ){
		return 0;
	}else
	if( m_nAllSplitRows == 2 && m_nAllSplitCols == 1 ){
		if( m_nVSplitPos - nMargin < yPos && yPos < m_nVSplitPos + nFrameWidth + nMargin ){
			return 1;
		}else{
			return 0;
		}
	}else
	if( m_nAllSplitRows == 1 && m_nAllSplitCols == 2 ){
		if( m_nHSplitPos - nMargin < xPos && xPos < m_nHSplitPos + nFrameWidth + nMargin ){
			return 2;
		}else{
			return 0;
		}
	}else{
		if( m_nVSplitPos - nMargin < yPos && yPos < m_nVSplitPos + nFrameWidth + nMargin &&
			m_nHSplitPos - nMargin < xPos && xPos < m_nHSplitPos + nFrameWidth + nMargin ){
			return 3;
		}else
		if( m_nVSplitPos - nMargin < yPos && yPos < m_nVSplitPos + nFrameWidth + nMargin ){
			return 1;
		}else
		if( m_nHSplitPos - nMargin < xPos && xPos < m_nHSplitPos + nFrameWidth + nMargin ){
			return 2;
		}else{
			return 0;
		}
	}
}

/*! ウィンドウの分割
	@param nHorizontal 水平クライアント座標 1以上で分割 0:分割しない  -1: 前の設定を保持
	@param nVertical   垂直クライアント座標 1以上で分割 0:分割しない  -1: 前の設定を保持
*/
void CSplitterWnd::DoSplit( int nHorizontal, int nVertical )
{
	int					nActivePane;
	int					nLimit = 32;
	RECT				rc;
	int					nAllSplitRowsOld = m_nAllSplitRows;	/* 分割行数 */
	int					nAllSplitColsOld = m_nAllSplitCols;	/* 分割桁数 */
	CEditView*			pcViewArr[MAXCOUNTOFVIEW];
//	int					i;
	BOOL				bVUp;
	BOOL				bHUp;
	BOOL				bSizeBox;
	CEditWnd*			pCEditWnd = (CEditWnd*)m_pcEditWnd;
	bVUp = FALSE;
	bHUp = FALSE;

	if( -1 == nHorizontal && -1 == nVertical ){
		nVertical = m_nVSplitPos;		/* 垂直分割位置 */
		nHorizontal = m_nHSplitPos;		/* 水平分割位置 */
	}

	if( 0 != nVertical || 0 != nHorizontal ){
		// 分割指示。まだ未作成なら2つ目以降のビューを作成します
		// 今のところは分割数に関係なく4つまで一度に作ります。
		pCEditWnd->CreateEditViewBySplit(2*2);
	}
	/*
	|| ファンクションキーを下に表示している場合はサイズボックスを表示しない
	|| ステータスパーを表示している場合はサイズボックスを表示しない
	*/
	if( NULL == pCEditWnd
	 ||( NULL != pCEditWnd->m_cFuncKeyWnd.GetHwnd()
	  && 1 == m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_Place	/* ファンクションキー表示位置／0:上 1:下 */
	  )
	){
		bSizeBox = FALSE;
	}else if( NULL != pCEditWnd->m_cTabWnd.GetHwnd()
		&& m_pShareData->m_Common.m_sTabBar.m_eTabPosition == TabPosition_Bottom ) {
		bSizeBox = FALSE;
	}else{
		bSizeBox = TRUE;
		/* ステータスパーを表示している場合はサイズボックスを表示しない */
		if( NULL != pCEditWnd->m_cStatusBar.GetStatusHwnd() ){
			bSizeBox = FALSE;
		}
	}
	if( NULL != pCEditWnd->m_cDlgFuncList.GetHwnd() ){
		EDockSide eDockSideFL = pCEditWnd->m_cDlgFuncList.GetDockSide();
		if( eDockSideFL == DOCKSIDE_RIGHT || eDockSideFL == DOCKSIDE_BOTTOM ){
			bSizeBox = FALSE;
		}
	}
	if( pCEditWnd->GetMiniMap().GetHwnd() ){
		bSizeBox = FALSE;
	}

	/* メインウィンドウが最大化されている場合はサイズボックスを表示しない */
	WINDOWPLACEMENT	wp;
	wp.length = sizeof( wp );
	::GetWindowPlacement( GetParentHwnd(), &wp );
	if( SW_SHOWMAXIMIZED == wp.showCmd ){
		bSizeBox = FALSE;
	}

	int v;
	for( v=0; v < m_nChildWndCount; v++ ){
		pcViewArr[v] = ( CEditView* )::GetWindowLongPtr( m_ChildWndArr[v], 0 );
	}
	::GetClientRect( GetHwnd(), &rc );
	if( nHorizontal < nLimit ){
		if( nHorizontal > 0 ){
			bHUp = TRUE;
		}
		nHorizontal = 0;
	}
	if( nHorizontal > rc.right - nLimit * 2 ){
		nHorizontal = 0;
	}
	if( nVertical < nLimit ){
		if( nVertical > 0 ){
			bVUp = TRUE;
		}
		nVertical = 0;
	}
	if( nVertical > rc.bottom - nLimit * 2 ){
		nVertical = 0;
	}
	m_nVSplitPos = nVertical;		/* 垂直分割位置 */
	m_nHSplitPos = nHorizontal;		/* 水平分割位置 */

	if( nVertical == 0 && nHorizontal == 0 ){
		m_nAllSplitRows = 1;	/* 分割行数 */
		m_nAllSplitCols = 1;	/* 分割桁数 */
		if( m_ChildWndArr[0] != NULL ) ::ShowWindow( m_ChildWndArr[0], SW_SHOW );
		if( m_ChildWndArr[1] != NULL ) ::ShowWindow( m_ChildWndArr[1], SW_HIDE );
		if( m_ChildWndArr[2] != NULL ) ::ShowWindow( m_ChildWndArr[2], SW_HIDE );
		if( m_ChildWndArr[3] != NULL ) ::ShowWindow( m_ChildWndArr[3], SW_HIDE );

		if( NULL != pcViewArr[0] ) pcViewArr[0]->SplitBoxOnOff( TRUE, TRUE, bSizeBox );		/* 縦・横の分割ボックスのＯＮ／ＯＦＦ */
//		if( NULL != pcViewArr[1] ) pcViewArr[1]->SplitBoxOnOff( FALSE, FALSE, FALSE );	/* 縦・横の分割ボックスのＯＮ／ＯＦＦ */
//		if( NULL != pcViewArr[2] ) pcViewArr[2]->SplitBoxOnOff( FALSE, FALSE, FALSE );	/* 縦・横の分割ボックスのＯＮ／ＯＦＦ */
//		if( NULL != pcViewArr[3] ) pcViewArr[3]->SplitBoxOnOff( FALSE, FALSE, FALSE );	/* 縦・横の分割ボックスのＯＮ／ＯＦＦ */

		OnSize( 0, 0, 0, 0 );

		if( nAllSplitRowsOld == 1 && nAllSplitColsOld == 1 ){
		}else
		if( nAllSplitRowsOld > 1 && nAllSplitColsOld == 1 ){
			if( bVUp ){
				/* ペインの表示状態を他のビューにコピー */
				if( NULL != pcViewArr[2] && NULL != pcViewArr[0] ){
					pcViewArr[2]->CopyViewStatus( pcViewArr[0] );
				}
			}else{
				/* ペインの表示状態を他のビューにコピー */
				if( m_nActivePane != 0 &&
					NULL != pcViewArr[m_nActivePane] && NULL != pcViewArr[0] ){
					pcViewArr[m_nActivePane]->CopyViewStatus( pcViewArr[0] );
				}
			}
		}else
		if( nAllSplitRowsOld == 1 && nAllSplitColsOld > 1 ){
			if( bHUp ){
				/* ペインの表示状態を他のビューにコピー */
				if( NULL != pcViewArr[1] && NULL != pcViewArr[0] ){
					pcViewArr[1]->CopyViewStatus( pcViewArr[0] );
				}
			}else{
				/* ペインの表示状態を他のビューにコピー */
				if( m_nActivePane != 0 &&
					NULL != pcViewArr[m_nActivePane] && NULL != pcViewArr[0] ){
					pcViewArr[m_nActivePane]->CopyViewStatus( pcViewArr[0] );
				}
			}
		}else{
			if( !bVUp && !bHUp ){
				/* ペインの表示状態を他のビューにコピー */
				if( m_nActivePane != 0 &&
					NULL != pcViewArr[m_nActivePane] && NULL != pcViewArr[0] ){
					pcViewArr[m_nActivePane]->CopyViewStatus( pcViewArr[0] );
				}
			}else
			if( bVUp && !bHUp ){
				/* ペインの表示状態を他のビューにコピー */
				if( NULL != pcViewArr[2] && NULL != pcViewArr[0] ){
					pcViewArr[2]->CopyViewStatus( pcViewArr[0] );
				}
			}else
			if( !bVUp && bHUp ){
				/* ペインの表示状態を他のビューにコピー */
				if( NULL != pcViewArr[1] && NULL != pcViewArr[0] ){
					pcViewArr[1]->CopyViewStatus( pcViewArr[0] );
				}
			}else{
				/* ペインの表示状態を他のビューにコピー */
				if( NULL != pcViewArr[3] && NULL != pcViewArr[0] ){
					pcViewArr[3]->CopyViewStatus( pcViewArr[0] );
				}
			}
		}
		nActivePane = 0;
	}else
	if( nVertical > 0 &&  nHorizontal == 0 ){
		m_nAllSplitRows = 2;	/* 分割行数 */
		m_nAllSplitCols = 1;	/* 分割桁数 */

		if( m_ChildWndArr[0] != NULL ) ::ShowWindow( m_ChildWndArr[0], SW_SHOW );
		if( m_ChildWndArr[1] != NULL ) ::ShowWindow( m_ChildWndArr[1], SW_HIDE );
		if( m_ChildWndArr[2] != NULL ) ::ShowWindow( m_ChildWndArr[2], SW_SHOW );
		if( m_ChildWndArr[3] != NULL ) ::ShowWindow( m_ChildWndArr[3], SW_HIDE );
		if( NULL != pcViewArr[0] ) pcViewArr[0]->SplitBoxOnOff( FALSE, FALSE, FALSE );	/* 縦・横の分割ボックスのＯＮ／ＯＦＦ */
//		if( NULL != pcViewArr[1] ) pcViewArr[1]->SplitBoxOnOff( FALSE, FALSE, FALSE );	/* 縦・横の分割ボックスのＯＮ／ＯＦＦ */
		if( NULL != pcViewArr[2] ) pcViewArr[2]->SplitBoxOnOff( FALSE, TRUE, bSizeBox );	/* 縦・横の分割ボックスのＯＮ／ＯＦＦ */
//		if( NULL != pcViewArr[3] ) pcViewArr[3]->SplitBoxOnOff( FALSE, FALSE, FALSE );	/* 縦・横の分割ボックスのＯＮ／ＯＦＦ */

		OnSize( 0, 0, 0, 0 );

		if( nAllSplitRowsOld == 1 && nAllSplitColsOld == 1 ){
			/* 上下に分割したとき */
			/* ペインの表示状態を他のビューにコピー */
			if( NULL != pcViewArr[0] && NULL != pcViewArr[2] ){
				pcViewArr[0]->CopyViewStatus( pcViewArr[2] );
			}
			// YAZAKI
			pcViewArr[2]->GetTextArea().SetViewTopLine( pcViewArr[0]->GetTextArea().GetViewTopLine() + pcViewArr[0]->GetTextArea().m_nViewRowNum );
		}
		else if( nAllSplitRowsOld > 1 && nAllSplitColsOld == 1 ){
		}
		else if( nAllSplitRowsOld == 1 && nAllSplitColsOld > 1 ){
		}
		else{
			if( bHUp ){
				/* ペインの表示状態を他のビューにコピー */
				if( NULL != pcViewArr[1] && NULL != pcViewArr[0] ){
					pcViewArr[1]->CopyViewStatus( pcViewArr[0] );
				}
				/* ペインの表示状態を他のビューにコピー */
				if( NULL != pcViewArr[3] && NULL != pcViewArr[2] ){
					pcViewArr[3]->CopyViewStatus( pcViewArr[2] );
				}
			}else{
				/* ペインの表示状態を他のビューにコピー */
				if( m_nActivePane != 0 &&
					m_nActivePane != 2 &&
					NULL != pcViewArr[0] &&
					NULL != pcViewArr[1] &&
					NULL != pcViewArr[2] &&
					NULL != pcViewArr[3]
				){
					pcViewArr[1]->CopyViewStatus( pcViewArr[0] );
					pcViewArr[3]->CopyViewStatus( pcViewArr[2] );
				}
			}
		}
		if( m_nActivePane == 0 || m_nActivePane == 1 ){
			// 2007.10.01 ryoji
			// 分割無しからの切替時のみ従来コードを実行してアクティブペインを決める。
			// それ以外の場合はペイン0をアクティブにする。
			// 従来は、上下に分割しておいて、
			// ・上下分割バーを動かす
			// ・ステータスバーなど各種バーの表示／非表示を切り替える
			// ・設定画面をOKで閉じる
			// ・左右も分割して左右分割を解除する
			// といった操作をするだけで下のペインがアクティブ化されることがあった。
			// （シンプルに0固定にしてしまっても良い気はするけれど．．．）
			nActivePane = 0;
			if( nAllSplitRowsOld == 1 && nAllSplitColsOld == 1 ){
				if ( pcViewArr[2]->GetTextArea().GetViewTopLine() < pcViewArr[2]->GetCaret().GetCaretLayoutPos().y ){
					nActivePane = 2;
				}
				else{
					nActivePane = 0;
				}
			}
		}
		else{
			nActivePane = 2;
		}
	}
	else if( nVertical == 0 &&  nHorizontal > 0 ){
		m_nAllSplitRows = 1;	/* 分割行数 */
		m_nAllSplitCols = 2;	/* 分割桁数 */

		if( m_ChildWndArr[0] != NULL ) ::ShowWindow( m_ChildWndArr[0], SW_SHOW );
		if( m_ChildWndArr[1] != NULL ) ::ShowWindow( m_ChildWndArr[1], SW_SHOW );
		if( m_ChildWndArr[2] != NULL ) ::ShowWindow( m_ChildWndArr[2], SW_HIDE );
		if( m_ChildWndArr[3] != NULL ) ::ShowWindow( m_ChildWndArr[3], SW_HIDE );
		if( NULL != pcViewArr[0] ) pcViewArr[0]->SplitBoxOnOff( FALSE, FALSE, FALSE );	/* 縦・横の分割ボックスのＯＮ／ＯＦＦ */
		if( NULL != pcViewArr[1] ) pcViewArr[1]->SplitBoxOnOff( TRUE, FALSE, bSizeBox );	/* 縦・横の分割ボックスのＯＮ／ＯＦＦ */
//		if( NULL != pcViewArr[2] ) pcViewArr[2]->SplitBoxOnOff( FALSE, FALSE );	/* 縦・横の分割ボックスのＯＮ／ＯＦＦ */
//		if( NULL != pcViewArr[3] ) pcViewArr[3]->SplitBoxOnOff( FALSE, FALSE );	/* 縦・横の分割ボックスのＯＮ／ＯＦＦ */

		OnSize( 0, 0, 0, 0 );

		if( nAllSplitRowsOld == 1 && nAllSplitColsOld == 1 ){
			/* ペインの表示状態を他のビューにコピー */
			if( NULL != pcViewArr[0] && NULL != pcViewArr[1] ){
				pcViewArr[0]->CopyViewStatus( pcViewArr[1] );
			}
		}else
		if( nAllSplitRowsOld > 1 && nAllSplitColsOld == 1 ){
		}else
		if( nAllSplitRowsOld == 1 && nAllSplitColsOld > 1 ){
		}else{
			if( bVUp ){
				/* ペインの表示状態を他のビューにコピー */
				if( NULL != pcViewArr[2] && NULL != pcViewArr[0] ){
					pcViewArr[2]->CopyViewStatus( pcViewArr[0] );
				}
				/* ペインの表示状態を他のビューにコピー */
				if( NULL != pcViewArr[3] && NULL != pcViewArr[1] ){
					pcViewArr[3]->CopyViewStatus( pcViewArr[1] );
				}
			}else{
				/* ペインの表示状態を他のビューにコピー */
				if( m_nActivePane != 0 &&
					m_nActivePane != 1 &&
					NULL != pcViewArr[0] &&
					NULL != pcViewArr[1] &&
					NULL != pcViewArr[2] &&
					NULL != pcViewArr[3]
				){
					pcViewArr[2]->CopyViewStatus( pcViewArr[0] );
					pcViewArr[3]->CopyViewStatus( pcViewArr[1] );
				}
			}
		}
		if( m_nActivePane == 0 || m_nActivePane == 2 ){
			nActivePane = 0;
		}else{
			nActivePane = 1;
		}
	}else{
		m_nAllSplitRows = 2;	/* 分割行数 */
		m_nAllSplitCols = 2;	/* 分割桁数 */
		if( m_ChildWndArr[0] != NULL ){ ::ShowWindow( m_ChildWndArr[0], SW_SHOW );}
		if( m_ChildWndArr[1] != NULL ){ ::ShowWindow( m_ChildWndArr[1], SW_SHOW );}
		if( m_ChildWndArr[2] != NULL ){ ::ShowWindow( m_ChildWndArr[2], SW_SHOW );}
		if( m_ChildWndArr[3] != NULL ){ ::ShowWindow( m_ChildWndArr[3], SW_SHOW );}
		if( NULL != pcViewArr[0] ){ pcViewArr[0]->SplitBoxOnOff( FALSE, FALSE, FALSE );}	/* 縦・横の分割ボックスのＯＮ／ＯＦＦ */
		if( NULL != pcViewArr[1] ){ pcViewArr[1]->SplitBoxOnOff( FALSE, FALSE, FALSE );}	/* 縦・横の分割ボックスのＯＮ／ＯＦＦ */
		if( NULL != pcViewArr[2] ){ pcViewArr[2]->SplitBoxOnOff( FALSE, FALSE, FALSE );}	/* 縦・横の分割ボックスのＯＮ／ＯＦＦ */
		if( NULL != pcViewArr[3] ){ pcViewArr[3]->SplitBoxOnOff( FALSE, FALSE, bSizeBox );}	/* 縦・横の分割ボックスのＯＮ／ＯＦＦ */

		OnSize( 0, 0, 0, 0 );

		if( nAllSplitRowsOld == 1 && nAllSplitColsOld == 1 ){
			/* ペインの表示状態を他のビューにコピー */
			if( NULL != pcViewArr[0] && NULL != pcViewArr[1] ){
				pcViewArr[0]->CopyViewStatus( pcViewArr[1] );
			}
			/* ペインの表示状態を他のビューにコピー */
			if( NULL != pcViewArr[0] && NULL != pcViewArr[2] ){
				pcViewArr[0]->CopyViewStatus( pcViewArr[2] );
			}
			/* ペインの表示状態を他のビューにコピー */
			if( NULL != pcViewArr[0] && NULL != pcViewArr[3] ){
				pcViewArr[0]->CopyViewStatus( pcViewArr[3] );
			}
		}else
		if( nAllSplitRowsOld > 1 && nAllSplitColsOld == 1 ){
			/* ペインの表示状態を他のビューにコピー */
			if( NULL != pcViewArr[0] && NULL != pcViewArr[1] ){
				pcViewArr[0]->CopyViewStatus( pcViewArr[1] );
			}
			/* ペインの表示状態を他のビューにコピー */
			if( NULL != pcViewArr[2] && NULL != pcViewArr[3] ){
				pcViewArr[2]->CopyViewStatus( pcViewArr[3] );
			}
		}else
		if( nAllSplitRowsOld == 1 && nAllSplitColsOld > 1 ){
			/* ペインの表示状態を他のビューにコピー */
			if( NULL != pcViewArr[0] && NULL != pcViewArr[2] ){
				pcViewArr[0]->CopyViewStatus( pcViewArr[2] );
			}
			/* ペインの表示状態を他のビューにコピー */
			if( NULL != pcViewArr[1] && NULL != pcViewArr[3] ){
				pcViewArr[1]->CopyViewStatus( pcViewArr[3] );
			}
		}else{
		}
		nActivePane = m_nActivePane;
	}
	OnSize( 0, 0, 0, 0 );

	/* アクティブになったことをペインに通知 */
	if( m_ChildWndArr[nActivePane] != NULL ){
		::PostMessageAny( m_ChildWndArr[nActivePane], MYWM_SETACTIVEPANE, 0, 0 );
	}

	return;
}

/* アクティブペインの設定 */
void CSplitterWnd::SetActivePane( int nIndex )
{
	assert( nIndex < MAXCOUNTOFVIEW );
	m_nActivePane = nIndex;
	return;
}


/* 縦分割ＯＮ／ＯＦＦ */
void CSplitterWnd::VSplitOnOff( void )
{
	RECT		rc;
	::GetClientRect( GetHwnd(), &rc );

	if( m_nAllSplitRows == 1 && m_nAllSplitCols == 1 ){
		DoSplit( 0, rc.bottom / 2 );
	}else
	if( m_nAllSplitRows == 1 && m_nAllSplitCols > 1 ){
		DoSplit( m_nHSplitPos, rc.bottom / 2 );
	}else
	if( m_nAllSplitRows > 1 && m_nAllSplitCols == 1 ){
		DoSplit( 0, 0 );
	}else{
		DoSplit( m_nHSplitPos, 0 );
	}
	return;
}



/* 横分割ＯＮ／ＯＦＦ */
void CSplitterWnd::HSplitOnOff( void )
{
	RECT		rc;
	::GetClientRect( GetHwnd(), &rc );

	if( m_nAllSplitRows == 1 && m_nAllSplitCols == 1 ){
		DoSplit( rc.right / 2, 0 );
	}else
	if( m_nAllSplitRows == 1 && m_nAllSplitCols > 1 ){
		DoSplit( 0, 0 );
	}else
	if( m_nAllSplitRows > 1 && m_nAllSplitCols == 1 ){
		DoSplit( rc.right / 2 , m_nVSplitPos );
	}else{
		DoSplit( 0, m_nVSplitPos );
	}
	return;
}



/* 縦横分割ＯＮ／ＯＦＦ */
void CSplitterWnd::VHSplitOnOff( void )
{
	int		nX;
	int		nY;
	RECT	rc;
	::GetClientRect( GetHwnd(), &rc );

	if( m_nAllSplitRows > 1 && m_nAllSplitCols > 1 ){
		nX = 0;
		nY = 0;
	}else{
		if( m_nAllSplitRows == 1){
			nY = rc.bottom / 2;
		}else{
			nY = m_nVSplitPos;
		}
		if( m_nAllSplitCols == 1 ){
			nX = rc.right / 2;
		}else{
			nX = m_nHSplitPos;
		}
	}
	DoSplit( nX, nY );

	return;
}


/* 前のペインを返す */
int CSplitterWnd::GetPrevPane( void )
{
	int		nPane;
	nPane = -1;
	if( m_nAllSplitRows == 1 &&	m_nAllSplitCols == 1 ){
		nPane = -1;
	}else
	if( m_nAllSplitRows == 2 &&	m_nAllSplitCols == 1 ){
		switch( m_nActivePane ){
		case 0:
			nPane = -1;
			break;
		case 2:
			nPane = 0;
			break;
		}
	}else
	if( m_nAllSplitRows == 1 &&	m_nAllSplitCols == 2 ){
		switch( m_nActivePane ){
		case 0:
			nPane = -1;
			break;
		case 1:
			nPane = 0;
			break;
		}
	}else{
		switch( m_nActivePane ){
		case 0:
			nPane = -1;
			break;
		case 1:
			nPane = 0;
			break;
		case 2:
			nPane = 1;
			break;
		case 3:
			nPane = 2;
			break;
		}
	}
	return nPane;
}




/* 次のペインを返す */
int CSplitterWnd::GetNextPane( void )
{
	int		nPane;
	nPane = -1;
	if( m_nAllSplitRows == 1 &&	m_nAllSplitCols == 1 ){
		nPane = -1;
	}else
	if( m_nAllSplitRows == 2 &&	m_nAllSplitCols == 1 ){
		switch( m_nActivePane ){
		case 0:
			nPane = 2;
			break;
		case 2:
			nPane = -1;
			break;
		}
	}else
	if( m_nAllSplitRows == 1 &&	m_nAllSplitCols == 2 ){
		switch( m_nActivePane ){
		case 0:
			nPane = 1;
			break;
		case 1:
			nPane = -1;
			break;
		}
	}else{
		switch( m_nActivePane ){
		case 0:
			nPane = 1;
			break;
		case 1:
			nPane = 2;
			break;
		case 2:
			nPane = 3;
			break;
		case 3:
			nPane = -1;
			break;
		}
	}
	return nPane;
}


/* 最初のペインを返す */
int CSplitterWnd::GetFirstPane( void )
{
	return 0;
}



/* 最後のペインを返す */
int CSplitterWnd::GetLastPane( void )
{
	int		nPane;
	if( m_nAllSplitRows == 1 &&	m_nAllSplitCols == 1 ){
		nPane = 0;
	}else
	if( m_nAllSplitRows == 1 &&	m_nAllSplitCols == 2 ){
		nPane = 1;
	}else
	if( m_nAllSplitRows == 2 &&	m_nAllSplitCols == 1 ){
		nPane = 2;
	}else{
		nPane = 3;
	}
	return nPane;
}




/* 描画処理 */
LRESULT CSplitterWnd::OnPaint( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	HDC			hdc;
	PAINTSTRUCT	ps;
	RECT		rc;
	RECT		rcFrame;
	int			nFrameWidth = 3;
	HBRUSH		hBrush;
	hdc = ::BeginPaint( hwnd, &ps );
	::GetClientRect( GetHwnd(), &rc );
	hBrush = ::CreateSolidBrush( ::GetSysColor( COLOR_3DFACE ) );
	if( m_nAllSplitRows > 1 ){
		::SetRect( &rcFrame, rc.left, m_nVSplitPos, rc.right, m_nVSplitPos + nFrameWidth );
		::FillRect( hdc, &rcFrame, hBrush );
	}
	if( m_nAllSplitCols > 1 ){
		::SetRect( &rcFrame, m_nHSplitPos, rc.top, m_nHSplitPos + nFrameWidth, rc.bottom );
		::FillRect( hdc, &rcFrame, hBrush );
	}
	::DeleteObject( hBrush );
	::EndPaint(hwnd, &ps);
	return 0L;
}




/* ウィンドウサイズの変更処理 */
LRESULT CSplitterWnd::OnSize( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	CEditWnd*	pCEditWnd = (CEditWnd*)m_pcEditWnd;
	CEditView*	pcViewArr[MAXCOUNTOFVIEW];
	int					i;
	RECT		rcClient;
	int			nFrameWidth = 3;
	BOOL		bSizeBox;
	for( i = 0; i < m_nChildWndCount; ++i ){
		pcViewArr[i] = ( CEditView* )::GetWindowLongPtr( m_ChildWndArr[i], 0 );
	}

	/*
	|| ファンクションキーを下に表示している場合はサイズボックスを表示しない
	|| ステータスパーを表示している場合はサイズボックスを表示しない
	*/
	if( NULL == pCEditWnd
	 ||( NULL != pCEditWnd->m_cFuncKeyWnd.GetHwnd()
	  && 1 == m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_Place	/* ファンクションキー表示位置／0:上 1:下 */
	  )
	){
		bSizeBox = FALSE;
	}else if( NULL != pCEditWnd->m_cTabWnd.GetHwnd()
		&& m_pShareData->m_Common.m_sTabBar.m_eTabPosition == TabPosition_Bottom ) {
		bSizeBox = FALSE;
	}else{
		bSizeBox = TRUE;
		/* ステータスパーを表示している場合はサイズボックスを表示しない */
		if( NULL != pCEditWnd->m_cStatusBar.GetStatusHwnd() ){
			bSizeBox = FALSE;
		}
	}
	if( NULL != pCEditWnd->m_cDlgFuncList.GetHwnd() ){
		EDockSide eDockSideFL = pCEditWnd->m_cDlgFuncList.GetDockSide();
		if( eDockSideFL == DOCKSIDE_RIGHT || eDockSideFL == DOCKSIDE_BOTTOM ){
			bSizeBox = FALSE;
		}
	}
	if( pCEditWnd->GetMiniMap().GetHwnd() ){
		bSizeBox = FALSE;
	}

	/* メインウィンドウが最大化されている場合はサイズボックスを表示しない */
	WINDOWPLACEMENT	wp;
	wp.length = sizeof( wp );
	::GetWindowPlacement( GetParentHwnd(), &wp );
	if( SW_SHOWMAXIMIZED == wp.showCmd ){
		bSizeBox = FALSE;
	}

	::GetClientRect( GetHwnd(), &rcClient );

	if( m_nAllSplitRows == 1 && m_nAllSplitCols == 1 ){
		if( m_ChildWndArr[0] != NULL ){
			::MoveWindow( m_ChildWndArr[0], 0, 0, rcClient.right,  rcClient.bottom, TRUE );		/* 子ウィンドウ配列 */

			pcViewArr[0]->SplitBoxOnOff( TRUE, TRUE, bSizeBox );		/* 縦・横の分割ボックスのＯＮ／ＯＦＦ */
		}
	}else
	if( m_nAllSplitRows == 2 && m_nAllSplitCols == 1 ){
		if( m_ChildWndArr[0] != NULL ){
			::MoveWindow( m_ChildWndArr[0], 0, 0, rcClient.right,  m_nVSplitPos, TRUE );		/* 子ウィンドウ配列 */
			pcViewArr[0]->SplitBoxOnOff( FALSE, FALSE, FALSE );	/* 縦・横の分割ボックスのＯＮ／ＯＦＦ */
		}
		if( m_ChildWndArr[2] != NULL ){
			::MoveWindow( m_ChildWndArr[2], 0, m_nVSplitPos + nFrameWidth, rcClient.right, rcClient.bottom - ( m_nVSplitPos + nFrameWidth ), TRUE );			/* 子ウィンドウ配列 */
			pcViewArr[2]->SplitBoxOnOff( FALSE, TRUE, bSizeBox );	/* 縦・横の分割ボックスのＯＮ／ＯＦＦ */
		}
	}else
	if( m_nAllSplitRows == 1 && m_nAllSplitCols == 2 ){
		if( m_ChildWndArr[0] != NULL ){
			::MoveWindow( m_ChildWndArr[0], 0, 0, m_nHSplitPos, rcClient.bottom, TRUE );			/* 子ウィンドウ配列 */
			pcViewArr[0]->SplitBoxOnOff( FALSE, FALSE, FALSE );	/* 縦・横の分割ボックスのＯＮ／ＯＦＦ */
		}
		if( m_ChildWndArr[1] != NULL ){
			::MoveWindow( m_ChildWndArr[1], m_nHSplitPos + nFrameWidth, 0, rcClient.right - ( m_nHSplitPos + nFrameWidth ),  rcClient.bottom, TRUE );			/* 子ウィンドウ配列 */
			pcViewArr[1]->SplitBoxOnOff( TRUE, FALSE, bSizeBox );	/* 縦・横の分割ボックスのＯＮ／ＯＦＦ */
		}
	}else{
		if( m_ChildWndArr[0] != NULL ){
			::MoveWindow( m_ChildWndArr[0], 0, 0, m_nHSplitPos,  m_nVSplitPos, TRUE );			/* 子ウィンドウ配列 */
			pcViewArr[0]->SplitBoxOnOff( FALSE, FALSE, FALSE );	/* 縦・横の分割ボックスのＯＮ／ＯＦＦ */
		}
		if( m_ChildWndArr[1] != NULL ){
			::MoveWindow( m_ChildWndArr[1], m_nHSplitPos + nFrameWidth, 0, rcClient.right - ( m_nHSplitPos + nFrameWidth ),  m_nVSplitPos, TRUE );			/* 子ウィンドウ配列 */
			pcViewArr[1]->SplitBoxOnOff( FALSE, FALSE, FALSE );	/* 縦・横の分割ボックスのＯＮ／ＯＦＦ */
		}
		if( m_ChildWndArr[2] != NULL ){
			::MoveWindow( m_ChildWndArr[2], 0, m_nVSplitPos + nFrameWidth , m_nHSplitPos,  rcClient.bottom - ( m_nVSplitPos + nFrameWidth ), TRUE );			/* 子ウィンドウ配列 */
			pcViewArr[2]->SplitBoxOnOff( FALSE, FALSE, FALSE );	/* 縦・横の分割ボックスのＯＮ／ＯＦＦ */
		}
		if( m_ChildWndArr[3] != NULL ){
			::MoveWindow( m_ChildWndArr[3], m_nHSplitPos + nFrameWidth, m_nVSplitPos + nFrameWidth, rcClient.right - ( m_nHSplitPos + nFrameWidth ),  rcClient.bottom - ( m_nVSplitPos + nFrameWidth ), TRUE );			/* 子ウィンドウ配列 */
			pcViewArr[3]->SplitBoxOnOff( FALSE, FALSE, bSizeBox );	/* 縦・横の分割ボックスのＯＮ／ＯＦＦ */
		}
	}
	//デスクトップがちらつくのでだめ!
	//::InvalidateRect( GetHwnd(), NULL, TRUE );	//再描画してね。	//@@@ 2003.06.11 MIK
	return 0L;
}



/* マウス移動時の処理 */
LRESULT CSplitterWnd::OnMouseMove( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{

	int		nHit;
	RECT	rc;
	int		xPos;
	int		yPos;

	xPos = (int)(short)LOWORD(lParam);
	yPos = (int)(short)HIWORD(lParam);

	nHit = HitTestSplitter( xPos, yPos );
	switch( nHit ){
	case 1:
		::SetCursor( ::LoadCursor( NULL, IDC_SIZENS ) );
		break;
	case 2:
		::SetCursor( ::LoadCursor( NULL, IDC_SIZEWE ) );
		break;
	case 3:
		::SetCursor( ::LoadCursor( NULL, IDC_SIZEALL ) );
		break;
	}
	if( 0 != m_bDragging ){		/* 分割バーをドラッグ中か */
		::GetClientRect( GetHwnd(), &rc );
		if( xPos < 1 ){
			xPos = 1;
		}
		if( xPos > rc.right - 6 ){
			xPos = rc.right - 6;
		}
		if( yPos < 1 ){
			yPos = 1;
		}
		if( yPos > rc.bottom - 6 ){
			yPos = rc.bottom - 6;
		}
		/* 分割トラッカーの表示 */
		DrawSplitter( xPos, yPos, TRUE );
//		MYTRACE( _T("xPos=%d yPos=%d \n"), xPos, yPos );
	}
	return 0L;
}



/* マウス左ボタン押下時の処理 */
LRESULT CSplitterWnd::OnLButtonDown( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	int		nHit;
	int		xPos;
	int		yPos;

	xPos = (int)(short)LOWORD(lParam);
	yPos = (int)(short)HIWORD(lParam);

	::SetFocus( GetParentHwnd() );
	/* 分割バーへのヒットテスト */
	nHit = HitTestSplitter( xPos, yPos );
	if( 0 != nHit ){
		m_bDragging = nHit;	/* 分割バーをドラッグ中か */
		::SetCapture( GetHwnd() );
	}
	/* 分割トラッカーの表示 */
	DrawSplitter( xPos, yPos, FALSE );

	return 0L;
}




/* マウス左ボタン解放時の処理 */
LRESULT CSplitterWnd::OnLButtonUp( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{

	int bDraggingOld;
	int nX;
	int nY;

	if( m_bDragging ){
		/* 分割トラッカーの表示 */
		DrawSplitter( m_nDragPosX, m_nDragPosY, FALSE );
		bDraggingOld = m_bDragging;
		m_bDragging = 0;
		::ReleaseCapture();
		if( NULL != m_hcurOld ){
			::SetCursor( m_hcurOld );
		}
		/* ウィンドウの分割 */
		if( m_nAllSplitRows == 1 ){
			nY = 0;
		}else{
			nY = m_nDragPosY;
		}
		if( m_nAllSplitCols == 1 ){
			nX = 0;
		}else{
			nX = m_nDragPosX;
		}
		if( bDraggingOld == 1 ){
			DoSplit( m_nHSplitPos, nY );
		}else
		if( bDraggingOld == 2 ){
			DoSplit( nX, m_nVSplitPos );
		}else
		if( bDraggingOld == 3 ){
			DoSplit( nX, nY );
		}
	}
	return 0L;
}




/* マウス左ボタンダブルクリック時の処理 */
LRESULT CSplitterWnd::OnLButtonDblClk( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	int nX;
	int nY;
	int	nHit;
	int	xPos;
	int	yPos;

	xPos = (int)(short)LOWORD(lParam);
	yPos = (int)(short)HIWORD(lParam);

	nHit = HitTestSplitter( xPos, yPos );
	if( nHit == 1 ){
		if( m_nAllSplitCols == 1 ){
			nX = 0;
		}else{
			nX = m_nHSplitPos;
		}
		DoSplit( nX , 0 );
	}else
	if( nHit == 2 ){
		if( m_nAllSplitRows == 1 ){
			nY = 0;
		}else{
			nY = m_nVSplitPos;
		}
		DoSplit( 0 , nY );
	}else
	if( nHit == 3 ){
		DoSplit( 0 , 0 );
	}
	OnMouseMove( GetHwnd(), 0, 0, MAKELONG( xPos, yPos ) );
	return 0L;
}




/* アプリケーション定義のメッセージ(WM_APP <= msg <= 0xBFFF) */
LRESULT CSplitterWnd::DispatchEvent_WM_APP( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	int nPosX;
	int nPosY;
	switch( uMsg ){
	case MYWM_DOSPLIT:
		nPosX = (int)wParam;
		nPosY = (int)lParam;
//		MYTRACE( _T("MYWM_DOSPLIT nPosX=%d nPosY=%d\n"), nPosX, nPosY );

		/* ウィンドウの分割 */
		if( 0 != m_nHSplitPos ){
			nPosX = m_nHSplitPos;
		}
		if( 0 != m_nVSplitPos ){
			nPosY = m_nVSplitPos;
		}
		DoSplit( nPosX , nPosY );
		break;
	case MYWM_SETACTIVEPANE:
		SetActivePane( (int)wParam );
		break;
	}
	return 0L;
}



