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
	L"any",
	L"attribute",
	L"boolean",
	L"case",
	L"char",
	L"const",
	L"context",
	L"default",
	L"double",
	L"enum",
	L"exception",
	L"FALSE",
	L"fixed",
	L"float",
	L"in",
	L"inout",
	L"interface",
	L"long",
	L"module",
	L"Object",
	L"octet",
	L"oneway",
	L"out",
	L"raises",
	L"readonly",
	L"sequence",
	L"short",
	L"string",
	L"struct",
	L"switch",
	L"TRUE",
	L"typedef",
	L"unsigned",
	L"union",
	L"void",
	L"wchar_t",
	L"wstring"
};
int g_nKeywordsCORBA_IDL = _countof(g_ppszKeywordsCORBA_IDL);
