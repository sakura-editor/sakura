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

/* awk */
void CType_Awk::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	_tcscpy( pType->m_szTypeName, _T("AWK") );
	_tcscpy( pType->m_szTypeExts, _T("awk") );

	//設定
	pType->m_cLineComment.CopyTo( 0, L"#", -1 );		/* 行コメントデリミタ */
	pType->m_eDefaultOutline = OUTLINE_TEXT;			/* アウトライン解析方法 */
	pType->m_nKeyWordSetIdx[0] = 6;						/* キーワードセット */
}



const wchar_t* g_ppszKeywordsAWK[] = {
	L"BEGIN",
	L"END",
	L"next",
	L"exit",
	L"func",
	L"function",
	L"return",
	L"if",
	L"else",
	L"for",
	L"in",
	L"do",
	L"while",
	L"break",
	L"continue",
	L"$0",
	L"$1",
	L"$2",
	L"$3",
	L"$4",
	L"$5",
	L"$6",
	L"$7",
	L"$8",
	L"$9",
	L"$10",
	L"$11",
	L"$12",
	L"$13",
	L"$14",
	L"$15",
	L"$16",
	L"$17",
	L"$18",
	L"$19",
	L"$20",
	L"FS",
	L"OFS",
	L"NF",
	L"RS",
	L"ORS",
	L"NR",
	L"FNR",
	L"ARGV",
	L"ARGC",
	L"ARGIND",
	L"FILENAME",
	L"ENVIRON",
	L"ERRNO",
	L"OFMT",
	L"CONVFMT",
	L"FIELDWIDTHS",
	L"IGNORECASE",
	L"RLENGTH",
	L"RSTART",
	L"SUBSEP",
	L"delete",
	L"index",
	L"jindex",
	L"length",
	L"jlength",
	L"substr",
	L"jsubstr",
	L"match",
	L"split",
	L"sub",
	L"gsub",
	L"sprintf",
	L"tolower",
	L"toupper",
	L"print",
	L"printf",
	L"getline",
	L"system",
	L"close",
	L"sin",
	L"cos",
	L"atan2",
	L"exp",
	L"log",
	L"int",
	L"sqrt",
	L"srand",
	L"rand",
	L"strftime",
	L"systime"
};
int g_nKeywordsAWK = _countof(g_ppszKeywordsAWK);

