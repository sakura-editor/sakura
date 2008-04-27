#include "stdafx.h"
#include "CType.h"

/* C/C++ */
// Oct. 31, 2000 JEPRO VC++�̐�������e�L�X�g�t�@�C�����ǂ߂�悤�ɂ���
// Jan. 24, 2004 genta �֘A�Â���D�܂����Ȃ��̂�dsw,dsp,dep,mak�͂͂���
//	2003.06.23 Moca �t�@�C��������̓��͕⊮�@�\
void CType_Cpp::InitTypeConfigImp(STypeConfig* pType)
{
	//���O�Ɗg���q
	_tcscpy( pType->m_szTypeName, _T("C/C++") );
	_tcscpy( pType->m_szTypeExts, _T("c,cpp,cxx,cc,cp,c++,h,hpp,hxx,hh,hp,h++,rc,hm") );

	//�ݒ�
	pType->m_cLineComment.CopyTo( 0, L"//", -1 );							/* �s�R�����g�f���~�^ */
	pType->m_cBlockComment.SetBlockCommentRule( 0, L"/*", L"*/" );			/* �u���b�N�R�����g�f���~�^ */
	pType->m_cBlockComment.SetBlockCommentRule( 1, L"#if 0", L"#endif" );	/* �u���b�N�R�����g�f���~�^2 */	//Jul. 11, 2001 JEPRO
	pType->m_nKeyWordSetIdx[0] = 0;											/* �L�[���[�h�Z�b�g */
	pType->m_nDefaultOutline = OUTLINE_CPP;									/* �A�E�g���C����͕��@ */
	pType->m_nSmartIndent = SMARTINDENT_CPP;								/* �X�}�[�g�C���f���g��� */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;					//���p���l��F�����\��	//Mar. 10, 2001 JEPRO
	pType->m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp = true;			//	Sep. 21, 2002 genta �Ί��ʂ̋������f�t�H���gON��
	pType->m_bUseHokanByFile = TRUE;										/*! ���͕⊮ �J���Ă���t�@�C�����������T�� */
}
