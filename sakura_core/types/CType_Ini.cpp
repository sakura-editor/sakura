#include "stdafx.h"
#include "types/CType.h"

/* �ݒ�t�@�C�� */
//Nov. 9, 2000 JEPRO Windows�W����ini, inf, cnf�t�@�C����sakura�L�[���[�h�ݒ�t�@�C��.kwd, �F�ݒ�t�@�C��.col ���ǂ߂�悤�ɂ���
void CType_Ini::InitTypeConfigImp(STypeConfig* pType)
{
	//���O�Ɗg���q
	_tcscpy( pType->m_szTypeName, _T("�ݒ�t�@�C��") );
	_tcscpy( pType->m_szTypeExts, _T("ini,inf,cnf,kwd,col") );
	
	//�ݒ�
	pType->m_cLineComment.CopyTo( 0, L"//", -1 );				/* �s�R�����g�f���~�^ */
	pType->m_cLineComment.CopyTo( 1, L";", -1 );				/* �s�R�����g�f���~�^2 */
	pType->m_eDefaultOutline = OUTLINE_TEXT;					/* �A�E�g���C����͕��@ */
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;	//�V���O���N�H�[�e�[�V�����������F�����\�����Ȃ�
	pType->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = false;	//�_�u���N�H�[�e�[�V�����������F�����\�����Ȃ�
}
