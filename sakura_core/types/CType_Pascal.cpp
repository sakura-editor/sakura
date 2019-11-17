/*! @file */
/*
	Copyright (C) 2008, kobake

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
#include "types/CType.h"
#include "view/colors/EColorIndexType.h"

/* Pascal */
//Mar. 10, 2001 JEPRO	半角数値を色分け表示
void CType_Pascal::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	wcscpy( pType->m_szTypeName, L"Pascal" );
	wcscpy( pType->m_szTypeExts, L"dpr,pas" );

	//設定
	pType->m_cLineComment.CopyTo( 0, L"//", -1 );					/* 行コメントデリミタ */		//Nov. 5, 2000 JEPRO 追加
	pType->m_cBlockComments[0].SetBlockCommentRule( L"{", L"}" );	/* ブロックコメントデリミタ */	//Nov. 5, 2000 JEPRO 追加
	pType->m_cBlockComments[1].SetBlockCommentRule( L"(*", L"*)" );	/* ブロックコメントデリミタ2 */	//@@@ 2001.03.10 by MIK
	pType->m_nStringType = 1;										/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */	//Nov. 5, 2000 JEPRO 追加
	pType->m_nKeyWordSetIdx[0] = 8;									/* キーワードセット */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;			//@@@ 2001.11.11 upd MIK
	pType->m_bStringLineOnly = true; // 文字列は行内のみ
}

const wchar_t* g_ppszKeywordsPASCAL[] = {
	L"and",
	L"array",
	L"as",
	L"asm",
	L"begin",
	L"break",
	L"case",
	L"class",
	L"const",
	L"constructor",
	L"destructor",
	L"dispinterface",
	L"dispose",
	L"div",
	L"do",
	L"downto",
	L"else",
	L"end",
	L"except",
	L"exit",
	L"exports",
	L"false",
	L"file",
	L"finalization",
	L"finally",
	L"for",
	L"function",
	L"goto",
	L"if",
	L"implementation",
	L"in",
	L"inherited",
	L"initialization",
	L"inline",
	L"interface",
	L"is",
	L"label",
	L"library",
	L"mod",
	L"new",
	L"nil",
	L"not",
	L"object",
	L"of",
	L"on",
	L"operator",
	L"or",
	L"out",
	L"override"
	L"packed",
	L"private",
	L"procedure",
	L"program",
	L"property",
	L"protected",
	L"public",
	L"published",
	L"raise",
	L"record",
	L"repeat",
	L"resourcestring",
	L"self",
	L"set",
	L"shl",
	L"shr",
	L"string",
	L"stringresource",
	L"then",
	L"threadvar",
	L"to",
	L"true",
	L"try",
	L"type",
	L"unit",
	L"until",
	L"uses",
	L"var",
	L"while",
	L"with",
	L"xor",
};
int g_nKeywordsPASCAL = _countof(g_ppszKeywordsPASCAL);
