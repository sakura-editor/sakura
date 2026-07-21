/*! @file */
/*
	2008.05.18 kobake CShareData から分離
*/
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CFORMATMANAGER_4161FE80_FFA1_4619_BD0A_74FF4F59BDDA_H_
#define SAKURA_CFORMATMANAGER_4161FE80_FFA1_4619_BD0A_74FF4F59BDDA_H_
#pragma once

#include "env/CSakuraEnvironment.h"	//env::ShareDataClient

//!書式管理
class CFormatManager : private env::ShareDataClient {
public:
	//書式 //@@@ 2002.2.9 YAZAKI
	// 共有DLLSHAREDATA依存
	const WCHAR* MyGetDateFormat( const SYSTEMTIME& systime, WCHAR* pszDest, int nDestLen );
	const WCHAR* MyGetTimeFormat( const SYSTEMTIME& systime, WCHAR* pszDest, int nDestLen );

	// 共有DLLSHAREDATA非依存
	const WCHAR* MyGetDateFormat( const SYSTEMTIME& systime, WCHAR* pszDest, int nDestLen, int nDateFormatType, const WCHAR* szDateFormat );
	const WCHAR* MyGetTimeFormat( const SYSTEMTIME& systime, WCHAR* pszDest, int nDestLen, int nTimeFormatType, const WCHAR* szTimeFormat );
};

#endif /* SAKURA_CFORMATMANAGER_4161FE80_FFA1_4619_BD0A_74FF4F59BDDA_H_ */
