/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "types/CType.h"
#include "view/colors/EColorIndexType.h"

/* Pascal */
//Mar. 10, 2001 JEPRO	半角数値を色分け表示
void CType_Pascal::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	wcscpy( pType->m_szTypeName, LTEXT("Pascal") );
	wcscpy( pType->m_szTypeExts, LTEXT("dpr,pas") );

	//設定
	pType->m_cLineComment.CopyTo( 0, LTEXT("//"), -1 );					/* 行コメントデリミタ */		//Nov. 5, 2000 JEPRO 追加
	pType->m_cBlockComments[0].SetBlockCommentRule( LTEXT("{"), LTEXT("}") );	/* ブロックコメントデリミタ */	//Nov. 5, 2000 JEPRO 追加
	pType->m_cBlockComments[1].SetBlockCommentRule( LTEXT("(*"), LTEXT("*)") );	/* ブロックコメントデリミタ2 */	//@@@ 2001.03.10 by MIK
	pType->m_nStringType = 1;										/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */	//Nov. 5, 2000 JEPRO 追加
	pType->m_nKeyWordSetIdx[0] = 8;									/* キーワードセット */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;			//@@@ 2001.11.11 upd MIK
	pType->m_bStringLineOnly = true; // 文字列は行内のみ
}

const WCHAR* g_ppszKeywordsPASCAL[] = {
	LTEXT("and"),
	LTEXT("exports"),
	LTEXT("mod"),
	LTEXT("shr"),
	LTEXT("array"),
	LTEXT("file"),
	LTEXT("nil"),
	LTEXT("string"),
	LTEXT("as"),
	LTEXT("finalization"),
	LTEXT("not"),
	LTEXT("stringresource"),
	LTEXT("asm"),
	LTEXT("finally"),
	LTEXT("object"),
	LTEXT("then"),
	LTEXT("begin"),
	LTEXT("for"),
	LTEXT("of"),
	LTEXT("case"),
	LTEXT("function"),
	LTEXT("or"),
	LTEXT("to"),
	LTEXT("class"),
	LTEXT("goto"),
	LTEXT("out"),
	LTEXT("try"),
	LTEXT("const"),
	LTEXT("if"),
	LTEXT("packed"),
	LTEXT("type"),
	LTEXT("constructor"),
	LTEXT("implementation"),
	LTEXT("procedure"),
	LTEXT("unit"),
	LTEXT("destructor"),
	LTEXT("in"),
	LTEXT("program"),
	LTEXT("until"),
	LTEXT("dispinterface"),
	LTEXT("inherited"),
	LTEXT("property"),
	LTEXT("uses"),
	LTEXT("div"),
	LTEXT("initialization"),
	LTEXT("raise"),
	LTEXT("var"),
	LTEXT("do"),
	LTEXT("inline"),
	LTEXT("record"),
	LTEXT("while"),
	LTEXT("downto"),
	LTEXT("interface"),
	LTEXT("repeat"),
	LTEXT("with"),
	LTEXT("else"),
	LTEXT("is"),
	LTEXT("resourcestring"),
	LTEXT("xor"),
	LTEXT("end"),
	LTEXT("label"),
	LTEXT("set"),
	LTEXT("except"),
	LTEXT("library"),
	LTEXT("shl"),
	LTEXT("private"),
	LTEXT("public"),
	LTEXT("published"),
	LTEXT("protected"),
	LTEXT("override")
};
int g_nKeywordsPASCAL = int(std::size(g_ppszKeywordsPASCAL));
