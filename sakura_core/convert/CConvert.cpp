#include "stdafx.h"
#include "CConvert.h"
#include "func/Funccode.h"
#include "CEol.h"
#include "charset/charcode.h"
#include "charset/CCodeMediator.h"
#include "charset/CShiftJis.h"
#include "charset/CJis.h"
#include "charset/CEuc.h"
#include "charset/CUnicodeBe.h"
#include "charset/CUtf8.h"
#include "charset/CUtf7.h"
#include "CConvert_ToLower.h"
#include "CConvert_ToUpper.h"
#include "CConvert_ToHankaku.h"
#include "CConvert_TabToSpace.h"
#include "CConvert_SpaceToTab.h"
#include "CConvert_ZenkanaToHankana.h"
#include "CConvert_ZeneisuToHaneisu.h"
#include "CConvert_HaneisuToZeneisu.h"
#include "CConvert_HankanaToZenkana.h"
#include "CConvert_HankanaToZenhira.h"
#include "CConvert_ToZenhira.h"
#include "CConvert_ToZenkana.h"
#include "CConvert_Trim.h"

#include "window/CEditWnd.h"

/* �@�\��ʂɂ��o�b�t�@�̕ϊ� */
void CConvertMediator::ConvMemory( CNativeW* pCMemory, EFunctionCode nFuncCode, int nTabWidth )
{
	// �R�[�h�ϊ��͂ł��邾��ANSI�ł�sakura�ƌ݊��̌��ʂ�������悤�Ɏ�������	// 2009.03.26 ryoji
	// xxx2SJIS:
	//   1. �o�b�t�@�̓��e��ANSI�ő����ɂȂ�悤 Unicode��SJIS �ϊ�����
	//   2. xxx��SJIS �ϊ���Ƀo�b�t�@���e��UNICODE�ő����ɖ߂��iSJIS��Unicode�j�̂Ɠ����Ȍ��ʂ𓾂邽�߂� xxx��Unicode �ϊ�����
	// SJIS2xxx:
	//   1. �o�b�t�@���e��ANSI�ő����ɕϊ��iUnicode��SJIS�j��� SJIS��xxx �ϊ�����̂Ɠ����Ȍ��ʂ𓾂邽�߂� Unicode��xxx �ϊ�����
	//   2. �o�b�t�@���e��UNICODE�ő����ɖ߂����߂� SJIS��Unicode �ϊ�����

	switch( nFuncCode ){
	//�R�[�h�ϊ�(xxx2SJIS)
	case F_CODECNV_AUTO2SJIS:
	case F_CODECNV_EMAIL:
	case F_CODECNV_EUC2SJIS:
	case F_CODECNV_UNICODE2SJIS:
	case F_CODECNV_UNICODEBE2SJIS:
	case F_CODECNV_UTF82SJIS:
	case F_CODECNV_UTF72SJIS:
		CShiftJis::UnicodeToSJIS(pCMemory->_GetMemory());
		break;
	//�R�[�h�ϊ�(SJIS2xxx)
	case F_CODECNV_SJIS2JIS:		CJis::UnicodeToJIS(pCMemory->_GetMemory());			break;
	case F_CODECNV_SJIS2EUC:		CEuc::UnicodeToEUC(pCMemory->_GetMemory());			break;
	case F_CODECNV_SJIS2UTF8:		CUtf8::UnicodeToUTF8(pCMemory->_GetMemory());		break;
	case F_CODECNV_SJIS2UTF7:		CUtf7::UnicodeToUTF7(pCMemory->_GetMemory());		break;
	}

	if( nFuncCode == F_CODECNV_AUTO2SJIS ){
		ECodeType ecode;
		CCodeMediator ccode( CEditWnd::Instance()->GetDocument() );
		ecode = ccode.CheckKanjiCode(
			reinterpret_cast<const char*>(pCMemory->_GetMemory()->GetRawPtr()),
			pCMemory->_GetMemory()->GetRawLength() );
		switch( ecode ){
		case CODE_JIS:			nFuncCode = F_CODECNV_EMAIL;			break;
		case CODE_EUC:			nFuncCode = F_CODECNV_EUC2SJIS;			break;
		case CODE_UNICODE:		nFuncCode = F_CODECNV_UNICODE2SJIS;		break;
		case CODE_UNICODEBE:	nFuncCode = F_CODECNV_UNICODEBE2SJIS;	break;
		case CODE_UTF8:			nFuncCode = F_CODECNV_UTF82SJIS;		break;
		case CODE_UTF7:			nFuncCode = F_CODECNV_UTF72SJIS;		break;
		}
	}

	switch( nFuncCode ){
	//������ϊ��A���`
	case F_TOLOWER:					CConvert_ToLower().CallConvert(pCMemory);			break;	// ������
	case F_TOUPPER:					CConvert_ToUpper().CallConvert(pCMemory);			break;	// �啶��
	case F_TOHANKAKU:				CConvert_ToHankaku().CallConvert(pCMemory);			break;	// �S�p�����p
	case F_TOHANKATA:				CConvert_ZenkanaToHankana().CallConvert(pCMemory);	break;	// �S�p�J�^�J�i�����p�J�^�J�i
	case F_TOZENEI:					CConvert_HaneisuToZeneisu().CallConvert(pCMemory);	break;	// ���p�p�����S�p�p��
	case F_TOHANEI:					CConvert_ZeneisuToHaneisu().CallConvert(pCMemory);	break;	// �S�p�p�������p�p��
	case F_TOZENKAKUKATA:			CConvert_ToZenkana().CallConvert(pCMemory);			break;	// ���p�{�S�Ђ灨�S�p�E�J�^�J�i
	case F_TOZENKAKUHIRA:			CConvert_ToZenhira().CallConvert(pCMemory);			break;	// ���p�{�S�J�^���S�p�E�Ђ炪��
	case F_HANKATATOZENKATA:		CConvert_HankanaToZenkana().CallConvert(pCMemory);	break;	// ���p�J�^�J�i���S�p�J�^�J�i
	case F_HANKATATOZENHIRA:		CConvert_HankanaToZenhira().CallConvert(pCMemory);	break;	// ���p�J�^�J�i���S�p�Ђ炪��
	//������ϊ��A���`
	case F_TABTOSPACE:				CConvert_TabToSpace(nTabWidth).CallConvert(pCMemory);break;	// TAB����
	case F_SPACETOTAB:				CConvert_SpaceToTab(nTabWidth).CallConvert(pCMemory);break;	// �󔒁�TAB
	case F_LTRIM:					CConvert_Trim(true).CallConvert(pCMemory);			break;	// 2001.12.03 hor
	case F_RTRIM:					CConvert_Trim(false).CallConvert(pCMemory);			break;	// 2001.12.03 hor
	//�R�[�h�ϊ�(xxx2SJIS)
	case F_CODECNV_EMAIL:			CJis::JISToUnicode(pCMemory->_GetMemory(), true);	break;
	case F_CODECNV_EUC2SJIS:		CEuc::EUCToUnicode(pCMemory->_GetMemory());			break;
	case F_CODECNV_UNICODE2SJIS:	/* ���ϊ� */										break;
	case F_CODECNV_UNICODEBE2SJIS:	CUnicodeBe::UnicodeBEToUnicode(pCMemory->_GetMemory());	break;
	case F_CODECNV_UTF82SJIS:		CUtf8::UTF8ToUnicode(pCMemory->_GetMemory());		break;
	case F_CODECNV_UTF72SJIS:		CUtf7::UTF7ToUnicode(pCMemory->_GetMemory());		break;
	//�R�[�h�ϊ�(SJIS2xxx)
	case F_CODECNV_SJIS2JIS:
	case F_CODECNV_SJIS2EUC:
	case F_CODECNV_SJIS2UTF8:
	case F_CODECNV_SJIS2UTF7:
		CShiftJis::SJISToUnicode(pCMemory->_GetMemory());
		break;
	}

	return;
}


