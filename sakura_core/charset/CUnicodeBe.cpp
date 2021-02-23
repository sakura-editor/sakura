/*! @file */
/*
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#include "CUnicodeBe.h"
#include "CEol.h"

#include "codechecker.h"

void CUnicodeBe::GetBom(CMemory* pcmemBom)
{
	static const BYTE UTF16BE_BOM[]={0xFE,0xFF};
	pcmemBom->SetRawData(UTF16BE_BOM, sizeof(UTF16BE_BOM));
}

void CUnicodeBe::GetEol(CMemory* pcmemEol, EEolType eEolType)
{
	static const struct{
		const void* pData;
		int nLen;
	}
	aEolTable[EOL_TYPE_NUM] = {
		{ "",					0 * sizeof(wchar_t) },	// EOL_NONE
		{ "\x00\x0d\x00\x0a",	2 * sizeof(wchar_t) },	// EOL_CRLF
		{ "\x00\x0a",			1 * sizeof(wchar_t) },	// EOL_LF
		{ "\x00\x0d",			1 * sizeof(wchar_t) },	// EOL_CR
		{ "\x00\x85",			1 * sizeof(wchar_t) },	// EOL_NEL
		{ "\x20\x28",			1 * sizeof(wchar_t) },	// EOL_LS
		{ "\x20\x29",			1 * sizeof(wchar_t) },	// EOL_PS
	};
	pcmemEol->SetRawData(aEolTable[eEolType].pData,aEolTable[eEolType].nLen);
}
