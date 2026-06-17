/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "charset/CCodeFactory.h"

#include "charset/CCodePage.h"

// move start	from CCodeMediator.h	2012/12/02 Uchi
#include "CEuc.h"
#include "CJis.h"
#include "CShiftJis.h"
#include "CUnicode.h"
#include "CUnicodeBe.h"
#include "CUtf7.h"
#include "CUtf8.h"
#include "CCesu8.h"
// move end
#include "CLatin1.h"

//! eCodeTypeに適合する CCodeBaseインスタンス を生成
CCodeBase* CCodeFactory::CreateCodeBase(
	ECodeType	eCodeType,		//!< 文字コード
	int			nFlag			//!< bit 0: MIME Encodeされたヘッダーをdecodeするかどうか
)
{
  	switch( eCodeType ){
	case CODE_SJIS:			return new CShiftJis();
	case CODE_EUC:			return new CEuc();
	case CODE_JIS:			return new CJis((nFlag&1)==1);
	case CODE_UNICODE:		return new CUnicode();
	case CODE_UTF8:			return new CUtf8();
	case CODE_UTF7:			return new CUtf7();
	case CODE_UNICODEBE:	return new CUnicodeBe();
	case CODE_CESU8:		return new CCesu8();
	case CODE_LATIN1:		return new CLatin1();	// 2010/3/20 Uchi
	case CODE_CPACP:		return new CCodePage(eCodeType);
	case CODE_CPOEM:		return new CCodePage(eCodeType);
	default:
		if( IsValidCodePageEx(eCodeType) ){
			return new CCodePage(eCodeType);
		}
		assert_warning(0);
	}
	return nullptr;
}

/*!
 * バイト列から文字列を読み込む。
 *
 * @param [in] code 読み込み元のバイト列。
 * @returns A→W変換結果。
 */
SLoadFromCodeResult CCodeFactory::LoadFromCode(ECodeType eCodeType, std::string_view code)
{
	// 入力データを準備する
	CMemory cmemSrc{ code.data(), code.size() };
	CNativeW cDest;

	// 変換を実行する
	const auto result = CreateCodeBase(eCodeType)->CodeToUnicode(cmemSrc, &cDest);

	// 読み込まれたデータを回収する
	std::wstring loaded{ cDest.GetStringPtr(), size_t(cDest.GetStringLength()) };

	return SLoadFromCodeResult{ result, code, std::size(code), std::move(loaded) };
}

/*!
 * 文字列をバイト列に書き込む。
 *
 * @param [in] wide 書き込み対象の文字列。
 * @returns W→A変換結果。
 */
SConvertToCodeResult CCodeFactory::ConvertToCode(ECodeType eCodeType, std::wstring_view wide)
{
	// 入力データを準備する
	CNativeW cSrc{ wide.data(), wide.size() };
	CMemory cDest;

	// 変換を実行する
	const auto result = CreateCodeBase(eCodeType)->UnicodeToCode(cSrc, &cDest);

	// 書き込まれたデータを回収する
	std::string loaded{ LPCSTR(cDest.GetRawPtr()), size_t(cDest.GetRawLength()) };

	return SConvertToCodeResult{ result, wide, std::size(wide), std::move(loaded) };
}
