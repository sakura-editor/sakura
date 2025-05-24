/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "CConvert.h"

#include "CConvert_CodeAutoToSjis.h"
#include "CConvert_CodeFromSjis.h"
#include "CConvert_CodeToSjis.h"
#include "CConvert_ToLower.h"
#include "CConvert_ToUpper.h"
#include "CConvert_ToHankaku.h"
#include "CConvert_TabToSpace.h"
#include "CConvert_SpaceToTab.h"
#include "CConvert_ZenkataToHankata.h"
#include "CConvert_ZeneisuToHaneisu.h"
#include "CConvert_HaneisuToZeneisu.h"
#include "CConvert_HankataToZenkata.h"
#include "CConvert_HankataToZenhira.h"
#include "CConvert_ToZenhira.h"
#include "CConvert_ToZenkata.h"
#include "CConvert_Trim.h"

#include "CSelectLang.h"
#include "String_define.h"
#include "util/MessageBoxF.h"

/*!
	コンストラクタ
 */
CConversionFacade::CConversionFacade(
	CKetaXInt nTabWidth,
	int nStartColumn,
	bool bEnableExtEol,
	const SEncodingConfig& sEncodingConfig,
	CCharWidthCache& cCharWidthCache
)
	: m_nTabWidth((Int)nTabWidth)
	, m_nStartColumn(nStartColumn)
	, m_bEnableExtEol(bEnableExtEol)
	, m_sEncodingConfig(sEncodingConfig)
	, m_cCharWidthCache(cCharWidthCache)
{
}

/* 機能種別によるバッファの変換 */
bool CConversionFacade::ConvMemory(EFunctionCode eFuncCode, CNativeW& cData) noexcept
{
	if (CallConvert(eFuncCode, &cData)) {
		return true;
	}
	else {
		// L"変換でエラーが発生しました"
		ErrorMessage(nullptr, LS(STR_CONVERT_ERR));
		return false;
	}
}

//! 変換機能を呼び出す
bool CConversionFacade::CallConvert(EFunctionCode eFuncCode, CNativeW* pcData) noexcept
{
	switch( eFuncCode ){
	//文字種変換、整形
	case F_TOLOWER:					return CConvert_ToLower().DoConvert(pcData);				// 小文字
	case F_TOUPPER:					return CConvert_ToUpper().DoConvert(pcData);				// 大文字
	case F_TOHANKAKU:				return CConvert_ToHankaku().DoConvert(pcData);				// 全角→半角
	case F_TOHANKATA:				return CConvert_ZenkataToHankata().DoConvert(pcData);		// 全角カタカナ→半角カタカナ
	case F_TOZENEI:					return CConvert_HaneisuToZeneisu().DoConvert(pcData);		// 半角英数→全角英数
	case F_TOHANEI:					return CConvert_ZeneisuToHaneisu().DoConvert(pcData);		// 全角英数→半角英数
	case F_TOZENKAKUKATA:			return CConvert_ToZenkata().DoConvert(pcData);				// 半角＋全ひら→全角・カタカナ
	case F_TOZENKAKUHIRA:			return CConvert_ToZenhira().DoConvert(pcData);				// 半角＋全カタ→全角・ひらがな
	case F_HANKATATOZENKATA:		return CConvert_HankataToZenkata().DoConvert(pcData);		// 半角カタカナ→全角カタカナ
	case F_HANKATATOZENHIRA:		return CConvert_HankataToZenhira().DoConvert(pcData);		// 半角カタカナ→全角ひらがな
	//文字種変換、整形
	case F_TABTOSPACE:				return CConvert_TabToSpace(m_nTabWidth, m_nStartColumn, m_bEnableExtEol).DoConvert(pcData);					// TAB→空白
	case F_SPACETOTAB:				return CConvert_SpaceToTab(m_nTabWidth, m_nStartColumn, m_bEnableExtEol, m_cCharWidthCache).DoConvert(pcData);	// 空白→TAB
	case F_LTRIM:					return CConvert_Trim(true, m_bEnableExtEol).DoConvert(pcData);
	case F_RTRIM:					return CConvert_Trim(false, m_bEnableExtEol).DoConvert(pcData);
	//コード変換(xxx2SJIS)
	case F_CODECNV_AUTO2SJIS:		return CConvert_CodeAutoToSjis(m_sEncodingConfig).DoConvert(pcData);
	case F_CODECNV_EMAIL:			return CConvert_CodeToSjis(CODE_JIS).DoConvert(pcData);
	case F_CODECNV_EUC2SJIS:		return CConvert_CodeToSjis(CODE_EUC).DoConvert(pcData);
	case F_CODECNV_UNICODE2SJIS:	return CConvert_CodeToSjis(CODE_UNICODE).DoConvert(pcData);
	case F_CODECNV_UNICODEBE2SJIS:	return CConvert_CodeToSjis(CODE_UNICODEBE).DoConvert(pcData);
	case F_CODECNV_UTF82SJIS:		return CConvert_CodeToSjis(CODE_UTF8).DoConvert(pcData);
	case F_CODECNV_UTF72SJIS:		return CConvert_CodeToSjis(CODE_UTF7).DoConvert(pcData);
	//コード変換(SJIS2xxx)
	case F_CODECNV_SJIS2JIS:		return CConvert_CodeFromSjis(CODE_JIS).DoConvert(pcData);
	case F_CODECNV_SJIS2EUC:		return CConvert_CodeFromSjis(CODE_EUC).DoConvert(pcData);
	case F_CODECNV_SJIS2UTF8:		return CConvert_CodeFromSjis(CODE_UTF8).DoConvert(pcData);
	case F_CODECNV_SJIS2UTF7:		return CConvert_CodeFromSjis(CODE_UTF7).DoConvert(pcData);
	default:
		break;
	}
	return false;
}
