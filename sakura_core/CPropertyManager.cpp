#include "stdafx.h"
#include "CPropertyManager.h"
#include "window/CEditWnd.h"
#include "CEditApp.h"
#include "env/DLLSHAREDATA.h"
#include "macro/CSMacroMgr.h"

CPropertyManager::CPropertyManager()
{
	/* �ݒ�v���p�e�B�V�[�g�̏������P */
	m_cPropCommon.Create( CEditWnd::Instance()->GetHwnd(), &CEditApp::Instance()->GetIcons(), CEditApp::Instance()->m_pcSMacroMgr, &CEditWnd::Instance()->GetMenuDrawer() );
	m_cPropTypes.Create( G_AppInstance(), CEditWnd::Instance()->GetHwnd() );
}

/*! ���ʐݒ� �v���p�e�B�V�[�g */
BOOL CPropertyManager::OpenPropertySheet( int nPageNum )
{
	// 2002.12.11 Moca ���̕����ōs���Ă����f�[�^�̃R�s�[��CPropCommon�Ɉړ��E�֐���
	// ���ʐݒ�̈ꎞ�ݒ�̈��SharaData���R�s�[����
	m_cPropCommon.InitData();
	
	/* �v���p�e�B�V�[�g�̍쐬 */
	if( m_cPropCommon.DoPropertySheet( nPageNum ) ){

		// 2002.12.11 Moca ���̕����ōs���Ă����f�[�^�̃R�s�[��CPropCommon�Ɉړ��E�֐���
		// ShareData �� �ݒ��K�p�E�R�s�[����
		// 2007.06.20 ryoji �O���[�v���ɕύX���������Ƃ��̓O���[�vID�����Z�b�g����
		BOOL bGroup = (GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd && !GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin);
		m_cPropCommon.ApplyData();
		// note: ��{�I�ɂ����œK�p���Ȃ��ŁAMYWM_CHANGESETTING���炽�ǂ��ēK�p���Ă��������B
		// ���E�B���h�E�ɂ͍Ō�ɒʒm����܂��B���́AOnChangeSetting �ɂ���܂��B
		// �����ł����K�p���Ȃ��ƁA�ق��̃E�B���h�E���ύX����܂���B
		
		CEditApp::Instance()->m_pcSMacroMgr->UnloadAll();	// 2007.10.19 genta �}�N���o�^�ύX�𔽉f���邽�߁C�ǂݍ��ݍς݂̃}�N����j������
		if( bGroup != (GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd && !GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin ) ){
			CAppNodeManager::Instance()->ResetGroupId();
		}

		/* �A�N�Z�����[�^�e�[�u���̍č쐬 */
		::SendMessageAny( GetDllShareData().m_sHandles.m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)0 );


		/* �ݒ�ύX�𔽉f������ */
		/* �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g���� */
		CAppNodeGroupHandle(0).SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			0,
			(LPARAM)CEditWnd::Instance()->GetHwnd(),
			CEditWnd::Instance()->GetHwnd()
		);

		return TRUE;
	}else{
		return FALSE;
	}
}



/*! �^�C�v�ʐݒ� �v���p�e�B�V�[�g */
BOOL CPropertyManager::OpenPropertySheetTypes( int nPageNum, CTypeConfig nSettingType )
{
	STypeConfig& types = CDocTypeManager().GetTypeSetting(nSettingType);
	m_cPropTypes.SetTypeData( types );
	// Mar. 31, 2003 genta �������팸�̂��߃|�C���^�ɕύX��ProperySheet���Ŏ擾����悤��
	//m_cPropTypes.m_CKeyWordSetMgr = GetDllShareData().m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr;

	/* �v���p�e�B�V�[�g�̍쐬 */
	if( m_cPropTypes.DoPropertySheet( nPageNum ) ){
		/* �ύX���ꂽ�ݒ�l�̃R�s�[ */
		int nTextWrapMethodOld = CEditWnd::Instance()->GetDocument().m_cDocType.GetDocumentAttribute().m_nTextWrapMethod;
		m_cPropTypes.GetTypeData( types );

		// 2008.06.01 nasukoji	�e�L�X�g�̐܂�Ԃ��ʒu�ύX�Ή�
		// �^�C�v�ʐݒ���Ăяo�����E�B���h�E�ɂ��ẮA�^�C�v�ʐݒ肪�ύX���ꂽ��
		// �܂�Ԃ����@�̈ꎞ�ݒ�K�p�����������ă^�C�v�ʐݒ��L���Ƃ���B
		if( nTextWrapMethodOld != CEditWnd::Instance()->GetDocument().m_cDocType.GetDocumentAttribute().m_nTextWrapMethod )		// �ݒ肪�ύX���ꂽ
			CEditWnd::Instance()->GetDocument().m_bTextWrapMethodCurTemp = false;	// �ꎞ�ݒ�K�p��������

		/* �A�N�Z�����[�^�e�[�u���̍č쐬 */
		::SendMessageAny( GetDllShareData().m_sHandles.m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)0 );

		/* �ݒ�ύX�𔽉f������ */
		/* �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g���� */
		CAppNodeGroupHandle(0).SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			0,
			(LPARAM)CEditWnd::Instance()->GetHwnd(),
			CEditWnd::Instance()->GetHwnd()
		);

		return TRUE;
	}else{
		return FALSE;
	}
}

