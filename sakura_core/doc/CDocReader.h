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

	wchar_t* GetAllData(int* pnDataLen);	/* 全行データを返す */
	const wchar_t* GetLineStr( CLogicInt nLine, CLogicInt* pnLineLen );
	const wchar_t* GetLineStrWithoutEOL( CLogicInt nLine, int* pnLineLen ); // 2003.06.22 Moca
	const wchar_t* GetFirstLinrStr( int* pnLineLen );	/* 順アクセスモード：先頭行を得る */
	const wchar_t* GetNextLinrStr( int* pnLineLen );	/* 順アクセスモード：次の行を得る */

private:
	const CDocLineMgr* m_pcDocLineMgr;
};
#endif /* SAKURA_CDOCREADER_0183B68F_2942_40E1_B2A4_59FE0C20268F_H_ */
