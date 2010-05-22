#include "stdafx.h"
#include "types/CType.h"
#include "doc/CDocOutline.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "view/colors/CColorStrategy.h"

void CType_Basis::InitTypeConfigImp(STypeConfig* pType)
{
	//���O�Ɗg���q
	_tcscpy( pType->m_szTypeName, _T("��{") );
	_tcscpy( pType->m_szTypeExts, _T("") );

	//�ݒ�
	pType->m_nMaxLineKetas = CLayoutInt(MAXLINEKETAS);			// �܂�Ԃ�����
	pType->m_eDefaultOutline = OUTLINE_TEXT;					// �A�E�g���C����͕��@
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;	// �V���O���N�H�[�e�[�V�����������F�����\�����Ȃ�	//Oct. 17, 2000 JEPRO
	pType->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = false;	// �_�u���N�H�[�e�[�V�����������F�����\�����Ȃ�	//Sept. 4, 2000 JEPRO

	//���K�\���L�[���[�h
	pType->m_bUseRegexKeyword = true;							// ���K�\���L�[���[�h���g����
	pType->m_RegexKeywordArr[0].m_nColorIndex = COLORIDX_URL;	// �F�w��ԍ�
	wcscpyn( pType->m_RegexKeywordArr[0].m_szKeyword,			// ���K�\���L�[���[�h
		L"/\\b[a-zA-Z]:\\\\[\\w\\-_\\.\\\\\\/\\$%~]*/k",		//   C:\�`�Ƀ}�b�`����p�^�[��
		sizeof(pType->m_RegexKeywordArr[0].m_szKeyword) );
	pType->m_RegexKeywordArr[1].m_nColorIndex = COLORIDX_URL;	// �F�w��ԍ�
	wcscpyn( pType->m_RegexKeywordArr[1].m_szKeyword,			// ���K�\���L�[���[�h
		L"/\\B\\\\\\\\[\\w\\-_\\.\\\\\\/\\$%~]+/k",				//   \\�`�Ƀ}�b�`����p�^�[��
		sizeof(pType->m_RegexKeywordArr[1].m_szKeyword) );
}
