#include "stdafx.h"
#include "types/CType.h"

/* �e�L�X�g */
//Sep. 20, 2000 JEPRO �e�L�X�g�̋K��l��80��120�ɕύX(�s��ꗗ.txt��������x�ǂ݂₷������)
//Nov. 15, 2000 JEPRO PostScript�t�@�C�����ǂ߂�悤�ɂ���
//Jan. 12, 2001 JEPRO readme.1st ���ǂ߂�悤�ɂ���
//Feb. 12, 2001 JEPRO .err �G���[���b�Z�[�W
//Nov.  6, 2002 genta doc��MS Word�ɏ����Ă�������͊O���i�֘A�Â��h�~�̂��߁j
//Nov.  6, 2002 genta log ��ǉ�
void CType_Text::InitTypeConfigImp(STypeConfig* pType)
{
	//���O�Ɗg���q
	_tcscpy( pType->m_szTypeName, _T("�e�L�X�g") );
	_tcscpy( pType->m_szTypeExts, _T("txt,log,1st,err,ps") );

	//�ݒ�
	pType->m_nMaxLineKetas = CLayoutInt(120);					/* �܂�Ԃ����� */
	pType->m_nDefaultOutline = OUTLINE_TEXT;					/* �A�E�g���C����͕��@ */
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;	//Oct. 17, 2000 JEPRO	�V���O���N�H�[�e�[�V�����������F�����\�����Ȃ�
	pType->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = false;	//Sept. 4, 2000 JEPRO	�_�u���N�H�[�e�[�V�����������F�����\�����Ȃ�
	pType->m_bKinsokuHead = FALSE;								/* �s���֑� */	//@@@ 2002.04.08 MIK
	pType->m_bKinsokuTail = FALSE;								/* �s���֑� */	//@@@ 2002.04.08 MIK
	pType->m_bKinsokuRet  = FALSE;								/* ���s�������Ԃ牺���� */	//@@@ 2002.04.13 MIK
	pType->m_bKinsokuKuto = FALSE;								/* ��Ǔ_���Ԃ牺���� */	//@@@ 2002.04.17 MIK
	wcscpy( pType->m_szKinsokuHead, L"!%),.:;?]}�����f�h�񁌁����A�B�X�r�t�v�x�z�l�J�K�T�U�E�R�S�I���j�C�D�F�G�H�n�p�����߁�" );		/* �s���֑� */	//@@@ 2002.04.13 MIK 
	wcscpy( pType->m_szKinsokuTail, L"$([{��\\�e�g�q�s�u�w�y�k���i�m�o�����" );		/* �s���֑� */	//@@@ 2002.04.08 MIK 
}
