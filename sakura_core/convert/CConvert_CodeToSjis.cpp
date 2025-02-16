/*! @file */
/*
	Copyright (C) 2021-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "CConvert_CodeToSjis.h"

#include "charset/CCodeFactory.h"

/*!
	コンストラクタ
 */
CConvert_CodeToSjis::CConvert_CodeToSjis(ECodeType eCodeType) noexcept
	: m_eCodeType(eCodeType)
{
}

/*!
	文字コード変換 xxx→SJIS

	@date 2009/03/26 ryoji コード変換はできるだけANSI版のsakuraと互換の結果が得られるように実装する
 */
bool CConvert_CodeToSjis::DoConvert(CNativeW* pcData)
{
	// バッファの内容がANSI版相当になるよう Unicode→SJIS 変換する
	const auto bin = CCodeFactory::CreateCodeBase(CODE_SJIS)->UnicodeToCode(*pcData);

	// xxx→SJIS 変換後にバッファ内容をUNICODE版相当に戻す（SJIS→Unicode）のと等価な結果を得るために
	// xxx→Unicode 変換する
	std::unique_ptr<CCodeBase> pcCodeBase;
	if (m_eCodeType == CODE_JIS) {
		// JIS変換はbase64デコードを行うモードを使う
		pcCodeBase = std::unique_ptr<CCodeBase>(CCodeFactory::CreateCodeBase(CODE_JIS, true));
	}
	else {
		// その他の変換
		pcCodeBase = CCodeFactory::CreateCodeBase(m_eCodeType);
	}

	// 指定された文字コードに基づいてUnicode変換する（変換エラーは無視する）
	*pcData = pcCodeBase->CodeToUnicode(bin);

	return true;
}
