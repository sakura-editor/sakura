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

CEditApp::CEditApp(HINSTANCE hInst, int nGroupId)
: m_hInst(hInst)
{
	//�w���p�쐬
	m_cIcons.Create( m_hInst );	//	CreateImage List

	//�h�L�������g�̍쐬
	m_pcEditDoc = new CEditDoc(this);
	m_pcEditDoc->Create( &m_cIcons );

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
	m_pcEditWnd = new CEditWnd();
	m_pcEditWnd->Create( nGroupId );

	//MRU�Ǘ�
	m_pcMruListener = new CMruListener();

	//�v���p�e�B�Ǘ�
	m_pcPropertyManager = new CPropertyManager();
}

CEditApp::~CEditApp()
{
	delete m_pcSMacroMgr;
	delete m_pcPropertyManager;
	delete m_pcMruListener;
	delete m_pcEditWnd;
	delete m_pcGrepAgent;
	delete m_pcVisualProgress;
	delete m_pcSaveAgent;
	delete m_pcLoadAgent;
	delete m_pcEditDoc;
}


/*! �w���v�t�@�C���̃t���p�X��Ԃ�
 
    @return �p�X���i�[�����o�b�t�@�̃|�C���^
 
    @note ���s�t�@�C���Ɠ����ʒu�� sakura.chm �t�@�C����Ԃ��B
        �p�X�� UNC �̂Ƃ��� _MAX_PATH �Ɏ��܂�Ȃ��\��������B
 
    @date 2002/01/19 aroka �GnMaxLen �����ǉ�
	@date 2007/10/23 kobake ���������̌����C��(in��out)
	@date 2007/10/23 kobake CEditApp�̃����o�֐��ɕύX
	@date 2007/10/23 kobake �V�O�j�`���ύX�Bconst�|�C���^��Ԃ������̃C���^�[�t�F�[�X�ɂ��܂����B
*/
LPCTSTR CEditApp::GetHelpFilePath() const
{
	static TCHAR szHelpFile[_MAX_PATH] = _T("");
	if(szHelpFile[0]==_T('\0')){
		GetExedir( szHelpFile, _T("sakura.chm") );
	}
	return szHelpFile;
}


/* ���j���[�A�C�e���ɑΉ�����w���v��\�� */
void CEditApp::ShowFuncHelp( HWND hwndParent, EFunctionCode nFuncID ) const
{
	/* �@�\ID�ɑΉ�����w���v�R���e�L�X�g�ԍ���Ԃ� */
	int		nHelpContextID = FuncID_To_HelpContextID( nFuncID );
	if( 0 != nHelpContextID ){
		// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		MyWinHelp(
			hwndParent,
			CEditApp::getInstance()->GetHelpFilePath(),
			HELP_CONTEXT,
			nHelpContextID
		);
	}
}
