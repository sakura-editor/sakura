#include "stdafx.h"
#include "types/CType.h"

/* HTML */
//Oct. 31, 2000 JEPRO VC++�̐�������e�L�X�g�t�@�C�����ǂݍ��߂�悤�ɂ���
//Feb. 7, 2001 JEPRO .cc/cp/c++/.hpp/hxx/hh/hp/h++��ǉ�	//Mar. 15, 2001 JEPRO .hm��ǉ�
//Feb. 2, 2005 genta �������̂ŃV���O���N�H�[�g�̐F������HTML�ł͍s��Ȃ�
void CType_Html::InitTypeConfigImp(STypeConfig* pType)
{
	_tcscpy( pType->m_szTypeName, _T("HTML") );
	_tcscpy( pType->m_szTypeExts, _T("html,htm,shtml,plg") );

	//�ݒ�
	pType->m_cBlockComment.SetBlockCommentRule( 0, L"<!--", L"-->" );	/* �u���b�N�R�����g�f���~�^ */
	pType->m_nStringType = 0;											/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
	pType->m_nKeyWordSetIdx[0] = 1;										/* �L�[���[�h�Z�b�g */
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;			//�V���O���N�H�[�g�̐F����OFF
}
