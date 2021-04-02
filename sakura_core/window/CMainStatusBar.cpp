﻿/*! @file */
/*
	Copyright (C) 2018-2021, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#include "StdAfx.h"
#include "CMainStatusBar.h"
#include "window/CEditWnd.h"
#include "CEditApp.h"
#include "apiwrap/CommonControl.h"

CMainStatusBar::CMainStatusBar(CEditWnd* pOwner)
: m_pOwner(pOwner)
, m_hwndStatusBar( NULL )
, m_hwndProgressBar( NULL )
{
}

//	キーワード：ステータスバー順序
/* ステータスバー作成 */
void CMainStatusBar::CreateStatusBar()
{
	if( m_hwndStatusBar )return;

	/* ステータスバー */
	m_hwndStatusBar = ::CreateWindowEx(
		WS_EX_RIGHT | WS_EX_COMPOSITED,
		STATUSCLASSNAME,
		nullptr,
		WS_CHILD/* | WS_VISIBLE*/ | SBARS_SIZEGRIP,	// 2007.03.08 ryoji WS_VISIBLE 除去
		0, 0, 0, 0, // X, Y, nWidth, nHeight
		m_pOwner->GetHwnd(),
		(HMENU)IDW_STATUSBAR,
		CEditApp::getInstance()->GetAppInstance(),
		nullptr
	);

	/* プログレスバー */
	m_hwndProgressBar = ::CreateWindowEx(
		WS_EX_TOOLWINDOW,
		PROGRESS_CLASS,
		NULL,
		WS_CHILD /*|  WS_VISIBLE*/,
		3,
		5,
		150,
		13,
		m_hwndStatusBar,
		NULL,
		CEditApp::getInstance()->GetAppInstance(),
		0
	);

	if( NULL != m_pOwner->m_cFuncKeyWnd.GetHwnd() ){
		m_pOwner->m_cFuncKeyWnd.SizeBox_ONOFF( FALSE );
	}

	//スプリッターの、サイズボックスの位置を変更
	m_pOwner->m_cSplitterWnd.DoSplit( -1, -1);
}

/* ステータスバー破棄 */
void CMainStatusBar::DestroyStatusBar()
{
	if( NULL != m_hwndProgressBar ){
		::DestroyWindow( m_hwndProgressBar );
		m_hwndProgressBar = NULL;
	}
	::DestroyWindow( m_hwndStatusBar );
	m_hwndStatusBar = NULL;

	if( NULL != m_pOwner->m_cFuncKeyWnd.GetHwnd() ){
		bool bSizeBox;
		if( GetDllShareData().m_Common.m_sWindow.m_nFUNCKEYWND_Place == 0 ){	/* ファンクションキー表示位置／0:上 1:下 */
			/* サイズボックスの表示／非表示切り替え */
			bSizeBox = false;
		}
		else{
			bSizeBox = true;
			/* ステータスパーを表示している場合はサイズボックスを表示しない */
			if( NULL != m_hwndStatusBar ){
				bSizeBox = false;
			}
		}
		m_pOwner->m_cFuncKeyWnd.SizeBox_ONOFF( bSizeBox );
	}
	//スプリッターの、サイズボックスの位置を変更
	m_pOwner->m_cSplitterWnd.DoSplit( -1, -1 );
}

/*!
	@brief メッセージの表示
	
	指定されたメッセージをステータスバーに表示する．
	メニューバー右端に入らないものや，桁位置表示を隠したくないものに使う
	
	呼び出し前にSendStatusMessage2IsEffective()で処理の有無を
	確認することで無駄な処理を省くことが出来る．

	@param msg [in] 表示するメッセージ
	@date 2005.07.09 genta 新規作成
	
	@sa SendStatusMessage2IsEffective
*/
void CMainStatusBar::SendStatusMessage2( const WCHAR* msg )
{
	if( NULL != m_hwndStatusBar ){
		SetStatusText(0, SBT_NOBORDERS, msg);
	}
}

/*!
	@brief 文字列をステータスバーの指定されたパートに表示する
	
	@param nIndex [in] パートのインデクス
	@param nOption [in] 描画オペレーションタイプ
	@param pszText [in] 表示テキスト
	@param textLen [in] 表示テキストの文字数
*/
bool CMainStatusBar::SetStatusText(int nIndex, int nOption, const WCHAR* pszText, size_t textLen /* = SIZE_MAX */)
{
	if( !m_hwndStatusBar ){
		assert(m_hwndStatusBar != NULL);
		return false;
	}
	// StatusBar_SetText 関数を呼びだすかどうかを判定する
	// （StatusBar_SetText は SB_SETTEXT メッセージを SendMessage で送信する）
	bool bDraw = true;
	do {
		// オーナードローの場合は SB_SETTEXT メッセージを無条件に発行するように判定
		// 本来表示に変化が無い場合には呼び出さない方が表示のちらつきが減るので好ましいが
		// 判定が難しいので諦める
		if( nOption == SBT_OWNERDRAW ){
			break;
		}
		// オーナードローではない場合で NULLの場合は空文字に置き換える
		// NULL を渡しても問題が無いのかどうか公式ドキュメントに記載されていない
		// NULL のままでも問題は発生しないようだが念の為に対策を追加
		if( pszText == NULL ){
			static const wchar_t emptyStr[] = L"";
			pszText = emptyStr;
			textLen = 0;
		}
		LRESULT res = ::StatusBar_GetTextLength( m_hwndStatusBar, nIndex );
		// 表示オペレーション値が変化する場合は SB_SETTEXT メッセージを発行
		if( HIWORD(res) != nOption ){
			break;
		}
		size_t prevTextLen = LOWORD(res);
		WCHAR prev[1024];
		// 設定済みの文字列長が長過ぎて取得できない場合は、SB_SETTEXT メッセージを発行
		if( prevTextLen >= _countof(prev) ){
			break;
		}
		// 設定する文字列長パラメータが SIZE_MAX（引数のデフォルト値）な場合は文字列長を取得
		if( textLen == SIZE_MAX ){
			textLen = wcslen(pszText);
		}
		// 設定済みの文字列長と設定する文字列長が異なる場合は、SB_SETTEXT メッセージを発行
		if( prevTextLen != textLen ){
			break;
		}
		if( prevTextLen > 0 ){
			::StatusBar_GetText( m_hwndStatusBar, nIndex, prev );
			// 設定済みの文字列と設定する文字列を比較して異なる場合は、SB_SETTEXT メッセージを発行
			bDraw = wcscmp(prev, pszText) != 0;
		}
		else {
			// 設定する文字列長が0の場合は設定する文字列長が0より大きい場合のみ設定する（既に空文字なら空文字を設定する必要は無い為）
			bDraw = textLen > 0;
		}
	}while (false);
	if (bDraw) {
		StatusBar_SetText(m_hwndStatusBar, nIndex | nOption, pszText);
	}
	return bDraw;
}
