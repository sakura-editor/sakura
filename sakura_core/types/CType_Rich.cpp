#include "StdAfx.h"
#include "types/CType.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "view/colors/CColorStrategy.h"

/* ���b�`�e�L�X�g */
//JUl. 10, 2001 JEPRO WinHelp���̂ɂ���P����
//Jul. 10, 2001 JEPRO �ǉ�
void CType_Rich::InitTypeConfigImp(STypeConfig* pType)
{
	//���O�Ɗg���q
	_tcscpy( pType->m_szTypeName, _T("���b�`�e�L�X�g") );
	_tcscpy( pType->m_szTypeExts, _T("rtf") );

	//�ݒ�
	pType->m_eDefaultOutline = OUTLINE_TEXT;					/* �A�E�g���C����͕��@ */
	pType->m_nKeyWordSetIdx[0]  = 15;							/* �L�[���[�h�Z�b�g */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;		/* ���p���l��F�����\�� */
	pType->m_nStringType = 0;									/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;	//�V���O���N�H�[�e�[�V�����������F�����\�����Ȃ�
	pType->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = false;	//�_�u���N�H�[�e�[�V�����������F�����\�����Ȃ�
	pType->m_ColorInfoArr[COLORIDX_URL].m_bDisp = false;		//URL�ɃA���_�[���C���������Ȃ�
}



//Jul. 10, 2001 JEPRO �ǉ�
const wchar_t* g_ppszKeywordsRTF[] = {
	L"\\ansi",
	L"\\b",
	L"\\bin",
	L"\\box",
	L"\\brdrb",
	L"\\brdrbar",
	L"\\brdrdb",
	L"\\brdrdot",
	L"\\brdrl",
	L"\\brdrr",
	L"\\brdrs",
	L"\\brdrsh",
	L"\\brdrt",
	L"\\brdrth",
	L"\\cell",
	L"\\cellx",
	L"\\cf",
	L"\\chftn",
	L"\\clmgf",
	L"\\clmrg",
	L"\\colortbl",
	L"\\deff",
	L"\\f",
	L"\\fi",
	L"\\field",
	L"\\fldrslt",
	L"\\fonttbl",
	L"\\footnote",
	L"\\fs",
	L"\\i"
	L"\\intbl",
	L"\\keep",
	L"\\keepn",
	L"\\li",
	L"\\line",
	L"\\mac",
	L"\\page",
	L"\\par",
	L"\\pard",
	L"\\pc",
	L"\\pich",
	L"\\pichgoal",
	L"\\picscalex",
	L"\\picscaley",
	L"\\pict",
	L"\\picw",
	L"\\picwgoal",
	L"\\plain",
	L"\\qc",
	L"\\ql",
	L"\\qr",
	L"\\ri",
	L"\\row",
	L"\\rtf",
	L"\\sa",
	L"\\sb",
	L"\\scaps",
	L"\\sect",
	L"\\sl",
	L"\\strike",
	L"\\tab",
	L"\\tqc",
	L"\\tqr",
	L"\\trgaph",
	L"\\trleft",
	L"\\trowd",
	L"\\trqc",
	L"\\trql",
	L"\\tx",
	L"\\ul",
	L"\\uldb",
	L"\\v",
	L"\\wbitmap",
	L"\\wbmbitspixel",
	L"\\wbmplanes",
	L"\\wbmwidthbytes",
	L"\\wmetafile",
	L"bmc",
	L"bml",
	L"bmr",
	L"emc",
	L"eml",
	L"emr"
};
int g_nKeywordsRTF = _countof(g_ppszKeywordsRTF);
