/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
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

	WCHAR* GetAllData(int* pnDataLen);	/* 全行データを返す */
	const WCHAR* GetLineStr( CLogicInt nLine, CLogicInt* pnLineLen );
	const WCHAR* GetLineStrWithoutEOL( CLogicInt nLine, int* pnLineLen ); // 2003.06.22 Moca
	const WCHAR* GetFirstLinrStr( int* pnLineLen );	/* 順アクセスモード：先頭行を得る */
	const WCHAR* GetNextLinrStr( int* pnLineLen );	/* 順アクセスモード：次の行を得る */

private:
	const CDocLineMgr* m_pcDocLineMgr;
};
#endif /* SAKURA_CDOCREADER_0183B68F_2942_40E1_B2A4_59FE0C20268F_H_ */
