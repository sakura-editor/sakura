#include "stdafx.h"
#include "types/CType.h"

/* Perl */
//Jul. 08, 2001 JEPRO Perl ���[�U�ɑ���
//Jul. 08, 2001 JEPRO �ǉ�
void CType_Perl::InitTypeConfigImp(STypeConfig* pType)
{
	//���O�Ɗg���q
	_tcscpy( pType->m_szTypeName, _T("Perl") );
	_tcscpy( pType->m_szTypeExts, _T("cgi,pl,pm") );

	//�ݒ�
	pType->m_cLineComment.CopyTo( 0, L"#", -1 );					/* �s�R�����g�f���~�^ */
	pType->m_nDefaultOutline = OUTLINE_PERL;						/* �A�E�g���C����͕��@ */
	pType->m_nKeyWordSetIdx[0]  = 11;								/* �L�[���[�h�Z�b�g */
	pType->m_nKeyWordSetIdx[1] = 12;								/* �L�[���[�h�Z�b�g2 */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;			/* ���p���l��F�����\�� */
	pType->m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp = true;	//�Ί��ʂ̋������f�t�H���gON	//Sep. 21, 2002 genta
}
