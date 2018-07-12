/*!	@file
@brief CViewCommanderクラスのコマンド(設定系)関数群

	2012/12/15	CViewCommander.cpp,CViewCommander_New.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2002, YAZAKI, genta
	Copyright (C) 2003, MIK
	Copyright (C) 2005, genta, aroka
	Copyright (C) 2006, genta, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

#include "typeprop/CDlgTypeList.h"
#include "dlg/CDlgFavorite.h"	//履歴の管理	//@@@ 2003.04.08 MIK
#include "CEditApp.h"
#include "util/shell.h"
#include "CPropertyManager.h"
#include "util/window.h"


/*! ツールバーの表示/非表示

	@date 2006.12.19 ryoji 表示切替は CEditWnd::LayoutToolBar(), CEditWnd::EndLayoutBars() で行うように変更
*/
void CViewCommander::Command_SHOWTOOLBAR( void )
{
	CEditWnd*	pCEditWnd = GetEditWindow();	//	Sep. 10, 2002 genta

	GetDllShareData().m_Common.m_sWindow.m_bDispTOOLBAR = ((NULL == pCEditWnd->m_cToolbar.GetToolbarHwnd())? TRUE: FALSE);	/* ツールバー表示 */
	pCEditWnd->LayoutToolBar();
	pCEditWnd->EndLayoutBars();

	//全ウインドウに変更を通知する。
	CAppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)MYBCN_TOOLBAR,
		(LPARAM)pCEditWnd->GetHwnd(),
		pCEditWnd->GetHwnd()
	);
}



/*! ファンクションキーの表示/非表示

	@date 2006.12.19 ryoji 表示切替は CEditWnd::LayoutFuncKey(), CEditWnd::EndLayoutBars() で行うように変更
*/
void CViewCommander::Command_SHOWFUNCKEY( void )
{
	CEditWnd*	pCEditWnd = GetEditWindow();	//	Sep. 10, 2002 genta

	GetDllShareData().m_Common.m_sWindow.m_bDispFUNCKEYWND = ((NULL == pCEditWnd->m_cFuncKeyWnd.GetHwnd())? TRUE: FALSE);	/* ファンクションキー表示 */
	pCEditWnd->LayoutFuncKey();
	pCEditWnd->EndLayoutBars();

	//全ウインドウに変更を通知する。
	CAppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)MYBCN_FUNCKEY,
		(LPARAM)pCEditWnd->GetHwnd(),
		pCEditWnd->GetHwnd()
	);
}



/*! タブ(ウインドウ)の表示/非表示

	@author MIK
	@date 2003.06.10 新規作成
	@date 2006.12.19 ryoji 表示切替は CEditWnd::LayoutTabBar(), CEditWnd::EndLayoutBars() で行うように変更
	@date 2007.06.20 ryoji グループIDリセット
 */
void CViewCommander::Command_SHOWTAB( void )
{
	CEditWnd*	pCEditWnd = GetEditWindow();	//	Sep. 10, 2002 genta

	GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd = ((NULL == pCEditWnd->m_cTabWnd.GetHwnd())? TRUE: FALSE);	/* タブバー表示 */
	pCEditWnd->LayoutTabBar();
	pCEditWnd->EndLayoutBars();

	// まとめるときは WS_EX_TOPMOST 状態を同期する	// 2007.05.18 ryoji
	if( GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd && !GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin )
	{
		GetEditWindow()->WindowTopMost(
			( (DWORD)::GetWindowLongPtr( GetEditWindow()->GetHwnd(), GWL_EXSTYLE ) & WS_EX_TOPMOST )? 1: 2
		);
	}

	//全ウインドウに変更を通知する。
	CAppNodeManager::getInstance()->ResetGroupId();
	CAppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)MYBCN_TAB,
		(LPARAM)pCEditWnd->GetHwnd(),
		pCEditWnd->GetHwnd()
	);
}



/*! ステータスバーの表示/非表示

	@date 2006.12.19 ryoji 表示切替は CEditWnd::LayoutStatusBar(), CEditWnd::EndLayoutBars() で行うように変更
*/
void CViewCommander::Command_SHOWSTATUSBAR( void )
{
	CEditWnd*	pCEditWnd = GetEditWindow();	//	Sep. 10, 2002 genta

	GetDllShareData().m_Common.m_sWindow.m_bDispSTATUSBAR = ((NULL == pCEditWnd->m_cStatusBar.GetStatusHwnd())? TRUE: FALSE);	/* ステータスバー表示 */
	pCEditWnd->LayoutStatusBar();
	pCEditWnd->EndLayoutBars();

	//全ウインドウに変更を通知する。
	CAppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)MYBCN_STATUSBAR,
		(LPARAM)pCEditWnd->GetHwnd(),
		pCEditWnd->GetHwnd()
	);
}



/*! ミニマップの表示/非表示

	@date 2014.07.14 新規作成
*/
void CViewCommander::Command_SHOWMINIMAP( void )
{
	CEditWnd*	pCEditWnd = GetEditWindow();	//	Sep. 10, 2002 genta

	GetDllShareData().m_Common.m_sWindow.m_bDispMiniMap = ((NULL == pCEditWnd->GetMiniMap().GetHwnd())? true: false);
	pCEditWnd->LayoutMiniMap();
	pCEditWnd->EndLayoutBars();

	//全ウインドウに変更を通知する。
	CAppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)MYBCN_MINIMAP,
		(LPARAM)pCEditWnd->GetHwnd(),
		pCEditWnd->GetHwnd()
	);
}



/* タイプ別設定一覧 */
void CViewCommander::Command_TYPE_LIST( void )
{
	CDlgTypeList			cDlgTypeList;
	CDlgTypeList::SResult	sResult;
	sResult.cDocumentType = GetDocument()->m_cDocType.GetDocumentType();
	sResult.bTempChange = true;
	if( cDlgTypeList.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), &sResult ) ){
		//	Nov. 29, 2000 genta
		//	一時的な設定適用機能を無理矢理追加
		if( sResult.bTempChange ){
			HandleCommand( F_CHANGETYPE, true, (LPARAM)sResult.cDocumentType.GetIndex() + 1, 0, 0, 0 );
		}
		else{
			/* タイプ別設定 */
			CEditApp::getInstance()->OpenPropertySheetTypes( -1, sResult.cDocumentType );
		}
	}
	return;
}



/*! タイプ別設定一時適用 */
void CViewCommander::Command_CHANGETYPE( int nTypePlusOne )
{
	CTypeConfig type = CTypeConfig(nTypePlusOne - 1);
	if( nTypePlusOne == 0 ){
		type = GetDocument()->m_cDocType.GetDocumentType();
	}
	if( type.IsValidType() && type.GetIndex() < GetDllShareData().m_nTypesCount ){
		const STypeConfigMini* pConfig;
		CDocTypeManager().GetTypeConfigMini(type, &pConfig);
		GetDocument()->m_cDocType.SetDocumentTypeIdx(pConfig->m_id, true);
		GetDocument()->m_cDocType.LockDocumentType();
		GetDocument()->OnChangeType();
	}
}



/* タイプ別設定 */
void CViewCommander::Command_OPTION_TYPE( void )
{
	CEditApp::getInstance()->OpenPropertySheetTypes( -1, GetDocument()->m_cDocType.GetDocumentType() );
}



/* 共通設定 */
void CViewCommander::Command_OPTION( void )
{
	/* 設定プロパティシート テスト用 */
	CEditApp::getInstance()->OpenPropertySheet( -1 );
}



/* フォント設定 */
void CViewCommander::Command_FONT( void )
{
	HWND	hwndFrame;
	hwndFrame = GetMainWindow();

	/* フォント設定ダイアログ */
	LOGFONT lf = GetDllShareData().m_Common.m_sView.m_lf;
	INT   nPointSize = GetDllShareData().m_Common.m_sView.m_nPointSize;
#if !defined(USE_UNFIXED_FONT)
#define USE_UNFIXED_FONT
#endif
#ifdef USE_UNFIXED_FONT
	bool bFixedFont = false;
#else
	bool bFixedFont = true;
#endif
	if( MySelectFont( &lf, &nPointSize, CEditWnd::getInstance()->m_cSplitterWnd.GetHwnd(), bFixedFont ) ){
		GetDllShareData().m_Common.m_sView.m_lf = lf;
		GetDllShareData().m_Common.m_sView.m_nPointSize = nPointSize;

		if( GetDllShareData().m_Common.m_sView.m_lf.lfPitchAndFamily & FIXED_PITCH  ){
			GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH = TRUE;	/* 現在のフォントは固定幅フォントである */
		}else{
			GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH = FALSE;	/* 現在のフォントは固定幅フォントである */
		}
		// 2010.09.06 プロポーショナルフォントでもたぶん矩形も使えるはず
		GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH = TRUE;
		/* 設定変更を反映させる */
		/* 全編集ウィンドウへメッセージをポストする */
		CAppNodeGroupHandle(0).PostMessageToAllEditors(
			MYWM_SAVEEDITSTATE,
			(WPARAM)0, (LPARAM)0, hwndFrame
		);
		CAppNodeGroupHandle(0).PostMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)0, (LPARAM)PM_CHANGESETTING_FONT, hwndFrame
		);

		/* キャレットの表示 */
//		::HideCaret( GetHwnd() );
//		::ShowCaret( GetHwnd() );

//		/* アクティブにする */
//		/* アクティブにする */
//		ActivateFrameWindow( hwndFrame );
	}
	return;
}



/*! フォントサイズ設定
	@param fontSize フォントサイズ（1/10ポイント単位）
	@param shift フォントサイズを拡大or縮小するための変更量(fontSize=0のとき有効)

	@note TrueTypeのみサポート

	@date 2013.04.10 novice 新規作成
*/
void CViewCommander::Command_SETFONTSIZE( int fontSize, int shift, int mode )
{
	// The point sizes recommended by "The Windows Interface: An Application Design Guide", 1/10ポイント単位
	static const INT sizeTable[] = { 8*10, 9*10, 10*10, (INT)(10.5*10), 11*10, 12*10, 14*10, 16*10, 18*10, 20*10, 22*10, 24*10, 26*10, 28*10, 36*10, 48*10, 72*10 };
	const LOGFONT& lf = (mode == 0 ? GetDllShareData().m_Common.m_sView.m_lf
		: GetEditWindow()->GetLogfont( mode == 2 ));
	INT nPointSize;

	// TrueTypeのみ対応
	if( OUT_STROKE_PRECIS != lf.lfOutPrecision && OUT_PS_ONLY_PRECIS != lf.lfOutPrecision ) {
		return;
	}

	if( !(0 <= mode && mode <= 2) ){
		return;
	}

	if( 0 != fontSize ){
		// フォントサイズを直接選択する場合
		nPointSize = t_max(sizeTable[0], t_min(sizeTable[_countof(sizeTable)-1], fontSize));
	} else if( 0 != shift ) {
		// 現在のフォントに対して、縮小or拡大したフォント選択する場合
		nPointSize = (mode == 0 ? GetDllShareData().m_Common.m_sView.m_nPointSize
			: GetEditWindow()->GetFontPointSize( mode == 2 ));

		// フォントの拡大or縮小するためのサイズ検索
		int i;
		for( i = 0; i < _countof(sizeTable); i++) {
			if( nPointSize <= sizeTable[i] ){
				int index = t_max(0, t_min((int)_countof(sizeTable) - 1, (int)(i + shift)));
				nPointSize = sizeTable[index];
				break;
			}
		}
	} else {
		// フォントサイズが変わらないので終了
		return;
	}
	// 新しいフォントサイズ設定
	int lfHeight = DpiPointsToPixels(-nPointSize, 10);
	int nTypeIndex = -1;
	if( mode == 0 ){
		GetDllShareData().m_Common.m_sView.m_lf.lfHeight = lfHeight;
		GetDllShareData().m_Common.m_sView.m_nPointSize = nPointSize;
	}else if( mode == 1 ){
		CTypeConfig nDocType = GetDocument()->m_cDocType.GetDocumentType();
		STypeConfig* type = new STypeConfig();
		if( !CDocTypeManager().GetTypeConfig( nDocType, *type ) ){
			// 謎のエラー
			return;
		}
		type->m_bUseTypeFont = true; // タイプ別フォントを有効にする
		type->m_lf = lf;
		type->m_lf.lfHeight = lfHeight;
		type->m_nPointSize = nPointSize;
		CDocTypeManager().SetTypeConfig( nDocType, *type );
		delete type;
		nTypeIndex = nDocType.GetIndex();
	}else if( mode == 2 ){
		GetDocument()->m_blfCurTemp = true;
		GetDocument()->m_lfCur = lf;
		GetDocument()->m_lfCur.lfHeight = lfHeight;
		GetDocument()->m_nPointSizeCur = nPointSize;
		GetDocument()->m_nPointSizeOrg = GetEditWindow()->GetFontPointSize(false);
	}

	HWND	hwndFrame;
	hwndFrame = GetMainWindow();

	/* 設定変更を反映させる */
	// 新たにタイプ別や一時設定が有効になってもフォント名は変わらないのでSIZEのみの変更通知をする
	if( mode == 0 || mode == 1 ){
		/* 全編集ウィンドウへメッセージをポストする */
		if( mode == 0 ){
			CAppNodeGroupHandle(0).PostMessageToAllEditors(
				MYWM_SAVEEDITSTATE,
				(WPARAM)0, (LPARAM)0, hwndFrame
			);
		}
		CAppNodeGroupHandle(0).PostMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)nTypeIndex, (LPARAM)PM_CHANGESETTING_FONTSIZE, hwndFrame
		);
	}else if( mode == 2 ){
		// 自分だけ更新
		GetDocument()->OnChangeSetting( true, false );
	}
}



/*! 現在のウィンドウ幅で折り返し

	@date 2002.01.14 YAZAKI 現在のウィンドウ幅で折り返されているときは、最大値にするように
	@date 2002.04.08 YAZAKI ときどきウィンドウ幅で折り返されないことがあるバグ修正。
	@date 2005.08.14 genta ここでの設定は共通設定に反映しない．
	@date 2005.10.22 aroka 現在のウィンドウ幅→最大値→文書タイプの初期値 をトグルにする

	@note 変更する順序を変更したときはCEditWnd::InitMenu()も変更すること
	@sa CEditWnd::InitMenu()
*/
void CViewCommander::Command_WRAPWINDOWWIDTH( void )	//	Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更
{
	// Jan. 8, 2006 genta 判定処理をm_pCommanderView->GetWrapMode()へ移動
	CEditView::TOGGLE_WRAP_ACTION nWrapMode;
	CKetaXInt newKetas;
	
	nWrapMode = m_pCommanderView->GetWrapMode( &newKetas );
	GetDocument()->m_nTextWrapMethodCur = WRAP_SETTING_WIDTH;
	GetDocument()->m_bTextWrapMethodCurTemp = !( GetDocument()->m_nTextWrapMethodCur == m_pCommanderView->m_pTypeData->m_nTextWrapMethod );
	if( nWrapMode == CEditView::TGWRAP_NONE ){
		return;	// 折り返し桁は元のまま
	}

	GetEditWindow()->ChangeLayoutParam( true, GetDocument()->m_cLayoutMgr.GetTabSpaceKetas(), GetDocument()->m_cLayoutMgr.m_tsvInfo.m_nTsvMode, newKetas );
	

	//	Aug. 14, 2005 genta 共通設定へは反映させない
//	m_pCommanderView->m_pTypeData->m_nMaxLineKetas = m_nViewColNum;

//	2013.12.30 左隅に移動しないように
//	m_pCommanderView->GetTextArea().SetViewLeftCol( CLayoutInt(0) );		/* 表示域の一番左の桁(0開始) */

	/* フォーカス移動時の再描画 */
	m_pCommanderView->RedrawAll();
	return;
}



//	from CViewCommander_New.cpp
/*!	履歴の管理(ダイアログ)
	@author	MIK
	@date	2003/04/07
*/
void CViewCommander::Command_Favorite( void )
{
	CDlgFavorite	cDlgFavorite;

	//ダイアログを表示する
	if( !cDlgFavorite.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)GetDocument() ) )
	{
		return;
	}

	return;
}



/*!
	@brief テキストの折り返し方法を変更する
	
	@param[in] nWrapMethod 折り返し方法
		WRAP_NO_TEXT_WRAP  : 折り返さない
		WRAP_SETTING_WIDTH ; 指定桁で折り返す
		WRAP_WINDOW_WIDTH  ; 右端で折り返す
	
	@note ウィンドウが左右に分割されている場合、左側のウィンドウ幅を折り返し幅とする。
	
	@date 2008.05.31 nasukoji	新規作成
	@date 2009.08.28 nasukoji	テキストの最大幅を算出する
*/
void CViewCommander::Command_TEXTWRAPMETHOD( int nWrapMethod )
{
	CEditDoc* pcDoc = GetDocument();

	// 現在の設定値と同じなら何もしない
	if( pcDoc->m_nTextWrapMethodCur == nWrapMethod )
		return;

	CKetaXInt nWidth;

	switch( nWrapMethod ){
	case WRAP_NO_TEXT_WRAP:		// 折り返さない
		nWidth = CKetaXInt(MAXLINEKETAS);	// アプリケーションの最大幅で折り返し
		break;

	case WRAP_SETTING_WIDTH:	// 指定桁で折り返す
		nWidth = pcDoc->m_cDocType.GetDocumentAttribute().m_nMaxLineKetas;
		break;

	case WRAP_WINDOW_WIDTH:		// 右端で折り返す
		// ウィンドウが左右に分割されている場合は左側のウィンドウ幅を使用する
		nWidth = m_pCommanderView->ViewColNumToWrapColNum( GetEditWindow()->GetView(0).GetTextArea().m_nViewColNum );
		break;

	default:
		return;		// 不正な値の時は何もしない
	}

	pcDoc->m_nTextWrapMethodCur = nWrapMethod;	// 設定を記憶

	// 折り返し方法の一時設定適用／一時設定適用解除	// 2008.06.08 ryoji
	pcDoc->m_bTextWrapMethodCurTemp = !( pcDoc->m_cDocType.GetDocumentAttribute().m_nTextWrapMethod == nWrapMethod );

	// 折り返し位置を変更
	GetEditWindow()->ChangeLayoutParam( false, pcDoc->m_cLayoutMgr.GetTabSpaceKetas(), pcDoc->m_cLayoutMgr.m_tsvInfo.m_nTsvMode, nWidth );

	// 2009.08.28 nasukoji	「折り返さない」ならテキスト最大幅を算出、それ以外は変数をクリア
	if( pcDoc->m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP ){
		pcDoc->m_cLayoutMgr.CalculateTextWidth();		// テキスト最大幅を算出する
		GetEditWindow()->RedrawAllViews( NULL );		// スクロールバーの更新が必要なので再表示を実行する
	}else{
		pcDoc->m_cLayoutMgr.ClearLayoutLineWidth();		// 各行のレイアウト行長の記憶をクリアする
	}
}



/*!
	@brief 文字カウント方法を変更する
	
	@param[in] nMode 文字カウント方法
		SELECT_COUNT_TOGGLE  : 文字カウント方法をトグル
		SELECT_COUNT_BY_CHAR ; 文字数でカウント
		SELECT_COUNT_BY_BYTE ; バイト数でカウント
*/
void CViewCommander::Command_SELECT_COUNT_MODE( int nMode )
{
	//設定には保存せず、View毎に持つフラグを設定
	//BOOL* pbDispSelCountByByte = &GetDllShareData().m_Common.m_sStatusbar.m_bDispSelCountByByte;
	ESelectCountMode* pnSelectCountMode = &GetEditWindow()->m_nSelectCountMode;

	if( nMode == SELECT_COUNT_TOGGLE ){
		//文字数⇔バイト数トグル
		ESelectCountMode nCurrentMode;
		if( *pnSelectCountMode == SELECT_COUNT_TOGGLE ){
			nCurrentMode = ( GetDllShareData().m_Common.m_sStatusbar.m_bDispSelCountByByte ?
								SELECT_COUNT_BY_BYTE :
								SELECT_COUNT_BY_CHAR );
		}else{
			nCurrentMode = *pnSelectCountMode;
		}
		*pnSelectCountMode = ( nCurrentMode == SELECT_COUNT_BY_BYTE ?
								SELECT_COUNT_BY_CHAR :
								SELECT_COUNT_BY_BYTE );
	}else if( nMode == SELECT_COUNT_BY_BYTE || nMode == SELECT_COUNT_BY_CHAR ){
		*pnSelectCountMode = ( ESelectCountMode )nMode;
	}
}



/*!	@brief 引用符の設定
	@date Jan. 29, 2005 genta 新規作成
*/
void CViewCommander::Command_SET_QUOTESTRING( const wchar_t* quotestr )
{
	if( quotestr == NULL )
		return;

	wcsncpy( GetDllShareData().m_Common.m_sFormat.m_szInyouKigou, quotestr,
		_countof( GetDllShareData().m_Common.m_sFormat.m_szInyouKigou ));
	
	GetDllShareData().m_Common.m_sFormat.m_szInyouKigou[ _countof( GetDllShareData().m_Common.m_sFormat.m_szInyouKigou ) - 1 ] = L'\0';
}
