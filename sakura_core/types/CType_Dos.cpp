#include "stdafx.h"
#include "types/CType.h"

/* MS-DOS�o�b�`�t�@�C�� */
void CType_Dos::InitTypeConfigImp(STypeConfig* pType)
{
	//���O�Ɗg���q
	_tcscpy( pType->m_szTypeName, _T("MS-DOS�o�b�`�t�@�C��") );
	_tcscpy( pType->m_szTypeExts, _T("bat") );

	//�ݒ�
	pType->m_cLineComment.CopyTo( 0, L"REM ", -1 );	/* �s�R�����g�f���~�^ */
	pType->m_nDefaultOutline = OUTLINE_TEXT;		/* �A�E�g���C����͕��@ */
	pType->m_nKeyWordSetIdx[0] = 7;					/* �L�[���[�h�Z�b�g */
}
