#include "StdAfx.h"
#include "CEditApp.h"
#include "doc/CEditDoc.h"
#include "window/CEditWnd.h"
#include "CLoadAgent.h"
#include "CSaveAgent.h"
#include "CVisualProgress.h"
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
	if( !m_pcEditDoc->Create( &m_cIcons ) ){
		ErrorMessage( NULL, _T("�h�L�������g�̍쐬�Ɏ��s���܂���") );
	}

	//IO�Ǘ�
	m_pcLoadAgent = new CLoadAgent();
	m_pcSaveAgent = new CSaveAgent();
	m_pcVisualProgress = new CVisualProgress();

	//GREP���[�h�Ǘ�
	m_pcGrepAgent = new CGrepAgent();

	//�ҏW���[�h
	CAppMode::getInstance();	//�E�B���h�E�����O�ɃC�x���g���󂯎�邽�߂ɂ����ŃC���X�^���X�쐬

	//�E�B���h�E�̍쐬
	m_pcEditWnd = new CEditWnd();
	m_pcEditWnd->Create( nGroupId );

	//MRU�Ǘ�
	m_pcMruListener = new CMruListener();

	//�}�N��
	m_pcSMacroMgr = new CSMacroMgr();

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
