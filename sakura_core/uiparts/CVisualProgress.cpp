/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "CVisualProgress.h"
#include "CWaitCursor.h"
#include "window/CEditWnd.h"
#include "apiwrap/CommonControl.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CVisualProgress::CVisualProgress()
{
}

CVisualProgress::~CVisualProgress()
{
	SAFE_DELETE(m_pcWaitCursor);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ロード前後                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CVisualProgress::OnBeforeLoad([[maybe_unused]] SLoadInfo* sLoadInfo)
{
	_Begin();
}

void CVisualProgress::OnAfterLoad([[maybe_unused]] const SLoadInfo& sLoadInfo)
{
	_End();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        セーブ前後                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CVisualProgress::OnBeforeSave([[maybe_unused]] const SSaveInfo& sSaveInfo)
{
	_Begin();
}

void CVisualProgress::OnFinalSave([[maybe_unused]] ESaveResult eSaveResult)
{
	_End();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      プログレス受信                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CVisualProgress::OnProgress(int nPer)
{
	_Doing(nPer);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CVisualProgress::_Begin()
{
	//砂時計
	if(!m_pcWaitCursor){
		m_pcWaitCursor = new CWaitCursor( CEditWnd::getInstance()->GetHwnd() );
	}

	//プログレスバー
	HWND hwndProgress = CEditWnd::getInstance()->m_cStatusBar.GetProgressHwnd();
	if( hwndProgress ){
		CEditWnd::getInstance()->m_cStatusBar.ShowProgressBar(true);
		//範囲設定・リセット
		ApiWrap::Progress_SetRange( hwndProgress, 0, 101 );
		ApiWrap::Progress_SetPos( hwndProgress, 0);
	}
}

void CVisualProgress::_Doing(int nPer)
{
	//プログレスバー更新
	HWND hwndProgress = CEditWnd::getInstance()->m_cStatusBar.GetProgressHwnd();
	if(hwndProgress){
		if( nOldValue != nPer ){
			ApiWrap::Progress_SetPos( hwndProgress, nPer + 1 ); // 2013.06.10 Moca Vista/7等でプログレスバーがアニメーションで遅れる対策
			ApiWrap::Progress_SetPos( hwndProgress, nPer );
			nOldValue = nPer;
		}
	}
}

void CVisualProgress::_End()
{
	//プログレスバー
	HWND hwndProgress = CEditWnd::getInstance()->m_cStatusBar.GetProgressHwnd();
	if( hwndProgress ){
		ApiWrap::Progress_SetPos( hwndProgress, 0);
		CEditWnd::getInstance()->m_cStatusBar.ShowProgressBar(false);
	}

	//砂時計
	SAFE_DELETE(m_pcWaitCursor);
}
