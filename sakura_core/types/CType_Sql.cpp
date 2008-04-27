#include "stdafx.h"
#include "types/CType.h"

/* PL/SQL */
void CType_Sql::InitTypeConfigImp(STypeConfig* pType)
{
	//���O�Ɗg���q
	_tcscpy( pType->m_szTypeName, _T("PL/SQL") );
	_tcscpy( pType->m_szTypeExts, _T("sql,plsql") );

	//�ݒ�
	pType->m_cLineComment.CopyTo( 0, L"--", -1 );					/* �s�R�����g�f���~�^ */
	pType->m_cBlockComment.SetBlockCommentRule( 0, L"/*", L"*/" );	/* �u���b�N�R�����g�f���~�^ */
	pType->m_nStringType = 1;										/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
	wcscpy( pType->m_szIndentChars, L"|��" );						/* ���̑��̃C���f���g�Ώە��� */
	pType->m_nKeyWordSetIdx[0] = 2;									/* �L�[���[�h�Z�b�g */
	pType->m_nDefaultOutline = OUTLINE_PLSQL;						/* �A�E�g���C����͕��@ */
}
