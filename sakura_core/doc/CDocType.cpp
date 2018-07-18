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
#include "env/DllShareData.h"

CDocType::CDocType(CEditDoc* pcDoc)
: m_pcDocRef(pcDoc)
, m_nSettingType( 0 )			// Sep. 11, 2002 genta
, m_typeConfig( GetDllShareData().m_TypeBasis )
, m_nSettingTypeLocked( false )	//	設定値変更可能フラグ
{
}

//! 文書種別の設定
void CDocType::SetDocumentType(CTypeConfig type, bool force, bool bTypeOnly )
{
	if( !m_nSettingTypeLocked || force ){
		m_nSettingType = type;
		if( false == CDocTypeManager().GetTypeConfig( m_nSettingType, m_typeConfig ) ){
			// 削除されてる/不正
			m_nSettingType = CDocTypeManager().GetDocumentTypeOfPath(m_pcDocRef->m_cDocFile.GetFilePath());
			CDocTypeManager().GetTypeConfig( m_nSettingType, m_typeConfig );
		}
		if( bTypeOnly ) return;	// bTypeOnly == true は特殊ケース（一時利用）に限定
		UnlockDocumentType();
	}else{
		// データは更新しておく
		CTypeConfig temp = CDocTypeManager().GetDocumentTypeOfId( m_typeConfig.m_id );
		if( temp.IsValidType() ){
			m_nSettingType = temp;
			CDocTypeManager().GetTypeConfig( m_nSettingType, m_typeConfig );
		}else{
			m_nSettingType = type;
			if( false == CDocTypeManager().GetTypeConfig( m_nSettingType, m_typeConfig ) ){
				m_nSettingType = CDocTypeManager().GetDocumentTypeOfPath(m_pcDocRef->m_cDocFile.GetFilePath());
				CDocTypeManager().GetTypeConfig( m_nSettingType, m_typeConfig );
			}
		}
		if( bTypeOnly ) return;
	}

	// タイプ別設定更新を反映
	CColorStrategyPool::getInstance()->OnChangeSetting();
	CFigureManager::getInstance()->OnChangeSetting();
	this->SetDocumentIcon();	// Sep. 11, 2002 genta
	m_pcDocRef->SetBackgroundImage();
}

void CDocType::SetDocumentTypeIdx( int id, bool force )
{
	int setId = m_typeConfig.m_id;
	if( !m_nSettingTypeLocked || force ){
		if( id != -1 ){
			setId = id;
		}
	}
	CTypeConfig temp = CDocTypeManager().GetDocumentTypeOfId( setId );
	if( temp.IsValidType() ){
		m_nSettingType = temp;
		m_typeConfig.m_nIdx = temp.GetIndex();
		m_typeConfig.m_id = setId;
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
