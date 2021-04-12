/*! @file */
/*
	Copyright (C) 2021, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
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
	const auto bin = CCodeFactory::CreateCodeBase(m_eCodeType)->UnicodeToCode(*pcData);

	// バッファ内容をUNICODE版相当に戻すために SJIS→Unicode 変換する
	*pcData = CCodeFactory::CreateCodeBase(CODE_SJIS)->CodeToUnicode(bin);

	return true;
}
