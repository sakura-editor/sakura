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
