/*!	@file
@brief CViewCommanderクラスのコマンド(ウィンドウ系)関数群

	2012/12/15	CViewCommander.cpp,CViewCommander_New.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro
	Copyright (C) 2001, MIK
	Copyright (C) 2002, YAZAKI, genta, MIK
	Copyright (C) 2003, MIK, genta
	Copyright (C) 2004, Moca, genta, crayonzen, Kazika
	Copyright (C) 2006, genta, ryoji, maru
	Copyright (C) 2007, ryoji, genta
	Copyright (C) 2008, syat
	Copyright (C) 2009, syat
	Copyright (C) 2010, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

#include "_main/CControlTray.h"
#include "util/os.h"
#include "env/CSakuraEnvironment.h"
#include "env/CShareData.h"


/* 上下に分割 */	//Sept. 17, 2000 jepro 説明の「縦」を「上下に」に変更
void CViewCommander::Command_SPLIT_V( void )
{
	GetEditWindow()->m_cSplitterWnd.VSplitOnOff();
	return;
}



/* 左右に分割 */	//Sept. 17, 2000 jepro 説明の「横」を「左右に」に変更
void CViewCommander::Command_SPLIT_H( void )
{
	GetEditWindow()->m_cSplitterWnd.HSplitOnOff();
	return;
}



/* 縦横に分割 */	//Sept. 17, 2000 jepro 説明に「に」を追加
void CViewCommander::Command_SPLIT_VH( void )
{
	GetEditWindow()->m_cSplitterWnd.VHSplitOnOff();
	return;
}



/* ウィンドウを閉じる */
void CViewCommander::Command_WINCLOSE( void )
{
	/* 閉じる */
	::PostMessage( GetMainWindow(), MYWM_CLOSE, 0, 								// 2007.02.13 ryoji WM_CLOSE→MYWM_CLOSEに変更
		(LPARAM)CAppNodeManager::getInstance()->GetNextTab( GetMainWindow() ) );	// タブまとめ時、次のタブに移動	2013/4/10 Uchi
	return;
}



/* すべてのウィンドウを閉じる */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更
void CViewCommander::Command_FILECLOSEALL( void )
{
	int nGroup = CAppNodeManager::getInstance()->GetEditNode( GetMainWindow() )->GetGroup();
	CControlTray::CloseAllEditor( TRUE, GetMainWindow(), FALSE, nGroup );	// 2006.12.25, 2007.02.13 ryoji 引数追加
	return;
}



/* このタブ以外を閉じる */	// 2008.11.22 syat
// 2009.12.26 syat このウィンドウ以外を閉じるとの兼用化
void CViewCommander::Command_TAB_CLOSEOTHER( void )
{
	int nGroup = 0;

	// ウィンドウ一覧を取得する
	EditNode* pEditNode;
	int nCount = CAppNodeManager::getInstance()->GetOpenedWindowArr( &pEditNode, TRUE );
	if( 0 >= nCount )return;

	for( int i = 0; i < nCount; i++ ){
		if( pEditNode[i].m_hWnd == GetMainWindow() ){
			pEditNode[i].m_hWnd = NULL;		//自分自身は閉じない
			nGroup = pEditNode[i].m_nGroup;
		}
	}

	//終了要求を出す
	CAppNodeGroupHandle(nGroup).RequestCloseEditor( pEditNode, nCount, FALSE, TRUE, GetMainWindow() );
	delete []pEditNode;
	return;
}



/*!	@brief ウィンドウ一覧ポップアップ表示処理（ファイル名のみ）
	@date  2006.03.23 fon 新規作成
	@date  2006.05.19 genta コマンド実行要因を表す引数追加
	@date  2007.07.07 genta コマンド実行要因の値を変更
*/
void CViewCommander::Command_WINLIST( int nCommandFrom )
{
	//ウィンドウ一覧をポップアップ表示する
	GetEditWindow()->PopupWinList(( nCommandFrom & FA_FROMKEYBOARD ) != FA_FROMKEYBOARD );
	// 2007.02.27 ryoji アクセラレータキーからでなければマウス位置に
}


/*! ウィンドウ一覧表示
*/
void CViewCommander::Command_DLGWINLIST( void )
{
	DWORD dwPid;
	HWND hwnd = GetDllShareData().m_sHandles.m_hwndTray;
	::GetWindowThreadProcessId(hwnd, &dwPid);
	::AllowSetForegroundWindow(dwPid);
	::PostMessage(hwnd, MYWM_DLGWINLIST, 0, 0);
}


/*!	@brief 重ねて表示

	@date 2002.01.08 YAZAKI 「左右に並べて表示」すると、
		裏で最大化されているエクスプローラが「元の大きさ」になるバグ修正。
	@date 2003.06.12 MIK タブウインドウ時は動作しないように
	@date 2004.03.19 crayonzen カレントウィンドウを最後に配置．
		ウィンドウが多い場合に2周目以降は右にずらして配置．
	@date 2004.03.20 genta Z-Orderの上から順に並べていくように．(SetWindowPosを利用)
	@date 2007.06.20 ryoji タブモードは解除せずグループ単位で並べる
*/
void CViewCommander::Command_CASCADE( void )
{
	int i;

	/* 現在開いている編集窓のリストを取得する */
	EditNode*	pEditNodeArr;
	int			nRowNum = CAppNodeManager::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE/*FALSE*/, TRUE );

	if( nRowNum > 0 ){
		struct WNDARR {
			HWND	hWnd;
			int		newX;
			int		newY;
		};

		WNDARR*	pWndArr = new WNDARR[nRowNum];
		int		count = 0;	//	処理対象ウィンドウカウント
		// Mar. 20, 2004 genta 現在のウィンドウを末尾に持っていくのに使う
		int		current_win_index = -1;

		// -----------------------------------------
		//	ウィンドウ(ハンドル)リストの作成
		// -----------------------------------------

		for( i = 0; i < nRowNum; ++i ){
			if( ::IsIconic( pEditNodeArr[i].GetHwnd() ) ){	//	最小化しているウィンドウは無視。
				continue;
			}
			if( !::IsWindowVisible( pEditNodeArr[i].GetHwnd() ) ){	//	不可視ウィンドウは無視。
				continue;
			}
			//	Mar. 20, 2004 genta
			//	現在のウィンドウを末尾に持っていくためここではスキップ
			if( pEditNodeArr[i].GetHwnd() == CEditWnd::getInstance()->GetHwnd() ){
				current_win_index = i;
				continue;
			}
			pWndArr[count].hWnd = pEditNodeArr[i].GetHwnd();
			count++;
		}

		//	Mar. 20, 2004 genta
		//	現在のウィンドウを末尾に挿入 inspired by crayonzen
		if( current_win_index >= 0 ){
			pWndArr[count].hWnd = pEditNodeArr[current_win_index].GetHwnd();
			count++;
		}

		//	デスクトップサイズを得る
		RECT	rcDesktop;
		//	May 01, 2004 genta マルチモニタ対応
		::GetMonitorWorkRect( m_pCommanderView->GetHwnd(), &rcDesktop );
		
		int width = (rcDesktop.right - rcDesktop.left ) * 4 / 5; // Mar. 9, 2003 genta 整数演算のみにする
		int height = (rcDesktop.bottom - rcDesktop.top ) * 4 / 5;
		int w_delta = ::GetSystemMetrics(SM_CXSIZEFRAME) + ::GetSystemMetrics(SM_CXSIZE);
		int h_delta = ::GetSystemMetrics(SM_CYSIZEFRAME) + ::GetSystemMetrics(SM_CYSIZE);
		int w_offset = rcDesktop.left; //Mar. 19, 2004 crayonzen 絶対値だとエクスプローラーのウィンドウに重なるので
		int h_offset = rcDesktop.top; //初期値をデスクトップ内に収める。

		// -----------------------------------------
		//	座標計算
		//
		//	Mar. 19, 2004 crayonzen
		//		左上をデスクトップ領域に合わせる(タスクバーが上・左にある場合のため)．
		//		ウィンドウが右下からはみ出たら左上に戻るが，
		//		2周目以降は開始位置を右にずらしてアイコンが見えるようにする．
		//
		//	Mar. 20, 2004 genta ここでは計算値を保管するだけでウィンドウの再配置は行わない
		// -----------------------------------------

		int roundtrip = 0; //２度目の描画以降で使用するカウント
		int sw_offset = w_delta; //右スライドの幅

		for(i = 0; i < count; ++i ){
			if (w_offset + width > rcDesktop.right || h_offset + height > rcDesktop.bottom){
				++roundtrip;
				if ((rcDesktop.right - rcDesktop.left) - sw_offset * roundtrip < width){
					//	これ以上右にずらせないときはしょうがないから左上に戻る
					roundtrip = 0;
				}
				//	ウィンドウ領域の左上にセット
				//	craonzen 初期値修正(２度目以降の描画で少しづつスライド)
				w_offset = rcDesktop.left + sw_offset * roundtrip;
				h_offset = rcDesktop.top;
			}
			
			pWndArr[i].newX = w_offset;
			pWndArr[i].newY = h_offset;

			w_offset += w_delta;
			h_offset += h_delta;
		}

		// -----------------------------------------
		//	最大化/非表示解除
		//	最大化されたウィンドウを元に戻す．これがないと，最大化ウィンドウが
		//	最大化状態のまま並び替えられてしまい，その後最大化動作が変になる．
		//
		//	Sep. 04, 2004 genta
		// -----------------------------------------
		for( i = 0; i < count; i++ ){
			::ShowWindow( pWndArr[i].hWnd, SW_RESTORE | SW_SHOWNA );
		}

		// -----------------------------------------
		//	ウィンドウ配置
		//
		//	Mar. 20, 2004 genta APIを素直に使ってZ-Orderの上から下の順で並べる．
		// -----------------------------------------

		// まずカレントを最前面に
		i = count - 1;
		
		::SetWindowPos(
			pWndArr[i].hWnd, HWND_TOP,
			pWndArr[i].newX, pWndArr[i].newY,
			width, height,
			0
		);

		// 残りを1つずつ下に入れていく
		while( --i >= 0 ){
			::SetWindowPos(
				pWndArr[i].hWnd, pWndArr[i + 1].hWnd,
				pWndArr[i].newX, pWndArr[i].newY,
				width, height,
				SWP_NOACTIVATE
			);
		}

		delete [] pWndArr;
		delete [] pEditNodeArr;
	}
	return;
}



//上下に並べて表示
void CViewCommander::Command_TILE_V( void )
{
	int i;

	/* 現在開いている編集窓のリストを取得する */
	EditNode*	pEditNodeArr;
	int			nRowNum = CAppNodeManager::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE/*FALSE*/, TRUE );

	if( nRowNum > 0 ){
		HWND*	phwndArr = new HWND[nRowNum];
		int		count = 0;
		//	デスクトップサイズを得る
		RECT	rcDesktop;
		//	May 01, 2004 genta マルチモニタ対応
		::GetMonitorWorkRect( m_pCommanderView->GetHwnd(), &rcDesktop );
		for( i = 0; i < nRowNum; ++i ){
			if( ::IsIconic( pEditNodeArr[i].GetHwnd() ) ){	//	最小化しているウィンドウは無視。
				continue;
			}
			if( !::IsWindowVisible( pEditNodeArr[i].GetHwnd() ) ){	//	不可視ウィンドウは無視。
				continue;
			}
			//	From Here Jul. 28, 2002 genta
			//	現在のウィンドウを先頭に持ってくる
			if( pEditNodeArr[i].GetHwnd() == CEditWnd::getInstance()->GetHwnd() ){
				phwndArr[count] = phwndArr[0];
				phwndArr[0] = CEditWnd::getInstance()->GetHwnd();
			}
			else {
				phwndArr[count] = pEditNodeArr[i].GetHwnd();
			}
			//	To Here Jul. 28, 2002 genta
			count++;
		}
		int height = (rcDesktop.bottom - rcDesktop.top ) / count;
		for(i = 0; i < count; ++i ){
			//	Jul. 21, 2002 genta
			::ShowWindow( phwndArr[i], SW_RESTORE );
			::SetWindowPos(
				phwndArr[i], 0,
				rcDesktop.left, rcDesktop.top + height * i, //Mar. 19, 2004 crayonzen 上端調整
				rcDesktop.right - rcDesktop.left, height,
				SWP_NOOWNERZORDER | SWP_NOZORDER
			);
		}
		::SetFocus( phwndArr[0] );	// Aug. 17, 2002 MIK

		delete [] phwndArr;
		delete [] pEditNodeArr;
	}
	return;
}



//左右に並べて表示
void CViewCommander::Command_TILE_H( void )
{
	int i;

	/* 現在開いている編集窓のリストを取得する */
	EditNode*	pEditNodeArr;
	int			nRowNum = CAppNodeManager::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE/*FALSE*/, TRUE );

	if( nRowNum > 0 ){
		HWND*	phwndArr = new HWND[nRowNum];
		int		count = 0;
		//	デスクトップサイズを得る
		RECT	rcDesktop;
		//	May 01, 2004 genta マルチモニタ対応
		::GetMonitorWorkRect( m_pCommanderView->GetHwnd(), &rcDesktop );
		for( i = 0; i < nRowNum; ++i ){
			if( ::IsIconic( pEditNodeArr[i].GetHwnd() ) ){	//	最小化しているウィンドウは無視。
				continue;
			}
			if( !::IsWindowVisible( pEditNodeArr[i].GetHwnd() ) ){	//	不可視ウィンドウは無視。
				continue;
			}
			//	From Here Jul. 28, 2002 genta
			//	現在のウィンドウを先頭に持ってくる
			if( pEditNodeArr[i].GetHwnd() == CEditWnd::getInstance()->GetHwnd() ){
				phwndArr[count] = phwndArr[0];
				phwndArr[0] = CEditWnd::getInstance()->GetHwnd();
			}
			else {
				phwndArr[count] = pEditNodeArr[i].GetHwnd();
			}
			//	To Here Jul. 28, 2002 genta
			count++;
		}
		int width = (rcDesktop.right - rcDesktop.left ) / count;
		for(i = 0; i < count; ++i ){
			//	Jul. 21, 2002 genta
			::ShowWindow( phwndArr[i], SW_RESTORE );
			::SetWindowPos(
				phwndArr[i], 0,
				width * i + rcDesktop.left, rcDesktop.top, // Oct. 18, 2003 genta タスクバーが左にある場合を考慮
				width, rcDesktop.bottom - rcDesktop.top,
				SWP_NOOWNERZORDER | SWP_NOZORDER
			);
		}
		::SetFocus( phwndArr[0] );	// Aug. 17, 2002 MIK
		delete [] phwndArr;
		delete [] pEditNodeArr;
	}
	return;
}



//	from CViewCommander_New.cpp
/*! 常に手前に表示
	@date 2004.09.21 Moca
*/
void CViewCommander::Command_WINTOPMOST( LPARAM lparam )
{
	GetEditWindow()->WindowTopMost( int(lparam) );
}



//Start 2004.07.14 Kazika 追加
/*!	@brief 結合して表示

	タブウィンドウの結合、非結合を切り替えるコマンドです。
	[共通設定]->[ウィンドウ]->[タブ表示 まとめない]の切り替えと同じです。
	@author Kazika
	@date 2004.07.14 Kazika 新規作成
	@date 2007.06.20 ryoji GetDllShareData().m_TabWndWndplの廃止，グループIDリセット
*/
void CViewCommander::Command_BIND_WINDOW( void )
{
	//タブモードであるならば
	if (GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd)
	{
		//タブウィンドウの設定を変更
		GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin = !GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin;

		// まとめるときは WS_EX_TOPMOST 状態を同期する	// 2007.05.18 ryoji
		if( !GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin )
		{
			GetEditWindow()->WindowTopMost(
				( (DWORD)::GetWindowLongPtr( GetEditWindow()->GetHwnd(), GWL_EXSTYLE ) & WS_EX_TOPMOST )? 1: 2
			);
		}

		//Start 2004.08.27 Kazika 変更
		//タブウィンドウの設定を変更をブロードキャストする
		CAppNodeManager::getInstance()->ResetGroupId();
		CAppNodeGroupHandle(0).PostMessageToAllEditors(
			MYWM_TAB_WINDOW_NOTIFY,						//タブウィンドウイベント
			(WPARAM)((GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin) ? TWNT_MODE_DISABLE : TWNT_MODE_ENABLE),//タブモード有効/無効化イベント
			(LPARAM)GetEditWindow()->GetHwnd(),	//CEditWndのウィンドウハンドル
			m_pCommanderView->GetHwnd());									//自分自身
		//End 2004.08.27 Kazika
	}
}
//End 2004.07.14 Kazika



/* グループを閉じる */	// 2007.06.20 ryoji 追加
void CViewCommander::Command_GROUPCLOSE( void )
{
	if( GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd && !GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin ){
		int nGroup = CAppNodeManager::getInstance()->GetEditNode( GetMainWindow() )->GetGroup();
		CControlTray::CloseAllEditor( TRUE, GetMainWindow(), TRUE, nGroup );
	}
	return;
}



/* 次のグループ */			// 2007.06.20 ryoji
void CViewCommander::Command_NEXTGROUP( void )
{
	CTabWnd* pcTabWnd = &GetEditWindow()->m_cTabWnd;
	if( pcTabWnd->GetHwnd() == NULL )
		return;
	pcTabWnd->NextGroup();
}



/* 前のグループ */			// 2007.06.20 ryoji
void CViewCommander::Command_PREVGROUP( void )
{
	CTabWnd* pcTabWnd = &GetEditWindow()->m_cTabWnd;
	if( pcTabWnd->GetHwnd() == NULL )
		return;
	pcTabWnd->PrevGroup();
}



/* タブを右に移動 */		// 2007.06.20 ryoji
void CViewCommander::Command_TAB_MOVERIGHT( void )
{
	CTabWnd* pcTabWnd = &GetEditWindow()->m_cTabWnd;
	if( pcTabWnd->GetHwnd() == NULL )
		return;
	pcTabWnd->MoveRight();
}



/* タブを左に移動 */		// 2007.06.20 ryoji
void CViewCommander::Command_TAB_MOVELEFT( void )
{
	CTabWnd* pcTabWnd = &GetEditWindow()->m_cTabWnd;
	if( pcTabWnd->GetHwnd() == NULL )
		return;
	pcTabWnd->MoveLeft();
}



/* 新規グループ */			// 2007.06.20 ryoji
void CViewCommander::Command_TAB_SEPARATE( void )
{
	CTabWnd* pcTabWnd = &GetEditWindow()->m_cTabWnd;
	if( pcTabWnd->GetHwnd() == NULL )
		return;
	pcTabWnd->Separate();
}



/* 次のグループに移動 */	// 2007.06.20 ryoji
void CViewCommander::Command_TAB_JOINTNEXT( void )
{
	CTabWnd* pcTabWnd = &GetEditWindow()->m_cTabWnd;
	if( pcTabWnd->GetHwnd() == NULL )
		return;
	pcTabWnd->JoinNext();
}



/* 前のグループに移動 */	// 2007.06.20 ryoji
void CViewCommander::Command_TAB_JOINTPREV( void )
{
	CTabWnd* pcTabWnd = &GetEditWindow()->m_cTabWnd;
	if( pcTabWnd->GetHwnd() == NULL )
		return;
	pcTabWnd->JoinPrev();
}



/* 左をすべて閉じる */		// 2008.11.22 syat
void CViewCommander::Command_TAB_CLOSELEFT( void )
{
	if( GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd ){
		int nGroup = 0;

		// ウィンドウ一覧を取得する
		EditNode* pEditNode;
		int nCount = CAppNodeManager::getInstance()->GetOpenedWindowArr( &pEditNode, TRUE );
		BOOL bSelfFound = FALSE;
		if( 0 >= nCount )return;

		for( int i = 0; i < nCount; i++ ){
			if( pEditNode[i].m_hWnd == GetMainWindow() ){
				pEditNode[i].m_hWnd = NULL;		//自分自身は閉じない
				nGroup = pEditNode[i].m_nGroup;
				bSelfFound = TRUE;
			}else if( bSelfFound ){
				pEditNode[i].m_hWnd = NULL;		//右は閉じない
			}
		}

		//終了要求を出す
		CAppNodeGroupHandle(nGroup).RequestCloseEditor( pEditNode, nCount, FALSE, TRUE, GetMainWindow() );
		delete []pEditNode;
	}
	return;
}



/* 右をすべて閉じる */		// 2008.11.22 syat
void CViewCommander::Command_TAB_CLOSERIGHT( void )
{
	if( GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd ){
		int nGroup = 0;

		// ウィンドウ一覧を取得する
		EditNode* pEditNode;
		int nCount = CAppNodeManager::getInstance()->GetOpenedWindowArr( &pEditNode, TRUE );
		BOOL bSelfFound = FALSE;
		if( 0 >= nCount )return;

		for( int i = 0; i < nCount; i++ ){
			if( pEditNode[i].m_hWnd == GetMainWindow() ){
				pEditNode[i].m_hWnd = NULL;		//自分自身は閉じない
				nGroup = pEditNode[i].m_nGroup;
				bSelfFound = TRUE;
			}else if( !bSelfFound ){
				pEditNode[i].m_hWnd = NULL;		//左は閉じない
			}
		}

		//終了要求を出す
		CAppNodeGroupHandle(nGroup).RequestCloseEditor( pEditNode, nCount, FALSE, TRUE, GetMainWindow() );
		delete []pEditNode;
	}
	return;
}



//縦方向に最大化
void CViewCommander::Command_MAXIMIZE_V( void )
{
	HWND	hwndFrame;
	RECT	rcOrg;
	RECT	rcDesktop;
	hwndFrame = GetMainWindow();
	::GetWindowRect( hwndFrame, &rcOrg );
	//	May 01, 2004 genta マルチモニタ対応
	::GetMonitorWorkRect( hwndFrame, &rcDesktop );
	::SetWindowPos(
		hwndFrame, 0,
		rcOrg.left, rcDesktop.top,
		rcOrg.right - rcOrg.left, rcDesktop.bottom - rcDesktop.top,
		SWP_NOOWNERZORDER | SWP_NOZORDER
	);
	return;
}



//2001.02.10 Start by MIK: 横方向に最大化
//横方向に最大化
void CViewCommander::Command_MAXIMIZE_H( void )
{
	HWND	hwndFrame;
	RECT	rcOrg;
	RECT	rcDesktop;

	hwndFrame = GetMainWindow();
	::GetWindowRect( hwndFrame, &rcOrg );
	//	May 01, 2004 genta マルチモニタ対応
	::GetMonitorWorkRect( hwndFrame, &rcDesktop );
	::SetWindowPos(
		hwndFrame, 0,
		rcDesktop.left, rcOrg.top,
		rcDesktop.right - rcDesktop.left, rcOrg.bottom - rcOrg.top,
		SWP_NOOWNERZORDER | SWP_NOZORDER
	);
	return;
}
//2001.02.10 End: 横方向に最大化



/* すべて最小化 */	//	Sept. 17, 2000 jepro 説明の「全て」を「すべて」に統一
void CViewCommander::Command_MINIMIZE_ALL( void )
{
	HWND*	phWndArr;
	int		i;
	int		j;
	j = GetDllShareData().m_sNodes.m_nEditArrNum;
	if( 0 == j ){
		return;
	}
	phWndArr = new HWND[j];
	for( i = 0; i < j; ++i ){
		phWndArr[i] = GetDllShareData().m_sNodes.m_pEditArr[i].GetHwnd();
	}
	for( i = 0; i < j; ++i ){
		if( IsSakuraMainWindow( phWndArr[i] ) )
		{
			if( ::IsWindowVisible( phWndArr[i] ) )
				::ShowWindow( phWndArr[i], SW_MINIMIZE );
		}
	}
	delete [] phWndArr;
	return;
}



/* 再描画 */
void CViewCommander::Command_REDRAW( void )
{
	/* フォーカス移動時の再描画 */
	m_pCommanderView->RedrawAll();
	return;
}



//アウトプットウィンドウ表示
void CViewCommander::Command_WIN_OUTPUT( void )
{
	// 2010.05.11 Moca CShareData::OpenDebugWindow()に統合
	// メッセージ表示ウィンドウをViewから親に変更
	// TraceOut経由ではCODE_UNICODE,こちらではCODE_SJISだったのを無指定に変更
	CShareData::getInstance()->OpenDebugWindow( GetMainWindow(), true );
	return;
}



//	from CViewCommander_New.cpp
/*!	@brief マクロ用アウトプットウインドウに表示
	@date 2006.04.26 maru 新規作成
*/
void CViewCommander::Command_TRACEOUT( const wchar_t* outputstr, int nLen, int nFlgOpt )
{
	if( outputstr == NULL )
		return;

	// 0x01 ExpandParameterによる文字列展開有無
	if (nFlgOpt & 0x01) {
		wchar_t Buffer[2048];
		CSakuraEnvironment::ExpandParameter(outputstr, Buffer, 2047);
		CShareData::getInstance()->TraceOutString( Buffer );
	} else {
		CShareData::getInstance()->TraceOutString(outputstr, nLen);
	}

	// 0x02 改行コードの有無
	if ((nFlgOpt & 0x02) == 0) CShareData::getInstance()->TraceOutString( L"\r\n" );

}
