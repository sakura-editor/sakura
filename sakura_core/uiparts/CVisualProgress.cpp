/*! @file */
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
#include "CVisualProgress.h"
#include "CWaitCursor.h"

#include "window/CEditWnd.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CVisualProgress::CVisualProgress()
: m_pcWaitCursor(NULL)
, nOldValue(-1)
{
}

CVisualProgress::~CVisualProgress()
{
	SAFE_DELETE(m_pcWaitCursor);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ロード前後                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CVisualProgress::OnBeforeLoad(SLoadInfo* sLoadInfo)
{
	_Begin();
}

void CVisualProgress::OnAfterLoad(const SLoadInfo& sLoadInfo)
{
	_End();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        セーブ前後                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CVisualProgress::OnBeforeSave(const SSaveInfo& sSaveInfo)
{
	_Begin();
}

void CVisualProgress::OnFinalSave(ESaveResult eSaveResult)
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
		::ShowWindow( hwndProgress, SW_SHOW );
		//範囲設定・リセット
		Progress_SetRange( hwndProgress, 0, 101 );
		Progress_SetPos( hwndProgress, 0);
	}
}

void CVisualProgress::_Doing(int nPer)
{
	//プログレスバー更新
	HWND hwndProgress = CEditWnd::getInstance()->m_cStatusBar.GetProgressHwnd();
	if(hwndProgress){
		if( nOldValue != nPer ){
			Progress_SetPos( hwndProgress, nPer + 1 ); // 2013.06.10 Moca Vista/7等でプログレスバーがアニメーションで遅れる対策
			Progress_SetPos( hwndProgress, nPer );
			nOldValue = nPer;
		}
	}
}

void CVisualProgress::_End()
{
	//プログレスバー
	HWND hwndProgress = CEditWnd::getInstance()->m_cStatusBar.GetProgressHwnd();
	if( hwndProgress ){
		Progress_SetPos( hwndProgress, 0);
		::ShowWindow( hwndProgress, SW_HIDE );
	}

	//砂時計
	SAFE_DELETE(m_pcWaitCursor);
}
