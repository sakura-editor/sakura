#include "stdafx.h"
#include "types/CType.h"

/* COBOL */
void CType_Cobol::InitTypeConfigImp(STypeConfig* pType)
{
	//���O�Ɗg���q
	_tcscpy( pType->m_szTypeName, _T("COBOL") );
	_tcscpy( pType->m_szTypeExts, _T("cbl,cpy,pco,cob") );	//Jun. 04, 2001 JEPRO KENCH���̏����ɏ]���ǉ�

	//�ݒ�
	pType->m_cLineComment.CopyTo( 0, L"*", 6 );			//Jun. 02, 2001 JEPRO �C��
	pType->m_cLineComment.CopyTo( 1, L"D", 6 );			//Jun. 04, 2001 JEPRO �ǉ�
	pType->m_nStringType = 1;							/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
	wcscpy( pType->m_szIndentChars, L"*" );				/* ���̑��̃C���f���g�Ώە��� */
	pType->m_nKeyWordSetIdx[0] = 3;						/* �L�[���[�h�Z�b�g */		//Jul. 10, 2001 JEPRO
	pType->m_nDefaultOutline = OUTLINE_COBOL;			/* �A�E�g���C����͕��@ */
	// �w�茅�c��	//2005.11.08 Moca
	pType->m_ColorInfoArr[COLORIDX_VERTLINE].m_bDisp = true;
	pType->m_nVertLineIdx[0] = CLayoutInt(7);
	pType->m_nVertLineIdx[1] = CLayoutInt(8);
	pType->m_nVertLineIdx[2] = CLayoutInt(12);
	pType->m_nVertLineIdx[3] = CLayoutInt(73);
}
