#include "stdafx.h"
#include "CEditApp.h"
#include "util/module.h"
#include "CEditWnd.h"
#include "util/shell.h"
#include "CCommandLine.h"

#pragma warning(disable:4355) //�uthis�|�C���^�����������X�g�Ŏg�p����܂����v�̌x���𖳌���

CEditApp::CEditApp(HINSTANCE hInst)
: m_hInst(hInst)
, m_cEditDoc(this)
{
	//�w���p�쐬
	m_cIcons.Create( m_hInst );	//	CreateImage List

	//�h�L�������g�̍쐬
	if( !m_cEditDoc.Create( m_hInst, &m_cIcons ) ){
		ErrorMessage( NULL, _T("�h�L�������g�̍쐬�Ɏ��s���܂���") );
	}

	//�E�B���h�E�̍쐬
	m_pcEditWnd = new CEditWnd();
	m_pcEditWnd->Create(
		hInst,
		CCommandLine::Instance()->GetGroupId()
	);
}

CEditApp::~CEditApp()
{
	delete m_pcEditWnd;
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
			CEditApp::Instance()->GetHelpFilePath(),
			HELP_CONTEXT,
			nHelpContextID
		);
	}
}
