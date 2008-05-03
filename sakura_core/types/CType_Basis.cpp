#include "stdafx.h"
#include "types/CType.h"

void _DefaultConfig(STypeConfig* pType)
{
//�L�[���[�h�F�f�t�H���g�J���[�ݒ�
/************************/
/* �^�C�v�ʐݒ�̋K��l */
/************************/

	pType->m_nMaxLineKetas = CLayoutInt(MAXLINEKETAS);	/* �܂�Ԃ����� */
	pType->m_nColmSpace = 0;					/* �����ƕ����̌��� */
	pType->m_nLineSpace = 1;					/* �s�Ԃ̂����� */
	pType->m_nTabSpace = CLayoutInt(4);					/* TAB�̕����� */
	for( int i = 0; i < MAX_KEYWORDSET_PER_TYPE; i++ ){
		pType->m_nKeyWordSetIdx[i] = -1;
	}
	wcscpy( pType->m_szTabViewString, _EDITL("^       ") );	/* TAB�\�������� */
	pType->m_bTabArrow = false;				/* �^�u���\�� */	// 2001.12.03 hor
	pType->m_bInsSpace = FALSE;				/* �X�y�[�X�̑}�� */	// 2001.12.03 hor
	
	//@@@ 2002.09.22 YAZAKI �ȉ��Am_cLineComment��m_cBlockComment���g���悤�ɏC��
	pType->m_cLineComment.CopyTo(0, L"", -1);	/* �s�R�����g�f���~�^ */
	pType->m_cLineComment.CopyTo(1, L"", -1);	/* �s�R�����g�f���~�^2 */
	pType->m_cLineComment.CopyTo(2, L"", -1);	/* �s�R�����g�f���~�^3 */	//Jun. 01, 2001 JEPRO �ǉ�
	pType->m_cBlockComment.SetBlockCommentRule(0, L"", L"");	/* �u���b�N�R�����g�f���~�^ */
	pType->m_cBlockComment.SetBlockCommentRule(1, L"", L"");	/* �u���b�N�R�����g�f���~�^2 */

	pType->m_nStringType = 0;					/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
	wcscpy( pType->m_szIndentChars, L"" );		/* ���̑��̃C���f���g�Ώە��� */

	pType->m_nColorInfoArrNum = COLORIDX_LAST;

	// 2001/06/14 Start by asa-o
	_tcscpy( pType->m_szHokanFile, _T("") );		/* ���͕⊮ �P��t�@�C�� */
	// 2001/06/14 End

	// 2001/06/19 asa-o
	pType->m_bHokanLoHiCase = FALSE;			/* ���͕⊮�@�\�F�p�啶���������𓯈ꎋ���� */

	//	2003.06.23 Moca �t�@�C��������̓��͕⊮�@�\
	pType->m_bUseHokanByFile = FALSE;			/*! ���͕⊮ �J���Ă���t�@�C�����������T�� */

	//@@@2002.2.4 YAZAKI
	pType->m_szExtHelp[0] = L'\0';
	pType->m_szExtHtmlHelp[0] = L'\0';
	pType->m_bHtmlHelpIsSingle = TRUE;

	pType->m_bAutoIndent = true;			/* �I�[�g�C���f���g */
	pType->m_bAutoIndent_ZENSPACE = true;	/* ���{��󔒂��C���f���g */
	pType->m_bRTrimPrevLine = FALSE;			/* 2005.10.11 ryoji ���s���ɖ����̋󔒂��폜 */

	pType->m_nIndentLayout = 0;	/* �܂�Ԃ���2�s�ڈȍ~���������\�� */


	for( int i = 0; i < COLORIDX_LAST; ++i ){
		GetDefaultColorInfo(&pType->m_ColorInfoArr[i],i);
	}
	pType->m_bLineNumIsCRLF = TRUE;				/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
	pType->m_nLineTermType = 1;					/* �s�ԍ���؂� 0=�Ȃ� 1=�c�� 2=�C�� */
	pType->m_cLineTermChar = L':';					/* �s�ԍ���؂蕶�� */
	pType->m_bWordWrap = FALSE;					/* �p�����[�h���b�v������ */
	pType->m_nCurrentPrintSetting = 0;				/* ���ݑI�����Ă������ݒ� */
	pType->m_nDefaultOutline = OUTLINE_TEXT;		/* �A�E�g���C����͕��@ */
	pType->m_nSmartIndent = SMARTINDENT_NONE;		/* �X�}�[�g�C���f���g��� */
	pType->m_nImeState = IME_CMODE_NOCONVERSION;	/* IME���� */

	pType->m_szOutlineRuleFilename[0] = L'\0';	//Dec. 4, 2000 MIK
	pType->m_bKinsokuHead = FALSE;				/* �s���֑� */	//@@@ 2002.04.08 MIK
	pType->m_bKinsokuTail = FALSE;				/* �s���֑� */	//@@@ 2002.04.08 MIK
	pType->m_bKinsokuRet  = FALSE;				/* ���s�������Ԃ牺���� */	//@@@ 2002.04.13 MIK
	pType->m_bKinsokuKuto = FALSE;				/* ��Ǔ_���Ԃ牺���� */	//@@@ 2002.04.17 MIK
	wcscpy( pType->m_szKinsokuHead, L"" );		/* �s���֑� */	//@@@ 2002.04.08 MIK
	wcscpy( pType->m_szKinsokuTail, L"" );		/* �s���֑� */	//@@@ 2002.04.08 MIK

	pType->m_bUseDocumentIcon = FALSE;			/* �����Ɋ֘A�Â���ꂽ�A�C�R�����g�� */

//@@@ 2001.11.17 add start MIK
	for(int i = 0; i < 100; i++)
	{
		pType->m_RegexKeywordArr[i].m_szKeyword[0] = L'\0';
		pType->m_RegexKeywordArr[i].m_nColorIndex = COLORIDX_REGEX1;
	}
	pType->m_bUseRegexKeyword = FALSE;
//		pType->m_nRegexKeyMagicNumber = 1;
//@@@ 2001.11.17 add end MIK

//@@@ 2006.04.10 fon ADD-start
	for(int i = 0; i < MAX_KEYHELP_FILE; i++){
		pType->m_KeyHelpArr[i].m_bUse = false;
		pType->m_KeyHelpArr[i].m_szAbout[0] = _T('\0');
		pType->m_KeyHelpArr[i].m_szPath[0] = _T('\0');
	}
	pType->m_bUseKeyWordHelp = FALSE;	/* �����I���@�\�̎g�p�� */
	pType->m_nKeyHelpNum = 0;			/* �o�^������ */
	pType->m_bUseKeyHelpAllSearch = FALSE;	/* �q�b�g�������̎���������(&A) */
	pType->m_bUseKeyHelpKeyDisp = FALSE;		/* 1�s�ڂɃL�[���[�h���\������(&W) */
	pType->m_bUseKeyHelpPrefix = FALSE;		/* �I��͈͂őO����v����(&P) */
//@@@ 2006.04.10 fon ADD-end

	// 2005.11.08 Moca �w��ʒu�c���̐ݒ�
	for(int i = 0; i < MAX_VERTLINES; i++ ){
		pType->m_nVertLineIdx[i] = CLayoutInt(0);
	}
}

void CType_Basis::InitTypeConfigImp(STypeConfig* pType)
{
	//�K��l�̍\�z
	_DefaultConfig(pType);

	//���O�Ɗg���q
	_tcscpy( pType->m_szTypeName, _T("��{") );
	_tcscpy( pType->m_szTypeExts, _T("") );

	//�ݒ�
	pType->m_nMaxLineKetas = CLayoutInt(MAXLINEKETAS);			// �܂�Ԃ�����
	pType->m_nDefaultOutline = OUTLINE_TEXT;					// �A�E�g���C����͕��@
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;	// �V���O���N�H�[�e�[�V�����������F�����\�����Ȃ�	//Oct. 17, 2000 JEPRO
	pType->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = false;	// �_�u���N�H�[�e�[�V�����������F�����\�����Ȃ�	//Sept. 4, 2000 JEPRO
}
