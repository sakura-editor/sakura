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
#include "CAppMode.h"
#include "window/CEditWnd.h"
#include "env/CSakuraEnvironment.h"

void CAppMode::OnAfterSave(const SSaveInfo& sSaveInfo)
{
	m_bViewMode = false;	/* ビューモード */

	// 名前を付けて保存から再ロードが除去された分の不足処理を追加（ANSI版との差異）	// 2009.08.12 ryoji
	if( IsDebugMode() ){
		SetDebugModeOFF();	// アウトプットウィンドウは通常ウィンドウ化
	}
}

//! デバッグモニタモードに設定
void CAppMode::SetDebugModeON()
{
	DLLSHAREDATA* pShare = &GetDllShareData();
	if( pShare->m_sHandles.m_hwndDebug ){
		if( IsSakuraMainWindow( pShare->m_sHandles.m_hwndDebug ) ){
			return;
		}
	}
	pShare->m_sHandles.m_hwndDebug = CEditWnd::getInstance()->GetHwnd();
	this->_SetDebugMode(true);
	this->SetViewMode(false);	// ビューモード	// 2001/06/23 N.Nakatani アウトプット窓への出力テキストの追加F_ADDTAIL_Wが抑止されるのでとりあえずビューモードは辞めました
	CEditWnd::getInstance()->UpdateCaption();
}

// 2005.06.24 Moca
//! デバックモニタモードの解除
void CAppMode::SetDebugModeOFF()
{
	DLLSHAREDATA* pShare = &GetDllShareData();
	if( pShare->m_sHandles.m_hwndDebug == CEditWnd::getInstance()->GetHwnd() ){
		pShare->m_sHandles.m_hwndDebug = NULL;
		this->_SetDebugMode(false);
		CEditWnd::getInstance()->UpdateCaption();
	}
}
