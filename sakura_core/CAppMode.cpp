#include "StdAfx.h"
#include "CAppMode.h"
#include "window/CEditWnd.h"
#include "env/CSakuraEnvironment.h"

void CAppMode::OnAfterSave(const SSaveInfo& sSaveInfo)
{
	m_bViewMode = false;	/* �r���[���[�h */

	// ���O��t���ĕۑ�����ă��[�h���������ꂽ���̕s��������ǉ��iANSI�łƂ̍��فj	// 2009.08.12 ryoji
	if( IsDebugMode() ){
		SetDebugModeOFF();	// �A�E�g�v�b�g�E�B���h�E�͒ʏ�E�B���h�E��
	}
}

//! �f�o�b�O���j�^���[�h�ɐݒ�
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
	this->SetViewMode(false);	// �r���[���[�h	// 2001/06/23 N.Nakatani �A�E�g�v�b�g���ւ̏o�̓e�L�X�g�̒ǉ�F_ADDTAIL_W���}�~�����̂łƂ肠�����r���[���[�h�͎��߂܂���
	CEditWnd::getInstance()->UpdateCaption();
}

// 2005.06.24 Moca
//! �f�o�b�N���j�^���[�h�̉���
void CAppMode::SetDebugModeOFF()
{
	DLLSHAREDATA* pShare = &GetDllShareData();
	if( pShare->m_sHandles.m_hwndDebug == CEditWnd::getInstance()->GetHwnd() ){
		pShare->m_sHandles.m_hwndDebug = NULL;
		this->_SetDebugMode(false);
		CEditWnd::getInstance()->UpdateCaption();
	}
}
