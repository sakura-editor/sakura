#include "stdafx.h"
#include "types/CType.h"

/* Pascal */
//Mar. 10, 2001 JEPRO	���p���l��F�����\��
void CType_Pascal::InitTypeConfigImp(STypeConfig* pType)
{
	//���O�Ɗg���q
	_tcscpy( pType->m_szTypeName, _T("Pascal") );
	_tcscpy( pType->m_szTypeExts, _T("dpr,pas") );

	//�ݒ�
	pType->m_cLineComment.CopyTo( 0, L"//", -1 );					/* �s�R�����g�f���~�^ */		//Nov. 5, 2000 JEPRO �ǉ�
	pType->m_cBlockComment.SetBlockCommentRule( 0, L"{", L"}" );	/* �u���b�N�R�����g�f���~�^ */	//Nov. 5, 2000 JEPRO �ǉ�
	pType->m_cBlockComment.SetBlockCommentRule( 1, L"(*", L"*)" );	/* �u���b�N�R�����g�f���~�^2 */	//@@@ 2001.03.10 by MIK
	pType->m_nStringType = 1;										/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */	//Nov. 5, 2000 JEPRO �ǉ�
	pType->m_nKeyWordSetIdx[0] = 8;									/* �L�[���[�h�Z�b�g */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;			//@@@ 2001.11.11 upd MIK
}
