/*
	2008.05.18 kobake CShareData ���番��
*/

#include "stdafx.h"
#include "env/DLLSHAREDATA.h"

#include "CTagJumpManager.h"


/*!
	@brief �^�O�W�����v���̕ۑ�

	�^�O�W�����v����Ƃ��ɁA�^�O�W�����v��̏���ۑ�����B

	@param[in] pTagJump �ۑ�����^�O�W�����v���
	@retval true	�ۑ�����
	@retval false	�ۑ����s

	@date 2004/06/21 �V�K�쐬
	@date 2004/06/22 Moca ��t�ɂȂ������ԌÂ������폜�������ɐV������������
*/
void CTagJumpManager::PushTagJump(const TagJump *pTagJump)
{
	int i = m_pShareData->m_sTagJump.m_TagJumpTop + 1;
	if( MAX_TAGJUMPNUM <= i ){
		i = 0;
	}
	if( m_pShareData->m_sTagJump.m_TagJumpNum < MAX_TAGJUMPNUM ){
		m_pShareData->m_sTagJump.m_TagJumpNum++;
	}
	m_pShareData->m_sTagJump.m_TagJump[i] = *pTagJump;
	m_pShareData->m_sTagJump.m_TagJumpTop = i;
}


/*!
	@brief �^�O�W�����v���̎Q��

	�^�O�W�����v�o�b�N����Ƃ��ɁA�^�O�W�����v���̏����Q�Ƃ���B

	@param[out] pTagJump �Q�Ƃ���^�O�W�����v���
	@retval true	�Q�Ɛ���
	@retval false	�Q�Ǝ��s

	@date 2004/06/21 �V�K�쐬
	@date 2004/06/22 Moca SetTagJump�ύX�ɂ��C��
*/
bool CTagJumpManager::PopTagJump(TagJump *pTagJump)
{
	if( 0 < m_pShareData->m_sTagJump.m_TagJumpNum ){
		*pTagJump = m_pShareData->m_sTagJump.m_TagJump[m_pShareData->m_sTagJump.m_TagJumpTop--];
		if( m_pShareData->m_sTagJump.m_TagJumpTop < 0 ){
			m_pShareData->m_sTagJump.m_TagJumpTop = MAX_TAGJUMPNUM - 1;
		}
		m_pShareData->m_sTagJump.m_TagJumpNum--;
		return true;
	}
	return false;
}
