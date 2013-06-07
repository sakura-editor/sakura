/*
	Copyright (C) 2008, kobake

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
#include "CDocType.h"
#include "CEditDoc.h"
#include "CEditApp.h"
#include "window/CEditWnd.h"
#include "CGrepAgent.h"
#include "view/colors/CColorStrategy.h"
#include "view/figures/CFigureManager.h"

CDocType::CDocType(CEditDoc* pcDoc)
: m_pcDocRef(pcDoc)
, m_nSettingType( 0 )			// Sep. 11, 2002 genta
, m_pType ( &CDocTypeManager().GetTypeSetting(m_nSettingType) )
, m_nSettingTypeLocked( false )	//	設定値変更可能フラグ
{
}

//! 文書種別の設定
void CDocType::SetDocumentType(CTypeConfig type, bool force, bool bTypeOnly )
{
	if( !m_nSettingTypeLocked || force ){
		m_nSettingType = type;
		m_pType = &CDocTypeManager().GetTypeSetting(m_nSettingType);
		if( bTypeOnly ) return;	// bTypeOnly == true は特殊ケース（一時利用）に限定
		UnlockDocumentType();
		CDocTypeManager().GetTypeSetting(m_nSettingType).m_nRegexKeyMagicNumber++;	//@@@ 2001.11.17 add MIK
		this->SetDocumentIcon();	// Sep. 11, 2002 genta
		m_pcDocRef->SetBackgroundImage();

		// タイプ別設定更新を反映
		CColorStrategyPool::getInstance()->OnChangeSetting();
		CFigureManager::getInstance()->OnChangeSetting();
	}
}

/*!
	アイコンの設定
	
	タイプ別設定に応じてウィンドウアイコンをファイルに関連づけられた物，
	または標準のものに設定する．
	
	@author genta
	@date 2002.09.10
*/
void CDocType::SetDocumentIcon()
{
	if( CEditApp::getInstance()->m_pcGrepAgent->m_bGrepMode )	// Grepモードの時はアイコンを変更しない
		return;
	
	HICON	hIconBig, hIconSmall;
	if( this->GetDocumentAttribute().m_bUseDocumentIcon )
		m_pcDocRef->m_pcEditWnd->GetRelatedIcon( m_pcDocRef->m_cDocFile.GetFilePath(), &hIconBig, &hIconSmall );
	else
		m_pcDocRef->m_pcEditWnd->GetDefaultIcon( &hIconBig, &hIconSmall );

	m_pcDocRef->m_pcEditWnd->SetWindowIcon( hIconBig, ICON_BIG );
	m_pcDocRef->m_pcEditWnd->SetWindowIcon( hIconSmall, ICON_SMALL );
}
