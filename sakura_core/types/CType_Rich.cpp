#include "stdafx.h"
#include "types/CType.h"

/* ���b�`�e�L�X�g */
//JUl. 10, 2001 JEPRO WinHelp���̂ɂ���P����
//Jul. 10, 2001 JEPRO �ǉ�
void CType_Rich::InitTypeConfigImp(STypeConfig* pType)
{
	//���O�Ɗg���q
	_tcscpy( pType->m_szTypeName, _T("���b�`�e�L�X�g") );
	_tcscpy( pType->m_szTypeExts, _T("rtf") );

	//�ݒ�
	pType->m_nDefaultOutline = OUTLINE_TEXT;					/* �A�E�g���C����͕��@ */
	pType->m_nKeyWordSetIdx[0]  = 15;							/* �L�[���[�h�Z�b�g */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;		/* ���p���l��F�����\�� */
	pType->m_nStringType = 0;									/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;	//�V���O���N�H�[�e�[�V�����������F�����\�����Ȃ�
	pType->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = FALSE;	//�_�u���N�H�[�e�[�V�����������F�����\�����Ȃ�
	pType->m_ColorInfoArr[COLORIDX_URL].m_bDisp = FALSE;		//URL�ɃA���_�[���C���������Ȃ�
}
