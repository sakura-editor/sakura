﻿/*! @file */
/*
	Copyright (C) 2008, kobake
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
#ifndef SAKURA_CDOCREADER_0183B68F_2942_40E1_B2A4_59FE0C20268F_H_
#define SAKURA_CDOCREADER_0183B68F_2942_40E1_B2A4_59FE0C20268F_H_
#pragma once

#include "basis/SakuraBasis.h"

class CDocLineMgr;

//as decorator
class CDocReader{
public:
	CDocReader(const CDocLineMgr& pcDocLineMgr) : m_pcDocLineMgr(&pcDocLineMgr) { }

	wchar_t* GetAllData(int* pnDataLen);	/* 全行データを返す */
	const wchar_t* GetLineStr( CLogicInt nLine, CLogicInt* pnLineLen );
	const wchar_t* GetLineStrWithoutEOL( CLogicInt nLine, int* pnLineLen ); // 2003.06.22 Moca
	const wchar_t* GetFirstLinrStr( int* pnLineLen );	/* 順アクセスモード：先頭行を得る */
	const wchar_t* GetNextLinrStr( int* pnLineLen );	/* 順アクセスモード：次の行を得る */

private:
	const CDocLineMgr* m_pcDocLineMgr;
};
#endif /* SAKURA_CDOCREADER_0183B68F_2942_40E1_B2A4_59FE0C20268F_H_ */
