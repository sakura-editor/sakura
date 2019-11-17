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

const wchar_t* g_ppszKeywordsCORBA_IDL[] = {
	L"abstract",
	L"any",
	L"attribute",
	L"boolean",
	L"case",
	L"char",
	L"component",
	L"const",
	L"consumes",
	L"context",
	L"custom",
	L"default",
	L"double",
	L"exception",
	L"emits",
	L"enum",
	L"eventtype",
	L"factory",
	L"FALSE",
	L"finder",
	L"fixed",
	L"float",
	L"getraises",
	L"home",
	L"import",
	L"in",
	L"inout",
	L"interface",
	L"local",
	L"long",
	L"module",
	L"multiple",
	L"native",
	L"Object",
	L"octet",
	L"oneway",
	L"out",
	L"primarykey",
	L"private",
	L"provides",
	L"public",
	L"publishes",
	L"raises",
	L"readonly",
	L"setraises",
	L"sequence",
	L"short",
	L"string",
	L"struct",
	L"supports",
	L"switch",
	L"TRUE",
	L"truncatable",
	L"typedef",
	L"typeid",
	L"typeprefix",
	L"unsigned",
	L"union",
	L"uses",
	L"ValueBase",
	L"valuetype",
	L"void",
	L"wchar",
	L"wchar_t",
	L"wstring"
};
int g_nKeywordsCORBA_IDL = _countof(g_ppszKeywordsCORBA_IDL);
