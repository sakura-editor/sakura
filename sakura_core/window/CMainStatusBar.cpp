#include "stdafx.h"
#include "CMainStatusBar.h"
#include "window/CEditWnd.h"
#include "CEditApp.h"

CMainStatusBar::CMainStatusBar(CEditWnd* pOwner)
: m_pOwner(pOwner)
, m_hwndStatusBar( NULL )
, m_hwndProgressBar( NULL )
{
}


//	�L�[���[�h�F�X�e�[�^�X�o�[����
/* �X�e�[�^�X�o�[�쐬 */
void CMainStatusBar::CreateStatusBar()
{
	if( m_hwndStatusBar )return;

	/* �X�e�[�^�X�o�[ */
	m_hwndStatusBar = ::CreateStatusWindow(
		WS_CHILD/* | WS_VISIBLE*/ | WS_EX_RIGHT | SBARS_SIZEGRIP,	// 2007.03.08 ryoji WS_VISIBLE ����
		_T(""),
		m_pOwner->GetHwnd(),
		IDW_STATUSBAR
	);

	/* �v���O���X�o�[ */
	m_hwndProgressBar = ::CreateWindowEx(
		WS_EX_TOOLWINDOW,
		PROGRESS_CLASS,
		NULL,
		WS_CHILD /*|  WS_VISIBLE*/,
		3,
		5,
		150,
		13,
		m_hwndStatusBar,
		NULL,
		CEditApp::Instance()->GetAppInstance(),
		0
	);

	if( NULL != m_pOwner->m_CFuncKeyWnd.GetHwnd() ){
		m_pOwner->m_CFuncKeyWnd.SizeBox_ONOFF( FALSE );
	}

	//�X�v���b�^�[�́A�T�C�Y�{�b�N�X�̈ʒu��ύX
	m_pOwner->m_cSplitterWnd.DoSplit( -1, -1);
}


/* �X�e�[�^�X�o�[�j�� */
void CMainStatusBar::DestroyStatusBar()
{
	if( NULL != m_hwndProgressBar ){
		::DestroyWindow( m_hwndProgressBar );
		m_hwndProgressBar = NULL;
	}
	::DestroyWindow( m_hwndStatusBar );
	m_hwndStatusBar = NULL;

	if( NULL != m_pOwner->m_CFuncKeyWnd.GetHwnd() ){
		bool bSizeBox;
		if( GetDllShareData().m_Common.m_sWindow.m_nFUNCKEYWND_Place == 0 ){	/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
			/* �T�C�Y�{�b�N�X�̕\���^��\���؂�ւ� */
			bSizeBox = false;
		}
		else{
			bSizeBox = true;
			/* �X�e�[�^�X�p�[��\�����Ă���ꍇ�̓T�C�Y�{�b�N�X��\�����Ȃ� */
			if( NULL != m_hwndStatusBar ){
				bSizeBox = false;
			}
		}
		m_pOwner->m_CFuncKeyWnd.SizeBox_ONOFF( bSizeBox );
	}
	//�X�v���b�^�[�́A�T�C�Y�{�b�N�X�̈ʒu��ύX
	m_pOwner->m_cSplitterWnd.DoSplit( -1, -1 );
}


/*!
	@brief ���b�Z�[�W�̕\��
	
	�w�肳�ꂽ���b�Z�[�W���X�e�[�^�X�o�[�ɕ\������D
	���j���[�o�[�E�[�ɓ���Ȃ����̂�C���ʒu�\�����B�������Ȃ����̂Ɏg��
	
	�Ăяo���O��SendStatusMessage2IsEffective()�ŏ����̗L����
	�m�F���邱�ƂŖ��ʂȏ������Ȃ����Ƃ��o����D

	@param msg [in] �\�����郁�b�Z�[�W
	@date 2005.07.09 genta �V�K�쐬
	
	@sa SendStatusMessage2IsEffective
*/
void CMainStatusBar::SendStatusMessage2( const TCHAR* msg )
{
	if( NULL != m_hwndStatusBar ){
		// �X�e�[�^�X�o�[��
		StatusBar_SetText( m_hwndStatusBar,0 | SBT_NOBORDERS,msg );
	}
}


void CMainStatusBar::SetStatusText(int nIndex, int nOption, const TCHAR* pszText)
{
	StatusBar_SetText( m_hwndStatusBar, nIndex | nOption, pszText );
}


