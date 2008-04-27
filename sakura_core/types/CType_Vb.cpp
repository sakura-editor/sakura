#include "stdafx.h"
#include "types/CType.h"

/* Visual Basic */
//JUl. 10, 2001 JEPRO VB   ���[�U�ɑ���
//Jul. 09, 2001 JEPRO �ǉ� //Dec. 16, 2002 MIK�ǉ� // Feb. 19, 2006 genta .vb�ǉ�
void CType_Vb::InitTypeConfigImp(STypeConfig* pType)
{
	//���O�Ɗg���q
	_tcscpy( pType->m_szTypeName, _T("Visual Basic") );
	_tcscpy( pType->m_szTypeExts, _T("bas,frm,cls,ctl,pag,dob,dsr,vb") );

	//�ݒ�
	pType->m_cLineComment.CopyTo( 0, L"'", -1 );				/* �s�R�����g�f���~�^ */
	pType->m_nDefaultOutline = OUTLINE_VB;						/* �A�E�g���C����͕��@ */
	pType->m_nKeyWordSetIdx[0]  = 13;							/* �L�[���[�h�Z�b�g */
	pType->m_nKeyWordSetIdx[1] = 14;							/* �L�[���[�h�Z�b�g2 */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;		/* ���p���l��F�����\�� */
	pType->m_nStringType = 1;									/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;	//�V���O���N�H�[�e�[�V�����������F�����\�����Ȃ�
}
