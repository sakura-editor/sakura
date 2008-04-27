#include "stdafx.h"
#include "types/CType.h"

/* awk */
void CType_Awk::InitTypeConfigImp(STypeConfig* pType)
{
	//���O�Ɗg���q
	_tcscpy( pType->m_szTypeName, _T("AWK") );
	_tcscpy( pType->m_szTypeExts, _T("awk") );

	//�ݒ�
	pType->m_cLineComment.CopyTo( 0, L"#", -1 );		/* �s�R�����g�f���~�^ */
	pType->m_nDefaultOutline = OUTLINE_TEXT;			/* �A�E�g���C����͕��@ */
	pType->m_nKeyWordSetIdx[0] = 6;						/* �L�[���[�h�Z�b�g */
}
