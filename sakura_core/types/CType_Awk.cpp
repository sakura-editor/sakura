/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "types/CType.h"

/* awk */
void CType_Awk::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	wcscpy( pType->m_szTypeName, LTEXT("AWK") );
	wcscpy( pType->m_szTypeExts, LTEXT("awk") );

	//設定
	pType->m_cLineComment.CopyTo( 0, LTEXT("#"), -1 );		/* 行コメントデリミタ */
	pType->m_eDefaultOutline = OUTLINE_TEXT;			/* アウトライン解析方法 */
	pType->m_nKeyWordSetIdx[0] = 6;						/* キーワードセット */
}

const WCHAR* g_ppszKeywordsAWK[] = {
	LTEXT("BEGIN"),
	LTEXT("END"),
	LTEXT("next"),
	LTEXT("exit"),
	LTEXT("func"),
	LTEXT("function"),
	LTEXT("return"),
	LTEXT("if"),
	LTEXT("else"),
	LTEXT("for"),
	LTEXT("in"),
	LTEXT("do"),
	LTEXT("while"),
	LTEXT("break"),
	LTEXT("continue"),
	LTEXT("$0"),
	LTEXT("$1"),
	LTEXT("$2"),
	LTEXT("$3"),
	LTEXT("$4"),
	LTEXT("$5"),
	LTEXT("$6"),
	LTEXT("$7"),
	LTEXT("$8"),
	LTEXT("$9"),
	LTEXT("$10"),
	LTEXT("$11"),
	LTEXT("$12"),
	LTEXT("$13"),
	LTEXT("$14"),
	LTEXT("$15"),
	LTEXT("$16"),
	LTEXT("$17"),
	LTEXT("$18"),
	LTEXT("$19"),
	LTEXT("$20"),
	LTEXT("FS"),
	LTEXT("OFS"),
	LTEXT("NF"),
	LTEXT("RS"),
	LTEXT("ORS"),
	LTEXT("NR"),
	LTEXT("FNR"),
	LTEXT("ARGV"),
	LTEXT("ARGC"),
	LTEXT("ARGIND"),
	LTEXT("FILENAME"),
	LTEXT("ENVIRON"),
	LTEXT("ERRNO"),
	LTEXT("OFMT"),
	LTEXT("CONVFMT"),
	LTEXT("FIELDWIDTHS"),
	LTEXT("IGNORECASE"),
	LTEXT("RLENGTH"),
	LTEXT("RSTART"),
	LTEXT("SUBSEP"),
	LTEXT("delete"),
	LTEXT("index"),
	LTEXT("jindex"),
	LTEXT("length"),
	LTEXT("jlength"),
	LTEXT("substr"),
	LTEXT("jsubstr"),
	LTEXT("match"),
	LTEXT("split"),
	LTEXT("sub"),
	LTEXT("gsub"),
	LTEXT("sprintf"),
	LTEXT("tolower"),
	LTEXT("toupper"),
	LTEXT("print"),
	LTEXT("printf"),
	LTEXT("getline"),
	LTEXT("system"),
	LTEXT("close"),
	LTEXT("sin"),
	LTEXT("cos"),
	LTEXT("atan2"),
	LTEXT("exp"),
	LTEXT("log"),
	LTEXT("int"),
	LTEXT("sqrt"),
	LTEXT("srand"),
	LTEXT("rand"),
	LTEXT("strftime"),
	LTEXT("systime")
};
int g_nKeywordsAWK = int(std::size(g_ppszKeywordsAWK));
