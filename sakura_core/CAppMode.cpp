#include "stdafx.h"
#include "CAppMode.h"
#include "env/CSakuraEnvironment.h"


//! �f�o�b�O���j�^���[�h�ɐݒ�
void CAppMode::SetDebugModeON()
{
	DLLSHAREDATA* pShare = &GetDllShareData();
	if( pShare->m_sHandles.m_hwndDebug ){
		if( IsSakuraMainWindow( pShare->m_sHandles.m_hwndDebug ) ){
			return;
		}
	}
	pShare->m_sHandles.m_hwndDebug = CEditWnd::Instance()->GetHwnd();
	this->_SetDebugMode(true);
	this->SetViewMode(false);	// �r���[���[�h	// 2001/06/23 N.Nakatani �A�E�g�v�b�g���ւ̏o�̓e�L�X�g�̒ǉ�F_ADDTAIL_W���}�~�����̂łƂ肠�����r���[���[�h�͎��߂܂���
	CEditWnd::Instance()->UpdateCaption();
}

// 2005.06.24 Moca
//! �f�o�b�N���j�^���[�h�̉���
void CAppMode::SetDebugModeOFF()
{
	DLLSHAREDATA* pShare = &GetDllShareData();
	if( pShare->m_sHandles.m_hwndDebug == CEditWnd::Instance()->GetHwnd() ){
		pShare->m_sHandles.m_hwndDebug = NULL;
		this->_SetDebugMode(false);
		CEditWnd::Instance()->UpdateCaption();
	}
}
