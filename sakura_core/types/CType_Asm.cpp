#include "stdafx.h"
#include "types/CType.h"

/* �A�Z���u�� */
//	2004.05.01 MIK/genta
//Mar. 10, 2001 JEPRO	���p���l��F�����\��
void CType_Asm::InitTypeConfigImp(STypeConfig* pType)
{
	//���O�Ɗg���q
	_tcscpy( pType->m_szTypeName, _T("�A�Z���u��") );
	_tcscpy( pType->m_szTypeExts, _T("asm") );

	//�ݒ�
	pType->m_cLineComment.CopyTo( 0, L";", -1 );			/* �s�R�����g�f���~�^ */
	pType->m_nDefaultOutline = OUTLINE_ASM;					/* �A�E�g���C����͕��@ */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;
}
