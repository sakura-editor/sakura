/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "types/CType.h"

const WCHAR* g_ppszKeywordsCORBA_IDL[] = {
	LTEXT("any"),
	LTEXT("attribute"),
	LTEXT("boolean"),
	LTEXT("case"),
	LTEXT("char"),
	LTEXT("const"),
	LTEXT("context"),
	LTEXT("default"),
	LTEXT("double"),
	LTEXT("enum"),
	LTEXT("exception"),
	LTEXT("FALSE"),
	LTEXT("fixed"),
	LTEXT("float"),
	LTEXT("in"),
	LTEXT("inout"),
	LTEXT("interface"),
	LTEXT("long"),
	LTEXT("module"),
	LTEXT("Object"),
	LTEXT("octet"),
	LTEXT("oneway"),
	LTEXT("out"),
	LTEXT("raises"),
	LTEXT("readonly"),
	LTEXT("sequence"),
	LTEXT("short"),
	LTEXT("string"),
	LTEXT("struct"),
	LTEXT("switch"),
	LTEXT("TRUE"),
	LTEXT("typedef"),
	LTEXT("unsigned"),
	LTEXT("union"),
	LTEXT("void"),
	LTEXT("wchar_t"),
	LTEXT("wstring")
};
int g_nKeywordsCORBA_IDL = int(std::size(g_ppszKeywordsCORBA_IDL));
