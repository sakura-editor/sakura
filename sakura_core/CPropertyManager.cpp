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
#include "CPropertyManager.h"
#include "env/DLLSHAREDATA.h"
#include "env/CDocTypeManager.h"

void CPropertyManager::Create( HWND hwndOwner, CImageListMgr* pImageList, CMenuDrawer* menu )
{
	/* �ݒ�v���p�e�B�V�[�g�̏������P */
	m_cPropCommon.Create( hwndOwner, pImageList, menu );
	m_cPropTypes.Create( G_AppInstance(), hwndOwner );
}

/*! ���ʐݒ� �v���p�e�B�V�[�g */
bool CPropertyManager::OpenPropertySheet( HWND hWnd, int nPageNum )
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

		// ������ɃL�[���[�h���㏑�����Ȃ��悤��
		CShareDataLockCounter* pLock = NULL;
		CShareDataLockCounter::WaitLock( m_cPropCommon.m_hwndParent, &pLock );

		m_cPropCommon.ApplyData();
		// note: ��{�I�ɂ����œK�p���Ȃ��ŁAMYWM_CHANGESETTING���炽�ǂ��ēK�p���Ă��������B
		// ���E�B���h�E�ɂ͍Ō�ɒʒm����܂��B���́AOnChangeSetting �ɂ���܂��B
		// �����ł����K�p���Ȃ��ƁA�ق��̃E�B���h�E���ύX����܂���B

		if( bGroup != (GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd && !GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin ) ){
			CAppNodeManager::getInstance()->ResetGroupId();
		}

		/* �A�N�Z�����[�^�e�[�u���̍č쐬 */
		::SendMessageAny( GetDllShareData().m_sHandles.m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)PM_CHANGESETTING_ALL );

		/* �ݒ�ύX�𔽉f������ */
		/* �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g���� */
		CAppNodeGroupHandle(0).SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)0,
			(LPARAM)PM_CHANGESETTING_ALL,
			hWnd
		);

		delete pLock;
		return true;
	}else{
		return false;
	}
}



/*! �^�C�v�ʐݒ� �v���p�e�B�V�[�g */
bool CPropertyManager::OpenPropertySheetTypes( HWND hWnd, int nPageNum, CTypeConfig nSettingType )
{
	STypeConfig& types = CDocTypeManager().GetTypeSetting(nSettingType);
	m_cPropTypes.SetTypeData( types );
	// Mar. 31, 2003 genta �������팸�̂��߃|�C���^�ɕύX��ProperySheet���Ŏ擾����悤��

	/* �v���p�e�B�V�[�g�̍쐬 */
	if( m_cPropTypes.DoPropertySheet( nPageNum ) ){
		// 2013.06.10 Moca ����I���܂őҋ@����
		CShareDataLockCounter* pLock = NULL;
		CShareDataLockCounter::WaitLock( m_cPropTypes.GetHwndParent(), &pLock );

		m_cPropTypes.GetTypeData( types );

		/* �A�N�Z�����[�^�e�[�u���̍č쐬 */
		::SendMessageAny( GetDllShareData().m_sHandles.m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)PM_CHANGESETTING_ALL );

		/* �ݒ�ύX�𔽉f������ */
		/* �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g���� */
		CAppNodeGroupHandle(0).SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)0,
			(LPARAM)PM_CHANGESETTING_ALL,
			hWnd
		);

		delete pLock;
		return true;
	}else{
		return false;
	}
}

