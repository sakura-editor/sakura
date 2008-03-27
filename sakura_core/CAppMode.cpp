#include "stdafx.h"
#include "CAppMode.h"


//! デバッグモニタモードに設定
void CAppMode::SetDebugModeON()
{
	DLLSHAREDATA* pShare = &GetDllShareData();
	if( pShare->m_hwndDebug ){
		if( CShareData::IsEditWnd( pShare->m_hwndDebug ) ){
			return;
		}
	}
	pShare->m_hwndDebug = CEditWnd::Instance()->GetHwnd();
	this->_SetDebugMode(true);
	this->SetViewMode(false);	// ビューモード	// 2001/06/23 N.Nakatani アウトプット窓への出力テキストの追加F_ADDTAIL_Wが抑止されるのでとりあえずビューモードは辞めました
	CEditWnd::Instance()->UpdateCaption();
}

// 2005.06.24 Moca
//! デバックモニタモードの解除
void CAppMode::SetDebugModeOFF()
{
	DLLSHAREDATA* pShare = &GetDllShareData();
	if( pShare->m_hwndDebug == CEditWnd::Instance()->GetHwnd() ){
		pShare->m_hwndDebug = NULL;
		this->_SetDebugMode(false);
		CEditWnd::Instance()->UpdateCaption();
	}
}
