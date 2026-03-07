/*! @file */
/*
	Copyright (C) 2021-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "CConvert_CodeFromSjis.h"

#include "charset/CCodeFactory.h"

/*!
	コンストラクタ
 */
CConvert_CodeFromSjis::CConvert_CodeFromSjis(ECodeType eCodeType) noexcept
	: m_eCodeType(eCodeType)
{
}

/*!
	文字コード変換 SJIS→xxx

	@date 2009/03/26 ryoji コード変換はできるだけANSI版のsakuraと互換の結果が得られるように実装する
 */
bool CConvert_CodeFromSjis::DoConvert(CNativeW* pcData)
{
	// バッファ内容をANSI版相当に変換（Unicode→SJIS）後に SJIS→xxx 変換するのと等価な結果を得るために
	// Unicode→xxx 変換する
	CMemory m;
	CCodeFactory::CreateCodeBase(m_eCodeType)->UnicodeToCode(*pcData, &m);

	// バッファ内容をUNICODE版相当に戻すために SJIS→Unicode 変換する
	CCodeFactory::CreateCodeBase(CODE_SJIS)->CodeToUnicode(m, pcData);

	return true;
}
