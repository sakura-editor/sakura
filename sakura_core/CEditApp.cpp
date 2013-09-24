/*
	Copyright (C) 2007, kobake

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
#include "CEditApp.h"
#include "doc/CEditDoc.h"
#include "window/CEditWnd.h"
#include "CLoadAgent.h"
#include "CSaveAgent.h"
#include "uiparts/CVisualProgress.h"
#include "recent/CMruListener.h"
#include "macro/CSMacroMgr.h"
#include "CPropertyManager.h"
#include "CGrepAgent.h"
#include "_main/CAppMode.h"
#include "_main/CCommandLine.h"
#include "util/module.h"
#include "util/shell.h"

void CEditApp::Create(HINSTANCE hInst, int nGroupId)
{
	m_hInst = hInst;

	//�w���p�쐬
	m_cIcons.Create( m_hInst );	//	CreateImage List

	//�h�L�������g�̍쐬
	m_pcEditDoc = new CEditDoc(this);
	m_pcEditDoc->Create();

	//IO�Ǘ�
	m_pcLoadAgent = new CLoadAgent();
	m_pcSaveAgent = new CSaveAgent();
	m_pcVisualProgress = new CVisualProgress();

	//GREP���[�h�Ǘ�
	m_pcGrepAgent = new CGrepAgent();

	//�ҏW���[�h
	CAppMode::getInstance();	//�E�B���h�E�����O�ɃC�x���g���󂯎�邽�߂ɂ����ŃC���X�^���X�쐬

	//�}�N��
	m_pcSMacroMgr = new CSMacroMgr();

	//�E�B���h�E�̍쐬
	m_pcEditWnd = CEditWnd::getInstance();
	m_pcEditWnd->Create( &m_cIcons, nGroupId );

	//MRU�Ǘ�
	m_pcMruListener = new CMruListener();

	//�v���p�e�B�Ǘ�
	m_pcPropertyManager = new CPropertyManager();
	m_pcPropertyManager->Create(
		m_pcEditWnd->GetHwnd(),
		&GetIcons(),
		&m_pcEditWnd->GetMenuDrawer()
	);
}

CEditApp::~CEditApp()
{
	delete m_pcSMacroMgr;
	delete m_pcPropertyManager;
	delete m_pcMruListener;
	delete m_pcGrepAgent;
	delete m_pcVisualProgress;
	delete m_pcSaveAgent;
	delete m_pcLoadAgent;
	delete m_pcEditDoc;
}

/*! ���ʐݒ� �v���p�e�B�V�[�g */
bool CEditApp::OpenPropertySheet( int nPageNum )
{
	/* �v���p�e�B�V�[�g�̍쐬 */
	bool bRet = m_pcPropertyManager->OpenPropertySheet( m_pcEditWnd->GetHwnd(), nPageNum );
	if( bRet ){
		// 2007.10.19 genta �}�N���o�^�ύX�𔽉f���邽�߁C�ǂݍ��ݍς݂̃}�N����j������
		m_pcSMacroMgr->UnloadAll();
	}

	return bRet;
}

/*! �^�C�v�ʐݒ� �v���p�e�B�V�[�g */
bool CEditApp::OpenPropertySheetTypes( int nPageNum, CTypeConfig nSettingType )
{
	int nTextWrapMethodOld = m_pcEditWnd->GetDocument()->m_cDocType.GetDocumentAttribute().m_nTextWrapMethod;

	bool bRet = m_pcPropertyManager->OpenPropertySheetTypes( m_pcEditWnd->GetHwnd(), nPageNum, nSettingType );
	if( bRet ){
		// 2008.06.01 nasukoji	�e�L�X�g�̐܂�Ԃ��ʒu�ύX�Ή�
		// �^�C�v�ʐݒ���Ăяo�����E�B���h�E�ɂ��ẮA�^�C�v�ʐݒ肪�ύX���ꂽ��
		// �܂�Ԃ����@�̈ꎞ�ݒ�K�p�����������ă^�C�v�ʐݒ��L���Ƃ���B
		if( nTextWrapMethodOld != m_pcEditWnd->GetDocument()->m_cDocType.GetDocumentAttribute().m_nTextWrapMethod ){		// �ݒ肪�ύX���ꂽ
			m_pcEditWnd->GetDocument()->m_bTextWrapMethodCurTemp = false;	// �ꎞ�ݒ�K�p��������
		}
	}

	return bRet;
}

