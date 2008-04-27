#include "stdafx.h"
#include "types/CType.h"

/* Java */
void CType_Java::InitTypeConfigImp(STypeConfig* pType)
{
	//���O�Ɗg���q
	_tcscpy( pType->m_szTypeName, _T("Java") );
	_tcscpy( pType->m_szTypeExts, _T("java,jav") );

	//�ݒ�
	pType->m_cLineComment.CopyTo( 0, L"//", -1 );					/* �s�R�����g�f���~�^ */
	pType->m_cBlockComment.SetBlockCommentRule( 0, L"/*", L"*/" );	/* �u���b�N�R�����g�f���~�^ */
	pType->m_nKeyWordSetIdx[0] = 4;									/* �L�[���[�h�Z�b�g */
	pType->m_nDefaultOutline = OUTLINE_JAVA;						/* �A�E�g���C����͕��@ */
	pType->m_nSmartIndent = SMARTINDENT_CPP;						/* �X�}�[�g�C���f���g��� */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;			//���p���l��F�����\��	//Mar. 10, 2001 JEPRO
	pType->m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp = true;	//�Ί��ʂ̋������f�t�H���gON��	//Sep. 21, 2002 genta
}
