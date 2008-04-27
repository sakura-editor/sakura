#include "stdafx.h"
#include "types/CType.h"

/* TeX */
//Oct. 31, 2000 JEPRO TeX  ���[�U�ɑ���
//Oct. 31, 2000 JEPRO TeX ���[�U�ɑ���	//Mar. 10, 2001 JEPRO �ǉ�
void CType_Tex::InitTypeConfigImp(STypeConfig* pType)
{
	//���O�Ɗg���q
	_tcscpy( pType->m_szTypeName, _T("TeX") );
	_tcscpy( pType->m_szTypeExts, _T("tex,ltx,sty,bib,log,blg,aux,bbl,toc,lof,lot,idx,ind,glo") );

	//�ݒ�
	pType->m_cLineComment.CopyTo( 0, L"%", -1 );				/* �s�R�����g�f���~�^ */
	pType->m_nDefaultOutline = OUTLINE_TEX;						/* �A�E�g���C����͕��@ */
	pType->m_nKeyWordSetIdx[0] = 9;								/* �L�[���[�h�Z�b�g */
	pType->m_nKeyWordSetIdx[1] = 10;							/* �L�[���[�h�Z�b�g2 */	//Jan. 19, 2001 JEPRO
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;	//�V���O���N�H�[�e�[�V�����������F�����\�����Ȃ�
	pType->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = false;	//�_�u���N�H�[�e�[�V�����������F�����\�����Ȃ�
}
