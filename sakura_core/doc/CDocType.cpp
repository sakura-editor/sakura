#include "stdafx.h"
#include "CDocType.h"

CDocType::CDocType(CEditDoc* pcDoc)
: m_pcDocRef(pcDoc)
, m_nSettingTypeLocked( false )	//	�ݒ�l�ύX�\�t���O
, m_nSettingType( 0 )			// Sep. 11, 2002 genta
{
}

//	������ʂ̐ݒ�
void CDocType::SetDocumentType(CTypeConfig type, bool force)
{
	if( !m_nSettingTypeLocked || force ){
		m_nSettingType = type;
		UnlockDocumentType();
		GetDllShareData().GetTypeSetting(m_nSettingType).m_nRegexKeyMagicNumber++;	//@@@ 2001.11.17 add MIK
		this->SetDocumentIcon();	// Sep. 11, 2002 genta
	}
}

/*!
	�A�C�R���̐ݒ�
	
	�^�C�v�ʐݒ�ɉ����ăE�B���h�E�A�C�R�����t�@�C���Ɋ֘A�Â���ꂽ���C
	�܂��͕W���̂��̂ɐݒ肷��D
	
	@author genta
	@date 2002.09.10
*/
void CDocType::SetDocumentIcon()
{
	if( CEditApp::Instance()->m_pcGrepAgent->m_bGrepMode )	// Grep���[�h�̎��̓A�C�R����ύX���Ȃ�
		return;
	
	HICON	hIconBig, hIconSmall;
	if( this->GetDocumentAttribute().m_bUseDocumentIcon )
		m_pcDocRef->m_pcEditWnd->GetRelatedIcon( m_pcDocRef->m_cDocFile.GetFilePath(), &hIconBig, &hIconSmall );
	else
		m_pcDocRef->m_pcEditWnd->GetDefaultIcon( &hIconBig, &hIconSmall );

	m_pcDocRef->m_pcEditWnd->SetWindowIcon( hIconBig, ICON_BIG );
	m_pcDocRef->m_pcEditWnd->SetWindowIcon( hIconSmall, ICON_SMALL );
}
